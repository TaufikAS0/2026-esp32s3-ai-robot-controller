#include <cassert>
#include "../02_Firmware/robot_controller/network_service.h"

// NetworkService.cpp is real; fake the GPIO-facing service and the board SDK.
static ControlLogic logic;
bool ControlService::beginNetworkChange(bool lost){return logic.beginNetworkChange(lost);}
void ControlService::endNetworkChange(){logic.endNetworkChange();}
ControlSnapshot ControlService::snapshot(){return {logic,0,{}};}
static void reset(){fakeNow=0;logic=ControlLogic{};Preferences::data.clear();Preferences::fail=false;WiFi=FakeWifi{};}
static bool contains(const char* text,const char* token){return strstr(text,token)!=nullptr;}
static void finish(NetworkService& n,std::vector<std::string> names,std::vector<int> signals={}) {
  WiFi.visible=names;WiFi.signals=signals;WiFi.scanResult=int(names.size());n.update();
}
int main(){
  reset();Preferences::data["unrelated"]="keep";
  {
    ControlService control;NetworkService n;n.begin(control);
    assert(Preferences::data.count("wifiProfilesV1") && Preferences::data["unrelated"]=="keep");
    n.update();assert(WiFi.attempts.empty() && logic.networkBusy && !strcmp(n.phaseName(),"scanning"));
    finish(n,{"HuaweiJIN"},{-70});assert(WiFi.attempts.back()=="HuaweiJIN");
    WiFi.gotIp();n.update();assert(!logic.networkBusy && n.apActive);
    assert(n.save("backup","password123")==200 && n.save("strong","password123")==200);
    assert(WiFi.attempts.size()==1); // Saving never roams a healthy link.
    char* json=n.settingsJson();assert(contains(json,"backup") && contains(json,"strongest_saved") && contains(json,"-58") && !contains(json,"password123"));cJSON_free(json);
    assert(logic.acquire(Owner::Manual,1,fakeNow));
    assert(n.startScan()==409 && n.connectSaved("backup")==409 && n.remove("backup")==409 && n.save("blocked","password123")==409);
    logic.stop("test");
    assert(n.startScan()==202);assert(!logic.acquire(Owner::Manual,2,fakeNow));
    finish(n,{"backup","backup","other"},{-80,-45,-40});assert(!logic.networkBusy);
    json=n.settingsJson();assert(contains(json,"complete") && contains(json,"-45"));cJSON_free(json);
    assert(n.connectSaved("backup")==202);fakeNow+=399;n.update();assert(WiFi.attempts.size()==1);
    ++fakeNow;n.update();assert(!strcmp(n.phaseName(),"scanning"));
    finish(n,{"strong","HuaweiJIN"},{-35,-70}); // Manual hidden/absent selection gets first try.
    assert(WiFi.attempts.back()=="backup");fakeNow+=15000;n.update();assert(WiFi.attempts.back()=="strong");
    fakeNow+=15000;n.update();assert(WiFi.attempts.back()=="HuaweiJIN");WiFi.gotIp();n.update();
    const auto count=WiFi.attempts.size();const auto scans=WiFi.scanCalls;fakeNow+=60000;n.update();
    assert(WiFi.attempts.size()==count && WiFi.scanCalls==scans); // no healthy roaming/scans
    assert(logic.acquire(Owner::Manual,3,fakeNow));Command cmd{3,1,.5f,.5f,70,70,true};assert(logic.command(cmd,fakeNow));logic.tick(fakeNow);
    WiFi.disconnect();n.update();assert(logic.networkBusy && logic.owner==Owner::None && !logic.output.laser && logic.output.left==0);
    assert(!logic.command(cmd,fakeNow));
    finish(n,{"HuaweiJIN","strong","backup","unknown-open"},{-80,-40,-60,-20});
    assert(WiFi.attempts.back()=="strong");fakeNow+=14999;n.update();assert(WiFi.attempts.back()=="strong");
    ++fakeNow;n.update();assert(WiFi.attempts.back()=="backup");fakeNow+=15000;n.update();assert(WiFi.attempts.back()=="HuaweiJIN");
    fakeNow+=15000;n.update();assert(!logic.networkBusy && !strcmp(n.phaseName(),"waiting"));
    auto attempts=WiFi.attempts.size();fakeNow+=29999;n.update();assert(WiFi.attempts.size()==attempts);
    ++fakeNow;n.update(true);assert(WiFi.scanCalls==scans+1);n.update();assert(!strcmp(n.phaseName(),"scanning"));
    finish(n,{});assert(WiFi.attempts.size()==attempts && !logic.networkBusy);
    json=n.settingsJson();assert(contains(json,"\"rssi\":null") && contains(json,"\"current_ssid\":\"\"") && contains(json,"120000"));cJSON_free(json);
    const auto emptyScans=WiFi.scanCalls;fakeNow+=119999;n.update();assert(WiFi.scanCalls==emptyScans);
    ++fakeNow;n.update();assert(WiFi.scanCalls==emptyScans+1);
  }
  // Persisted profiles survive boot; strongest is selected, unrelated NVS stays.
  logic=ControlLogic{};WiFi=FakeWifi{};
  {ControlService c;NetworkService n;n.begin(c);n.update();finish(n,{"backup","HuaweiJIN"},{-40,-50});assert(WiFi.attempts.back()=="backup");}
  reset();Preferences::data["wifiProfilesV1"]="[]";
  {ControlService c;NetworkService n;n.begin(c);n.update();assert(WiFi.attempts.empty() && !WiFi.scanCalls && n.apActive);}
  reset();Preferences::data["wifiProfilesV1"]="corrupt";
  {ControlService c;NetworkService n;n.begin(c);n.update();assert(WiFi.attempts.empty() && Preferences::data["wifiProfilesV1"]=="corrupt");}
  reset();{ControlService c;NetworkService n;n.begin(c);Preferences::fail=true;assert(n.save("badwrite","password123")==503 && n.connectSaved("badwrite")==400);}
  // Even a weak saved network beyond the 24 public scan rows remains eligible.
  reset();{ControlService c;NetworkService n;n.begin(c);n.update();std::vector<std::string> names;
    for(int i=0;i<30;++i)names.push_back("unknown"+std::to_string(i));
    names.push_back("HuaweiJIN");finish(n,names);assert(WiFi.attempts.back()=="HuaweiJIN");}
  // Scan failure never uses stale results or blindly reconnects. Manual override still works.
  reset();{ControlService c;NetworkService n;n.begin(c);n.update();WiFi.scanResult=-2;n.update();assert(WiFi.attempts.empty() && !logic.networkBusy);
    assert(n.connectSaved("HuaweiJIN")==202);fakeNow+=400;n.update();fakeNow+=15000;n.update();assert(WiFi.attempts.back()=="HuaweiJIN");
    WiFi.gotIp();n.update();assert(n.startScan()==202);fakeNow+=15000;n.update();assert(!logic.networkBusy);
    assert(n.remove("HuaweiJIN")==202);fakeNow+=400;n.update();assert(!logic.networkBusy && !strcmp(n.phaseName(),"waiting"));
    assert(Preferences::data["wifiProfilesV1"]=="[]");}
  // Full candidate array, duplicate BSSIDs use maximum RSSI, equal signals retain
  // saved order, and reorder/delete cannot attach a cached signal to another SSID.
  reset();{ControlService c;NetworkService n;n.begin(c);
    for(const char* ssid:{"second","third","fourth","fifth"})assert(n.save(ssid,"password123")==200);
    n.update();finish(n,{"HuaweiJIN","second","third","fourth","fifth","fifth"},{-70,-50,-50,-80,-90,-30});
    assert(WiFi.attempts.back()=="fifth");
    for(const char* expected:{"second","third","HuaweiJIN","fourth"}){fakeNow+=15000;n.update();assert(WiFi.attempts.back()==expected);}
    fakeNow+=15000;n.update();assert(!logic.networkBusy);
    assert(n.remove("second")==200);char* j=n.settingsJson();ApiJson::Json root(cJSON_Parse(j),cJSON_Delete);cJSON_free(j);
    auto profiles=cJSON_GetObjectItem(root.get(),"profiles");auto third=cJSON_GetArrayItem(profiles,1);
    assert(!strcmp(cJSON_GetObjectItem(third,"ssid")->valuestring,"third"));assert(cJSON_GetObjectItem(third,"rssi")->valueint==-50);
  }
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
  WifiCycle cycle;cycle.reset(3,UINT32_MAX-10,2);assert(cycle.take(UINT32_MAX-10)==2);assert(cycle.timedOut(14989));assert(cycle.take(15000)==0);assert(cycle.take(30000)==1);assert(cycle.take(45000)==-1);
}
