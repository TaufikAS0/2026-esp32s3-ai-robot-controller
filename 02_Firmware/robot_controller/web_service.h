#pragma once
#include <WebServer.h>
#include <esp_http_server.h>
#include "ota_service.h"
#include "eye_service.h"
#include "tcp_service.h"
class WebService {
 public:
  WebService(ControlService& control, NetworkService& network, OtaService& ota, EyeService& eyes, TcpService& tcp)
    : control_(control), network_(network), ota_(ota), eyes_(eyes), tcp_(tcp) {}
  void begin();
  void update();
 private:
  WebServer server_{80};
  httpd_handle_t fastServer_=nullptr;
  void beginFastControl();
  static esp_err_t fastControl(httpd_req_t* request);
  char* statusJson();
  ControlService& control_;
  NetworkService& network_;
  OtaService& ota_;
  EyeService& eyes_;
  TcpService& tcp_;
  bool uploadOk_ = false, uploadFailed_ = false, webOta_ = false;
  bool uploadComplete_ = false;
  uint32_t uploadAt_ = 0, restartAt_ = 0;
  uint32_t applicationBytes_ = 0, otaSlotBytes_ = 0;
  void error(int code, const char* message);
  void status();
  void eyes();
  void eyeCheck();
  void eyeCatalog();
  void armSettings();
  void armPolicy();
  int applyArmPolicy(struct cJSON* root);
  void acquire();
  void mode();
  void command();
  void release();
  void settings();
  void upload();
  void finishUpload();
};
