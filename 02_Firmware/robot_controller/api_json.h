#pragma once
#include <cJSON.h>
#include <memory>
#include <cmath>
#include <stdint.h>
#include <cstring>
namespace ApiJson {
using Json=std::unique_ptr<cJSON,decltype(&cJSON_Delete)>;
inline bool fields(cJSON* root,std::initializer_list<const char*> names) {
  if(!cJSON_IsObject(root)||cJSON_GetArraySize(root)!=int(names.size())) return false;
  for(const char* name:names) {
    unsigned count=0;for(cJSON* p=root->child;p;p=p->next) if(p->string&&strcmp(p->string,name)==0) ++count;
    if(count!=1)return false;
  }return true;
}
inline cJSON* item(cJSON* root,const char* key){return cJSON_GetObjectItemCaseSensitive(root,key);}
inline bool integer(cJSON* root,const char* key,uint32_t& value){
  auto p=item(root,key);
  if(!cJSON_IsNumber(p)||!std::isfinite(p->valuedouble)||p->valuedouble<1||p->valuedouble>UINT32_MAX||std::floor(p->valuedouble)!=p->valuedouble)return false;
  value=uint32_t(p->valuedouble);return true;
}
inline bool number(cJSON* root,const char* key,float& value){auto p=item(root,key);if(!cJSON_IsNumber(p))return false;value=p->valuedouble;return std::isfinite(value);}
}
