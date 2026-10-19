#pragma once
#include "Arduino.h"
#include <map>
class Preferences {
 public:
  static inline std::map<std::string,std::string> data;
  static inline bool fail=false;
  bool begin(const char*,bool){return !fail;}
  bool isKey(const char* key){return data.count(key)>0;}
  String getString(const char* key,const char* fallback){auto it=data.find(key);return String(it==data.end()?fallback:it->second.c_str());}
  size_t putString(const char* key,const char* value){if(fail)return 0;data[key]=value;return strlen(value);}
  size_t putString(const char* key,const String& value){return putString(key,value.c_str());}
  void remove(const char* key){data.erase(key);}
};
