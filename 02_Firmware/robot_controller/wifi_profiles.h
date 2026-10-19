#pragma once
#include "api_json.h"
#include "tcp_framer.h"

struct WifiProfile { char ssid[33]{};char password[64]{}; };
struct WifiProfiles {
  static constexpr unsigned capacity=5;
  WifiProfile entries[capacity]{};
  unsigned count=0;
  static bool validSsid(const char* value) {
    const size_t n=strlen(value);if(!n || n>32)return false;
    if(!TcpFramer::validUtf8(value,n))return false;
    for(size_t i=0;i<n;++i)if(static_cast<unsigned char>(value[i])<32 || value[i]==127)return false;
    return true;
  }
  static bool validPassword(const char* value) {
    const size_t n=strlen(value);if(n && (n<8 || n>63))return false;
    for(size_t i=0;i<n;++i)if(value[i]<32 || value[i]>126)return false;
    return true;
  }
  int find(const char* ssid) const { for(unsigned i=0;i<count;++i)if(!strcmp(ssid,entries[i].ssid))return int(i);return -1; }
  bool save(const char* ssid,const char* password) {
    if(!validSsid(ssid)||!validPassword(password))return false;
    int at=find(ssid);if(at<0){if(count==capacity)return false;at=int(count++);}
    strcpy(entries[at].ssid,ssid);strcpy(entries[at].password,password);return true;
  }
  bool remove(const char* ssid) {
    const int at=find(ssid);if(at<0)return false;
    for(unsigned i=unsigned(at);i+1<count;++i)entries[i]=entries[i+1];
    entries[--count]=WifiProfile{};return true;
  }
  bool reorder(cJSON* array) {
    if(!cJSON_IsArray(array) || cJSON_GetArraySize(array)!=int(count))return false;
    WifiProfiles next;unsigned mask=0;
    for(unsigned i=0;i<count;++i) {
      auto name=cJSON_GetArrayItem(array,i);if(!cJSON_IsString(name))return false;
      const int at=find(name->valuestring);if(at<0 || (mask&(1u<<at)))return false;
      mask|=1u<<at;next.entries[next.count++]=entries[at];
    }
    *this=next;return true;
  }
  ApiJson::Json json(bool secrets=false) const {
    ApiJson::Json array(cJSON_CreateArray(),cJSON_Delete);
    for(unsigned i=0;i<count;++i) {
      auto p=cJSON_CreateObject();cJSON_AddItemToArray(array.get(),p);
      cJSON_AddStringToObject(p,"ssid",entries[i].ssid);
      if(secrets)cJSON_AddStringToObject(p,"password",entries[i].password);
      else {cJSON_AddNumberToObject(p,"priority",i+1);cJSON_AddBoolToObject(p,"secured",entries[i].password[0]!=0);}
    }return array;
  }
  static bool parse(const char* text,WifiProfiles& out) {
    // NVS schema is a bounded JSON array, written atomically under one key.
    if(strlen(text)>2048 || strstr(text,"\\u0000"))return false;
    ApiJson::Json root(cJSON_ParseWithOpts(text,nullptr,true),cJSON_Delete);
    if(!cJSON_IsArray(root.get()) || cJSON_GetArraySize(root.get())>int(capacity))return false;
    WifiProfiles next;
    for(auto p=root->child;p;p=p->next) {
      if(!ApiJson::fields(p,{"ssid","password"}) || !cJSON_IsString(ApiJson::item(p,"ssid")) || !cJSON_IsString(ApiJson::item(p,"password")))return false;
      const char* ssid=ApiJson::item(p,"ssid")->valuestring;
      if(next.find(ssid)>=0 || !next.save(ssid,ApiJson::item(p,"password")->valuestring))return false;
    }out=next;return true;
  }
};

// Pure fallback timing: no Wi-Fi/NVS/actuator operations. One attempt per profile.
class WifiCycle {
 public:
  static constexpr uint32_t attemptMs=15000,retryMs=30000;
  unsigned next=0,count=0,first=0;
  uint32_t started=0;
  void reset(unsigned profiles,uint32_t now,unsigned preferred=0) { count=profiles;next=0;first=preferred<count?preferred:0;started=now; }
  int take(uint32_t now) {
    if(next>=count)return -1;
    started=now;const unsigned position=next++;
    return int(position==0?first:position<=first?position-1:position);
  }
  bool timedOut(uint32_t now) const { return uint32_t(now-started)>=attemptMs; }
};
