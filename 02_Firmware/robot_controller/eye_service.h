#pragma once
#include <Wire.h>
#include "control_service.h"
#include "eye_raster.h"
enum class EyeCheckMode { Animate, Scan, Reinit, White, Checker, Identity };
enum class EyeTarget { Both, Left, Right };
inline const char* eyeCheckName(EyeCheckMode m) {
  static const char* names[] = {"animate","scan","reinit","white","checker","identity"};
  return names[static_cast<unsigned>(m)];
}
struct EyeCheckSnapshot {
  EyeCheckMode mode = EyeCheckMode::Animate;
  EyeTarget target = EyeTarget::Both;
  uint32_t request = 0, remainingMs = 0, lastScanMs = 0;
  bool scanComplete = false;
  uint8_t scanned = 0, found[16]{};
  uint8_t leftProbe = 255, rightProbe = 255;
};
struct EyeSnapshot {
  EyeSettings settings;
  EyeFrame frame;
  bool taskReady = false, leftOnline = false, rightOnline = false;
  uint32_t errors = 0, frames = 0;
  EyeCheckSnapshot check;
};
class EyeService {
 public:
  explicit EyeService(ControlService& control) : control_(control) {}
  bool begin();
  bool configure(const EyeSettings& settings);
  bool check(EyeCheckMode mode, EyeTarget target);
  EyeSnapshot snapshot();
 private:
  static void task(void* argument);
  bool command(uint8_t address, const uint8_t* data, size_t size);
  bool initialize(uint8_t address, uint8_t brightness, bool rotated);
  bool display(uint8_t address, const EyeRaster& raster);
  ControlService& control_;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  EyeSnapshot state_;
  EyeCheckMode requestedMode_ = EyeCheckMode::Animate;
  EyeTarget requestedTarget_ = EyeTarget::Both;
  uint32_t requested_ = 0;
};
