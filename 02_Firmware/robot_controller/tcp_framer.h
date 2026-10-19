#pragma once
#include <cstddef>
#include <cstdint>

// One bounded NDJSON frame. Deadline is from FIRST byte, not last progress.
class TcpFramer {
 public:
  enum class Result { More, Line, Invalid, TooLarge };
  static constexpr size_t maxBytes = 1024;
  static constexpr uint32_t deadlineMs = 250;
  const char* line() const { return bytes_; }
  size_t size() const { return length_; }
  bool expired(uint32_t now) const { return length_ && uint32_t(now - started_) >= deadlineMs; }
  void reset() { length_ = 0; bytes_[0] = 0; }
  static bool validUtf8(const char* text,size_t size) {
    for(size_t i=0;i<size;) {
      const auto c=static_cast<unsigned char>(text[i++]);
      if(c<0x80)continue;
      unsigned count=0;uint32_t value=0,minimum=0;
      if(c>=0xc2 && c<=0xdf){count=1;value=c&31;minimum=0x80;}
      else if(c>=0xe0 && c<=0xef){count=2;value=c&15;minimum=0x800;}
      else if(c>=0xf0 && c<=0xf4){count=3;value=c&7;minimum=0x10000;}
      else return false;
      if(i+count>size)return false;
      while(count--) { const auto next=static_cast<unsigned char>(text[i++]);if((next&0xc0)!=0x80)return false;value=(value<<6)|(next&63); }
      if(value<minimum || value>0x10ffff || (value>=0xd800 && value<=0xdfff))return false;
    }return true;
  }
  Result push(char c, uint32_t now) {
    if (c == '\n') {
      if (!length_) return Result::Invalid;
      bytes_[length_] = 0;
      return Result::Line;
    }
    if (!c || (static_cast<unsigned char>(c) < 32 && c != '\t' && c != '\r')) return Result::Invalid;
    if (length_ == maxBytes) return Result::TooLarge;
    if (!length_) started_ = now;
    bytes_[length_++] = c;
    return Result::More;
  }
 private:
  char bytes_[maxBytes + 1]{};
  size_t length_ = 0;
  uint32_t started_ = 0;
};
