#pragma once
#include <cmath>
#include <cstring>
#include "control_logic.h"

#include "eye_catalog.h"

struct EyeSettings {
  bool enabled = true, automatic = true, blink = true, swap = false;
  bool rotateLeft = true, rotateRight = true; // Current lab mounting: both panels upside down.
  Expression expression = Expression::Happy;
  EyePlayMode playMode = EyePlayMode::Reactive;
  EyeCategory category = EyeCategory::All;
  uint8_t energy = 2;
  uint8_t brightness = 160;
  float tempo = 1;
  static bool valid(const EyeSettings& s) {
    return static_cast<unsigned>(s.expression) < kEyeExpressionCount &&
      static_cast<unsigned>(s.playMode)<4 && static_cast<unsigned>(s.category)<kEyeCategoryCount && s.energy>=1 && s.energy<=3 && s.brightness > 0 &&
      std::isfinite(s.tempo) && s.tempo >= .5f && s.tempo <= 2;
  }
};
struct EyePose { float x = 64, y = 32, width = 56, height = 36, pupilX = 0, pupilY = 0, phase = 0; };
struct EyeFrame {
  Expression expression = Expression::Neutral;
  EyePose left, right;
  bool blinking = false;
  const char* reaction = "idle";
};
class EyeAnimator {
 public:
  EyeFrame update(const EyeSettings& settings, const Output& out, bool maintenance, uint32_t now) {
    const float drive = (out.left + out.right) * .5f;
    const float turn = (out.left - out.right) * .5f; // Robot's left is negative.
    const bool moving = std::fabs(out.left) > .04f || std::fabs(out.right) > .04f;
    const bool hands = out.servoEnabled && (std::fabs(out.armLeft - 90) > 15 || std::fabs(out.armRight - 90) > 15);
    if (moving || hands || out.laser) lastActive_ = now;
    EyeFrame f;
    f.expression = settings.expression;
    float gazeX = 0, gazeY = 0;
    const float phase = float(now % 60000) * .001f * settings.tempo;
    if (maintenance) { f.expression = Expression::Sleepy; f.reaction = "ota"; }
    else if (settings.automatic) {
      if (moving) {
        gazeX = turn * 24;
        if (std::fabs(turn) > .1f) { f.expression = Expression::Curious; f.reaction = turn < 0 ? "turn_left" : "turn_right"; }
        else if (drive < -.04f) { f.expression = Expression::Surprised; f.reaction = "reverse"; gazeY = -4; }
        else { f.expression = Expression::Angry; f.reaction = "forward_focus"; gazeY = 2; }
      } else if (out.laser) { f.expression = Expression::Angry; f.reaction = "laser_focus"; }
      else if (hands) {
        const float lift = (out.armLeft + out.armRight) * .5f;
        f.expression = lift < 55 ? Expression::Shy : lift > 125 ? Expression::Happy : Expression::Curious;
        f.reaction = "hands"; gazeX = (out.armRight - out.armLeft) / 180 * 18;
        gazeY = (lift - 90) / 90 * 9;
      } else {
        const uint32_t bucket=uint32_t(float(now)/ (settings.playMode==EyePlayMode::Playful ? 900.f : 1800.f) * settings.tempo);
        f.expression=choose(settings.category,settings.playMode==EyePlayMode::Playful ? bucket*17+5 : bucket,true);
        f.reaction="idle_playful"; gazeX=std::sin(phase*.8f)*float(3+settings.energy*3);
      }
    } else f.reaction = "manual_expression";
    if(settings.automatic && settings.playMode==EyePlayMode::Showcase && !maintenance) {
      f.expression=choose(settings.category,uint32_t(float(now)*settings.tempo/2200.f));
      f.reaction="showcase";
    }
    const auto& entry=eyeEntry(f.expression);
    float width=entry.width,height=entry.height;
    if(f.expression==Expression::Sleepy || f.expression==Expression::Dozing) gazeY+=6;
    if(f.expression==Expression::Shy) { gazeX=-8; gazeY=6; }
    if(f.expression==Expression::SideEye) gazeX=13;
    if(f.expression==Expression::Thinking) { gazeX=-9; gazeY=-6; }
    if(f.expression==Expression::Giggle || f.expression==Expression::Panic) gazeY+=std::sin(phase*16)*settings.energy*1.5f;
    if(f.expression==Expression::Excited || f.expression==Expression::Party) gazeY+=std::sin(phase*9)*settings.energy*2.f;
    if(f.expression==Expression::Chaos || f.expression==Expression::Glitch) {
      gazeX=std::sin(float(now/130)*7.f)*float(4+settings.energy*4);
      height*=.6f+.4f*std::fabs(std::sin(phase*9));
    }
    // Smooth pose changes; bounded dt also makes millis rollover safe.
    const uint32_t dt = now - lastFrame_;
    const float ease = lastFrame_ ? std::fmin(1.f, float(dt) / 140.f) : 1.f;
    x_ += (gazeX - x_) * ease; y_ += (gazeY - y_) * ease;
    width_ += (width - width_) * ease; height_ += (height - height_) * ease;
    lastFrame_ = now;
    const uint32_t cycle = uint32_t(4100 / settings.tempo);
    const uint32_t blinkPhase = now % cycle;
    f.blinking = settings.blink && !maintenance && blinkPhase > cycle - 140;
    const float pulse = std::sin(phase * 2) * 1.2f;
    f.left = {64 + x_, 32 + y_ + pulse, width_, f.blinking ? 3.f : height_};
    f.right = f.left;
    f.left.phase=f.right.phase=phase;
    decorate(f.left,f.expression,true,phase,settings.energy);
    decorate(f.right,f.expression,false,phase,settings.energy);
    if(f.blinking) f.left.height=f.right.height=3;
    if(entry.shape==EyeShape::Googly) {
      f.left.pupilX=std::sin(phase*3)*10; f.left.pupilY=std::cos(phase*2)*7;
      f.right.pupilX=std::sin(phase*3+1.8f)*10; f.right.pupilY=std::cos(phase*2+2)*7;
      if(f.expression==Expression::Derp) { f.left.pupilX=12; f.right.pupilX=-12; f.right.y+=5; }
      if(f.expression==Expression::Pleading) f.left.pupilX=f.right.pupilX=f.left.pupilY=f.right.pupilY=0;
    }
    const uint32_t winkCycle=uint32_t(1600/settings.tempo);
    const bool wink=(now%winkCycle)<350;
    if(settings.blink && !maintenance) {
      if((f.expression==Expression::WinkLeft || f.expression==Expression::LoveWink) && wink) f.left.height=3;
      if(f.expression==Expression::WinkRight && wink) f.right.height=3;
      if(f.expression==Expression::Alternating) { if((now%winkCycle)<winkCycle/2) f.left.height=3; else f.right.height=3; }
      if(f.expression==Expression::DoubleBlink && ((now%winkCycle)<120 || ((now%winkCycle)>240 && (now%winkCycle)<360))) f.left.height=f.right.height=3;
    }
    if ((f.expression == Expression::Curious || f.expression==Expression::Skeptical) && !f.blinking) f.right.height *= .75f;
    // Idle wink every ~11 seconds, separate from the coordinated blink.
    if (settings.automatic && settings.blink && !moving && !maintenance && now % 11000 > 10780) f.left.height = 3;
    return f;
  }
 public:
  // Also used to generate honest thumbnail reference frames in native tests.
  static void decorate(EyePose& p, Expression e, bool left, float phase, uint8_t energy) {
    switch(e) {
      case Expression::Laugh: p.height*=.45f+.55f*std::fabs(std::sin(phase*6)); p.y+=std::sin(phase*12)*2; break;
      case Expression::Giggle: p.height*=left ? .55f+.3f*std::sin(phase*10) : .85f; p.x+=std::sin(phase*12)*2; break;
      case Expression::Shy: p.height*=.78f; p.x+=left ? 2.f : -2.f; break;
      case Expression::Skeptical: if(left) { p.y-=5; p.height*=1.2f; } else { p.y+=3; p.height*=.6f; } break;
      case Expression::Thinking: p.y-=5; if(!left) p.height*=.55f; break;
      case Expression::Determined: p.width*=1.08f; p.height*=.65f; break;
      case Expression::Sneaky: p.x+=std::sin(phase*1.5f)*9; p.height*=left ? .7f : 1.f; break;
      case Expression::Surprised: p.height*=.9f+.1f*std::sin(phase*5); break;
      case Expression::Panic: p.x+=std::sin(phase*23+(left?0.f:2.f))*energy*2; break;
      case Expression::Dozing: p.y+=3; break;
      case Expression::Bored: p.height*=left ? .6f : .9f; p.x-=7; break;
      case Expression::Zen: p.width*=.9f; p.y+=std::sin(phase)*2; break;
      case Expression::Sad: p.y+=5; p.width*=.85f; break;
      case Expression::Pleading: p.width*=.9f; p.y-=3; break;
      case Expression::Party: p.y+=std::sin(phase*7+(left?0.f:3.14f))*energy*3; break;
      case Expression::Sparkle: p.width*=.75f+.25f*std::fabs(std::sin(phase*5)); break;
      case Expression::Dizzy: p.x+=std::sin(phase*4)*7; p.y+=std::cos(phase*4)*5; break;
      default: break;
    }
  }
 private:
  static Expression choose(EyeCategory category, uint32_t index, bool boxOnly=false) {
    unsigned count=0;
    for(unsigned i=0;i<kEyeExpressionCount;++i) if((category==EyeCategory::All || kEyeCatalog[i].category==category) && (!boxOnly || isBoxEye(static_cast<Expression>(i)))) ++count;
    if(!count) return Expression::Happy;
    unsigned selected=index%count;
    for(unsigned i=0;i<kEyeExpressionCount;++i) if((category==EyeCategory::All || kEyeCatalog[i].category==category) && (!boxOnly || isBoxEye(static_cast<Expression>(i)))) {
      if(selected--==0) return static_cast<Expression>(i);
    }
    return Expression::Happy;
  }
  uint32_t lastActive_ = 0, lastFrame_ = 0;
  float x_ = 0, y_ = 0, width_ = 56, height_ = 36;
};
