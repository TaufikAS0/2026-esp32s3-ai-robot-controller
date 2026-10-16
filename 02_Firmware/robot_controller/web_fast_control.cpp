#include "web_service.h"
#include "api_json.h"
using namespace ApiJson;
namespace {
esp_err_t fastError(httpd_req_t* req,int code,const char* message) {
  httpd_resp_set_status(req,code==400?"400 Bad Request":code==403?"403 Forbidden":code==408?"408 Request Timeout":"500 Internal Server Error");
  const String body=String("{\"error\":\"")+message+"\"}";
  return httpd_resp_send(req,body.c_str(),body.length());
}
}

void WebService::beginFastControl() {
  httpd_config_t config=HTTPD_DEFAULT_CONFIG();
  config.server_port=81;config.ctrl_port=32769;
  config.core_id=1;config.task_priority=2;config.stack_size=8192;
  config.max_open_sockets=4;config.max_uri_handlers=20;
  config.recv_wait_timeout=1;config.send_wait_timeout=1;config.lru_purge_enable=true;
  if(httpd_start(&fastServer_,&config)!=ESP_OK){fastServer_=nullptr;return;}
  const char* paths[]={"/api/v1/status","/api/v1/command","/api/v1/control/acquire","/api/v1/control/release",
    "/api/v1/control/mode","/api/v1/stop","/api/v1/arms/settings","/api/v1/arms/policy"};
  for(unsigned i=0;i<8;++i) {
    httpd_uri_t route{};route.uri=paths[i];route.method=i==0?HTTP_GET:HTTP_POST;
    route.handler=fastControl;route.user_ctx=this;
    if(httpd_register_uri_handler(fastServer_,&route)!=ESP_OK){httpd_stop(fastServer_);fastServer_=nullptr;return;}
    route.method=HTTP_OPTIONS;
    if(httpd_register_uri_handler(fastServer_,&route)!=ESP_OK){httpd_stop(fastServer_);fastServer_=nullptr;return;}
  }
}

int WebService::applyArmPolicy(cJSON* root) {
  ArmPolicy p;
  if(!fields(root,{"min_left","max_left","min_right","max_right","dynamic","amplitude_deg","period_ms"}) ||
     !number(root,"min_left",p.leftMin)||!number(root,"max_left",p.leftMax)||
     !number(root,"min_right",p.rightMin)||!number(root,"max_right",p.rightMax)||
     !number(root,"amplitude_deg",p.amplitude)||!integer(root,"period_ms",p.periodMs)||!cJSON_IsBool(item(root,"dynamic")))return 400;
  p.dynamic=cJSON_IsTrue(item(root,"dynamic"));
  if(!ArmPolicy::valid(p))return 400;
  return control_.configureArmPolicy(p)?200:409;
}
void WebService::armPolicy() {
  const String body=server_.arg("plain");
  if(!server_.header("Content-Type").startsWith("application/json")||!body.length()||body.length()>Config::maxJson){error(400,"invalid_arm_policy");return;}
  Json root(cJSON_ParseWithOpts(body.c_str(),nullptr,true),cJSON_Delete);
  const int code=applyArmPolicy(root.get());
  if(code!=200){error(code,code==400?"invalid_arm_policy":"release_control_and_place_arms_inside_limits");return;}
  server_.send(200,"application/json","{\"ok\":true}");
}

esp_err_t WebService::fastControl(httpd_req_t* req) {
  auto& self=*static_cast<WebService*>(req->user_ctx);
  httpd_resp_set_type(req,"application/json");
  httpd_resp_set_hdr(req,"Cache-Control","no-store");
  // LAN stays open; browser CORS is limited to the dashboard origin on this host.
  char origin[128]{},host[128]{};
  if(httpd_req_get_hdr_value_len(req,"Origin")) {
    if(httpd_req_get_hdr_value_str(req,"Origin",origin,sizeof(origin))!=ESP_OK ||
       httpd_req_get_hdr_value_str(req,"Host",host,sizeof(host))!=ESP_OK)return fastError(req,400,"invalid_origin");
    String hostname=host;int colon=hostname.indexOf(':');if(colon>=0)hostname=hostname.substring(0,colon);
    const String expected=String("http://")+hostname;
    if(String(origin)!=expected && String(origin)!=expected+":80")return fastError(req,403,"origin_not_allowed");
    httpd_resp_set_hdr(req,"Access-Control-Allow-Origin",origin);
    httpd_resp_set_hdr(req,"Vary","Origin");
    httpd_resp_set_hdr(req,"Access-Control-Allow-Methods","GET, POST, OPTIONS");
    httpd_resp_set_hdr(req,"Access-Control-Allow-Headers","Content-Type");
    httpd_resp_set_hdr(req,"Access-Control-Max-Age","600");
  }
  if(req->method==HTTP_OPTIONS){httpd_resp_set_status(req,"204 No Content");return httpd_resp_send(req,nullptr,0);}
  if(req->method==HTTP_GET) {
    char* text=self.statusJson();
    if(!text)return fastError(req,500,"out_of_memory");
    const auto result=httpd_resp_sendstr(req,text);cJSON_free(text);return result;
  }
  char body[1025]{},type[64]{};
  if(req->content_len==0 || req->content_len>Config::maxJson ||
     httpd_req_get_hdr_value_str(req,"Content-Type",type,sizeof(type))!=ESP_OK || strncmp(type,"application/json",16)!=0)
    return fastError(req,400,"invalid_json_body");
  unsigned received=0;
  while(received<req->content_len) {
    const int n=httpd_req_recv(req,body+received,req->content_len-received);
    if(n<=0)return fastError(req,408,"incomplete_body");
    received+=n;
  }
  if(memchr(body,0,received))return fastError(req,400,"invalid_json_body");
  Json root(cJSON_ParseWithOpts(body,nullptr,true),cJSON_Delete);
  int code=200;String response="{\"ok\":true}";
  const char* error=nullptr;const char* path=req->uri;
  if(strcmp(path,"/api/v1/command")==0) {
    Command c;
    if(!fields(root.get(),{"session","sequence","left","right","arm_left","arm_right","laser"}) ||
       !integer(root.get(),"session",c.session)||!integer(root.get(),"sequence",c.sequence)||
       !number(root.get(),"left",c.left)||!number(root.get(),"right",c.right)||
       !number(root.get(),"arm_left",c.armLeft)||!number(root.get(),"arm_right",c.armRight)||!cJSON_IsBool(item(root.get(),"laser"))) {
      code=400;error="invalid_command";
    } else {
      c.laser=cJSON_IsTrue(item(root.get(),"laser"));
      if(!ControlLogic::valid(c)||!self.control_.snapshot().logic.armPolicy.allows(c.armLeft,c.armRight)){code=400;error="arm_limits_or_range";}
      else if(!self.control_.command(c)){code=409;error="session_or_sequence_rejected";}
    }
  } else if(strcmp(path,"/api/v1/control/acquire")==0) {
    if(!fields(root.get(),{"owner"})||!cJSON_IsString(item(root.get(),"owner"))){code=400;error="invalid_owner";}
    else {
      const String name=item(root.get(),"owner")->valuestring;
      const Owner owner=name=="manual"?Owner::Manual:name=="program"?Owner::Program:Owner::None;
      uint32_t session=0;
      if(owner==Owner::None){code=400;error="invalid_owner";}
      else if(!self.control_.acquire(owner,session)){code=409;error="mode_or_control_unavailable";}
      else response="{\"session\":"+String(session)+",\"timeout_ms\":500}";
    }
  } else if(strcmp(path,"/api/v1/control/release")==0) {
    uint32_t session=0;
    if(!fields(root.get(),{"session"})||!integer(root.get(),"session",session)){code=400;error="invalid_session";}
    else if(!self.control_.release(session)){code=409;error="session_rejected";}
  } else if(strcmp(path,"/api/v1/control/mode")==0) {
    if(!fields(root.get(),{"mode"})||!cJSON_IsString(item(root.get(),"mode"))){code=400;error="invalid_mode";}
    else {
      const String mode=item(root.get(),"mode")->valuestring;
      if(mode!="manual"&&mode!="auto"){code=400;error="invalid_mode";}
      else if(!self.control_.setMode(mode=="manual"?ControlMode::Manual:ControlMode::Auto)){code=409;error="mode_unavailable";}
    }
  } else if(strcmp(path,"/api/v1/arms/settings")==0) {
    ArmMotionSettings settings;
    if(!fields(root.get(),{"speed_dps","acceleration_dps2"})||!number(root.get(),"speed_dps",settings.speed)||
       !number(root.get(),"acceleration_dps2",settings.acceleration)||!ArmMotionSettings::valid(settings)){code=400;error="invalid_arm_settings";}
    else if(!self.control_.configureArms(settings)){code=409;error="release_control_before_arm_settings";}
  } else if(strcmp(path,"/api/v1/arms/policy")==0) {
    code=self.applyArmPolicy(root.get());
    if(code!=200)error=code==400?"invalid_arm_policy":"release_control_and_place_arms_inside_limits";
  } else if(strcmp(path,"/api/v1/stop")==0) {
    if(!fields(root.get(),{})){code=400;error="invalid_stop";}
    else if(!self.control_.stop("stop")){code=503;error="task_unresponsive";}
  } else {code=404;error="not_found";}
  if(error)response=String("{\"error\":\"")+error+"\"}";
  httpd_resp_set_status(req,code==200?"200 OK":code==400?"400 Bad Request":code==409?"409 Conflict":code==503?"503 Service Unavailable":"404 Not Found");
  return httpd_resp_send(req,response.c_str(),response.length());
}
