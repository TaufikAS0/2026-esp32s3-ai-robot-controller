#include "web_service.h"
#include "firmware_version.h"
#include "network_defaults.h"
#include "web_ui.h"
#include <Update.h>
#include <cJSON.h>
#include <memory>
namespace {
using Json = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>;
Json parse(WebServer& server) {
  String body = server.arg("plain");
  if (!server.header("Content-Type").startsWith("application/json") ||
      !body.length() || body.length() > Config::maxJson) return Json(nullptr, cJSON_Delete);
  return Json(cJSON_ParseWithOpts(body.c_str(), nullptr, true), cJSON_Delete);
}
bool fields(cJSON* root, std::initializer_list<const char*> names) {
  if (!cJSON_IsObject(root) || cJSON_GetArraySize(root) != int(names.size())) return false;
  for (const char* name : names) {
    unsigned count = 0;
    for (cJSON* p = root->child; p; p = p->next) if (p->string && strcmp(p->string, name) == 0) ++count;
    if (count != 1) return false;
  }
  return true;
}
cJSON* item(cJSON* root, const char* key) { return cJSON_GetObjectItemCaseSensitive(root, key); }
bool integer(cJSON* root, const char* key, uint32_t& value) {
  auto p = item(root, key);
  if (!cJSON_IsNumber(p) || !std::isfinite(p->valuedouble) || p->valuedouble < 1 ||
      p->valuedouble > UINT32_MAX || std::floor(p->valuedouble) != p->valuedouble) return false;
  value = uint32_t(p->valuedouble); return true;
}
bool number(cJSON* root, const char* key, float& value) {
  auto p = item(root, key);
  if (!cJSON_IsNumber(p)) return false;
  value = p->valuedouble; return std::isfinite(value);
}
}
void WebService::error(int code, const char* message) {
  server_.send(code, "application/json", String("{\"error\":\"") + message + "\"}");
}
bool WebService::authorize() {
  String provided = server_.header("Authorization");
  String expected = "Bearer " + network_.token;
  if (provided.length() != expected.length()) { error(401, "unauthorized"); return false; }
  unsigned mismatch = 0;
  for (size_t i = 0; i < provided.length(); ++i) mismatch |= provided[i] ^ expected[i];
  if (mismatch) { error(401, "unauthorized"); return false; }
  return true;
}
void WebService::begin() {
  const char* headers[] = {"Authorization", "Content-Type"};
  server_.collectHeaders(headers, 2);
  server_.on("/", HTTP_GET, [this]() { server_.send_P(200, "text/html; charset=utf-8", kWebUi); });
  server_.on("/api/v1/status", HTTP_GET, [this]() { if (authorize()) status(); });
  server_.on("/api/v1/control/acquire", HTTP_POST, [this]() { if (authorize()) acquire(); });
  server_.on("/api/v1/command", HTTP_POST, [this]() { if (authorize()) command(); });
  server_.on("/api/v1/control/release", HTTP_POST, [this]() { if (authorize()) release(); });
  server_.on("/api/v1/stop", HTTP_POST, [this]() {
    if (!authorize()) return;
    if (!control_.stop("stop")) { error(503, "task_unresponsive"); return; }
    server_.send(200, "application/json", "{\"ok\":true}");
  });
  server_.on("/api/v1/settings", HTTP_POST, [this]() { if (authorize()) settings(); });
  server_.on("/api/v1/update", HTTP_POST, [this]() { finishUpload(); }, [this]() { upload(); });
  server_.onNotFound([this]() { error(404, "not_found"); });
  server_.begin();
}
void WebService::status() {
  auto s = control_.snapshot();
  const auto& l = s.logic;
  Json root(cJSON_CreateObject(), cJSON_Delete);
  cJSON_AddStringToObject(root.get(), "firmware_version", kFirmwareVersion);
  cJSON_AddNumberToObject(root.get(), "api_version", 1);
  cJSON_AddNumberToObject(root.get(), "uptime_ms", millis());
  cJSON_AddStringToObject(root.get(), "owner", l.owner == Owner::Manual ? "manual" : l.owner == Owner::Program ? "program" : "none");
  cJSON_AddNumberToObject(root.get(), "session", l.session);
  cJSON_AddNumberToObject(root.get(), "sequence", l.lastSequence);
  cJSON_AddStringToObject(root.get(), "reason", l.reason);
  cJSON_AddBoolToObject(root.get(), "maintenance", l.maintenance);
  cJSON_AddBoolToObject(root.get(), "arduino_ota", ota_.enabled);
  cJSON_AddBoolToObject(root.get(), "sta_connected", WiFi.status() == WL_CONNECTED);
  cJSON_AddStringToObject(root.get(), "sta_target_ssid", NetworkDefaults::stationSsid);
  cJSON_AddStringToObject(root.get(), "sta_ssid", WiFi.SSID().c_str());
  cJSON_AddStringToObject(root.get(), "sta_ip", WiFi.localIP().toString().c_str());
  cJSON_AddBoolToObject(root.get(), "ap_active", network_.apActive);
  cJSON_AddStringToObject(root.get(), "ap_ip", WiFi.softAPIP().toString().c_str());
  cJSON* out = cJSON_AddObjectToObject(root.get(), "commanded_output");
  cJSON_AddNumberToObject(out, "left", l.output.left); cJSON_AddNumberToObject(out, "right", l.output.right);
  cJSON_AddNumberToObject(out, "arm_left", l.output.armLeft); cJSON_AddNumberToObject(out, "arm_right", l.output.armRight);
  cJSON_AddBoolToObject(out, "laser", l.output.laser); cJSON_AddBoolToObject(out, "servo_enabled", l.output.servoEnabled);
  char* json = cJSON_PrintUnformatted(root.get());
  if (!json) { error(503, "out_of_memory"); return; }
  server_.send(200, "application/json", json); cJSON_free(json);
}
void WebService::acquire() {
  auto root = parse(server_);
  if (!fields(root.get(), {"owner"}) || !cJSON_IsString(item(root.get(), "owner"))) { error(400, "invalid_owner"); return; }
  String name = item(root.get(), "owner")->valuestring;
  Owner owner = name == "manual" ? Owner::Manual : name == "program" ? Owner::Program : Owner::None;
  if (owner == Owner::None) { error(400, "invalid_owner"); return; }
  uint32_t session;
  if (!control_.acquire(owner, session)) { error(409, "control_unavailable"); return; }
  server_.send(200, "application/json", "{\"session\":" + String(session) + ",\"timeout_ms\":500}");
}
void WebService::command() {
  auto root = parse(server_); Command c;
  if (!fields(root.get(), {"session", "sequence", "left", "right", "arm_left", "arm_right", "laser"}) ||
      !integer(root.get(), "session", c.session) || !integer(root.get(), "sequence", c.sequence) ||
      !number(root.get(), "left", c.left) || !number(root.get(), "right", c.right) ||
      !number(root.get(), "arm_left", c.armLeft) || !number(root.get(), "arm_right", c.armRight) ||
      !cJSON_IsBool(item(root.get(), "laser"))) { error(400, "invalid_command"); return; }
  c.laser = cJSON_IsTrue(item(root.get(), "laser"));
  if (!ControlLogic::valid(c)) { error(400, "out_of_range"); return; }
  if (!control_.command(c)) { error(409, "session_or_sequence_rejected"); return; }
  server_.send(200, "application/json", "{\"ok\":true}");
}
void WebService::release() {
  auto root = parse(server_); uint32_t session;
  if (!fields(root.get(), {"session"}) || !integer(root.get(), "session", session)) { error(400, "invalid_session"); return; }
  if (!control_.release(session)) { error(409, "session_rejected"); return; }
  server_.send(200, "application/json", "{\"ok\":true}");
}
void WebService::settings() {
  auto root = parse(server_);
  if (fields(root.get(), {"arduino_ota"}) && cJSON_IsBool(item(root.get(), "arduino_ota"))) {
    if (webOta_ || ota_.busy) { error(409, "ota_busy"); return; }
    ota_.setEnabled(cJSON_IsTrue(item(root.get(), "arduino_ota")));
  } else if (fields(root.get(), {"ssid", "password"}) && cJSON_IsString(item(root.get(), "ssid")) &&
             cJSON_IsString(item(root.get(), "password"))) {
    if (webOta_ || ota_.busy) { error(409, "ota_busy"); return; }
    if (!control_.stop("wifi_config")) { error(503, "task_unresponsive"); return; }
    if (!network_.configure(item(root.get(), "ssid")->valuestring, item(root.get(), "password")->valuestring)) {
      error(400, "invalid_wifi_config"); return;
    }
  } else { error(400, "invalid_settings"); return; }
  server_.send(200, "application/json", "{\"ok\":true}");
}
void WebService::upload() {
  HTTPUpload& u = server_.upload();
  if (u.status == UPLOAD_FILE_START) {
    // A single multipart request must contain exactly one application image.
    if (webOta_) { uploadFailed_ = true; return; }
    uploadAuthorized_ = authorize(); uploadOk_ = false; uploadFailed_ = false; uploadComplete_ = false;
    if (!uploadAuthorized_) return;
    if (webOta_ || ota_.busy || ota_.enabled || u.name != "firmware") { uploadFailed_ = true; return; }
    webOta_ = true; uploadAt_ = millis();
    if (!control_.beginMaintenance() || !Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) uploadFailed_ = true;
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (!uploadAuthorized_ || !webOta_ || uploadFailed_) return;
    uploadAt_ = millis();
    if (Update.write(u.buf, u.currentSize) != u.currentSize) uploadFailed_ = true;
  } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
    if (!uploadAuthorized_ || !webOta_) return;
    uploadComplete_ = u.status == UPLOAD_FILE_END && !uploadFailed_;
    if (u.status == UPLOAD_FILE_ABORTED) {
      Update.abort(); webOta_ = false; uploadFailed_ = true; control_.endMaintenance();
    }
  }
}
void WebService::finishUpload() {
  if (!authorize()) return;
  uploadOk_ = webOta_ && uploadComplete_ && !uploadFailed_ && Update.end(true);
  if (!uploadOk_) {
    if (webOta_) { Update.abort(); control_.endMaintenance(); }
    webOta_ = false; error(400, "ota_failed_or_disabled"); return;
  }
  webOta_ = false;
  server_.send(200, "application/json", "{\"ok\":true,\"rebooting\":true}");
  restartAt_ = millis() + 500;
}
void WebService::update() {
  server_.handleClient();
  if (webOta_ && millis() - uploadAt_ > 10000) {
    Update.abort(); webOta_ = false; uploadFailed_ = true; control_.endMaintenance();
  }
  if (restartAt_ && int32_t(millis() - restartAt_) >= 0) ESP.restart();
}
