#pragma once
#include <cmath>
#include <stdint.h>

struct ArmMotionSettings {
  float speed = 60, acceleration = 240;
  static bool valid(const ArmMotionSettings& s) {
    return std::isfinite(s.speed) && std::isfinite(s.acceleration) &&
      s.speed >= 5 && s.speed <= 120 && s.acceleration >= 20 && s.acceleration <= 600;
  }
};
// Reference trajectory only; no position feedback from hobby servos exists.
class ArmMotion {
 public:
  float position = 90, velocity = 0;
  void hold(float angle) { position=angle; velocity=0; }
  float step(float target, float dt, const ArmMotionSettings& limits) {
    if(dt<=0) return position;
    const float distance=target-position;
    if(std::fabs(distance)<.001f && std::fabs(velocity)<.01f) { hold(target); return position; }
    // Slow down before arrival; continuous targets do not restart a trajectory.
    const float braking=std::fmax(0.f,std::sqrt(2*limits.acceleration*std::fabs(distance))-limits.acceleration*dt);
    const float desired=std::copysign(std::fmin(limits.speed,braking),distance);
    const float dv=limits.acceleration*dt;
    const float old=velocity;
    velocity+=std::fmax(-dv,std::fmin(dv,desired-velocity));
    float next=position+(old+velocity)*.5f*dt;
    if(std::fabs(distance)<.03f && std::fabs(velocity)<=dv) { hold(target); return position; }
    // Endpoint clamp can truncate final acceleration: this is not jerk-limited planning.
    if((target-position)*(target-next)<=0 && distance*velocity>=0) { hold(target); return position; }
    if(next<0 || next>180) { hold(std::fmax(0.f,std::fmin(180.f,next))); return position; }
    position=next;return position;
  }
};
