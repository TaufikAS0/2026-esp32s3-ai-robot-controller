#include "actuator_driver.h"
bool ActuatorDriver::begin() {
  const uint8_t pins[] = {Config::right1, Config::right2, Config::left1,
                         Config::left2, Config::armLeft, Config::armRight, Config::laser};
  for (auto pin : pins) { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }
  // S3 LEDC timers are paired: motor channels 0..3; servo channels 4..5.
  bool ok = ledcAttachChannel(Config::right1, Config::motorHz, 10, 0);
  ok = ledcAttachChannel(Config::right2, Config::motorHz, 10, 1) && ok;
  ok = ledcAttachChannel(Config::left1, Config::motorHz, 10, 2) && ok;
  ok = ledcAttachChannel(Config::left2, Config::motorHz, 10, 3) && ok;
  ok = ledcAttachChannel(Config::armLeft, Config::servoHz, 14, 4) && ok;
  ok = ledcAttachChannel(Config::armRight, Config::servoHz, 14, 5) && ok;
  ready_ = ok; state_.ready = ok;
  return ok;
}
bool ActuatorDriver::motor(uint8_t a, uint8_t b, float value) {
  // Clear inactive leg before applying PWM to active leg.
  bool inactive, active;
  if (value >= 0) {
    inactive = ledcWrite(b, 0); active = ledcWrite(a, inactive ? lroundf(value * 1023) : 0);
  } else {
    inactive = ledcWrite(a, 0); active = ledcWrite(b, inactive ? lroundf(-value * 1023) : 0);
  }
  return inactive && active;
}
uint32_t ActuatorDriver::servoDuty(float angle) {
  float us = Config::servoMinUs + angle / 180 * (Config::servoMaxUs - Config::servoMinUs);
  return lroundf(us * 16383 / 20000);
}
bool ActuatorDriver::write(const Output& out) {
  if (!ready_) return false;
  bool ok = motor(Config::left1, Config::left2, out.left);
  ok = motor(Config::right1, Config::right2, out.right) && ok;
  state_.armLeftPulseAngle = Config::invertLeftArm ? 180 - out.armLeft : out.armLeft;
  state_.armRightPulseAngle = Config::invertRightArm ? 180 - out.armRight : out.armRight;
  ok = ledcWrite(Config::armLeft, out.servoEnabled ? servoDuty(state_.armLeftPulseAngle) : 0) && ok;
  ok = ledcWrite(Config::armRight, out.servoEnabled ? servoDuty(state_.armRightPulseAngle) : 0) && ok;
  digitalWrite(Config::laser, out.laser ? HIGH : LOW);
  state_.left1 = ledcRead(Config::left1); state_.left2 = ledcRead(Config::left2);
  state_.right1 = ledcRead(Config::right1); state_.right2 = ledcRead(Config::right2);
  state_.leftHz = ledcReadFreq(out.left >= 0 ? Config::left1 : Config::left2);
  state_.rightHz = ledcReadFreq(out.right >= 0 ? Config::right1 : Config::right2);
  state_.writeOk = ok;
  return ok;
}
