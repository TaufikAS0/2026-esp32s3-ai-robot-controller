#pragma once
#include <WebServer.h>
#include "ota_service.h"
class WebService {
 public:
  WebService(ControlService& control, NetworkService& network, OtaService& ota)
    : control_(control), network_(network), ota_(ota) {}
  void begin();
  void update();
 private:
  WebServer server_{80};
  ControlService& control_;
  NetworkService& network_;
  OtaService& ota_;
  bool uploadOk_ = false, uploadFailed_ = false, webOta_ = false;
  bool uploadComplete_ = false;
  uint32_t uploadAt_ = 0, restartAt_ = 0;
  void error(int code, const char* message);
  void status();
  void acquire();
  void command();
  void release();
  void settings();
  void upload();
  void finishUpload();
};
