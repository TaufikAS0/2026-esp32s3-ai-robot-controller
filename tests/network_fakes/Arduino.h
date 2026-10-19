#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
using portMUX_TYPE=int;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(p) ((void)(p))
#define portEXIT_CRITICAL(p) ((void)(p))
constexpr int HEX=16;
class String {
 public:
  std::string value;
  String()=default;
  String(const char* s):value(s?s:""){}
  String(uint32_t n,int):value(std::to_string(n)){}
  const char* c_str() const{return value.c_str();}
  size_t length() const{return value.size();}
  bool startsWith(const char* s) const{return value.rfind(s,0)==0;}
  int indexOf(char c,unsigned at=0) const{auto n=value.find(c,at);return n==std::string::npos?-1:int(n);}
  String substring(unsigned start) const{return String(value.substr(start).c_str());}
  String substring(unsigned start,unsigned end) const{return String(value.substr(start,end-start).c_str());}
  String& operator+=(char c){value+=c;return *this;}
  bool operator==(const char* s)const{return value==s;}
  bool operator==(const String& s)const{return value==s.value;}
  bool operator!=(const String& s)const{return value!=s.value;}
};
inline String operator+(const char* a,const String& b){return String((std::string(a)+b.value).c_str());}
inline String operator+(const String& a,const char* b){return String((a.value+b).c_str());}
inline uint32_t fakeNow=0;
inline uint32_t millis(){return fakeNow;}
struct FakeSerial {
  void println(const char*){}
  template<class... Args>void printf(const char*,Args...){}
  int available(){return 0;}
  char read(){return 0;}
};
inline FakeSerial Serial;
struct FakeESP { uint64_t getEfuseMac(){return 0x112233;} };
inline FakeESP ESP;
