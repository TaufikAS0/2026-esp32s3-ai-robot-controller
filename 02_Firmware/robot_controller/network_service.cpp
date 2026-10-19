#include "network_service.h"
#include "network_defaults.h"
#include <esp_system.h>
bool NetworkService::reserve(bool lost) {
  if(guard_ || scanning_ || phase_==Phase::Pending || phase_==Phase::Connecting)return false;
  if(!control_->beginNetworkChange(lost))return false;
  guard_=true;return true;
}
void NetworkService::releaseGuard() { if(guard_) { control_->endNetworkChange();guard_=false; } }
bool NetworkService::persist(const WifiProfiles& next) {
  if(!storageReady_)return false;
  auto json=next.json(true);char* text=cJSON_PrintUnformatted(json.get());if(!text)return false;
  const bool ok=preferences_.putString("wifiProfilesV1",text)==strlen(text);
  cJSON_free(text);if(ok)profiles_=next;return ok;
}
void NetworkService::begin(ControlService& control) {
  control_=&control;
  WiFi.onEvent([this](WiFiEvent_t event,WiFiEventInfo_t info) {
    if(event==ARDUINO_EVENT_WIFI_STA_GOT_IP) { connectCount_.fetch_add(1);wasConnected_=true; }
    if(event==ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
      lastDisconnectReason_=info.wifi_sta_disconnected.reason;disconnectCount_.fetch_add(1);
      if(wasConnected_.exchange(false))linkLost_=true;
    }
  });
  WiFi.persistent(false); // Profile rotation must not rewrite the SDK's flash config each attempt.
  WiFi.mode(WIFI_AP_STA);WiFi.setAutoReconnect(false);WiFi.setSleep(false);
  storageReady_=preferences_.begin("robot",false);
  hostname="ai-robot-"+String(uint32_t(ESP.getEfuseMac()&0xffffff),HEX);
  apName=hostname+"-setup";apPassword=NetworkDefaults::apPassword;
  if(storageReady_) {
    preferences_.remove("apiToken");preferences_.remove("otaPassword");
    if(preferences_.getString("apPassword","")!=apPassword)preferences_.putString("apPassword",apPassword);
  }
  if(storageReady_ && preferences_.isKey("wifiProfilesV1")) {
    const String stored=preferences_.getString("wifiProfilesV1","");
    if(!WifiProfiles::parse(stored.c_str(),profiles_)) { storageReady_=false;Serial.println("WiFi profiles invalid; recovery AP only, NVS preserved"); }
  } else {
    WifiProfiles initial;
    const String ssid=preferences_.getString("ssid",NetworkDefaults::stationSsid);
    const String password=preferences_.getString("wifiPassword",NetworkDefaults::stationPassword);
    if(!initial.save(ssid.c_str(),password.c_str()))initial.save(NetworkDefaults::stationSsid,NetworkDefaults::stationPassword);
    if(!persist(initial))profiles_=initial;
  }
  WiFi.setHostname(hostname.c_str());
  apActive=WiFi.softAP(apName.c_str(),apPassword.c_str());
  Serial.println("Access: open LAN; WiFi profiles saved locally; passwords never returned");
  Serial.printf("Reset reason: %d; profiles: %u; recovery AP: %s\n",int(esp_reset_reason()),profiles_.count,apName.c_str());
  phaseAt_=millis()-WifiCycle::retryMs;
}
String NetworkService::targetSsid() {
  char copy[33];portENTER_CRITICAL(&targetMux_);memcpy(copy,target_,sizeof(copy));portEXIT_CRITICAL(&targetMux_);return String(copy);
}
const char* NetworkService::phaseName() const {
  switch(phase_.load()) {case Phase::Connected:return "connected";case Phase::Connecting:return "connecting";case Phase::Pending:return "pending";default:return "waiting";}
}
int NetworkService::save(const char* ssid,const char* password) {
  WifiProfiles next=profiles_;if(!next.save(ssid,password))return 400;
  if(!reserve())return 409;
  const bool ok=persist(next);releaseGuard();return ok?200:503;
}
int NetworkService::remove(const char* ssid) {
  WifiProfiles next=profiles_;if(!next.remove(ssid))return 400;
  if(!reserve())return 409;
  const bool wasActive=WiFi.status()==WL_CONNECTED && WiFi.SSID()==ssid;
  const bool ok=persist(next);
  if(ok && wasActive) { pending_=0;phase_=Phase::Pending;phaseAt_=millis();return 202; }
  releaseGuard();return ok?200:503;
}
int NetworkService::reorder(cJSON* ssids) {
  WifiProfiles next=profiles_;if(!next.reorder(ssids))return 400;
  if(!reserve())return 409;
  const bool ok=persist(next);releaseGuard();return ok?200:503;
}
int NetworkService::connectSaved(const char* ssid) {
  const int index=profiles_.find(ssid);if(index<0)return 400;
  if(!reserve())return 409;
  pending_=index;phase_=Phase::Pending;phaseAt_=millis();return 202;
}
bool NetworkService::configure(const String& ssid,const String& password) {
  if(save(ssid.c_str(),password.c_str())!=200)return false;
  return connectSaved(ssid.c_str())==202;
}
void NetworkService::attempt(int index) {
  portENTER_CRITICAL(&targetMux_);strcpy(target_,profiles_.entries[index].ssid);portEXIT_CRITICAL(&targetMux_);
  WiFi.begin(profiles_.entries[index].ssid,profiles_.entries[index].password);
  phase_=Phase::Connecting;Serial.printf("WiFi attempt priority=%d\n",index+1);
}
void NetworkService::nextAttempt() {
  const int index=cycle_.take(millis());
  if(index>=0)attempt(index);
  else { phase_=Phase::Waiting;phaseAt_=millis();releaseGuard(); }
}
int NetworkService::startScan() {
  if(!reserve())return 409;
  WiFi.scanDelete();scanCount_=0;scanState_="running";scanAt_=millis();
  const int result=WiFi.scanNetworks(true,true,false,120);scanning_=true;
  if(result>=0)finishScan();
  else if(result!=WIFI_SCAN_RUNNING) { scanning_=false;scanState_="failed";releaseGuard();return 503; }
  return 202;
}
void NetworkService::finishScan() {
  const int count=WiFi.scanComplete();
  if(count==WIFI_SCAN_RUNNING && millis()-scanAt_<15000)return;
  scanState_=count>=0?"complete":"failed";
  for(int i=0;i<count;++i) {
    const String name=WiFi.SSID(i);if(!WifiProfiles::validSsid(name.c_str()))continue;
    unsigned at=0;for(;at<scanCount_;++at)if(name==scan_[at].ssid)break;
    if(at==scanCount_) { if(scanCount_==24)continue;strcpy(scan_[at].ssid,name.c_str());scan_[at].rssi=-200;++scanCount_; }
    if(WiFi.RSSI(i)>scan_[at].rssi) { scan_[at].rssi=WiFi.RSSI(i);scan_[at].secured=WiFi.encryptionType(i)!=WIFI_AUTH_OPEN; }
  }
  WiFi.scanDelete();scanning_=false;releaseGuard();
}
char* NetworkService::settingsJson() {
  ApiJson::Json root(cJSON_CreateObject(),cJSON_Delete);
  cJSON_AddBoolToObject(root.get(),"storage_ready",storageReady_);
  cJSON_AddStringToObject(root.get(),"state",phaseName());
  cJSON_AddBoolToObject(root.get(),"connected",WiFi.status()==WL_CONNECTED);
  cJSON_AddStringToObject(root.get(),"current_ssid",WiFi.SSID().c_str());
  cJSON_AddStringToObject(root.get(),"target_ssid",targetSsid().c_str());
  cJSON_AddStringToObject(root.get(),"sta_ip",WiFi.localIP().toString().c_str());
  cJSON_AddStringToObject(root.get(),"ap_ssid",apName.c_str());
  cJSON_AddStringToObject(root.get(),"ap_ip",WiFi.softAPIP().toString().c_str());
  cJSON_AddBoolToObject(root.get(),"ap_active",apActive);
  auto profiles=profiles_.json();cJSON_AddItemToObject(root.get(),"profiles",profiles.release());
  cJSON_AddStringToObject(root.get(),"scan_state",scanState_);
  auto list=cJSON_AddArrayToObject(root.get(),"networks");
  for(unsigned i=0;i<scanCount_;++i) {
    auto e=cJSON_CreateObject();cJSON_AddItemToArray(list,e);
    cJSON_AddStringToObject(e,"ssid",scan_[i].ssid);cJSON_AddNumberToObject(e,"rssi",scan_[i].rssi);cJSON_AddBoolToObject(e,"secured",scan_[i].secured);
  }return cJSON_PrintUnformatted(root.get());
}
void NetworkService::update(bool otaEnabled) {
  if(!otaEnabled && !control_->snapshot().logic.maintenance) {
    if(linkLost_.load() && !guard_ && !scanning_) {
      if(reserve(true)) { linkLost_=false;cycle_.reset(profiles_.count,millis());nextAttempt(); }
    }
    if(scanning_)finishScan();
    if(phase_==Phase::Pending && millis()-phaseAt_>=400) {
      wasConnected_=false;linkLost_=false;WiFi.disconnect();
      cycle_.reset(profiles_.count,millis(),unsigned(pending_));nextAttempt();
    } else if(phase_==Phase::Connecting) {
      if(WiFi.status()==WL_CONNECTED) { phase_=Phase::Connected;phaseAt_=millis();releaseGuard(); }
      else if(cycle_.timedOut(millis())) { wasConnected_=false;WiFi.disconnect();nextAttempt(); }
    } else if(phase_==Phase::Waiting && !scanning_ && profiles_.count && millis()-phaseAt_>=WifiCycle::retryMs) {
      if(reserve()) { cycle_.reset(profiles_.count,millis());nextAttempt(); }
    }
  }
  if(disconnectCount_.load()!=reportedDisconnectCount_) {
    reportedDisconnectCount_=disconnectCount_.load();
    Serial.printf("WiFi disconnected: uptime=%u reason=%u count=%u\n",millis(),lastDisconnectReason_.load(),reportedDisconnectCount_);
  }
  for(unsigned i=0;i<64 && Serial.available();++i) {
    const char c=Serial.read();if(c=='\r')continue;
    if(c=='\n') {
      if(serialLine_=="info")Serial.printf("WiFi state=%s IP=%s AP=%s profiles=%u\n",phaseName(),WiFi.localIP().toString().c_str(),WiFi.softAPIP().toString().c_str(),profiles_.count);
      else if(serialLine_.startsWith("wifi ") && !otaEnabled) {
        const int split=serialLine_.indexOf('|',5);
        const bool ok=split>=0 && configure(serialLine_.substring(5,split),serialLine_.substring(split+1));
        Serial.println(ok?"WiFi profile saved; connection queued":"WiFi profile rejected or control busy");
      }
      serialLine_="";
    } else if(serialLine_.length()<128)serialLine_+=c;else serialLine_="";
  }
}
