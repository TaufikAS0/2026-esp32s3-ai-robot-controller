#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
class NetworkService {
 public:
  void begin();
  void update();
  bool configure(const String& ssid, const String& password);
  String apPassword;
  String hostname, apName;
  bool apActive = false;
 private:
  Preferences preferences_;
  uint32_t attemptAt_ = 0;
  String serialLine_;
  void connect();
};
