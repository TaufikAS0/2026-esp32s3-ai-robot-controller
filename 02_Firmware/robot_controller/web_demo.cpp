#include "web_service.h"
#include "api_json.h"
using namespace ApiJson;
void WebService::demoStatus() {
  char* text=demo_.statusJson();if(!text){error(503,"allocation_failed");return;}
  server_.sendHeader("Cache-Control","no-store");server_.send(200,"application/json",text);cJSON_free(text);
}
void WebService::demoAction() {
  const String body=server_.arg("plain");
  if(!server_.header("Content-Type").startsWith("application/json")||!body.length()||body.length()>1024||!boundedDepth(body.c_str(),2)||memchr(body.c_str(),0,body.length())||body.indexOf("\\u0000")>=0){error(400,"invalid_demo_request");return;}
  Json root(cJSON_ParseWithOpts(body.c_str(),nullptr,true),cJSON_Delete);
  const String path=server_.uri();int code=400;
  if(path=="/api/v1/demo/stop") {uint32_t id=0;if(fields(root.get(),{"run_id"})&&integer(root.get(),"run_id",id))code=demo_.stop(id);}
  else {
    if(webOta_||ota_.busy||ota_.enabled||control_.snapshot().logic.maintenance){error(409,"ota_busy");return;}
    if(path=="/api/v1/demo/config")code=demo_.configure(root.get());
    else if(path=="/api/v1/demo/exhibition/config")code=demo_.configureExhibition(root.get());
    else if(path=="/api/v1/demo/exhibition/center"&&fields(root.get(),{"confirmed_center","confirmed_floor","confirmed_hands"})&&cJSON_IsTrue(item(root.get(),"confirmed_center"))&&cJSON_IsTrue(item(root.get(),"confirmed_floor"))&&cJSON_IsTrue(item(root.get(),"confirmed_hands")))code=demo_.centerExhibition();
    else if(path=="/api/v1/demo/exhibition/start"&&fields(root.get(),{"mode","accept_estimates"})&&cJSON_IsTrue(item(root.get(),"accept_estimates"))&&cJSON_IsString(item(root.get(),"mode"))) {
      const char* mode=item(root.get(),"mode")->valuestring;
      if(!strcmp(mode,"robot")||!strcmp(mode,"simulation"))code=demo_.startExhibition(!strcmp(mode,"simulation"));
    }
    else if(path=="/api/v1/demo/zero"&&fields(root.get(),{"confirmed_center","confirmed_fence"})&&cJSON_IsTrue(item(root.get(),"confirmed_center"))&&cJSON_IsTrue(item(root.get(),"confirmed_fence")))code=demo_.zero();
    else if(path=="/api/v1/demo/start"&&fields(root.get(),{"mode"})&&cJSON_IsString(item(root.get(),"mode"))) {
      const char* mode=item(root.get(),"mode")->valuestring;
      if(!strcmp(mode,"simulation")||!strcmp(mode,"robot"))code=demo_.start(!strcmp(mode,"simulation"));
    }else if(path=="/api/v1/demo/calibration/start"&&fields(root.get(),{"kind"})&&cJSON_IsString(item(root.get(),"kind"))) {
      const char* k=item(root.get(),"kind")->valuestring;
      if(!strcmp(k,"forward")||!strcmp(k,"cw")||!strcmp(k,"ccw"))code=demo_.calibrationStart(!strcmp(k,"forward")?0:!strcmp(k,"cw")?1:2);
    }else if(path=="/api/v1/demo/calibration/result") {
      uint32_t id=0;float measured=0,coast=0;
      if(fields(root.get(),{"sample_id","measured","coast_cm"})&&integer(root.get(),"sample_id",id)&&number(root.get(),"measured",measured)&&number(root.get(),"coast_cm",coast))code=demo_.calibrationResult(id,measured,coast);
    }
  }
  if(code>=400){error(code,code==409?"demo_not_ready_or_control_busy":code==503?"demo_storage_or_task_unavailable":"invalid_demo_configuration");return;}
  server_.send(code,"application/json","{\"ok\":true}");
}
