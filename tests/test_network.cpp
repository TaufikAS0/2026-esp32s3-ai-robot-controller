#include <cassert>
#include "../02_Firmware/robot_controller/network_service.h"

// NetworkService.cpp is real; fake the GPIO-facing service and the board SDK.
static ControlLogic logic;
bool ControlService::beginNetworkChange(bool lost){return logic.beginNetworkChange(lost);}
void ControlService::endNetworkChange(){logic.endNetworkChange();}
ControlSnapshot ControlService::snapshot(){return {logic,0,{}};}
static void reset(){fakeNow=0;logic=ControlLogic{};Preferences::data.clear();Preferences::fail=false;WiFi=FakeWifi{};}
static bool contains(const char* text,const char* token){return strstr(text,token)!=nullptr;}
int main(){
  reset();Preferences::data["unrelated"]="keep";
  {
    ControlService control;NetworkService n;n.begin(control);
    assert(Preferences::data.count("wifiProfilesV1"));assert(Preferences::data["unrelated"]=="keep");
    assert(n.apActive);n.update();assert(WiFi.attempts.size()==1 && WiFi.attempts[0]=="HuaweiJIN" && logic.networkBusy);
    WiFi.gotIp();n.update();assert(!logic.networkBusy);
    assert(n.save("backup","password123")==200);
    assert(WiFi.attempts.size()==1); // save never disconnects healthy link
    char* json=n.settingsJson();assert(contains(json,"backup") && !contains(json,"password123") && !contains(json,"jayaabadi100"));cJSON_free(json);
    assert(logic.acquire(Owner::Manual,1,fakeNow));
    assert(n.startScan()==409 && n.connectSaved("backup")==409 && n.remove("backup")==409);
    assert(n.save("blocked","password123")==409);
    assert(logic.session==1);logic.stop("test");
    assert(n.startScan()==202);assert(logic.networkBusy);
    assert(!logic.acquire(Owner::Manual,2,fakeNow));
    WiFi.visible={"backup","backup","other"};WiFi.scanResult=3;n.update();assert(!logic.networkBusy);
    json=n.settingsJson();assert(contains(json,"complete"));cJSON_free(json);
    assert(n.connectSaved("backup")==202);assert(logic.networkBusy);
    fakeNow+=399;n.update();assert(WiFi.attempts.size()==1);
    ++fakeNow;n.update();assert(WiFi.attempts.back()=="backup");
    fakeNow+=14999;n.update();assert(WiFi.attempts.size()==2);
    ++fakeNow;n.update();assert(WiFi.attempts.back()=="HuaweiJIN");
    WiFi.gotIp();n.update();assert(!logic.networkBusy);
    const auto count=WiFi.attempts.size();fakeNow+=60000;n.update();assert(WiFi.attempts.size()==count); // no roaming while healthy
    assert(logic.acquire(Owner::Manual,3,fakeNow));Command cmd{3,1,.5f,.5f,70,70,true};assert(logic.command(cmd,fakeNow));logic.tick(fakeNow);
    WiFi.disconnect();n.update();assert(logic.networkBusy && logic.owner==Owner::None && !logic.output.laser && logic.output.left==0);
    assert(!logic.command(cmd,fakeNow)); // old target never resumes
    fakeNow+=15000;n.update();assert(WiFi.attempts.back()=="backup");
    fakeNow+=15000;n.update();assert(!logic.networkBusy && !strcmp(n.phaseName(),"waiting"));
    auto attempts=WiFi.attempts.size();fakeNow+=29999;n.update();assert(WiFi.attempts.size()==attempts);
    ++fakeNow;n.update(true);assert(WiFi.attempts.size()==attempts); // OTA enabled suppresses automatic retries
    n.update();assert(WiFi.attempts.size()==attempts+1);
  }
  // Stored list survives reboot, legacy profile is NOT rewritten over it.
  logic=ControlLogic{};WiFi=FakeWifi{};
  {ControlService c;NetworkService n;n.begin(c);n.update();WiFi.gotIp();n.update();assert(n.connectSaved("backup")==202);}
  reset();Preferences::data["wifiProfilesV1"]="[]";
  {ControlService c;NetworkService n;n.begin(c);n.update();assert(WiFi.attempts.empty());assert(n.apActive);}
  reset();Preferences::data["wifiProfilesV1"]="corrupt";
  {ControlService c;NetworkService n;n.begin(c);n.update();assert(WiFi.attempts.empty());assert(Preferences::data["wifiProfilesV1"]=="corrupt");}
  // Failed NVS write must not update in-memory list.
  reset();{ControlService c;NetworkService n;n.begin(c);Preferences::fail=true;assert(n.save("badwrite","password123")==503);assert(n.connectSaved("badwrite")==400);}
  WifiProfiles p;assert(!p.save("","password123"));assert(!p.save("bad","short"));
  assert(!p.save("\xc0\x80","password123"));assert(p.save("open",""));
  for(int i=0;i<4;++i)assert(p.save(("ssid"+std::to_string(i)).c_str(),"password123"));
  assert(!p.save("sixth","password123"));
  auto before=p.json(true);WifiProfiles parsed;char* text=cJSON_PrintUnformatted(before.get());assert(WifiProfiles::parse(text,parsed));cJSON_free(text);
  assert(parsed.count==5);assert(!WifiProfiles::parse("[{\"ssid\":\"x\",\"password\":\"\"},{\"ssid\":\"x\",\"password\":\"\"}]",parsed));
  assert(parsed.count==5); // malformed parse is atomic
  ApiJson::Json order(cJSON_Parse("[\"ssid3\",\"ssid2\",\"ssid1\",\"ssid0\",\"open\"]"),cJSON_Delete);
  assert(p.reorder(order.get()));assert(!strcmp(p.entries[0].ssid,"ssid3"));
  assert(p.remove("ssid2"));assert(p.count==4);
  // JSON escaping of five max-size passwords still round-trips under NVS bound.
  WifiProfiles escaped;
  for(int i=0;i<5;++i)assert(escaped.save(("escaped"+std::to_string(i)).c_str(),std::string(63,'\\').c_str()));
  auto escapedJson=escaped.json(true);text=cJSON_PrintUnformatted(escapedJson.get());assert(WifiProfiles::parse(text,parsed));cJSON_free(text);assert(parsed.count==5);
  // Scan deadline releases reservation, and deleting the last active profile keeps AP-only state.
  reset();{ControlService c;NetworkService n;n.begin(c);n.update();WiFi.gotIp();n.update();
    assert(n.startScan()==202);fakeNow+=15000;n.update();assert(!logic.networkBusy);
    assert(n.remove("HuaweiJIN")==202);fakeNow+=400;n.update();assert(!logic.networkBusy && !strcmp(n.phaseName(),"waiting"));
    auto attempts=WiFi.attempts.size();fakeNow+=60000;n.update();assert(WiFi.attempts.size()==attempts);assert(Preferences::data["wifiProfilesV1"]=="[]");}
  WifiCycle cycle;cycle.reset(3,UINT32_MAX-10,2);assert(cycle.take(UINT32_MAX-10)==2);assert(cycle.timedOut(14989));assert(cycle.take(15000)==0);assert(cycle.take(30000)==1);assert(cycle.take(45000)==-1);
}
