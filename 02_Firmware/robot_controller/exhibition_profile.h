#pragma once
#include "demo_planner.h"

// FLOOR exhibition estimates accepted by the operator, not measured calibration.
// Width/length bound the estimated AXLE CENTER, not the table or robot envelope.
// Runtime-only: never overwrite robot/demoV1 or mark its calibration bits valid.
struct ExhibitionProfile {
  float width=30,length=30;
  bool valid() const {return std::isfinite(width)&&std::isfinite(length)&&width>=15&&width<=200&&length>=15&&length<=200;}
  static float referenceIntegral() {
    MotorRamp ramp;DemoConfig c;c.deadbandPwm=.5f;float sum=0;
    for(unsigned i=0;i<100;++i)sum+=c.effort(ramp.step(.8f,i*10))*.01f;
    return sum;
  }
  DemoConfig model() const {
    DemoConfig c;
    // 0.1 is a mathematical offset required by the legacy planner, NOT body size.
    // Its physical boundary becomes exactly +/-width/2, +/-length/2 for the axle.
    c.width=width+.2f;c.length=length+.2f;c.radius=.1f;c.margin=0;c.coast=0;
    c.speed=3.5f;c.turn=30;c.pwm=.8f;c.minPwm=.6f;c.deadbandPwm=.5f;
    c.baseLeft=c.baseRight=90;c.minLeft=90;c.maxLeft=150;c.minRight=88;c.maxRight=92;
    const float integral=referenceIntegral();
    c.linearGain=3.5f/integral;c.cwGain=c.ccwGain=45.f/integral;
    return c;
  }
};
