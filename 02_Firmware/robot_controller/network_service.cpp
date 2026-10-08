#include "network_service.h"
#include <esp_system.h>
#include <esp_random.h>
String NetworkService::secret(const char* key) {
  String value = preferences_.getString(key, "");
  if (!value.length()) {
    uint8_t bytes[16]; esp_fill_random(bytes, sizeof(bytes));
    char hex[33];
    for (size_t i = 0; i < sizeof(bytes); ++i) snprintf(hex + i * 2, 3, "%02x", bytes[i]);
    value = hex; preferences_.putString(key, value);
  }
  return value;
}
void NetworkService::begin() {
  // Do not enable a default AP before its generated credentials are configured.
  WiFi.mode(WIFI_STA);
  preferences_.begin("robot", false);
  hostname = "ai-robot-" + String(uint32_t(ESP.getEfuseMac() & 0xffffff), HEX);
  apName = hostname + "-setup";
  token = secret("apiToken"); apPassword = secret("apPassword"); otaPassword = secret("otaPassword");
  Serial.println("API token: " + token);
  Serial.println("AP password: " + apPassword);
  Serial.println("OTA password: " + otaPassword);
  Serial.println("WiFi setup: wifi <ssid>|<password> (USB serial, 115200 baud)");
  WiFi.setHostname(hostname.c_str());
  connect();
}
void NetworkService::connect() {
  String ssid = preferences_.getString("ssid", "");
  if (ssid.length()) WiFi.begin(ssid.c_str(), preferences_.getString("wifiPassword", "").c_str());
  attemptAt_ = millis();
}
bool NetworkService::configure(const String& ssid, const String& password) {
  if (!ssid.length() || ssid.length() > 32 || password.length() > 63) return false;
  preferences_.putString("ssid", ssid);
  preferences_.putString("wifiPassword", password);
  WiFi.disconnect(); connect(); return true;
}
void NetworkService::update() {
  if (!apActive && WiFi.status() != WL_CONNECTED && millis() - attemptAt_ >= 15000) {
    apActive = WiFi.softAP(apName.c_str(), apPassword.c_str());
    Serial.println("Setup AP: " + apName + " / " + WiFi.softAPIP().toString());
  }
  // Bound serial work per iteration; never echo credentials.
  for (unsigned i = 0; i < 64 && Serial.available(); ++i) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (serialLine_ == "info") {
        Serial.println("API token: " + token);
        Serial.println("AP password: " + apPassword);
        Serial.println("OTA password: " + otaPassword);
        Serial.println("STA IP: " + WiFi.localIP().toString());
        Serial.println("AP IP: " + WiFi.softAPIP().toString());
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
