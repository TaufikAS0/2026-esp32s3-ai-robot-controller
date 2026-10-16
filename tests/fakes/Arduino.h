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
#ifdef ROBOT_EYE_TEST
#include <cstddef>
using TickType_t = uint32_t;
using portMUX_TYPE = int;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(p) ((void)(p))
#define portEXIT_CRITICAL(p) ((void)(p))
constexpr int pdPASS = 1;
inline uint32_t pdMS_TO_TICKS(uint32_t n) { return n; }
uint32_t millis();
inline uint32_t micros() { return millis()*1000; }
TickType_t xTaskGetTickCount();
int xTaskCreatePinnedToCore(void (*fn)(void*), const char*, uint32_t, void*, int, void*, int);
void vTaskDelayUntil(TickType_t*, TickType_t);
#endif
