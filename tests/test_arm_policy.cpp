#include <cassert>
#include <limits>
#include "../02_Firmware/robot_controller/control_logic.h"

int main(){
  ArmPolicy p;p.leftMin=20;p.leftMax=140;p.rightMin=30;p.rightMax=150;
  assert(ArmPolicy::valid(p));
  auto bad=p;bad.leftMin=bad.leftMax;assert(!ArmPolicy::valid(bad));
  bad=p;bad.amplitude=std::numeric_limits<float>::quiet_NaN();assert(!ArmPolicy::valid(bad));
  bad=p;bad.periodMs=999;assert(!ArmPolicy::valid(bad));
  ControlLogic c;assert(c.configureArmPolicy(p));assert(c.acquire(Owner::Manual,11,0));
  Command cmd{11,1,0,0,90,90,false};assert(c.command(cmd,1));c.tick(10);
  assert(!c.configureArmPolicy(p));
  cmd.sequence++;cmd.armLeft=19;assert(!c.command(cmd,20));assert(c.lastCommandMs==1);
  cmd.armLeft=90;cmd.armRight=151;assert(!c.command(cmd,30));assert(c.lastCommandMs==1);
  c.stop("stop");
  bad=p;bad.leftMax=80;assert(!c.configureArmPolicy(bad));assert(c.armPolicy.leftMax==140);
  p.dynamic=true;p.amplitude=30;p.periodMs=1000;assert(c.configureArmPolicy(p));
  assert(c.acquire(Owner::Manual,12,40));
  cmd={12,1,.8f,.8f,139,31,false};
  bool active=false;
  for(unsigned t=50;t<3000;t+=10){
    ++cmd.sequence;assert(c.command(cmd,t));c.tick(t);
    active|=c.dynamicActive;
    assert(c.output.armLeft>=20&&c.output.armLeft<=140&&c.output.armRight>=30&&c.output.armRight<=150);
  }
  assert(active);
  const float held=c.output.armLeft;
  c.tick(3500);assert(c.owner==Owner::None&&!c.dynamicActive&&c.output.armLeft==held);
  c.tick(4000);assert(c.output.armLeft==held);
  // Stationary wheels and in-place turns never initiate dynamic arm gestures.
  assert(c.acquire(Owner::Manual,13,4010));cmd={13,1,0,0,90,90,false};
  assert(c.command(cmd,4011));c.tick(4020);assert(!c.dynamicActive);
  cmd.sequence++;cmd.left=.8f;cmd.right=-.8f;assert(c.command(cmd,4030));c.tick(4030);assert(!c.dynamicActive);
  // Compare forward/reverse phases from identical references: simulation only.
  ControlLogic f,r;assert(f.configureArmPolicy(p)&&r.configureArmPolicy(p));
  assert(f.acquire(Owner::Manual,20,0)&&r.acquire(Owner::Manual,21,0));
  Command fc{20,1,.8f,.8f,90,90,false},rc{21,1,-.8f,-.8f,90,90,false};
  for(unsigned t=10;t<500;t+=10){++fc.sequence;++rc.sequence;assert(f.command(fc,t)&&r.command(rc,t));f.tick(t);r.tick(t);}
  assert(f.dynamicActive&&r.dynamicActive);
  assert((f.effectiveArmLeft-90)*(r.effectiveArmLeft-90)<0);
  // Natural default: accelerate, cruise and brake; a90-degree move <2 seconds.
  ArmMotion trajectory;trajectory.hold(20);ArmMotionSettings limits;
  float peak=0,first=0;unsigned reached=0;
  for(unsigned i=1;i<=220;++i){const float before=trajectory.position;trajectory.step(110,.01f,limits);
    if(i==1)first=trajectory.position-before;
    peak=std::fmax(peak,std::fabs(trajectory.velocity));
    assert(trajectory.position>=20&&trajectory.position<=110);
    if(trajectory.position==110&&trajectory.velocity==0){reached=i;break;}
  }
  assert(first<.03f&&peak>=59&&reached>100&&reached<200);
}
