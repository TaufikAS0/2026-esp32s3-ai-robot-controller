#pragma once
#include <Arduino.h>
class FakeWire {
 public:
  bool begin(uint8_t sda, uint8_t scl, uint32_t hz);
  void setClock(uint32_t hz);
  void setTimeOut(uint16_t ms);
  void beginTransmission(uint8_t address);
  size_t write(uint8_t byte);
  size_t write(const uint8_t* bytes, size_t count);
  uint8_t endTransmission();
};
extern FakeWire Wire;
