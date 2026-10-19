#pragma once
#include <cmath>
#include <stdint.h>
#include "config.h"
#include "arm_motion.h"
#include "arm_policy.h"

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
  uint32_t transport = 0; // 0 HTTP; nonzero identifies one TCP connection.
  const char* reason = "boot";
  bool maintenance = false, hasCommand = false;
  bool networkBusy = false;
  Output output;
  Command target;
  ArmMotionSettings armSettings;
  ArmPolicy armPolicy;
  float effectiveArmLeft=90,effectiveArmRight=90;
  bool dynamicActive=false;
  bool configureArmPolicy(const ArmPolicy& p) {
    if(maintenance || networkBusy || owner!=Owner::None || !ArmPolicy::valid(p) ||
       (output.servoEnabled && !p.allows(output.armLeft,output.armRight))) return false;
    armPolicy=p;dynamicPhase_=0;return true;
  }
  bool configureArms(const ArmMotionSettings& s) {
    if(maintenance || networkBusy || !ArmMotionSettings::valid(s) || owner!=Owner::None) return false;
    armSettings=s; return true;
  }

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
  bool setMode(ControlMode requested, bool idleOnly = false) {
    if (maintenance || networkBusy || (idleOnly && owner != Owner::None)) return false;
    if (mode != requested) { stop("mode_changed"); mode = requested; }
    return true;
  }
  bool acquire(Owner requested, uint32_t id, uint32_t now, uint32_t connection = 0) {
    expire(now);
    if (maintenance || networkBusy || !id || requested == Owner::None ||
        (mode == ControlMode::Manual && requested != Owner::Manual) ||
        (mode == ControlMode::Auto && requested != Owner::Program)) return false;
    if (owner != Owner::None && !(owner == Owner::Program && requested == Owner::Manual))
      return false;
    stop("acquired");
    owner = requested; session = id; lastCommandMs = now; transport = connection;
    return true;
  }
  bool command(const Command& c, uint32_t now, uint32_t connection = 0) {
    expire(now);
    if (maintenance || networkBusy || owner == Owner::None || c.session != session || connection != transport ||
        c.sequence <= lastSequence || !valid(c) || !armPolicy.allows(c.armLeft,c.armRight)) return false;
    target = c; lastSequence = c.sequence; lastCommandMs = now; hasCommand = true;
    reason = "running";
    return true;
  }
  bool release(uint32_t id, uint32_t connection = 0) {
    if (owner == Owner::None || session != id || transport != connection) return false;
    stop("released"); return true;
  }
  void stop(const char* why, bool disableServos = false) {
    owner = Owner::None; session = 0; transport = 0; lastSequence = 0; hasCommand = false;
    target.left = target.right = 0; target.laser = false;
    output.left = output.right = 0; output.laser = false;
    if (disableServos) output.servoEnabled = false;
    left_.reset(); right_.reset(); armsLeft_.hold(output.armLeft); armsRight_.hold(output.armRight);
    target.armLeft=output.armLeft;target.armRight=output.armRight; reason = why;
    dynamicActive=false;dynamicPhase_=0;
    effectiveArmLeft=output.armLeft;effectiveArmRight=output.armRight;
  }
  void beginMaintenance() { maintenance = true; stop("ota", true); }
  bool beginNetworkChange(bool linkLost = false) {
    if (maintenance || networkBusy || (!linkLost && owner != Owner::None)) return false;
    networkBusy = true; stop(linkLost ? "wifi_disconnected" : "wifi_change"); return true;
  }
  void endNetworkChange() { if(networkBusy) { networkBusy=false; stop("wifi_ready"); } }
  void endMaintenance() { maintenance = false; stop("ota_finished", true); }
  void tick(uint32_t now) {
    const float dt=lastTick_ ? std::fmin(float(uint32_t(now-lastTick_)),float(Config::tickMs))*.001f : Config::tickMs*.001f;
    lastTick_=now;
    expire(now);
    if (!hasCommand || maintenance) return;
    // With pulses off, 90 degrees is only a boot reference. The first complete
    // command explicitly seeds the pulse reference; do not sweep through 90.
    // The caller must supply an accepted pose; no physical position is sensed.
    if(!output.servoEnabled) {
      armsLeft_.hold(target.armLeft); armsRight_.hold(target.armRight);
    }
    output.left = left_.step(target.left, now);
    output.right = right_.step(target.right, now);
    const float forward=(output.left+output.right)*.5f;
    dynamicActive=armPolicy.dynamic && target.left*target.right>0 && output.left*output.right>0 && std::fabs(forward)>.02f;
    float offset=0;
    if(dynamicActive) {
      dynamicPhase_+=dt*6283.1853f/armPolicy.periodMs*(forward>0?1.f:-1.f);
      if(std::fabs(dynamicPhase_)>6.2831853f) dynamicPhase_=std::fmod(dynamicPhase_,6.2831853f);
      offset=armPolicy.amplitude*std::fabs(forward)*std::sin(dynamicPhase_);
    } else dynamicPhase_=0;
    effectiveArmLeft=ArmPolicy::clamp(target.armLeft+offset,armPolicy.leftMin,armPolicy.leftMax);
    effectiveArmRight=ArmPolicy::clamp(target.armRight-offset,armPolicy.rightMin,armPolicy.rightMax);
    output.armLeft = armsLeft_.step(effectiveArmLeft,dt,armSettings);
    output.armRight = armsRight_.step(effectiveArmRight,dt,armSettings);
    const float safeLeft=ArmPolicy::clamp(output.armLeft,armPolicy.leftMin,armPolicy.leftMax);
    const float safeRight=ArmPolicy::clamp(output.armRight,armPolicy.rightMin,armPolicy.rightMax);
    if(safeLeft!=output.armLeft) armsLeft_.hold(safeLeft);
    if(safeRight!=output.armRight) armsRight_.hold(safeRight);
    output.armLeft=safeLeft;output.armRight=safeRight;
    output.servoEnabled = true; output.laser = target.laser;
  }
 private:
  MotorRamp left_, right_;
  ArmMotion armsLeft_,armsRight_;
  uint32_t lastTick_=0;
  float dynamicPhase_=0;
};
