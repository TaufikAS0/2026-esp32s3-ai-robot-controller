#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <atomic>
#include "wifi_profiles.h"
#include "control_service.h"
class NetworkService {
 public:
  void begin(ControlService& control);
  void update(bool otaEnabled=false);
  bool configure(const String& ssid,const String& password);
  int save(const char* ssid,const char* password);
  int remove(const char* ssid);
  int reorder(cJSON* ssids);
  int connectSaved(const char* ssid);
  int startScan();
  char* settingsJson(); // HTTP80/main task only; never returns passwords
  String targetSsid();
  const char* phaseName() const;
  String apPassword,hostname,apName;
  std::atomic<bool> apActive{false};
  uint32_t disconnectCount() const { return disconnectCount_.load(); }
  uint32_t disconnectReason() const { return lastDisconnectReason_.load(); }
  uint32_t connectCount() const { return connectCount_.load(); }
 private:
  enum class Phase { Waiting, Connecting, Connected, Pending };
  std::atomic<Phase> phase_{Phase::Waiting};
  Preferences preferences_;
  WifiProfiles profiles_;
  WifiCycle cycle_;
  ControlService* control_=nullptr;
  bool storageReady_=false,guard_=false,scanning_=false;
  const char* scanState_="idle";
  struct ScanEntry { char ssid[33]{};int rssi=0;bool secured=false; } scan_[24];
  unsigned scanCount_=0;
  uint32_t scanAt_=0,phaseAt_=0;
  int pending_=-1;
  char target_[33]{};
  portMUX_TYPE targetMux_=portMUX_INITIALIZER_UNLOCKED;
  std::atomic<bool> wasConnected_{false},linkLost_{false};
  std::atomic<uint32_t> disconnectCount_{0},lastDisconnectReason_{0},connectCount_{0};
  uint32_t reportedDisconnectCount_=0;
  String serialLine_;
  bool reserve(bool lost=false);
  void releaseGuard();
  bool persist(const WifiProfiles& next);
  void attempt(int index);
  void nextAttempt();
  void finishScan();
};
