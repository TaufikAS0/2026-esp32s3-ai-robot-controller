#pragma once
#include <ArduinoOTA.h>
#include "control_service.h"
#include "network_service.h"
class OtaService {
 public:
  void begin(ControlService& control, NetworkService& network);
  void setEnabled(bool enabled);
  void update();
  bool enabled = false, busy = false;
 private:
  ControlService* control_ = nullptr;
};
