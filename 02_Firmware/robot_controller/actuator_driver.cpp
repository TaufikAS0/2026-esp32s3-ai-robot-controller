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
  ready_ = ok;
  return ok;
}
void ActuatorDriver::motor(uint8_t a, uint8_t b, float value) {
  // Clear inactive leg before applying PWM to active leg.
  if (value >= 0) { ledcWrite(b, 0); ledcWrite(a, lroundf(value * 1023)); }
  else { ledcWrite(a, 0); ledcWrite(b, lroundf(-value * 1023)); }
}
uint32_t ActuatorDriver::servoDuty(float angle) {
  float us = Config::servoMinUs + angle / 180 * (Config::servoMaxUs - Config::servoMinUs);
  return lroundf(us * 16383 / 20000);
}
void ActuatorDriver::write(const Output& out) {
  if (!ready_) return;
  motor(Config::left1, Config::left2, out.left);
  motor(Config::right1, Config::right2, out.right);
  ledcWrite(Config::armLeft, out.servoEnabled ? servoDuty(out.armLeft) : 0);
  ledcWrite(Config::armRight, out.servoEnabled ? servoDuty(out.armRight) : 0);
  digitalWrite(Config::laser, out.laser ? HIGH : LOW);
}
