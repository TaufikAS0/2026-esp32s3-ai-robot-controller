#pragma once
#include <Arduino.h>
#include "actuator_driver.h"
struct ControlSnapshot { ControlLogic logic; uint32_t appliedGeneration; PwmSnapshot pwm; };
class ControlService {
 public:
  bool begin();
  bool acquire(Owner owner, uint32_t& session, uint32_t connection = 0);
  bool setMode(ControlMode mode, bool idleOnly = false);
  bool configureArms(const ArmMotionSettings& settings);
  bool configureArmPolicy(const ArmPolicy& policy);
  bool command(const Command& command, uint32_t connection = 0);
  bool release(uint32_t session, uint32_t connection = 0);
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
  PwmSnapshot pwm_;
  uint32_t generation_ = 0, appliedGeneration_ = 0;
  volatile bool initialized_ = false, hardwareReady_ = false;
};
