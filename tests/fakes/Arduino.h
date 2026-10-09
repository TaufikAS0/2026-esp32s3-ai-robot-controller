#pragma once
#include <stdint.h>
#include <cmath>
constexpr int OUTPUT = 1, HIGH = 1, LOW = 0;
void pinMode(uint8_t, int);
void digitalWrite(uint8_t, int);
bool ledcAttachChannel(uint8_t, uint32_t, uint8_t, uint8_t);
bool ledcWrite(uint8_t, uint32_t);
uint32_t ledcRead(uint8_t);
uint32_t ledcReadFreq(uint8_t);
