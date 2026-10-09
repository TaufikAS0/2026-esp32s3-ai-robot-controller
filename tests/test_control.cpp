#include <cassert>
#include <limits>
#include "../02_Firmware/robot_controller/control_logic.h"

int main() {
  ControlLogic c;
  assert(!c.output.servoEnabled && c.output.left == 0 && !c.output.laser);
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
}
