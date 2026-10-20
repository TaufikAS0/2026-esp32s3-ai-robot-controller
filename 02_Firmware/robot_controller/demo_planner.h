#pragma once
#include "api_json.h"
#include "control_logic.h"
#include <algorithm>

// Portable, bounded, ideal differential-drive model. No GPIO, Wi-Fi or position sensor.
struct DemoConfig {
  float width=0,length=0,radius=0,margin=10,coast=0,speed=5,turn=30,pwm=.15f;
  float minPwm=0,deadbandPwm=0;
  float baseLeft=90,baseRight=90,minLeft=0,maxLeft=180,minRight=0,maxRight=180;
  float linearGain=40,cwGain=240,ccwGain=240;
  bool armsConfirmed=false;
  unsigned calibration=0;
  float reserve() const {return radius+margin+coast+speed*.5f;}
  float halfX() const {return width*.5f-reserve();}
  float halfY() const {return length*.5f-reserve();}
  bool valid() const {
    const float values[]={width,length,radius,margin,coast,speed,turn,pwm,minPwm,deadbandPwm,baseLeft,baseRight,minLeft,maxLeft,minRight,maxRight,linearGain,cwGain,ccwGain};
    for(float v:values)if(!std::isfinite(v))return false;
    return width>0&&width<=1000&&length>0&&length<=1000&&radius>0&&margin>=0&&coast>=0&&
      speed>0&&speed<=5&&turn>0&&turn<=30&&pwm>0&&pwm<=.8f&&
      minPwm>=0&&minPwm<=pwm&&deadbandPwm>=0&&deadbandPwm<pwm&&
      (minPwm==0?deadbandPwm==0:minPwm>deadbandPwm)&&
      minLeft>=0&&maxLeft<=180&&minLeft<maxLeft&&minRight>=0&&maxRight<=180&&minRight<maxRight&&
      baseLeft>=minLeft&&baseLeft<=maxLeft&&baseRight>=minRight&&baseRight<=maxRight&&
      linearGain>0&&linearGain<=2000&&cwGain>0&&cwGain<=10000&&ccwGain>0&&ccwGain<=10000&&
      halfX()>2.5f&&halfY()>2.5f&&std::hypot(halfX(),halfY())>=5.1f&&calibration<=7;
  }
  float effort(float applied) const {return std::copysign(std::max(0.f,std::fabs(applied)-deadbandPwm),applied);}
  float commandPwm(float requestedEffort) const {return requestedEffort<=0?0:std::min(pwm,std::max(minPwm,deadbandPwm+requestedEffort));}
  bool minimumSpeedsAllowed() const {const float e=effort(minPwm);return e*linearGain<=speed+.001f&&e*cwGain<=turn+.001f&&e*ccwGain<=turn+.001f;}
  bool ready() const {return valid()&&armsConfirmed&&calibration==7&&minimumSpeedsAllowed();}
  static bool parse(cJSON* r,DemoConfig& out,bool stored=false) {
    using namespace ApiJson;DemoConfig c;
    // Accept the exact legacy schema or the exact schema with both new fields.
    // Strip one occurrence on a copy so duplicate/unknown fields still fail.
    Json known(cJSON_Duplicate(r,true),cJSON_Delete);if(!known)return false;
    if(item(r,"min_pwm")||item(r,"deadband_pwm")) {
      if(!number(r,"min_pwm",c.minPwm)||!number(r,"deadband_pwm",c.deadbandPwm))return false;
      cJSON_DeleteItemFromObjectCaseSensitive(known.get(),"min_pwm");cJSON_DeleteItemFromObjectCaseSensitive(known.get(),"deadband_pwm");
    }
    if(stored) {
      if(!fields(known.get(),{"width_cm","length_cm","radius_cm","margin_cm","coast_cm","speed_cm_s","turn_deg_s","pwm","base_left","base_right","min_left","max_left","min_right","max_right","arms_confirmed","linear_gain","cw_gain","ccw_gain","calibration"}))return false;
      uint32_t mask=0;auto m=item(r,"calibration");if(!cJSON_IsNumber(m)||m->valuedouble<0||m->valuedouble>7||std::floor(m->valuedouble)!=m->valuedouble)return false;
      mask=uint32_t(m->valuedouble);c.calibration=mask;
      if(!number(r,"linear_gain",c.linearGain)||!number(r,"cw_gain",c.cwGain)||!number(r,"ccw_gain",c.ccwGain))return false;
    } else if(!fields(known.get(),{"width_cm","length_cm","radius_cm","margin_cm","coast_cm","speed_cm_s","turn_deg_s","pwm","base_left","base_right","min_left","max_left","min_right","max_right","arms_confirmed"}))return false;
    if(!number(r,"width_cm",c.width)||!number(r,"length_cm",c.length)||!number(r,"radius_cm",c.radius)||
       !number(r,"margin_cm",c.margin)||!number(r,"coast_cm",c.coast)||!number(r,"speed_cm_s",c.speed)||
       !number(r,"turn_deg_s",c.turn)||!number(r,"pwm",c.pwm)||!number(r,"base_left",c.baseLeft)||
       !number(r,"base_right",c.baseRight)||!number(r,"min_left",c.minLeft)||!number(r,"max_left",c.maxLeft)||
       !number(r,"min_right",c.minRight)||!number(r,"max_right",c.maxRight)||!cJSON_IsBool(item(r,"arms_confirmed")))return false;
    c.armsConfirmed=cJSON_IsTrue(item(r,"arms_confirmed"));if(!c.valid())return false;out=c;return true;
  }
  ApiJson::Json json() const {
    ApiJson::Json r(cJSON_CreateObject(),cJSON_Delete);
    const char* names[]={"width_cm","length_cm","radius_cm","margin_cm","coast_cm","speed_cm_s","turn_deg_s","pwm","base_left","base_right","min_left","max_left","min_right","max_right","linear_gain","cw_gain","ccw_gain"};
    const float values[]={width,length,radius,margin,coast,speed,turn,pwm,baseLeft,baseRight,minLeft,maxLeft,minRight,maxRight,linearGain,cwGain,ccwGain};
    for(unsigned i=0;i<17;++i)cJSON_AddNumberToObject(r.get(),names[i],values[i]);
    cJSON_AddNumberToObject(r.get(),"min_pwm",minPwm);cJSON_AddNumberToObject(r.get(),"deadband_pwm",deadbandPwm);
    cJSON_AddBoolToObject(r.get(),"arms_confirmed",armsConfirmed);cJSON_AddNumberToObject(r.get(),"calibration",calibration);return r;
  }
};
struct DemoPoint {float x=0,y=0;};
struct DemoPose {float x=0,y=0,heading=0;}; // cm; radians clockwise from +Y
enum class DemoPhase {Idle,Choose,Turn,Drive,Brake,Gesture,Fault};
inline const char* demoPhaseName(DemoPhase p) {const char* n[]={"idle","choose","turn","drive","brake","gesture","fault"};return n[unsigned(p)];}
class DemoPlanner {
 public:
  DemoConfig config;
  DemoPose pose;
  DemoPoint goal,trace[256]{};
  unsigned traceCount=0,traceNext=0,gesture=0,visits=0;
  DemoPhase phase=DemoPhase::Idle;
  const char* reason="idle";
  float left=0,right=0,armLeft=90,armRight=90;
  uint8_t eye=0;
  void start(const DemoConfig& c,uint32_t seed,uint32_t now) {
    *this=DemoPlanner{};config=c;rng_=seed?seed:1;last_=phaseAt_=now;
    armLeft=c.baseLeft;armRight=c.baseRight;
    const bool usable=c.valid()&&c.minimumSpeedsAllowed();phase=usable?DemoPhase::Choose:DemoPhase::Fault;reason=usable?"running":"invalid_config_or_minimum_speed";record();
  }
  void stop(const char* why) {phase=DemoPhase::Idle;left=right=0;eye=0;reason=why;}
  bool step(const Output& applied,uint32_t now) {
    if(phase==DemoPhase::Idle||phase==DemoPhase::Fault)return false;
    const uint32_t elapsed=now-last_;last_=now;
    if(elapsed>300)return fail("planner_stalled");
    const float dt=elapsed*.001f;
    const float effectiveLeft=config.effort(applied.left),effectiveRight=config.effort(applied.right);
    const float v=(effectiveLeft+effectiveRight)*.5f*config.linearGain;
    const float differential=(effectiveLeft-effectiveRight)*.5f;
    const float w=differential*(differential>=0?config.cwGain:config.ccwGain)*pi/180;
    const float mid=pose.heading+w*dt*.5f;
    pose.x+=v*std::sin(mid)*dt;pose.y+=v*std::cos(mid)*dt;pose.heading=wrap(pose.heading+w*dt);
    if(!std::isfinite(pose.x)||!std::isfinite(pose.y)||std::fabs(pose.x)>config.width*.5f-config.radius-config.margin || std::fabs(pose.y)>config.length*.5f-config.radius-config.margin)return fail("model_boundary");
    if(uint32_t(now-traceAt_)>=200){record();traceAt_=now;}
    left=right=0;eye=0;armLeft=config.baseLeft;armRight=config.baseRight;
    const float dx=goal.x-pose.x,dy=goal.y-pose.y,distance=std::hypot(dx,dy);
    const float error=wrap(std::atan2(dx,dy)-pose.heading);
    switch(phase) {
      case DemoPhase::Choose: {
        bool found=false;
        for(unsigned i=0;i<64;++i){goal={(random()*2-1)*config.halfX(),(random()*2-1)*config.halfY()};if(std::hypot(goal.x-pose.x,goal.y-pose.y)>=5){found=true;break;}}
        if(!found) {
          goal={pose.x<=0?config.halfX():-config.halfX(),pose.y<=0?config.halfY():-config.halfY()};
          if(std::hypot(goal.x-pose.x,goal.y-pose.y)<5)return fail("area_too_small");
        }
        phase=DemoPhase::Turn;phaseAt_=now;break;
      }
      case DemoPhase::Turn:
        if(std::fabs(error)<pi/90){if(std::fabs(applied.left)<.01f&&std::fabs(applied.right)<.01f){phase=DemoPhase::Drive;phaseAt_=now;}}
        else {const float gain=error>=0?config.cwGain:config.ccwGain;const float p=config.commandPwm(std::min(config.turn,std::fabs(error)*180/pi*3)/gain);left=error>=0?p:-p;right=-left;}
        break;
      case DemoPhase::Drive:
        if(distance<.75f){phase=DemoPhase::Brake;phaseAt_=now;}
        else if(std::fabs(error)>pi/12){phase=DemoPhase::Turn;phaseAt_=now;}
        else left=right=config.commandPwm(std::min(config.speed,distance*1.5f)/config.linearGain);
        break;
      case DemoPhase::Brake:
        if(std::fabs(applied.left)<.001f&&std::fabs(applied.right)<.001f&&uint32_t(now-phaseAt_)>=500){phase=DemoPhase::Gesture;phaseAt_=now;gesture=nextRandom()%3;}
        break;
      case DemoPhase::Gesture: {
        if(std::fabs(applied.left)>.001f||std::fabs(applied.right)>.001f)return fail("gesture_wheels_nonzero");
        const uint32_t t=now-phaseAt_;eye=uint8_t(gesture+1);
        const float wave=.5f-.5f*std::cos(4*pi*std::min(1.f,t/3200.f));
        if(gesture<2)armLeft+=excursion(config.baseLeft,config.minLeft,config.maxLeft)*wave;
        if(gesture==1)armRight+=excursion(config.baseRight,config.minRight,config.maxRight)*wave;
        if(t>=4000&&std::fabs(applied.armLeft-config.baseLeft)<.1f&&std::fabs(applied.armRight-config.baseRight)<.1f){++visits;phase=DemoPhase::Choose;phaseAt_=now;eye=0;}
        else if(t>20000)return fail("arm_reference_not_restored");
        break;
      }
      default:break;
    }
    if(phase!=DemoPhase::Gesture&&uint32_t(now-phaseAt_)>120000)return fail("phase_timeout");
    return true;
  }
 private:
  static constexpr float pi=3.14159265358979323846f;
  uint32_t rng_=1,last_=0,phaseAt_=0,traceAt_=0;
  static float wrap(float x){return std::atan2(std::sin(x),std::cos(x));}
  static float excursion(float base,float low,float high){return high-base>=base-low?std::min(10.f,high-base):-std::min(10.f,base-low);}
  uint32_t nextRandom(){rng_^=rng_<<13;rng_^=rng_>>17;rng_^=rng_<<5;return rng_;}
  float random(){return float(nextRandom()>>8)/16777216.f;}
  void record(){trace[traceNext]={pose.x,pose.y};traceNext=(traceNext+1)%256;traceCount=std::min(256u,traceCount+1);}
  bool fail(const char* why){phase=DemoPhase::Fault;left=right=0;eye=0;reason=why;return false;}
};
