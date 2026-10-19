#pragma once
#include "control_service.h"
#include "network_service.h"
struct cJSON;
class TcpService {
 public:
  static constexpr uint16_t port=9000;
  TcpService(ControlService& control,NetworkService& network):control_(control),network_(network) {}
  bool begin();
  bool ready() const { return ready_.load(); }
 private:
  ControlService& control_;
  NetworkService& network_;
  std::atomic<bool> ready_{false};
  int listener_=-1;
  static void task(void* context);
  static void addNetwork(cJSON* reply,void* context);
};
