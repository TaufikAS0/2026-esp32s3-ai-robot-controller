#include <cassert>
#include <limits>
#include <initializer_list>
#include "../02_Firmware/robot_controller/eye_raster.h"
int main() {
  EyeSettings settings;
  assert(EyeSettings::valid(settings));
  settings.tempo = std::numeric_limits<float>::quiet_NaN(); assert(!EyeSettings::valid(settings));
  settings.tempo = 2.1f; assert(!EyeSettings::valid(settings));
  settings.tempo = 1; settings.brightness = 0; assert(!EyeSettings::valid(settings));
  settings.brightness = 160;
  Expression parsed;
  assert(!parseExpression("unknown", parsed)); assert(!parseExpression(nullptr, parsed));
  assert(parseExpression("love", parsed) && parsed == Expression::Love);
  EyeAnimator animator; Output out;
  out.left = out.right = 1;
  auto f = animator.update(settings,out,false,1000);
  assert(f.expression == Expression::Angry && strcmp(f.reaction,"forward_focus") == 0);
  out.left = -1; out.right = 1;
  f = animator.update(settings,out,false,2000);
  assert(f.left.x < 64 && strcmp(f.reaction,"turn_left") == 0);
  out.left = 1; out.right = -1;
  f = animator.update(settings,out,false,3000); assert(f.left.x > 64);
  out.left = out.right = -1;
  f = animator.update(settings,out,false,4000); assert(f.expression == Expression::Surprised);
  out.left = out.right = 0; out.servoEnabled = true; out.armLeft = out.armRight = 0;
  f = animator.update(settings,out,false,5000); assert(f.expression == Expression::Shy);
  out.armLeft = out.armRight = 180;
  f = animator.update(settings,out,false,6000); assert(f.expression == Expression::Happy);
  out.armLeft = out.armRight = 90;
  f = animator.update(settings,out,false,22000); assert(strcmp(f.reaction,"idle_playful")==0);
  settings.automatic = false; settings.expression = Expression::Love;
  out.left = out.right = 1;
  f = animator.update(settings,out,false,23000); assert(f.expression == Expression::Love);
  f = animator.update(settings,out,true,24000); assert(f.expression == Expression::Sleepy);
  settings.blink = true;
  f = animator.update(settings,out,false,4090); assert(f.blinking && f.left.height == 3);
  settings.blink = false;
  f = animator.update(settings,out,false,4090); assert(!f.blinking);
  // Guard both ends of the framebuffer and verify each preset produces pixels.
  struct Guarded { uint32_t before = 0x12345678; EyeRaster raster; uint32_t after = 0x87654321; } guarded;
  for (unsigned i=0;i<kEyeExpressionCount;++i) {
    settings.expression = static_cast<Expression>(i);
    f = animator.update(settings,out,false,10000+i*1000);
    guarded.raster.eye(f.left,f.expression,true);
    unsigned pixels = 0; for (auto b : guarded.raster.pixels) if (b) ++pixels;
    assert(pixels > 0);
    assert(guarded.before == 0x12345678 && guarded.after == 0x87654321);
  }
  settings.automatic=false; settings.blink=true;
  settings.expression=Expression::WinkLeft;
  f=animator.update(settings,out,false,16010); assert(f.left.height==3 && f.right.height>4);
  settings.expression=Expression::WinkRight;
  f=animator.update(settings,out,false,17610); assert(f.right.height==3 && f.left.height>4);
  settings.automatic=true; settings.playMode=EyePlayMode::Showcase;
  settings.category=EyeCategory::Wild;
  for(uint32_t t=0;t<30000;t+=2200) {
    f=animator.update(settings,out,false,t); assert(eyeEntry(f.expression).category==EyeCategory::Wild);
  }
  settings.category=EyeCategory::All;
  for(auto mode:{EyePlayMode::Reactive,EyePlayMode::Playful}) {
    settings.playMode=mode; out=Output{};
    for(uint32_t t=0;t<80000;t+=901) {
      f=animator.update(settings,out,false,t); assert(isBoxEye(f.expression));
    }
    out.servoEnabled=true;out.armLeft=out.armRight=0;
    f=animator.update(settings,out,false,81000);assert(isBoxEye(f.expression));
  }
  settings.energy=0; assert(!EyeSettings::valid(settings));
  settings.energy=4; assert(!EyeSettings::valid(settings)); settings.energy=3;
  settings.automatic=false; settings.playMode=EyePlayMode::Manual;
  guarded.raster.clear(); guarded.raster.text(0,0,"v0.5.0");
  guarded.raster.text(0,20,"2026-10-16"); guarded.raster.text(0,40,"07:44:57Z UTC");
  assert(guarded.raster.pixels[1] != 0);
  // Rollover does not create nonfinite or offscreen poses.
  animator.update(settings,out,false,UINT32_MAX-20);
  f = animator.update(settings,out,false,40);
  assert(std::isfinite(f.left.x) && f.left.x >= 40 && f.left.x <= 88);
}
