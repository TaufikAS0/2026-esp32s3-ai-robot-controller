#include "ota_service.h"
void OtaService::begin(ControlService& control, NetworkService& network) {
  control_ = &control;
  ArduinoOTA.setHostname(network.hostname.c_str());
  // Open-lab profile: ArduinoOTA has no password (still OFF by default).
  ArduinoOTA.onStart([this]() {
    busy = true;
    if (!control_->beginMaintenance()) { Serial.println("OTA stop acknowledgement failed"); ESP.restart(); }
  });
  ArduinoOTA.onError([this](ota_error_t) {
    // Only an upload that entered maintenance may change the control lease on error.
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
