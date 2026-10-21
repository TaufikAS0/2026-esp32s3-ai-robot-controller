#pragma once
#include <cmath>
#include <stdint.h>
struct ArmPolicy {
  float leftMin=0,leftMax=180,rightMin=0,rightMax=180;
  bool dynamic=false;
  float amplitude=8;
  uint32_t periodMs=2400;
  static bool valid(const ArmPolicy& p) {
    return std::isfinite(p.leftMin)&&std::isfinite(p.leftMax)&&std::isfinite(p.rightMin)&&std::isfinite(p.rightMax)&&
      p.leftMin>=0&&p.leftMin<p.leftMax&&p.leftMax<=180&&p.rightMin>=0&&p.rightMin<p.rightMax&&p.rightMax<=180&&
      std::isfinite(p.amplitude)&&p.amplitude>=0&&p.amplitude<=30&&p.periodMs>=1000&&p.periodMs<=10000;
  }
  bool allows(float l,float r) const { return l>=leftMin&&l<=leftMax&&r>=rightMin&&r<=rightMax; }
  static float clamp(float v,float lo,float hi) {return std::fmax(lo,std::fmin(hi,v));}
};
