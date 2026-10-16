#pragma once
#include <cstring>
#include "eye_logic.h"
// SSD1306 page-format framebuffer. All drawing clips to the 128x64 panel.
class EyeRaster {
 public:
  uint8_t pixels[1024]{};
  void clear() { memset(pixels, 0, sizeof(pixels)); }
  void dot(int x, int y) { if (x >= 0 && x < 128 && y >= 0 && y < 64) pixels[x + (y / 8) * 128] |= uint8_t(1 << (y % 8)); }
  void eye(const EyePose& p, Expression expression, bool left) {
    clear();
    for (int y = 0; y < 64; ++y) for (int x = 0; x < 128; ++x) {
      float dx = (x - p.x) / (p.width * .5f), dy = (y - p.y) / (p.height * .5f);
      bool on = false;
      if (p.height <= 4) on = std::fabs(dx) < 1 && std::fabs(dy) < 1;
      else {
        const auto shape=eyeEntry(expression).shape;
        if(std::fabs(dx)>1.3f || std::fabs(dy)>1.3f) continue;
        const bool radial=shape==EyeShape::Star || shape==EyeShape::Ring || shape==EyeShape::Spiral || shape==EyeShape::Sharingan || shape==EyeShape::Rinnegan;
        const float r=radial ? std::sqrt(dx*dx+dy*dy) : 0;
        const float angle=radial ? std::atan2(dy,dx) : 0;
        on = dx*dx*dx*dx + dy*dy*dy*dy <= 1;
        if(shape==EyeShape::Heart) {
          const float yy=-dy*1.2f,xx=dx*1.15f,a=xx*xx+yy*yy-1;
          on=a*a*a-xx*xx*yy*yy*yy<=0;
        } else if(shape==EyeShape::Happy) on=on && dy<-.2f+.8f*dx*dx;
        else if(shape==EyeShape::Focus) on=on && dy>(left?.45f*dx:-.45f*dx)-.45f;
        else if(shape==EyeShape::Sad) on=on && dy>(left?-.4f*dx:.4f*dx)-.3f;
        else if(shape==EyeShape::Star) on=r<.65f+.28f*std::cos(5*(angle+p.phase*.25f));
        else if(shape==EyeShape::Ring) on=r<1 && r>.62f && (expression!=Expression::Loading || std::sin(angle-p.phase*5)>-.4f);
        else if(shape==EyeShape::Cross) on=std::fabs(dx)<1 && std::fabs(dy)<1 && std::fabs(std::fabs(dx)-std::fabs(dy))<.26f;
        else if(shape==EyeShape::Spiral) on=r<1 && std::sin(r*17-angle*2+p.phase*5)>.1f;
        else if(shape==EyeShape::Dollar) on=(std::fabs(dx)<.12f && std::fabs(dy)<1) || (std::fabs(dx-.58f*std::sin(dy*5))<.2f && std::fabs(dy)<.85f);
        else if(shape==EyeShape::Googly) {
          const float px=(x-p.x-p.pupilX)/8,py=(y-p.y-p.pupilY)/10;
          on=dx*dx+dy*dy<1 && px*px+py*py>1;
        }
        if(shape==EyeShape::Sharingan) {
          on=(r<1 && r>.86f) || (r<.62f && r>.56f) || r<.19f;
          for(unsigned t=0;t<3;++t) {
            const float theta=p.phase*.9f+t*2.0943951f;
            const float tx=.6f*std::cos(theta),ty=.6f*std::sin(theta);
            const float ux=dx-tx,uy=dy-ty;
            if(ux*ux+uy*uy<.028f) on=true;
            const float tailX=tx-.19f*std::sin(theta),tailY=ty+.19f*std::cos(theta);
            if((dx-tailX)*(dx-tailX)+(dy-tailY)*(dy-tailY)<.012f) on=true;
          }
        }
        if(shape==EyeShape::Rinnegan) {
          const float pulse=.015f*std::sin(p.phase*2);
          on=r<.13f;
          for(unsigned ring=1;ring<=4;++ring) if(std::fabs(r-(ring*.23f+pulse))<.035f) on=true;
        }
        if(shape==EyeShape::Cat || shape==EyeShape::Fox || shape==EyeShape::Snake || shape==EyeShape::Toad) {
          const float pupil=.10f+.04f*(1+std::sin(p.phase*1.4f));
          on=dx*dx+dy*dy<1;
          if(shape==EyeShape::Fox) on=on && dy>(left?.55f*dx:-.55f*dx)-.4f;
          if(shape==EyeShape::Snake) on=on && dy>-.6f+.3f*std::fabs(dx);
          const bool pupilOff=shape==EyeShape::Toad ? (std::fabs(dx)<.72f && std::fabs(dy)<.18f) :
            (std::fabs(dx)<(shape==EyeShape::Snake ? .07f : pupil)*(1-.5f*std::fabs(dy)) && std::fabs(dy)<.85f);
          if(pupilOff) on=false;
          if((dx+.35f)*(dx+.35f)+(dy+.3f)*(dy+.3f)<.012f) on=true;
        }
        if(expression==Expression::Glitch && (y+int(p.phase*20))%11<3) on=false;
        if(expression==Expression::Cry && std::fabs(dx)>.72f && std::fabs(dx)<.9f && dy>.35f) on=true;
      }
      if (on) dot(x, y);
    }
    if ((expression == Expression::Happy || expression == Expression::Love) && p.height > 4) {
      for (int i = 0; i < 3; ++i) for (int j = 0; j < 5; ++j) { dot(24 + i * 5 + j / 3, 48 + j); dot(94 + i * 5 + j / 3, 48 + j); }
    }
  }
  void text(int x, int y, const char* s) {
    // Compact identity font: only characters required by semver/ISO8601/UTC.
    static const uint8_t digits[10][5] = {
      {62,81,73,69,62},{0,66,127,64,0},{66,97,81,73,70},{33,65,69,75,49},{24,20,18,127,16},
      {39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30}};
    for (; *s; ++s, x += 6) {
      uint8_t glyph[5]{};
      if (*s >= '0' && *s <= '9') memcpy(glyph, digits[*s - '0'], 5);
      else if (*s == 'v' || *s == 'V') { const uint8_t g[5] = {31,32,64,32,31}; memcpy(glyph,g,5); }
      else if (*s == 'T') { const uint8_t g[5] = {1,1,127,1,1}; memcpy(glyph,g,5); }
      else if (*s == 'U') { const uint8_t g[5] = {63,64,64,64,63}; memcpy(glyph,g,5); }
      else if (*s == 'C') { const uint8_t g[5] = {62,65,65,65,34}; memcpy(glyph,g,5); }
      else if (*s == 'Z') { const uint8_t g[5] = {97,81,73,69,67}; memcpy(glyph,g,5); }
      else if (*s == 'L') { const uint8_t g[5] = {127,64,64,64,64}; memcpy(glyph,g,5); }
      else if (*s == 'R') { const uint8_t g[5] = {127,9,25,41,70}; memcpy(glyph,g,5); }
      else if (*s == 'D') { const uint8_t g[5] = {127,65,65,34,28}; memcpy(glyph,g,5); }
      else if (*s == '-') glyph[1]=glyph[2]=glyph[3]=8;
      else if (*s == ':') glyph[2]=36;
      else if (*s == '.') glyph[2]=64;
      for (int col=0;col<5;++col) for(int bit=0;bit<7;++bit) if(glyph[col] & (1<<bit)) dot(x+col,y+bit);
    }
  }
};
