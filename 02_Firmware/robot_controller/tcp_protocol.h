#pragma once
#include "api_json.h"
#include "control_logic.h"

// Same protocol/ownership logic is compiled on host and on device.
// Control adapter supplies snapshot/acquire/command/release/setMode/stop.
template<class Control> class TcpProtocol {
 public:
  TcpProtocol(Control& control, uint32_t connection, const char* version, const char* build)
    : control_(control), connection_(connection), version_(version), build_(build) {}
  void close() { if (session_) control_.release(session_, connection_); session_ = 0; }
  ApiJson::Json handle(const char* text, bool queued=false) {
    using namespace ApiJson;
    if(strstr(text,"\\u0000") || !boundedDepth(text)) return failure(0,"invalid_json");
    Json root(cJSON_ParseWithOpts(text, nullptr, true), cJSON_Delete);
    uint32_t id=0, v=0;
    if (!integer(root.get(),"id",id) || !integer(root.get(),"v",v) || v!=1 ||
        !cJSON_IsString(item(root.get(),"type"))) return failure(id,"invalid_envelope");
    const char* type=item(root.get(),"type")->valuestring;
    if (strcmp(type,"hello")==0 || strcmp(type,"status")==0) {
      if(!fields(root.get(),{"v","id","type"})) return failure(id,"invalid_fields");
      auto r=success(id); auto data=item(r.get(),"data");
      cJSON_AddNumberToObject(data,"protocol_version",1);
      cJSON_AddStringToObject(data,"firmware_version",version_);
      cJSON_AddStringToObject(data,"build_time_utc",build_);
      cJSON_AddNumberToObject(data,"timeout_ms",Config::timeoutMs);
      cJSON_AddNumberToObject(data,"max_frame_bytes",1024);
      cJSON_AddNumberToObject(data,"frame_timeout_ms",250);
      if(strcmp(type,"status")==0) addStatus(data);
      return r;
    }
    if (strcmp(type,"mode")==0) {
      if(!fields(root.get(),{"v","id","type","mode"}) || !cJSON_IsString(item(root.get(),"mode"))) return failure(id,"invalid_mode");
      const char* mode=item(root.get(),"mode")->valuestring;
      if(strcmp(mode,"auto") && strcmp(mode,"manual")) return failure(id,"invalid_mode");
      if(!control_.setMode(strcmp(mode,"auto")==0?ControlMode::Auto:ControlMode::Manual,true)) return failure(id,"release_control_before_mode");
      return success(id);
    }
    if (strcmp(type,"acquire")==0) {
      if(!fields(root.get(),{"v","id","type"})) return failure(id,"invalid_fields");
      uint32_t session=0;
      if(!connection_ || !control_.acquire(Owner::Program,session,connection_)) return failure(id,"mode_or_control_unavailable");
      session_=session; auto r=success(id);auto data=item(r.get(),"data");
      cJSON_AddNumberToObject(data,"session",session_);
      cJSON_AddNumberToObject(data,"timeout_ms",Config::timeoutMs);
      return r;
    }
    if (strcmp(type,"command")==0) {
      if(queued)return failure(id,"command_pipeline_not_allowed");
      Command c;
      if(!fields(root.get(),{"v","id","type","session","sequence","left","right","arm_left","arm_right","laser"}) ||
         !integer(root.get(),"session",c.session) || !integer(root.get(),"sequence",c.sequence) ||
         !number(root.get(),"left",c.left) || !number(root.get(),"right",c.right) ||
         !number(root.get(),"arm_left",c.armLeft) || !number(root.get(),"arm_right",c.armRight) ||
         !cJSON_IsBool(item(root.get(),"laser"))) return failure(id,"invalid_command");
      c.laser=cJSON_IsTrue(item(root.get(),"laser"));
      if(!ControlLogic::valid(c) || !control_.snapshot().logic.armPolicy.allows(c.armLeft,c.armRight)) return failure(id,"arm_limits_or_range");
      if(!session_ || session_!=c.session || !control_.command(c,connection_)) return failure(id,"session_or_sequence_rejected");
      auto r=success(id);cJSON_AddNumberToObject(item(r.get(),"data"),"sequence",c.sequence);
      return r; // Accepted target, not physical arrival or GPIO acknowledgement.
    }
    if(strcmp(type,"release")==0) {
      uint32_t session=0;
      if(!fields(root.get(),{"v","id","type","session"}) || !integer(root.get(),"session",session)) return failure(id,"invalid_session");
      if(!session_ || session_!=session || !control_.release(session,connection_)) return failure(id,"session_rejected");
      session_=0;return success(id);
    }
    if(strcmp(type,"stop")==0) {
      if(!fields(root.get(),{"v","id","type"})) return failure(id,"invalid_fields");
      if(!control_.stop("stop")) return failure(id,"task_unresponsive");
      session_=0;return success(id);
    }
    return failure(id,"unknown_type");
  }
 private:
  static bool boundedDepth(const char* text) {
    unsigned depth=0;bool quoted=false,escaped=false;
    for(const char* p=text;*p;++p) {
      if(quoted) { if(escaped)escaped=false;else if(*p=='\\')escaped=true;else if(*p=='"')quoted=false; }
      else if(*p=='"')quoted=true;
      else if(*p=='{' || *p=='[') { if(++depth>16)return false; }
      else if(*p=='}' || *p==']') { if(!depth)return false;--depth; }
    }
    return !depth && !quoted;
  }
  Control& control_;
  uint32_t connection_, session_=0;
  const char* version_;
  const char* build_;
  static ApiJson::Json success(uint32_t id) {
    ApiJson::Json r(cJSON_CreateObject(),cJSON_Delete);
    cJSON_AddNumberToObject(r.get(),"id",id);cJSON_AddBoolToObject(r.get(),"ok",true);
    cJSON_AddObjectToObject(r.get(),"data");return r;
  }
  static ApiJson::Json failure(uint32_t id,const char* error) {
    ApiJson::Json r(cJSON_CreateObject(),cJSON_Delete);
    cJSON_AddNumberToObject(r.get(),"id",id);cJSON_AddBoolToObject(r.get(),"ok",false);
    cJSON_AddStringToObject(r.get(),"error",error);return r;
  }
  void addStatus(cJSON* data) {
    const auto s=control_.snapshot();const auto& l=s.logic;
    cJSON_AddStringToObject(data,"mode",l.mode==ControlMode::Manual?"manual":"auto");
    cJSON_AddStringToObject(data,"owner",l.owner==Owner::None?"none":l.owner==Owner::Manual?"manual":"program");
    cJSON_AddNumberToObject(data,"session",l.session);
    cJSON_AddNumberToObject(data,"sequence",l.lastSequence);
    cJSON_AddStringToObject(data,"control_transport",l.owner==Owner::None?"none":l.transport?"tcp":"http");
    cJSON_AddStringToObject(data,"reason",l.reason);
    cJSON_AddBoolToObject(data,"maintenance",l.maintenance);
    auto out=cJSON_AddObjectToObject(data,"commanded_output");
    cJSON_AddNumberToObject(out,"left",l.output.left);cJSON_AddNumberToObject(out,"right",l.output.right);
    cJSON_AddNumberToObject(out,"arm_left",l.output.armLeft);cJSON_AddNumberToObject(out,"arm_right",l.output.armRight);
    cJSON_AddBoolToObject(out,"laser",l.output.laser);cJSON_AddBoolToObject(out,"servo_enabled",l.output.servoEnabled);
    auto arms=cJSON_AddObjectToObject(data,"arm_motion");
    cJSON_AddNumberToObject(arms,"target_left",l.target.armLeft);cJSON_AddNumberToObject(arms,"target_right",l.target.armRight);
    cJSON_AddNumberToObject(arms,"effective_left",l.effectiveArmLeft);cJSON_AddNumberToObject(arms,"effective_right",l.effectiveArmRight);
    cJSON_AddNumberToObject(arms,"min_left",l.armPolicy.leftMin);cJSON_AddNumberToObject(arms,"max_left",l.armPolicy.leftMax);
    cJSON_AddNumberToObject(arms,"min_right",l.armPolicy.rightMin);cJSON_AddNumberToObject(arms,"max_right",l.armPolicy.rightMax);
    cJSON_AddBoolToObject(arms,"moving",l.hasCommand && (std::fabs(l.effectiveArmLeft-l.output.armLeft)>.05f || std::fabs(l.effectiveArmRight-l.output.armRight)>.05f));
    cJSON_AddBoolToObject(data,"pwm_ready",s.pwm.ready);cJSON_AddBoolToObject(data,"pwm_write_ok",s.pwm.writeOk);
  }
};
