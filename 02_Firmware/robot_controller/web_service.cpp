#include "web_service.h"
#include "firmware_version.h"
#include "network_defaults.h"
#include "web_ui.h"
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <cJSON.h>
#include <memory>
#include "api_json.h"
using namespace ApiJson;
namespace {
Json parse(WebServer& server) {
  String body=server.arg("plain");
  if(!server.header("Content-Type").startsWith("application/json")||!body.length()||body.length()>Config::maxJson) return Json(nullptr,cJSON_Delete);
  return Json(cJSON_ParseWithOpts(body.c_str(),nullptr,true),cJSON_Delete);
}
}
void WebService::error(int code, const char* message) {
  server_.send(code, "application/json", String("{\"error\":\"") + message + "\"}");
}
void WebService::begin() {
  // Image verification reads flash; cache immutable sizes once, not per status poll.
  applicationBytes_ = ESP.getSketchSize();
  const esp_partition_t* running = esp_ota_get_running_partition();
  otaSlotBytes_ = running ? running->size : 0;
  const char* headers[] = {"Content-Type"};
  server_.collectHeaders(headers, 1);
  server_.on("/", HTTP_GET, [this]() { server_.sendHeader("Cache-Control", "no-store"); server_.send_P(200, "text/html; charset=utf-8", kWebUi); });
  server_.on("/api/v1/status", HTTP_GET, [this]() { status(); });
  server_.on("/api/v1/control/mode", HTTP_POST, [this]() { mode(); });
  server_.on("/api/v1/control/acquire", HTTP_POST, [this]() { acquire(); });
  server_.on("/api/v1/command", HTTP_POST, [this]() { command(); });
  server_.on("/api/v1/control/release", HTTP_POST, [this]() { release(); });
  server_.on("/api/v1/stop", HTTP_POST, [this]() {
    if (!control_.stop("stop")) { error(503, "task_unresponsive"); return; }
    server_.send(200, "application/json", "{\"ok\":true}");
  });
  server_.on("/api/v1/arms/settings", HTTP_POST, [this]() { armSettings(); });
  server_.on("/api/v1/arms/policy", HTTP_POST, [this]() { armPolicy(); });
  server_.on("/api/v1/eyes/catalog", HTTP_GET, [this]() { eyeCatalog(); });
  server_.on("/api/v1/eyes/check", HTTP_POST, [this]() { eyeCheck(); });
  server_.on("/api/v1/eyes", HTTP_POST, [this]() { eyes(); });
  server_.on("/api/v1/settings", HTTP_POST, [this]() { settings(); });
  server_.on("/api/v1/wifi",HTTP_GET,[this](){wifiSettings();});
  server_.on("/api/v1/demo",HTTP_GET,[this](){demoStatus();});
  for(const auto path:{"/api/v1/demo/config","/api/v1/demo/zero","/api/v1/demo/start","/api/v1/demo/stop","/api/v1/demo/calibration/start","/api/v1/demo/calibration/result","/api/v1/demo/exhibition/config","/api/v1/demo/exhibition/center","/api/v1/demo/exhibition/start"})
    server_.on(path,HTTP_POST,[this](){demoAction();});
  for(const auto path:{"/api/v1/wifi/scan","/api/v1/wifi/save","/api/v1/wifi/delete","/api/v1/wifi/order","/api/v1/wifi/connect"})
    server_.on(path,HTTP_POST,[this](){wifiAction();});
  server_.on("/api/v1/update", HTTP_POST, [this]() { finishUpload(); }, [this]() { upload(); });
  server_.onNotFound([this]() { error(404, "not_found"); });
  server_.begin();
  beginFastControl();
}
void WebService::status() {
  char* json=statusJson();
  if(!json){error(503,"out_of_memory");return;}
  server_.sendHeader("Cache-Control","no-store");
  server_.send(200,"application/json",json);cJSON_free(json);
}
char* WebService::statusJson() {
  auto s = control_.snapshot();
  const auto& l = s.logic;
  Json root(cJSON_CreateObject(), cJSON_Delete);
  cJSON_AddStringToObject(root.get(), "firmware_version", kFirmwareVersion);
  cJSON_AddStringToObject(root.get(), "build_time_utc", kFirmwareBuildUtc);
  cJSON_AddNumberToObject(root.get(), "api_version", 1);
  cJSON_AddStringToObject(root.get(), "mode", l.mode == ControlMode::Manual ? "manual" : "auto");
  cJSON_AddStringToObject(root.get(), "access_mode", "open_lan");
  auto channel=cJSON_AddObjectToObject(root.get(),"control_channel");
  cJSON_AddBoolToObject(channel,"ready",fastServer_!=nullptr);
  cJSON_AddNumberToObject(channel,"port",81);
  auto tcp=cJSON_AddObjectToObject(root.get(),"tcp_control");
  cJSON_AddBoolToObject(tcp,"ready",tcp_.ready());
  cJSON_AddNumberToObject(tcp,"port",TcpService::port);
  cJSON_AddNumberToObject(tcp,"protocol_version",1);
  cJSON_AddNumberToObject(root.get(), "uptime_ms", millis());
  cJSON_AddStringToObject(root.get(), "owner", l.owner == Owner::Manual ? "manual" : l.owner == Owner::Program ? "program" : "none");
  cJSON_AddNumberToObject(root.get(), "session", l.session);
  cJSON_AddNumberToObject(root.get(), "sequence", l.lastSequence);
  cJSON_AddStringToObject(root.get(),"control_transport",l.owner==Owner::None?"none":l.demoOwned?"demo":l.transport?"tcp":"http");
  cJSON_AddStringToObject(root.get(), "reason", l.reason);
  cJSON_AddBoolToObject(root.get(), "maintenance", l.maintenance);
  cJSON_AddBoolToObject(root.get(), "arduino_ota", ota_.enabled);
  cJSON_AddBoolToObject(root.get(), "wifi_sleep", WiFi.getSleep());
  cJSON_AddBoolToObject(root.get(), "sta_connected", WiFi.status() == WL_CONNECTED);
  cJSON_AddStringToObject(root.get(), "sta_target_ssid", network_.targetSsid().c_str());
  cJSON_AddStringToObject(root.get(),"wifi_state",network_.phaseName());
  cJSON_AddBoolToObject(root.get(),"network_busy",l.networkBusy);
  cJSON_AddStringToObject(root.get(), "sta_ssid", WiFi.SSID().c_str());
  cJSON_AddStringToObject(root.get(), "sta_ip", WiFi.localIP().toString().c_str());
  cJSON_AddStringToObject(root.get(), "sta_mac", WiFi.macAddress().c_str());
  cJSON_AddNumberToObject(root.get(), "wifi_disconnect_count", network_.disconnectCount());
  cJSON_AddNumberToObject(root.get(), "wifi_connect_count", network_.connectCount());
  cJSON_AddNumberToObject(root.get(), "wifi_disconnect_reason", network_.disconnectReason());
  cJSON_AddNumberToObject(root.get(), "reset_reason", esp_reset_reason());
  cJSON_AddBoolToObject(root.get(), "ap_active", network_.apActive);
  cJSON_AddStringToObject(root.get(), "ap_ip", WiFi.softAPIP().toString().c_str());
  cJSON* hardware = cJSON_AddObjectToObject(root.get(), "hardware");
  cJSON_AddStringToObject(hardware, "chip_model", ESP.getChipModel());
  cJSON_AddNumberToObject(hardware, "cpu_freq_mhz", ESP.getCpuFreqMHz());
  cJSON_AddNumberToObject(hardware, "heap_size_bytes", ESP.getHeapSize());
  cJSON_AddNumberToObject(hardware, "free_heap_bytes", ESP.getFreeHeap());
  cJSON_AddNumberToObject(hardware, "min_free_heap_bytes", ESP.getMinFreeHeap());
  cJSON_AddNumberToObject(hardware, "psram_size_bytes", ESP.getPsramSize());
  cJSON_AddNumberToObject(hardware, "free_psram_bytes", ESP.getFreePsram());
  cJSON_AddNumberToObject(hardware, "flash_size_bytes", ESP.getFlashChipSize());
  cJSON_AddNumberToObject(hardware, "application_bytes", applicationBytes_);
  cJSON_AddNumberToObject(hardware, "ota_slot_bytes", otaSlotBytes_);
  if (WiFi.status() == WL_CONNECTED) cJSON_AddNumberToObject(hardware, "wifi_rssi_dbm", WiFi.RSSI());
  else cJSON_AddNullToObject(hardware, "wifi_rssi_dbm");
  const auto eye = eyes_.snapshot();
  cJSON* e = cJSON_AddObjectToObject(root.get(), "eyes");
  cJSON_AddBoolToObject(e,"enabled",eye.settings.enabled);
  cJSON_AddBoolToObject(e,"automatic",eye.settings.automatic);
  cJSON_AddBoolToObject(e,"blink",eye.settings.blink);
  cJSON_AddBoolToObject(e,"swap",eye.settings.swap);
  cJSON_AddBoolToObject(e,"rotate_left",eye.settings.rotateLeft);
  cJSON_AddBoolToObject(e,"rotate_right",eye.settings.rotateRight);
  cJSON_AddNumberToObject(e,"brightness",eye.settings.brightness);
  cJSON_AddNumberToObject(e,"tempo",eye.settings.tempo);
  cJSON_AddStringToObject(e,"expression",expressionName(eye.settings.expression));
  cJSON_AddStringToObject(e,"play_mode",eyePlayName(eye.settings.playMode));
  cJSON_AddStringToObject(e,"category",eyeCategoryId(eye.settings.category));
  cJSON_AddNumberToObject(e,"energy",eye.settings.energy);
  cJSON_AddStringToObject(e,"active_expression",expressionName(eye.frame.expression));
  cJSON_AddStringToObject(e,"reaction",eye.frame.reaction);
  cJSON_AddBoolToObject(e,"task_ready",eye.taskReady);
  cJSON_AddBoolToObject(e,"left_online",eye.leftOnline);
  cJSON_AddBoolToObject(e,"right_online",eye.rightOnline);
  cJSON_AddNumberToObject(e,"errors",eye.errors);
  cJSON_AddNumberToObject(e,"frames",eye.frames);
  cJSON_AddNumberToObject(e,"work_us",eye.workUs);
  cJSON_AddNumberToObject(e,"max_work_us",eye.maxWorkUs);
  cJSON_AddNumberToObject(e,"overruns",eye.overruns);
  cJSON_AddNumberToObject(e,"sda",Config::eyeSda);
  cJSON_AddNumberToObject(e,"scl",Config::eyeScl);
  cJSON_AddNumberToObject(e,"left_address",Config::eyeLeftAddress);
  cJSON_AddNumberToObject(e,"right_address",Config::eyeRightAddress);
  cJSON* check=cJSON_AddObjectToObject(e,"check");
  cJSON_AddStringToObject(check,"mode",eyeCheckName(eye.check.mode));
  cJSON_AddStringToObject(check,"target",eye.check.target==EyeTarget::Both ? "both" : eye.check.target==EyeTarget::Left ? "left" : "right");
  cJSON_AddNumberToObject(check,"request",eye.check.request);
  cJSON_AddNumberToObject(check,"remaining_ms",eye.check.remainingMs);
  cJSON_AddBoolToObject(check,"scan_complete",eye.check.scanComplete);
  cJSON_AddNumberToObject(check,"scanned",eye.check.scanned);
  cJSON_AddNumberToObject(check,"last_scan_ms",eye.check.lastScanMs);
  cJSON_AddNumberToObject(check,"left_probe_code",eye.check.leftProbe);
  cJSON_AddNumberToObject(check,"right_probe_code",eye.check.rightProbe);
  cJSON* found=cJSON_AddArrayToObject(check,"found_addresses");
  for(unsigned a=8;a<120;++a) if(eye.check.found[a/8] & (1<<(a%8))) cJSON_AddItemToArray(found,cJSON_CreateNumber(a));
  for (unsigned i=0;i<2;++i) {
    const auto& pose = i ? eye.frame.right : eye.frame.left;
    cJSON* p = cJSON_AddObjectToObject(e,i ? "right" : "left");
    cJSON_AddNumberToObject(p,"x",pose.x); cJSON_AddNumberToObject(p,"y",pose.y);
    cJSON_AddNumberToObject(p,"pupilX",pose.pupilX); cJSON_AddNumberToObject(p,"pupilY",pose.pupilY);
    cJSON_AddNumberToObject(p,"phase",pose.phase);
    cJSON_AddNumberToObject(p,"width",pose.width); cJSON_AddNumberToObject(p,"height",pose.height);
  }
  auto arms=cJSON_AddObjectToObject(root.get(),"arm_motion");
  cJSON_AddNumberToObject(arms,"speed_dps",l.armSettings.speed);
  cJSON_AddNumberToObject(arms,"acceleration_dps2",l.armSettings.acceleration);
  cJSON_AddNumberToObject(arms,"target_left",l.target.armLeft);
  cJSON_AddNumberToObject(arms,"target_right",l.target.armRight);
  cJSON_AddBoolToObject(arms,"moving",l.hasCommand && (std::fabs(l.effectiveArmLeft-l.output.armLeft)>.05f || std::fabs(l.effectiveArmRight-l.output.armRight)>.05f));
  cJSON_AddBoolToObject(arms,"wheel_channels_swapped",Config::swapMotorSides);
  cJSON_AddNumberToObject(arms,"effective_left",l.effectiveArmLeft);
  cJSON_AddNumberToObject(arms,"effective_right",l.effectiveArmRight);
  cJSON_AddNumberToObject(arms,"min_left",l.armPolicy.leftMin);
  cJSON_AddNumberToObject(arms,"max_left",l.armPolicy.leftMax);
  cJSON_AddNumberToObject(arms,"min_right",l.armPolicy.rightMin);
  cJSON_AddNumberToObject(arms,"max_right",l.armPolicy.rightMax);
  cJSON_AddBoolToObject(arms,"dynamic",l.armPolicy.dynamic);
  cJSON_AddBoolToObject(arms,"dynamic_active",l.dynamicActive);
  cJSON_AddNumberToObject(arms,"amplitude_deg",l.armPolicy.amplitude);
  cJSON_AddNumberToObject(arms,"period_ms",l.armPolicy.periodMs);
  cJSON* out = cJSON_AddObjectToObject(root.get(), "commanded_output");
  cJSON_AddNumberToObject(out, "left", l.output.left); cJSON_AddNumberToObject(out, "right", l.output.right);
  cJSON_AddNumberToObject(out, "arm_left", l.output.armLeft); cJSON_AddNumberToObject(out, "arm_right", l.output.armRight);
  cJSON_AddBoolToObject(out, "laser", l.output.laser); cJSON_AddBoolToObject(out, "servo_enabled", l.output.servoEnabled);
  cJSON* pwm = cJSON_AddObjectToObject(root.get(), "pwm");
  cJSON_AddBoolToObject(pwm, "ready", s.pwm.ready);
  cJSON_AddBoolToObject(pwm, "write_ok", s.pwm.writeOk);
  cJSON_AddNumberToObject(pwm, "left1_duty", s.pwm.left1);
  cJSON_AddNumberToObject(pwm, "left2_duty", s.pwm.left2);
  cJSON_AddNumberToObject(pwm, "right1_duty", s.pwm.right1);
  cJSON_AddNumberToObject(pwm, "right2_duty", s.pwm.right2);
  cJSON_AddNumberToObject(pwm, "left_active_hz", s.pwm.leftHz);
  cJSON_AddNumberToObject(pwm, "right_active_hz", s.pwm.rightHz);
  cJSON_AddNumberToObject(pwm, "arm_left_pulse_angle", s.pwm.armLeftPulseAngle);
  cJSON_AddNumberToObject(pwm, "arm_right_pulse_angle", s.pwm.armRightPulseAngle);
  return cJSON_PrintUnformatted(root.get());
}
void WebService::mode() {
  auto root = parse(server_);
  if (!fields(root.get(), {"mode"}) || !cJSON_IsString(item(root.get(), "mode"))) { error(400, "invalid_mode"); return; }
  String name = item(root.get(), "mode")->valuestring;
  if (name != "manual" && name != "auto") { error(400, "invalid_mode"); return; }
  if (!control_.setMode(name == "manual" ? ControlMode::Manual : ControlMode::Auto)) { error(409, "mode_unavailable"); return; }
  server_.send(200, "application/json", "{\"ok\":true}");
}
void WebService::acquire() {
  auto root = parse(server_);
  if (!fields(root.get(), {"owner"}) || !cJSON_IsString(item(root.get(), "owner"))) { error(400, "invalid_owner"); return; }
  String name = item(root.get(), "owner")->valuestring;
  Owner owner = name == "manual" ? Owner::Manual : name == "program" ? Owner::Program : Owner::None;
  if (owner == Owner::None) { error(400, "invalid_owner"); return; }
  uint32_t session;
  if (!control_.acquire(owner, session)) { error(409, "mode_or_control_unavailable"); return; }
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
  if (!ControlLogic::valid(c) || !control_.snapshot().logic.armPolicy.allows(c.armLeft,c.armRight)) { error(400, "out_of_range"); return; }
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
    if (webOta_ || ota_.busy || control_.snapshot().logic.networkBusy) { error(409, "ota_or_network_busy"); return; }
    ota_.setEnabled(cJSON_IsTrue(item(root.get(), "arduino_ota")));
  } else if (fields(root.get(), {"ssid", "password"}) && cJSON_IsString(item(root.get(), "ssid")) &&
             cJSON_IsString(item(root.get(), "password"))) {
    if (webOta_ || ota_.busy || ota_.enabled) { error(409, "ota_busy"); return; }
    if(server_.arg("plain").indexOf("\\u0000")>=0){error(400,"invalid_wifi_config");return;}
    int code=network_.save(item(root.get(),"ssid")->valuestring,item(root.get(),"password")->valuestring);
    if(code==200)code=network_.connectSaved(item(root.get(),"ssid")->valuestring);
    if(code>=400){error(code,code==409?"release_control_or_wait_for_network":"invalid_wifi_config_or_storage");return;}
  } else { error(400, "invalid_settings"); return; }
  server_.send(200, "application/json", "{\"ok\":true}");
}
void WebService::upload() {
  HTTPUpload& u = server_.upload();
  if (u.status == UPLOAD_FILE_START) {
    // A single multipart request must contain exactly one application image.
    if (webOta_) { uploadFailed_ = true; return; }
    uploadOk_ = false; uploadFailed_ = false; uploadComplete_ = false;
    if (webOta_ || ota_.busy || ota_.enabled || control_.snapshot().logic.networkBusy || u.name != "firmware") { uploadFailed_ = true; return; }
    webOta_ = true; uploadAt_ = millis();
    if (!control_.beginMaintenance() || !Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) uploadFailed_ = true;
  } else if (u.status == UPLOAD_FILE_WRITE) {
    if (!webOta_ || uploadFailed_) return;
    uploadAt_ = millis();
    if (Update.write(u.buf, u.currentSize) != u.currentSize) uploadFailed_ = true;
  } else if (u.status == UPLOAD_FILE_END || u.status == UPLOAD_FILE_ABORTED) {
    if (!webOta_) return;
    uploadComplete_ = u.status == UPLOAD_FILE_END && !uploadFailed_;
    if (u.status == UPLOAD_FILE_ABORTED) {
      Update.abort(); webOta_ = false; uploadFailed_ = true; control_.endMaintenance();
    }
  }
}
void WebService::finishUpload() {
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

void WebService::eyes() {
  auto root = parse(server_);
  EyeSettings s=eyes_.snapshot().settings; float brightness;
  const bool oriented=fields(root.get(), {"enabled","automatic","blink","swap","expression","brightness","tempo","play_mode","category","energy","rotate_left","rotate_right"});
  const bool modern=oriented || fields(root.get(), {"enabled","automatic","blink","swap","expression","brightness","tempo","play_mode","category","energy"});
  if ((!modern && !fields(root.get(), {"enabled","automatic","blink","swap","expression","brightness","tempo"})) ||
      !cJSON_IsBool(item(root.get(),"enabled")) || !cJSON_IsBool(item(root.get(),"automatic")) ||
      !cJSON_IsBool(item(root.get(),"blink")) || !cJSON_IsBool(item(root.get(),"swap")) ||
      !cJSON_IsString(item(root.get(),"expression")) ||
      !parseExpression(item(root.get(),"expression")->valuestring,s.expression) ||
      !number(root.get(),"brightness",brightness) || brightness<1 || brightness>255 ||
      std::floor(brightness)!=brightness || !number(root.get(),"tempo",s.tempo)) {
    error(400,"invalid_eye_settings"); return;
  }
  if (control_.snapshot().logic.maintenance) { error(409,"maintenance"); return; }
  s.enabled=cJSON_IsTrue(item(root.get(),"enabled"));
  s.automatic=cJSON_IsTrue(item(root.get(),"automatic"));
  s.blink=cJSON_IsTrue(item(root.get(),"blink")); s.swap=cJSON_IsTrue(item(root.get(),"swap"));
  s.brightness=uint8_t(brightness);
  if(modern) {
    uint32_t energy;
    if(!cJSON_IsString(item(root.get(),"play_mode")) || !cJSON_IsString(item(root.get(),"category")) ||
       !parseEyePlay(item(root.get(),"play_mode")->valuestring,s.playMode) ||
       !parseEyeCategory(item(root.get(),"category")->valuestring,s.category) ||
       !integer(root.get(),"energy",energy) || energy>3 || s.automatic==(s.playMode==EyePlayMode::Manual)) {
      error(400,"invalid_eye_settings"); return;
    }
    s.energy=uint8_t(energy);
  } else if(!s.automatic) s.playMode=EyePlayMode::Manual;
  else if(s.playMode==EyePlayMode::Manual) s.playMode=EyePlayMode::Reactive;

  if(oriented) {
    if(!cJSON_IsBool(item(root.get(),"rotate_left")) || !cJSON_IsBool(item(root.get(),"rotate_right"))) {
      error(400,"invalid_eye_settings"); return;
    }
    s.rotateLeft=cJSON_IsTrue(item(root.get(),"rotate_left"));
    s.rotateRight=cJSON_IsTrue(item(root.get(),"rotate_right"));
  }
  if (!eyes_.configure(s)) { error(400,"invalid_eye_settings"); return; }
  server_.send(200,"application/json","{\"ok\":true}");
}

void WebService::eyeCheck() {
  auto root=parse(server_);
  if(!fields(root.get(),{"mode","target"}) || !cJSON_IsString(item(root.get(),"mode")) ||
      !cJSON_IsString(item(root.get(),"target"))) { error(400,"invalid_eye_check"); return; }
  const String mode=item(root.get(),"mode")->valuestring;
  const String target=item(root.get(),"target")->valuestring;
  unsigned m=0; for(;m<6;++m) if(mode==eyeCheckName(static_cast<EyeCheckMode>(m))) break;
  if(m==6 || (target!="both" && target!="left" && target!="right")) { error(400,"invalid_eye_check"); return; }
  if(control_.snapshot().logic.maintenance) { error(409,"maintenance"); return; }
  const auto t=target=="both" ? EyeTarget::Both : target=="left" ? EyeTarget::Left : EyeTarget::Right;
  if(!eyes_.snapshot().taskReady) { error(503,"eye_bus_unavailable"); return; }
  eyes_.check(static_cast<EyeCheckMode>(m),t);
  server_.send(202,"application/json","{\"ok\":true,\"queued\":true}");
}

void WebService::eyeCatalog() {
  Json root(cJSON_CreateObject(),cJSON_Delete);
  auto categories=cJSON_AddArrayToObject(root.get(),"categories");
  for(unsigned i=0;i<kEyeCategoryCount;++i) {
    auto c=cJSON_CreateObject(); cJSON_AddItemToArray(categories,c);
    cJSON_AddStringToObject(c,"id",eyeCategoryId(static_cast<EyeCategory>(i)));
    cJSON_AddStringToObject(c,"label",eyeCategoryLabel(static_cast<EyeCategory>(i)));
  }
  static const char* shapes[]={"solid","happy","heart","focus","sad","star","dollar","spiral","cross","ring","googly","sharingan","rinnegan","cat","fox","snake","toad"};
  auto presets=cJSON_AddArrayToObject(root.get(),"presets");
  for(const auto& entry:kEyeCatalog) {
    auto e=cJSON_CreateObject(); cJSON_AddItemToArray(presets,e);
    cJSON_AddStringToObject(e,"id",entry.id); cJSON_AddStringToObject(e,"label",entry.label);
    cJSON_AddStringToObject(e,"category",eyeCategoryId(entry.category));
    cJSON_AddStringToObject(e,"shape",shapes[static_cast<unsigned>(entry.shape)]);
    const auto index=static_cast<Expression>(&entry-kEyeCatalog);
    cJSON_AddStringToObject(e,"family",isBoxEye(index) ? "box" : "special");
    cJSON_AddNumberToObject(e,"width",entry.width); cJSON_AddNumberToObject(e,"height",entry.height);
  }
  char* json=cJSON_PrintUnformatted(root.get());
  if(!json) { error(503,"allocation_failed"); return; }
  server_.send(200,"application/json",json); cJSON_free(json);
}

void WebService::armSettings() {
  auto root=parse(server_); ArmMotionSettings settings;
  if(!fields(root.get(),{"speed_dps","acceleration_dps2"}) ||
     !number(root.get(),"speed_dps",settings.speed) || !number(root.get(),"acceleration_dps2",settings.acceleration) ||
     !ArmMotionSettings::valid(settings)) { error(400,"invalid_arm_settings");return; }
  if(!control_.configureArms(settings)) { error(409,"release_control_before_arm_settings");return; }
  server_.send(200,"application/json","{\"ok\":true}");
}
