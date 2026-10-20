#include <cassert>
#include "../02_Firmware/robot_controller/demo_service.h"
static ControlLogic logic;
static unsigned commands=0,acquires=0;
ControlSnapshot ControlService::snapshot(){return {logic,0,{}};}
bool ControlService::beginNetworkChange(bool lost){return logic.beginNetworkChange(lost);}
void ControlService::endNetworkChange(){logic.endNetworkChange();}
bool ControlService::acquireDemo(uint32_t& session,uint32_t epoch){session=77;++acquires;return logic.acquireDemo(session,fakeNow,epoch);}
bool ControlService::commandDemo(const Command& c,uint8_t eye){++commands;const bool ok=logic.command(c,fakeNow,0,true);if(ok)logic.demoEye=eye;return ok;}
bool ControlService::releaseDemo(uint32_t session){return logic.release(session,0,true);}
struct DemoHarness {static void tick(DemoService& d){d.tick(fakeNow);}};
static void advance(DemoService& d,unsigned count=1){for(unsigned i=0;i<count;++i){fakeNow+=20;logic.tick(fakeNow-10);logic.tick(fakeNow);DemoHarness::tick(d);}}
static DemoConfig config(){DemoConfig c;c.width=140;c.length=100;c.radius=10;c.armsConfirmed=true;return c;}
static ApiJson::Json request(const DemoConfig& c){auto j=c.json();for(const char* key:{"linear_gain","cw_gain","ccw_gain","calibration"})cJSON_DeleteItemFromObject(j.get(),key);return j;}
int main(){
  ControlService control;NetworkService network;network.begin(control);DemoService demo(control,network);assert(demo.begin());
  auto c=config();auto j=request(c);assert(demo.configure(j.get())==200);assert(Preferences::data.count("demoV1"));
  assert(demo.start(false)==409);assert(demo.start(true)==202);auto id=demo.snapshot().run;
  advance(demo,20000);assert(demo.snapshot().planner.visits>3&&commands==0&&acquires==0);
  for(unsigned i=0;i<50;++i) {
    char* text=demo.statusJson();assert(text);ApiJson::Json status(cJSON_Parse(text),cJSON_Delete);cJSON_free(text);
    assert(cJSON_IsTrue(ApiJson::item(status.get(),"estimated")));
    assert(cJSON_GetArraySize(ApiJson::item(status.get(),"trace"))==256);
    assert(cJSON_IsTrue(ApiJson::item(status.get(),"simulation")));
  }
  assert(demo.stop(id+1)==409&&demo.snapshot().running);assert(demo.stop(id)==200);
  logic.mode=ControlMode::Auto;
  for(unsigned kind=0;kind<3;++kind){
    assert(demo.zero()==200);assert(demo.calibrationStart(kind)==202);advance(demo,50);auto s=demo.snapshot();
    assert(!s.running&&!s.zero&&s.integral>0&&logic.output.left==0&&!logic.demoOwned);
    assert(demo.calibrationResult(s.sample,-1,0)==400);
    assert(demo.calibrationResult(s.sample,s.integral*(kind?240:40),0)==200);
    assert(demo.calibrationResult(s.sample,1,0)==409);
  }
  assert(demo.snapshot().config.ready());assert(demo.start(false)==409);
  assert(demo.zero()==200);assert(demo.start(false)==202);advance(demo,500);
  assert(demo.snapshot().running&&logic.demoOwned&&commands>0&&!logic.dynamicActive);
  fakeNow+=301;DemoHarness::tick(demo);assert(!demo.snapshot().running&&logic.owner==Owner::None&&logic.output.left==0);
  assert(demo.zero()==200);logic.stop("external");assert(demo.start(false)==409);
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.stop("global_stop");advance(demo);assert(!demo.snapshot().running&&!demo.snapshot().zero);
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.stop("test");assert(logic.acquire(Owner::Program,99,fakeNow,4));advance(demo);assert(logic.session==99);logic.stop("test");
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.beginMaintenance();advance(demo);assert(!demo.snapshot().running&&logic.maintenance);logic.endMaintenance();
  for(DemoPhase phase:{DemoPhase::Choose,DemoPhase::Turn,DemoPhase::Drive,DemoPhase::Brake,DemoPhase::Gesture}) {
    logic.output.servoEnabled=false; // Test fixture: operator has accepted the base pose again.
    assert(demo.zero()==200);assert(demo.start(false)==202);
    unsigned count=0;while(demo.snapshot().planner.phase!=phase&&count++<20000)advance(demo);
    assert(demo.snapshot().planner.phase==phase);auto s=demo.snapshot();const float arm=logic.output.armLeft;
    assert(demo.stop(s.run)==200);advance(demo,10);assert(!demo.snapshot().running&&logic.output.left==0&&logic.output.right==0&&logic.output.armLeft==arm);
  }
  c.pwm=.8f;c.minPwm=.6f;c.deadbandPwm=.5f;j=request(c);assert(demo.configure(j.get())==200);assert(demo.snapshot().config.calibration==0);
  for(unsigned kind=0;kind<3;++kind){
    logic.output.servoEnabled=false;assert(demo.zero()==200);assert(demo.calibrationStart(kind)==202);advance(demo,50);const auto s=demo.snapshot();
    assert(!s.running&&s.integral>0&&s.integral<.3f);
    assert(demo.calibrationResult(s.sample,s.integral*(kind==0?20:120),0)==200);
  }
  assert(demo.snapshot().config.ready());
  logic.output.servoEnabled=false;assert(demo.zero()==200);WiFi.gotIp();assert(demo.start(false)==202);WiFi.disconnect();advance(demo);
  assert(!demo.snapshot().running&&!demo.snapshot().zero&&logic.owner==Owner::None);
  auto stored=Preferences::data["demoV1"];DemoService reboot(control,network);assert(reboot.begin());assert(!reboot.snapshot().zero&&!reboot.snapshot().running&&!reboot.snapshot().config.armsConfirmed);
  assert(reboot.snapshot().config.calibration==7&&Preferences::data["demoV1"]==stored);
  Preferences::data["demoV1"]="bad";DemoService corrupt(control,network);assert(corrupt.begin());assert(!corrupt.snapshot().storage&&Preferences::data["demoV1"]=="bad");
}
