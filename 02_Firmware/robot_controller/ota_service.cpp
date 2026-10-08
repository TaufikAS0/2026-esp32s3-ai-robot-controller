#include "ota_service.h"
void OtaService::begin(ControlService& control, NetworkService& network) {
  control_ = &control;
  ArduinoOTA.setHostname(network.hostname.c_str());
  ArduinoOTA.setPassword(network.otaPassword.c_str());
  ArduinoOTA.onStart([this]() {
    busy = true;
    if (!control_->beginMaintenance()) { Serial.println("OTA stop acknowledgement failed"); ESP.restart(); }
  });
  ArduinoOTA.onError([this](ota_error_t) {
    // Authentication errors can occur before onStart: do not disturb an active lease.
    if (busy) { busy = false; control_->endMaintenance(); }
  });
  ArduinoOTA.onEnd([this]() { busy = false; });
}
void OtaService::setEnabled(bool value) {
  if (value == enabled) return;
  if (value) ArduinoOTA.begin(); else ArduinoOTA.end();
  enabled = value;
}
void OtaService::update() { if (enabled) ArduinoOTA.handle(); }
