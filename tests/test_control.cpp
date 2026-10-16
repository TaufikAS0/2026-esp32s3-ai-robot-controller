#include <cassert>
#include <limits>
#include "../02_Firmware/robot_controller/control_logic.h"

int main() {
  ControlLogic c;
  assert(!c.output.servoEnabled && c.output.left == 0 && !c.output.laser);
  ControlLogic seed;
  assert(seed.acquire(Owner::Manual,777,0));
  Command initial; initial.session=777; initial.sequence=1;
  initial.armLeft=2; initial.armRight=71;
  assert(seed.command(initial,1)); seed.tick(10);
  assert(seed.output.armLeft==2 && seed.output.armRight==71);
  assert(c.mode == ControlMode::Manual);
  assert(!c.acquire(Owner::Program, 42, 0));
  assert(c.setMode(ControlMode::Auto));
  assert(!c.acquire(Owner::Manual, 42, 0));
  assert(c.acquire(Owner::Program, 42, 0));
  Command cmd{42, 1, 1, -1, 0, 180, true};
  assert(c.command(cmd, 1)); c.tick(10);
  assert(c.output.left > 0 && c.output.right < 0 && c.output.servoEnabled);
  assert(!c.command(cmd, 100)); assert(c.lastCommandMs == 1);
  cmd.sequence = 2; cmd.left = 1.1f;
  assert(!c.command(cmd, 101)); assert(c.lastSequence == 1);
  cmd.left = std::numeric_limits<float>::quiet_NaN(); assert(!c.command(cmd, 102));
  cmd.left = 0; cmd.session = 43; assert(!c.command(cmd, 103));
  cmd.session = 42; cmd.armLeft = -1; assert(!c.command(cmd, 104));
  c.tick(501);
  assert(c.owner == Owner::None && c.output.left == 0 && !c.output.laser);
  assert(c.output.armRight == 180 && c.output.servoEnabled);
  cmd.armLeft = 90; assert(!c.command(cmd, 502));
  assert(c.acquire(Owner::Program, 44, 510));
  assert(c.setMode(ControlMode::Manual));
  assert(c.session == 0 && c.output.left == 0 && !c.output.laser);
  assert(c.acquire(Owner::Manual, 45, 520));
  assert(!c.acquire(Owner::Program, 46, 530));
  cmd.session = 45; cmd.sequence = 1; cmd.left = 1;
  assert(c.command(cmd, 540)); c.tick(550);
  c.stop("stop"); assert(c.session == 0 && c.output.left == 0);
  c.beginMaintenance(); assert(!c.acquire(Owner::Manual, 47, 560));
  assert(!c.setMode(ControlMode::Auto));
  assert(!c.output.servoEnabled); c.endMaintenance();
  assert(c.setMode(ControlMode::Auto));
  assert(c.acquire(Owner::Program, 48, UINT32_MAX - 100));
  c.tick(399); assert(c.owner == Owner::None);  // Clock wrap: exactly 500 ms.
  assert(c.acquire(Owner::Program, 49, 600));
  cmd.session = 49; cmd.sequence = 1; assert(c.command(cmd, 601));
  c.tick(610); cmd.sequence = 2;
  assert(!c.command(cmd, 1101));  // Expired command cannot resurrect lease even before tick.

  MotorRamp motor;
  for (unsigned t = 0; t < 250; t += 10) motor.step(1, t);
  assert(motor.value > 0.99);
  unsigned zeroAt = 0;
  for (unsigned t = 250; t < 600; t += 10) {
    float value = motor.step(-1, t);
    if (value == 0) { zeroAt = t; break; }
    assert(value > 0);
  }
  assert(zeroAt > 0);
  assert(motor.step(-1, zeroAt + 30) == 0);
  assert(motor.step(-1, zeroAt + 40) < 0);
  motor.reset(); assert(motor.value == 0);
  ArmMotion arm; ArmMotionSettings limits;
  float previous=90,previousVelocity=0;
  for(unsigned i=0;i<400;++i) {
    const float next=arm.step(170,.01f,limits);
    assert(next>=previous && next<=170 && next-previous<=limits.speed*.01f+.001f);
    assert(std::fabs(arm.velocity)<=limits.speed+.001f);
    if(next<169.95f) assert(std::fabs(arm.velocity-previousVelocity)<=limits.acceleration*.01f+.001f);
    previous=next;previousVelocity=arm.velocity;
  }
  assert(arm.position==170 && arm.velocity==0);
  arm.hold(90);for(unsigned i=0;i<50;++i)arm.step(170,.01f,limits);
  const float beforeRetarget=arm.position;
  arm.step(10,.01f,limits);assert(std::fabs(arm.position-beforeRetarget)<=.451f);
  for(unsigned i=0;i<700;++i) arm.step(10,.01f,limits);
  assert(arm.position==10 && arm.velocity==0);
  ControlLogic smooth; assert(smooth.acquire(Owner::Manual,99,0));
  Command pose{99,1,0,0,90,90,false};assert(smooth.command(pose,1)); smooth.tick(10);
  pose.sequence=2;pose.armLeft=170;pose.armRight=10;assert(smooth.command(pose,11));
  for(unsigned t=20;t<400;t+=10) {
    smooth.tick(t);
    if(t%100==0){++pose.sequence;assert(smooth.command(pose,t));}
  }
  assert(smooth.output.armLeft<110 && smooth.output.armRight>70);
  const float held=smooth.output.armLeft;
  smooth.tick(801); assert(smooth.owner==Owner::None && smooth.output.armLeft==held);
  smooth.tick(900);assert(smooth.output.armLeft==held);
  assert(smooth.configureArms({25,60}));
  assert(!smooth.configureArms({0,60}));assert(!smooth.configureArms({45,601}));
  assert(smooth.acquire(Owner::Manual,100,901));assert(!smooth.configureArms({25,60}));
}
