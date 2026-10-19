#include <cassert>
#include <limits>
#include "../02_Firmware/robot_controller/demo_planner.h"
static DemoConfig config(){DemoConfig c;c.width=120;c.length=90;c.radius=10;return c;}
int main(){
  DemoConfig bad;assert(!bad.valid());auto c=config();assert(c.valid()&&!c.ready());
  auto json=c.json();DemoConfig parsed;assert(DemoConfig::parse(json.get(),parsed,true));
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
  // Leases are separated by producer identity even when an external client knows
  // the public demo session ID. A stale release cannot stop a replacement owner.
  ControlLogic l;l.mode=ControlMode::Auto;assert(l.acquireDemo(11,0));Command cmd{11,1,0,0,90,90,false};
  assert(!l.command(cmd,1)&&!l.command(cmd,1,22)&&!l.release(11)&&!l.release(11,22));
  assert(l.command(cmd,1,0,true));l.stop("operator");assert(!l.demoOwned&&!l.demoEye);
  assert(l.acquire(Owner::Program,12,2,5));assert(!l.release(11,0,true));assert(l.session==12);
  l.stop("test");const auto epoch=l.stopEpoch;l.setMode(ControlMode::Manual);l.setMode(ControlMode::Auto);assert(!l.acquireDemo(20,3,epoch));
  assert(l.acquireDemo(21,3));assert(l.command(Command{21,1,.2f,.2f,90,90,false},3,0,true));l.tick(503);assert(l.owner==Owner::None&&l.output.left==0&&!l.demoOwned);
}
