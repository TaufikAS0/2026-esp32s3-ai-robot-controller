#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <atomic>
class NetworkService {
 public:
  void begin();
  void update();
  bool configure(const String& ssid, const String& password);
  String apPassword;
  String hostname, apName;
  std::atomic<bool> apActive{false};
  uint32_t disconnectCount() const { return disconnectCount_.load(); }
  uint32_t disconnectReason() const { return lastDisconnectReason_.load(); }
  uint32_t connectCount() const { return connectCount_.load(); }
 private:
  Preferences preferences_;
  uint32_t attemptAt_ = 0;
  std::atomic<uint32_t> disconnectCount_{0};
  std::atomic<uint32_t> connectCount_{0};
  std::atomic<uint32_t> lastDisconnectReason_{0};
  uint32_t reportedDisconnectCount_ = 0;
  int lastStatus_ = -1;
  String serialLine_;
  void connect();
};
