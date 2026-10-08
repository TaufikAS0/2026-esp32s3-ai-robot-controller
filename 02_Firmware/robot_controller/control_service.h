#pragma once
#include <Arduino.h>
#include "actuator_driver.h"
struct ControlSnapshot { ControlLogic logic; uint32_t appliedGeneration; };
class ControlService {
 public:
  bool begin();
  bool acquire(Owner owner, uint32_t& session);
  bool command(const Command& command);
  bool release(uint32_t session);
  bool stop(const char* reason, bool disableServo = false);
  bool beginMaintenance();
  void endMaintenance();
  ControlSnapshot snapshot();
 private:
  static void task(void* argument);
  bool awaitApplied(uint32_t generation);
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  ControlLogic logic_;
  ActuatorDriver driver_;
  uint32_t generation_ = 0, appliedGeneration_ = 0;
  volatile bool initialized_ = false, hardwareReady_ = false;
};
