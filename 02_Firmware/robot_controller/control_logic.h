#pragma once
#include <cmath>
#include <stdint.h>
#include "config.h"

enum class ControlMode { Manual, Auto };
enum class Owner { None, Manual, Program };
struct Command {
  uint32_t session = 0, sequence = 0;
  float left = 0, right = 0, armLeft = 90, armRight = 90;
  bool laser = false;
};
struct Output {
  float left = 0, right = 0, armLeft = 90, armRight = 90;
  bool laser = false, servoEnabled = false;
};
class MotorRamp {
 public:
  float value = 0;
  void reset() { value = 0; waiting_ = false; }
  float step(float target, uint32_t now) {
    if (waiting_) {
      if (uint32_t(now - zeroAt_) < Config::reversalMs) return 0;
      waiting_ = false;
    }
    bool reversing = value * target < 0;
    float next = reversing ? 0 : target;
    float diff = next - value;
    if (std::fabs(diff) <= Config::rampPerTick) value = next;
    else value += diff > 0 ? Config::rampPerTick : -Config::rampPerTick;
    if (reversing && value == 0) { waiting_ = true; zeroAt_ = now; }
    return value;
  }
 private:
  bool waiting_ = false;
  uint32_t zeroAt_ = 0;
};

// Pure logic: caller serializes access; no network or GPIO dependencies.
class ControlLogic {
 public:
  ControlMode mode = ControlMode::Manual;
  Owner owner = Owner::None;
  uint32_t session = 0, lastSequence = 0, lastCommandMs = 0;
  const char* reason = "boot";
  bool maintenance = false, hasCommand = false;
  Output output;
  Command target;

  static bool valid(const Command& c) {
    return c.sequence > 0 && std::isfinite(c.left) && std::isfinite(c.right) &&
      std::isfinite(c.armLeft) && std::isfinite(c.armRight) &&
      c.left >= -1 && c.left <= 1 && c.right >= -1 && c.right <= 1 &&
      c.armLeft >= 0 && c.armLeft <= 180 && c.armRight >= 0 && c.armRight <= 180;
  }
  void expire(uint32_t now) {
    if (owner != Owner::None && uint32_t(now - lastCommandMs) >= Config::timeoutMs)
      stop("timeout");
  }
  bool setMode(ControlMode requested) {
    if (maintenance) return false;
    if (mode != requested) { stop("mode_changed"); mode = requested; }
    return true;
  }
  bool acquire(Owner requested, uint32_t id, uint32_t now) {
    expire(now);
    if (maintenance || !id || requested == Owner::None ||
        (mode == ControlMode::Manual && requested != Owner::Manual) ||
        (mode == ControlMode::Auto && requested != Owner::Program)) return false;
    if (owner != Owner::None && !(owner == Owner::Program && requested == Owner::Manual))
      return false;
    stop("acquired");
    owner = requested; session = id; lastCommandMs = now;
    return true;
  }
  bool command(const Command& c, uint32_t now) {
    expire(now);
    if (maintenance || owner == Owner::None || c.session != session ||
        c.sequence <= lastSequence || !valid(c)) return false;
    target = c; lastSequence = c.sequence; lastCommandMs = now; hasCommand = true;
    reason = "running";
    return true;
  }
  void stop(const char* why, bool disableServos = false) {
    owner = Owner::None; session = 0; lastSequence = 0; hasCommand = false;
    target.left = target.right = 0; target.laser = false;
    output.left = output.right = 0; output.laser = false;
    if (disableServos) output.servoEnabled = false;
    left_.reset(); right_.reset(); reason = why;
  }
  void beginMaintenance() { maintenance = true; stop("ota", true); }
  void endMaintenance() { maintenance = false; stop("ota_finished", true); }
  void tick(uint32_t now) {
    expire(now);
    if (!hasCommand || maintenance) return;
    output.left = left_.step(target.left, now);
    output.right = right_.step(target.right, now);
    output.armLeft = target.armLeft; output.armRight = target.armRight;
    output.servoEnabled = true; output.laser = target.laser;
  }
 private:
  MotorRamp left_, right_;
};
