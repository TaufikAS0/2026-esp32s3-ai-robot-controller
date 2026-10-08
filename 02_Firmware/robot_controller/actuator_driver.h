#pragma once
#include <Arduino.h>
#include "control_logic.h"
class ActuatorDriver {
 public:
  bool begin();
  void write(const Output& output);
 private:
  bool ready_ = false;
  static void motor(uint8_t a, uint8_t b, float value);
  static uint32_t servoDuty(float angle);
};
