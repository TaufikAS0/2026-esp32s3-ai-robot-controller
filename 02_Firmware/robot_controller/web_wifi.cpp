#include "web_service.h"
#include "api_json.h"
using namespace ApiJson;
void WebService::wifiSettings() {
  char* text=network_.settingsJson();if(!text){error(503,"allocation_failed");return;}
  server_.sendHeader("Cache-Control","no-store");server_.send(200,"application/json",text);cJSON_free(text);
}
void WebService::wifiAction() {
  if(webOta_ || ota_.busy || ota_.enabled || control_.snapshot().logic.maintenance){error(409,"ota_or_control_busy");return;}
  const String body=server_.arg("plain");
  if(body.indexOf("\\u0000")>=0 || memchr(body.c_str(),0,body.length())){error(400,"invalid_wifi_request");return;}
  if(!server_.header("Content-Type").startsWith("application/json") || !body.length() || body.length()>1024){error(400,"invalid_wifi_request");return;}
  Json root(cJSON_ParseWithOpts(body.c_str(),nullptr,true),cJSON_Delete);
  const String path=server_.uri();int code=400;
  if(path=="/api/v1/wifi/scan" && fields(root.get(),{}))code=network_.startScan();
  else if(path=="/api/v1/wifi/save" && fields(root.get(),{"ssid","password"}) && cJSON_IsString(item(root.get(),"ssid")) && cJSON_IsString(item(root.get(),"password")))
    code=network_.save(item(root.get(),"ssid")->valuestring,item(root.get(),"password")->valuestring);
  else if(path=="/api/v1/wifi/order" && fields(root.get(),{"ssids"}))code=network_.reorder(item(root.get(),"ssids"));
  else if(fields(root.get(),{"ssid"}) && cJSON_IsString(item(root.get(),"ssid"))) {
    if(path=="/api/v1/wifi/delete")code=network_.remove(item(root.get(),"ssid")->valuestring);
    else if(path=="/api/v1/wifi/connect")code=network_.connectSaved(item(root.get(),"ssid")->valuestring);
  }
  if(code>=400){error(code,code==409?"release_control_or_wait_for_network":code==503?"wifi_storage_or_scan_failed":"invalid_wifi_request");return;}
  server_.send(200+(code==202?2:0),"application/json",code==202?"{\"ok\":true,\"queued\":true}":"{\"ok\":true}");
}
