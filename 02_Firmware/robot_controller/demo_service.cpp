#include "demo_service.h"
#include <esp_random.h>
namespace {
struct Lock {SemaphoreHandle_t m;explicit Lock(SemaphoreHandle_t mutex):m(mutex){xSemaphoreTake(m,portMAX_DELAY);}~Lock(){xSemaphoreGive(m);}};
}
bool DemoService::begin() {
  mutex_=xSemaphoreCreateMutex();if(!mutex_)return false;
  state_.storage=preferences_.begin("robot",false);
  if(state_.storage&&preferences_.isKey("demoV1")) {
    const String saved=preferences_.getString("demoV1","");
    ApiJson::Json root(saved.length()<=2048&&ApiJson::boundedDepth(saved.c_str(),2)?cJSON_ParseWithOpts(saved.c_str(),nullptr,true):nullptr,cJSON_Delete);
    if(!DemoConfig::parse(root.get(),state_.config,true))state_.storage=false;
  }
  epoch_=control_.snapshot().logic.stopEpoch;
  state_.config.armsConfirmed=false; // Stored limits are not a physical pose confirmation after reboot.
  state_.ready=xTaskCreatePinnedToCore(task,"demo",8192,this,2,nullptr,0)==pdPASS;
  return state_.ready;
}
DemoSnapshot DemoService::snapshot(){if(!mutex_)return {};Lock lock(mutex_);return state_;}
bool DemoService::idle(const ControlLogic& l) const {
  return !state_.running&&!l.maintenance&&!l.networkBusy&&l.owner==Owner::None&&l.output.left==0&&l.output.right==0;
}
bool DemoService::armsAllowed(const ControlLogic& l) const {
  const auto& c=state_.config;
  const bool pose=!l.output.servoEnabled||(std::fabs(l.output.armLeft-c.baseLeft)<=1&&std::fabs(l.output.armRight-c.baseRight)<=1);
  return pose&&c.armsConfirmed&&c.minLeft>=l.armPolicy.leftMin&&c.maxLeft<=l.armPolicy.leftMax&&c.minRight>=l.armPolicy.rightMin&&c.maxRight<=l.armPolicy.rightMax;
}
bool DemoService::persist(const DemoConfig& c) {
  if(!state_.storage)return false;
  if(!control_.beginNetworkChange())return false; // Existing exclusive idle gate blocks TCP/HTTP acquire during flash.
  auto json=c.json();char* text=cJSON_PrintUnformatted(json.get());
  const bool ok=text&&preferences_.putString("demoV1",text)==strlen(text);cJSON_free(text);
  control_.endNetworkChange();if(ok)state_.config=c;return ok;
}
int DemoService::configure(cJSON* root) {
  if(!mutex_)return 503;
  Lock lock(mutex_);DemoConfig c;
  if(!DemoConfig::parse(root,c))return 400;
  if(!idle(control_.snapshot().logic))return 409;
  if(c.pwm==state_.config.pwm&&c.minPwm==state_.config.minPwm&&c.deadbandPwm==state_.config.deadbandPwm){c.linearGain=state_.config.linearGain;c.cwGain=state_.config.cwGain;c.ccwGain=state_.config.ccwGain;c.calibration=state_.config.calibration;}
  if(!persist(c))return 503;
  state_.zero=false;sampleReady_=false;state_.reason="configured_set_zero";return 200;
}
int DemoService::zero() {
  if(!mutex_)return 503;
  Lock lock(mutex_);auto l=control_.snapshot().logic;
  if(!idle(l))return 409;
  if(!state_.config.valid())return 400;
  state_.planner=DemoPlanner{};state_.planner.config=state_.config;
  state_.zero=true;epoch_=l.stopEpoch;state_.reason="zero_set_by_operator";return 200;
}
int DemoService::start(bool simulation) {
  if(!mutex_)return 503;
  Lock lock(mutex_);auto l=control_.snapshot().logic;
  if(!state_.ready||!state_.storage||!state_.config.valid()||!state_.config.minimumSpeedsAllowed())return 400;
  if(!idle(l))return 409;
  if(!simulation&&(!state_.zero||epoch_!=l.stopEpoch||!state_.config.ready()||!armsAllowed(l)||l.mode!=ControlMode::Auto))return 409;
  if(!simulation&&!control_.acquireDemo(session_,l.stopEpoch))return 409;
  ++state_.run;if(!state_.run)++state_.run;
  state_.simulation=simulation;state_.calibrating=false;state_.running=true;state_.zero=false;sampleReady_=false;
  state_.reason=simulation?"simulation":"running";
  const uint32_t now=millis();state_.planner.start(state_.config,esp_random(),now);last_=sent_=now;sequence_=0;
  epoch_=control_.snapshot().logic.stopEpoch;
  if(simulation){virtual_=ControlLogic{};virtual_.mode=ControlMode::Auto;virtual_.acquireDemo(1,now);}
  return 202;
}
void DemoService::finish(const char* reason) {
  const bool fault=state_.planner.phase==DemoPhase::Fault;
  // Scoped release never stops a replacement HTTP/TCP owner.
  if(state_.running&&!state_.simulation)control_.releaseDemo(session_);
  state_.running=false;state_.calibrating=false;state_.zero=false;state_.reason=reason;
  state_.planner.stop(reason);if(fault)state_.planner.phase=DemoPhase::Fault;session_=0;
}
int DemoService::stop(uint32_t run) {
  if(!mutex_)return 503;
  Lock lock(mutex_);
  if(run!=state_.run)return 409;
  sampleReady_=false;finish("demo_stop");return 200;
}
int DemoService::calibrationStart(unsigned kind) {
  if(!mutex_)return 503;
  Lock lock(mutex_);auto l=control_.snapshot().logic;
  if(kind>2||!state_.config.valid())return 400;
  if(kind==0&&state_.config.linearGain*state_.config.effort(state_.config.pwm)+state_.config.coast>state_.config.halfY())return 400;
  if(!state_.ready||!idle(l)||!state_.zero||epoch_!=l.stopEpoch||!armsAllowed(l)||l.mode!=ControlMode::Auto)return 409;
  if(!control_.acquireDemo(session_,l.stopEpoch))return 409;
  ++state_.run;if(!state_.run)++state_.run;
  state_.running=true;state_.simulation=false;state_.calibrating=true;state_.zero=false;
  state_.sample=state_.run;state_.sampleKind=kind;state_.integral=0;sampleReady_=false;
  state_.reason="calibration_one_second";state_.planner.stop("calibration");
  last_=sent_=calAt_=millis();sequence_=0;epoch_=control_.snapshot().logic.stopEpoch;return 202;
}
int DemoService::calibrationResult(uint32_t sample,float measured,float coast) {
  if(!mutex_)return 503;
  Lock lock(mutex_);
  if(!sampleReady_||state_.running||sample!=state_.sample||uint32_t(millis()-sampleAt_)>120000||!idle(control_.snapshot().logic))return 409;
  if(!std::isfinite(measured)||!std::isfinite(coast)||measured<=0||coast<0||state_.integral<=0)return 400;
  DemoConfig c=state_.config;const float gain=measured/state_.integral;
  if(state_.sampleKind==0)c.linearGain=gain;else if(state_.sampleKind==1)c.cwGain=gain;else c.ccwGain=gain;
  c.coast=std::max(c.coast,coast);c.calibration|=1u<<state_.sampleKind;if(!c.valid())return 400;
  if(!persist(c))return 503;
  sampleReady_=false;state_.zero=false;state_.reason="calibration_saved_set_zero";return 200;
}
void DemoService::task(void* context) {
  auto& self=*static_cast<DemoService*>(context);TickType_t wake=xTaskGetTickCount();
  for(;;){{Lock lock(self.mutex_);self.tick(millis());}vTaskDelayUntil(&wake,pdMS_TO_TICKS(20));}
}
void DemoService::tick(uint32_t now) {
  const auto l=control_.snapshot().logic;
  if(!state_.running){if(state_.zero&&epoch_!=l.stopEpoch)state_.zero=false;return;}
  if(uint32_t(now-last_)>300){sampleReady_=false;finish("planner_stalled");return;}
  if(l.stopEpoch!=epoch_||l.maintenance||l.networkBusy||network_.linkLostPending()||(!state_.simulation&&(!l.demoOwned||l.session!=session_))){sampleReady_=false;finish("control_or_network_cancelled");return;}
  const uint32_t elapsed=now-last_;last_=now;
  if(state_.calibrating) {
    const auto& o=l.output;const float left=state_.config.effort(o.left),right=state_.config.effort(o.right);
    const float effort=state_.sampleKind==0?(left+right)*.5f:std::fabs((left-right)*.5f);
    state_.integral+=effort*elapsed*.001f;
    if(uint32_t(now-calAt_)>=1000){finish("calibration_complete_enter_measurement");sampleReady_=true;sampleAt_=now;return;}
  }else {
    if(state_.simulation){virtual_.tick(now-elapsed/2);virtual_.tick(now);}
    if(!state_.planner.step(state_.simulation?virtual_.output:l.output,now)){const char* reason=state_.planner.reason;finish(reason);return;}
  }
  if(sequence_&&uint32_t(now-sent_)<100)return;
  if(sequence_==UINT32_MAX){finish("sequence_exhausted");return;}
  Command c;c.session=state_.simulation?1:session_;c.sequence=++sequence_;c.laser=false;
  if(state_.calibrating){const float p=state_.config.pwm;c.left=state_.sampleKind==2?-p:p;c.right=state_.sampleKind==1?-p:p;c.armLeft=state_.config.baseLeft;c.armRight=state_.config.baseRight;}
  else {const auto& p=state_.planner;c.left=p.left;c.right=p.right;c.armLeft=p.armLeft;c.armRight=p.armRight;}
  const bool ok=state_.simulation?virtual_.command(c,now,0,true):control_.commandDemo(c,state_.calibrating?0:state_.planner.eye);
  if(!ok){sampleReady_=false;finish("command_rejected");return;}sent_=now;
}
char* DemoService::statusJson() {
  const auto s=snapshot();const auto l=control_.snapshot().logic;
  ApiJson::Json root(cJSON_CreateObject(),cJSON_Delete);auto r=root.get();auto c=s.config.json();cJSON_AddItemToObject(r,"config",c.release());
  cJSON_AddBoolToObject(r,"ready",s.ready);cJSON_AddBoolToObject(r,"storage_ready",s.storage);
  cJSON_AddBoolToObject(r,"running",s.running);cJSON_AddBoolToObject(r,"simulation",s.simulation);cJSON_AddBoolToObject(r,"calibrating",s.calibrating);
  cJSON_AddBoolToObject(r,"zero_valid",s.zero);cJSON_AddBoolToObject(r,"calibration_ready",s.config.ready());
  cJSON_AddBoolToObject(r,"minimum_speed_allowed",s.config.minimumSpeedsAllowed());
  const bool poseOk=!l.output.servoEnabled||(std::fabs(l.output.armLeft-s.config.baseLeft)<=1&&std::fabs(l.output.armRight-s.config.baseRight)<=1);
  const bool limits=poseOk&&s.config.minLeft>=l.armPolicy.leftMin&&s.config.maxLeft<=l.armPolicy.leftMax&&s.config.minRight>=l.armPolicy.rightMin&&s.config.maxRight<=l.armPolicy.rightMax;
  cJSON_AddBoolToObject(r,"can_start_robot",s.ready&&s.storage&&!s.running&&s.zero&&s.config.ready()&&limits&&l.mode==ControlMode::Auto&&!l.maintenance&&!l.networkBusy&&l.owner==Owner::None);
  cJSON_AddNumberToObject(r,"run_id",s.run);cJSON_AddNumberToObject(r,"sample_id",s.sample);cJSON_AddNumberToObject(r,"sample_kind",s.sampleKind);
  cJSON_AddNumberToObject(r,"sample_integral",s.integral);cJSON_AddStringToObject(r,"reason",s.reason);
  cJSON_AddStringToObject(r,"phase",s.calibrating?"calibration":demoPhaseName(s.planner.phase));cJSON_AddBoolToObject(r,"estimated",true);
  auto pose=cJSON_AddObjectToObject(r,"pose");cJSON_AddNumberToObject(pose,"x_cm",s.planner.pose.x);cJSON_AddNumberToObject(pose,"y_cm",s.planner.pose.y);cJSON_AddNumberToObject(pose,"heading_deg",s.planner.pose.heading*180/3.14159265f);
  auto goal=cJSON_AddObjectToObject(r,"goal");cJSON_AddNumberToObject(goal,"x_cm",s.planner.goal.x);cJSON_AddNumberToObject(goal,"y_cm",s.planner.goal.y);
  cJSON_AddNumberToObject(r,"visits",s.planner.visits);cJSON_AddNumberToObject(r,"gesture",s.planner.gesture);
  auto trace=cJSON_AddArrayToObject(r,"trace");for(unsigned i=0;i<s.planner.traceCount;++i){auto p=cJSON_CreateArray();cJSON_AddItemToArray(trace,p);const auto& point=s.planner.trace[(s.planner.traceNext+256-s.planner.traceCount+i)%256];cJSON_AddItemToArray(p,cJSON_CreateNumber(point.x));cJSON_AddItemToArray(p,cJSON_CreateNumber(point.y));}
  return cJSON_PrintUnformatted(r);
}
