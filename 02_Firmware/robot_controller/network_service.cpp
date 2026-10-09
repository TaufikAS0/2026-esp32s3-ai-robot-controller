#include "network_service.h"
#include "network_defaults.h"
#include <esp_system.h>
void NetworkService::begin() {
  // Do not enable a default AP before its lab credentials are configured.
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      lastDisconnectReason_.store(info.wifi_sta_disconnected.reason);
      disconnectCount_.fetch_add(1);
    }
  });
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  Serial.printf("Reset reason: %d; free heap: %u\n", int(esp_reset_reason()), ESP.getFreeHeap());
  // Responsive LAN control takes priority over modem power saving in this lab.
  if (!WiFi.setSleep(false)) Serial.println("WiFi sleep disable failed");
  preferences_.begin("robot", false);
  hostname = "ai-robot-" + String(uint32_t(ESP.getEfuseMac() & 0xffffff), HEX);
  apName = hostname + "-setup";
  // Explicit open-lab migration: remove only obsolete authentication keys.
  preferences_.remove("apiToken");
  preferences_.remove("otaPassword");
  apPassword = NetworkDefaults::apPassword;
  // Correct existing devices too, preserving unrelated NVS.
  if (preferences_.getString("apPassword", "") != apPassword)
    preferences_.putString("apPassword", apPassword);
  if (preferences_.getString("ssid", "") != NetworkDefaults::stationSsid)
    preferences_.putString("ssid", NetworkDefaults::stationSsid);
  if (preferences_.getString("wifiPassword", "") != NetworkDefaults::stationPassword)
    preferences_.putString("wifiPassword", NetworkDefaults::stationPassword);
  Serial.println("Access: open LAN; API and OTA require no credentials");
  Serial.println("AP password: " + apPassword);

  Serial.println("WiFi setup: wifi <ssid>|<password> (USB serial, 115200 baud)");
  WiFi.setHostname(hostname.c_str());
  connect();
}
void NetworkService::connect() {
  WiFi.begin(NetworkDefaults::stationSsid, NetworkDefaults::stationPassword);
  Serial.println(String("STA target: ") + NetworkDefaults::stationSsid);
  attemptAt_ = millis();
}
bool NetworkService::configure(const String& ssid, const String& password) {
  // This internal-lab build cannot be redirected to an old/arbitrary profile.
  if (ssid != NetworkDefaults::stationSsid || password != NetworkDefaults::stationPassword) return false;
  WiFi.disconnect(); connect(); return true;
}
void NetworkService::update() {
  const int status = WiFi.status();
  const uint32_t disconnects = disconnectCount_.load();
  if (disconnects != reportedDisconnectCount_) {
    reportedDisconnectCount_ = disconnects;
    Serial.printf("WiFi disconnected: reason=%u count=%u\n", lastDisconnectReason_.load(), disconnects);
  }
  if (status != lastStatus_) {
    lastStatus_ = status;
    Serial.printf("WiFi status=%d IP=%s RSSI=%d heap=%u\n", status,
      WiFi.localIP().toString().c_str(), WiFi.RSSI(), ESP.getFreeHeap());
  }
  // Do not rely only on the core's reason-dependent automatic reconnect.
  // Allow a full connection/DHCP window, then recover a stalled attempt.
  if (status == WL_CONNECTED) attemptAt_ = millis();
  if (!apActive && WiFi.status() != WL_CONNECTED && millis() - attemptAt_ >= 15000) {
    WiFi.mode(WIFI_AP_STA);
    apActive = WiFi.softAP(apName.c_str(), apPassword.c_str());
    Serial.println("Setup AP: " + apName + " / " + WiFi.softAPIP().toString());
  }
  if (status != WL_CONNECTED && millis() - attemptAt_ >= 30000) {
    Serial.println("WiFi retry: HuaweiJIN");
    WiFi.disconnect();
    connect();
  }
  // Bound serial work per iteration; never echo credentials.
  for (unsigned i = 0; i < 64 && Serial.available(); ++i) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (serialLine_ == "info") {
        Serial.println("Access: open LAN; API and OTA require no credentials");
        Serial.println("AP password: " + apPassword);

        Serial.println("STA IP: " + WiFi.localIP().toString());
        Serial.println("STA SSID: " + WiFi.SSID());
        Serial.println(String("STA connected: ") + (WiFi.status() == WL_CONNECTED ? "true" : "false"));
        Serial.println("AP IP: " + WiFi.softAPIP().toString());
        Serial.printf("WiFi reason=%u disconnects=%u RSSI=%d heap=%u uptime=%u\n",
          lastDisconnectReason_.load(), disconnectCount_.load(), WiFi.RSSI(), ESP.getFreeHeap(), millis());
      } else if (serialLine_.startsWith("wifi ")) {
        int split = serialLine_.indexOf('|', 5);
        bool ok = split >= 0 && configure(serialLine_.substring(5, split), serialLine_.substring(split + 1));
        Serial.println(ok ? "WiFi configuration saved" : "Invalid WiFi configuration");
      }
      serialLine_ = "";
    } else if (serialLine_.length() < 128) serialLine_ += c;
    else serialLine_ = "";
  }
}
