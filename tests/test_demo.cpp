#include <cassert>
#include <limits>
#include "../02_Firmware/robot_controller/demo_planner.h"
#include "../02_Firmware/robot_controller/exhibition_profile.h"
static DemoConfig config(){DemoConfig c;c.width=120;c.length=90;c.radius=10;return c;}
int main(){
  DemoConfig bad;assert(!bad.valid());auto c=config();assert(c.valid()&&!c.ready());
  auto json=c.json();DemoConfig parsed;assert(DemoConfig::parse(json.get(),parsed,true));
  cJSON_DeleteItemFromObject(json.get(),"min_pwm");cJSON_DeleteItemFromObject(json.get(),"deadband_pwm");
  assert(DemoConfig::parse(json.get(),parsed,true)&&parsed.minPwm==0&&parsed.deadbandPwm==0);
  cJSON_AddNumberToObject(json.get(),"width_cm",100);assert(!DemoConfig::parse(json.get(),parsed,true));
  assert(!DemoConfig::parse(json.get(),parsed)); // Persisted calibration cannot be injected via config API.
  c.calibration=7;c.armsConfirmed=true;assert(c.ready());
  c.width=1;assert(!c.valid());c=config();c.speed=6;assert(!c.valid());
  c=config();c.width=c.length=2*(c.reserve()+2.6f);assert(!c.valid());
  assert(!ApiJson::boundedDepth("{\"a\":[[[1]]]}",2));assert(ApiJson::boundedDepth("{\"a\":\"{\\\"}\"}",2));
  c=config();c.baseLeft=181;assert(!c.valid());c=config();c.radius=std::numeric_limits<float>::quiet_NaN();assert(!c.valid());
  c=config();c.minLeft=72;c.maxLeft=79;c.baseLeft=75;c.minRight=100;c.maxRight=108;c.baseRight=104;
  for(uint32_t seed=1;seed<=12;++seed){
    ControlLogic v;v.mode=ControlMode::Auto;v.armPolicy.dynamic=true;
    uint32_t now=seed==12?UINT32_MAX-1000:1000,sequence=0;
    assert(v.acquireDemo(7,now));DemoPlanner p;p.start(c,seed,now);
    if(seed==1){v.armSettings.speed=1;v.armSettings.acceleration=1;}
    for(unsigned i=0;i<60000;++i){
      now+=20;v.tick(now-10);v.tick(now);assert(p.step(v.output,now));
      if(i%5==0){Command cmd{7,++sequence,p.left,p.right,p.armLeft,p.armRight,false};assert(v.command(cmd,now,0,true));}
      assert(std::fabs(p.pose.x)<=c.width*.5f-c.radius-c.margin);
      assert(std::fabs(p.pose.y)<=c.length*.5f-c.radius-c.margin);
      assert(p.traceCount<=256 && p.armLeft>=72&&p.armLeft<=79&&p.armRight>=100&&p.armRight<=108);
      assert(!v.dynamicActive);
      if(p.phase==DemoPhase::Gesture)assert(p.left==0&&p.right==0&&v.output.left==0&&v.output.right==0);
      if(p.phase==DemoPhase::Choose&&p.visits)assert(std::fabs(v.output.armLeft-c.baseLeft)<.1f&&std::fabs(v.output.armRight-c.baseRight)<.1f);
    }
    assert(p.visits>10);p.stop("test");assert(!p.step(v.output,now+20));
  }
  DemoPlanner stalled;stalled.start(config(),1,0);assert(!stalled.step(Output{},301));assert(!strcmp(stalled.reason,"planner_stalled"));
  // Deadband and minimum drive are demo-local. Raw HTTP/TCP PWM is unchanged.
  c=config();c.pwm=.8f;c.minPwm=.6f;c.deadbandPwm=.5f;c.linearGain=20;c.cwGain=120;c.ccwGain=110;
  c.width=c.length=70;c.calibration=7;c.armsConfirmed=true;assert(c.ready());
  assert(c.effort(.4f)==0&&std::fabs(c.effort(-.6f)+.1f)<.0001f);
  assert(c.commandPwm(0)==0&&c.commandPwm(.001f)==.6f&&c.commandPwm(100)==.8f);
  auto extended=c.json();for(const char* key:{"linear_gain","cw_gain","ccw_gain","calibration"})cJSON_DeleteItemFromObject(extended.get(),key);
  assert(DemoConfig::parse(extended.get(),parsed)&&parsed.minPwm==.6f&&parsed.deadbandPwm==.5f);
  cJSON_AddNumberToObject(extended.get(),"min_pwm",.6);assert(!DemoConfig::parse(extended.get(),parsed));
  auto invalid=c;invalid.pwm=.81f;assert(!invalid.valid());invalid=c;invalid.deadbandPwm=invalid.minPwm;assert(!invalid.valid());
  invalid=c;invalid.linearGain=100;assert(invalid.valid()&&!invalid.minimumSpeedsAllowed()&&!invalid.ready());
  DemoPlanner locked;locked.start(invalid,1,0);assert(locked.phase==DemoPhase::Fault);
  DemoPlanner noMotion;noMotion.start(c,1,0);Output below;below.left=below.right=.4f;assert(noMotion.step(below,20)&&noMotion.pose.x==0&&noMotion.pose.y==0);
  for(uint32_t seed=1;seed<=6;++seed){
    ControlLogic v;v.mode=ControlMode::Auto;uint32_t now=1000,sequence=0;assert(v.acquireDemo(7,now));DemoPlanner p;p.start(c,seed,now);
    for(unsigned i=0;i<30000;++i){
      now+=20;v.tick(now-10);v.tick(now);assert(p.step(v.output,now));
      if(i%5==0){Command command{7,++sequence,p.left,p.right,p.armLeft,p.armRight,false};assert(v.command(command,now,0,true));}
      for(float output:{p.left,p.right})assert(output==0||(std::fabs(output)>=c.minPwm&&std::fabs(output)<=c.pwm));
      assert(std::fabs((c.effort(v.output.left)+c.effort(v.output.right))*.5f*c.linearGain)<=c.speed+.001f);
      assert(std::fabs(p.pose.x)<=c.width*.5f-c.radius-c.margin&&std::fabs(p.pose.y)<=c.length*.5f-c.radius-c.margin);
    }
    assert(p.visits>5);
  }
  // Leases are separated by producer identity even when an external client knows
  // the public demo session ID. A stale release cannot stop a replacement owner.
  ControlLogic l;l.mode=ControlMode::Auto;assert(l.acquireDemo(11,0));Command cmd{11,1,0,0,90,90,false};
  assert(!l.command(cmd,1)&&!l.command(cmd,1,22)&&!l.release(11)&&!l.release(11,22));
  assert(l.command(cmd,1,0,true));l.stop("operator");assert(!l.demoOwned&&!l.demoEye);
  assert(l.acquire(Owner::Program,12,2,5));assert(!l.release(11,0,true));assert(l.session==12);
  l.stop("test");const auto epoch=l.stopEpoch;l.setMode(ControlMode::Manual);l.setMode(ControlMode::Auto);assert(!l.acquireDemo(20,3,epoch));
  assert(l.acquireDemo(21,3));assert(l.command(Command{21,1,.2f,.2f,90,90,false},3,0,true));l.tick(503);assert(l.owner==Owner::None&&l.output.left==0&&!l.demoOwned);
  ExhibitionProfile profile;assert(profile.valid());
  assert(ExhibitionProfile::referenceIntegral()>0&&ExhibitionProfile::referenceIntegral()<.3f);
  auto exhibit=profile.model();assert(exhibit.valid()&&exhibit.minimumSpeedsAllowed()&&!exhibit.ready()&&exhibit.calibration==0);
  assert(std::fabs(exhibit.width*.5f-exhibit.radius-exhibit.margin-15)<.001f);
  for(float invalidSize:{0.f,14.9f,201.f,std::numeric_limits<float>::quiet_NaN()}){auto badProfile=profile;badProfile.width=invalidSize;assert(!badProfile.valid());}
  for(unsigned seed=1;seed<=10;++seed){
    ControlLogic v;v.mode=ControlMode::Auto;uint32_t now=seed==10?UINT32_MAX-1000:1000,seq=0;
    assert(v.acquireDemo(7,now));DemoPlanner p;p.start(exhibit,seed,now);p.exhibition=true;
    bool raised=false,smile=false;
    for(unsigned i=0;i<40000;++i){
      now+=20;v.tick(now-10);v.tick(now);assert(p.step(v.output,now));
      if(i%5==0)assert(v.command(Command{7,++seq,p.left,p.right,p.armLeft,p.armRight,false},now,0,true));
      assert(std::fabs(p.pose.x)<=15.001f&&std::fabs(p.pose.y)<=15.001f&&p.traceCount<=256);
      assert(p.armLeft>=90&&p.armLeft<=150&&p.armRight==90);
      if(p.armLeft>90){raised=true;assert(p.phase==DemoPhase::Gesture&&p.eye==1&&p.left==0&&p.right==0&&v.output.left==0&&v.output.right==0);}
      if(p.phase==DemoPhase::Gesture&&p.eye==1)smile=true;
      if(p.phase==DemoPhase::Choose&&p.visits)assert(std::fabs(v.output.armLeft-90)<.1f);
    }
    assert(p.visits>5&&raised&&smile);
  }
}
