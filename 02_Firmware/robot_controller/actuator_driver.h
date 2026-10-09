#pragma once
#include <Arduino.h>
#include "control_logic.h"
struct PwmSnapshot {
  bool ready = false, writeOk = false;
  uint32_t left1 = 0, left2 = 0, right1 = 0, right2 = 0;
  uint32_t leftHz = 0, rightHz = 0;
  float armLeftPulseAngle = 90, armRightPulseAngle = 90;
};
class ActuatorDriver {
 public:
  bool begin();
  bool write(const Output& output);
  PwmSnapshot snapshot() const { return state_; }
 private:
  bool ready_ = false;
  PwmSnapshot state_;
  static bool motor(uint8_t a, uint8_t b, float value);
  static uint32_t servoDuty(float angle);
};
