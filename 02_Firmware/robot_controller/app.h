#pragma once
#include "web_service.h"
class App {
 public:
  void begin();
  void update();
 private:
  ControlService control_;
  NetworkService network_;
  OtaService ota_;
  EyeService eyes_{control_};
  WebService web_{control_, network_, ota_, eyes_};
};
