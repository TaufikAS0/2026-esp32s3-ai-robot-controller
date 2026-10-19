#pragma once
#include "web_service.h"
#include "tcp_service.h"
class App {
 public:
  void begin();
  void update();
 private:
  ControlService control_;
  NetworkService network_;
  OtaService ota_;
  EyeService eyes_{control_};
  TcpService tcp_{control_,network_};
  WebService web_{control_, network_, ota_, eyes_, tcp_};
};
