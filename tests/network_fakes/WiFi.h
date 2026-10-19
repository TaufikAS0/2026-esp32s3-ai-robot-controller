#pragma once
#include "Arduino.h"
#include <functional>
#include <vector>
constexpr int WIFI_AP_STA=3,WL_CONNECTED=3,WIFI_SCAN_RUNNING=-1,WIFI_AUTH_OPEN=0;
enum WiFiEvent_t { ARDUINO_EVENT_WIFI_STA_GOT_IP,ARDUINO_EVENT_WIFI_STA_DISCONNECTED };
struct WiFiEventInfo_t { struct { unsigned reason=8; } wifi_sta_disconnected; };
struct IP { String toString(){return String("192.168.4.1");} };
struct FakeWifi {
  std::function<void(WiFiEvent_t,WiFiEventInfo_t)> callback;
  int statusValue=0,scanResult=-1;
  std::string current;
  std::vector<std::string> attempts;
  std::vector<std::string> visible;
  void onEvent(std::function<void(WiFiEvent_t,WiFiEventInfo_t)> fn){callback=fn;}
  void mode(int){}
  void persistent(bool){}
  void setAutoReconnect(bool){}
  void setSleep(bool){}
  void setHostname(const char*){}
  bool softAP(const char*,const char*){return true;}
  void begin(const char* ssid,const char*){attempts.push_back(ssid);current=ssid;statusValue=0;}
  void disconnect(){statusValue=0;if(callback)callback(ARDUINO_EVENT_WIFI_STA_DISCONNECTED,{});}
  void gotIp(){statusValue=WL_CONNECTED;if(callback)callback(ARDUINO_EVENT_WIFI_STA_GOT_IP,{});}
  int status(){return statusValue;}
  String SSID(){return String(current.c_str());}
  String SSID(int index){return String(visible.at(index).c_str());}
  int RSSI(int){return -55;}
  int encryptionType(int){return 1;}
  IP localIP(){return {};}
  IP softAPIP(){return {};}
  void scanDelete(){}
  int scanNetworks(bool,bool,bool,int){scanResult=-1;return -1;}
  int scanComplete(){return scanResult;}
};
inline FakeWifi WiFi;
