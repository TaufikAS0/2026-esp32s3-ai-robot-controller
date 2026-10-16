#pragma once
#include <stdint.h>
#include <stddef.h>
namespace Config {
constexpr uint8_t laser = 4, armLeft = 5, armRight = 6;
constexpr uint8_t right1 = 10, right2 = 11, left1 = 12, left2 = 13;
constexpr uint32_t tickMs = 10, timeoutMs = 500, reversalMs = 40;
constexpr float rampPerTick = 0.04f;
constexpr uint32_t motorHz = 20000, servoHz = 50;
constexpr bool swapMotorSides = true; // Installed wheel channels are opposite logical sides.
constexpr bool invertLeftMotor = false, invertRightMotor = true;
constexpr bool invertLeftArm = true, invertRightArm = false;
constexpr uint32_t servoMinUs = 500, servoMaxUs = 2500;
constexpr uint8_t eyeSda = 8, eyeScl = 9;
constexpr uint8_t eyeRightAddress = 0x3C, eyeLeftAddress = 0x3D;
constexpr uint32_t eyeBusHz = 400000, eyeFrameMs = 60;
constexpr size_t maxJson = 1024;
}
