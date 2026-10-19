#include "eye_service.h"
#include "firmware_version.h"
bool EyeService::begin() {
  return xTaskCreatePinnedToCore(task, "eyes", 6144, this, 1, nullptr, 0) == pdPASS;
}
bool EyeService::configure(const EyeSettings& settings) {
  if (!EyeSettings::valid(settings)) return false;
  portENTER_CRITICAL(&mux_); state_.settings = settings; portEXIT_CRITICAL(&mux_);
  return true;
}
bool EyeService::check(EyeCheckMode mode, EyeTarget target) {
  if (static_cast<unsigned>(mode)>5 || static_cast<unsigned>(target)>2) return false;
  portENTER_CRITICAL(&mux_);
  requestedMode_=mode; requestedTarget_=target; ++requested_;
  portEXIT_CRITICAL(&mux_);
  return true;
}
EyeSnapshot EyeService::snapshot() {
  portENTER_CRITICAL(&mux_); auto result = state_; portEXIT_CRITICAL(&mux_);
  return result;
}
bool EyeService::command(uint8_t address, const uint8_t* data, size_t size) {
  Wire.beginTransmission(address); Wire.write(uint8_t(0)); Wire.write(data, size);
  return Wire.endTransmission() == 0;
}
bool EyeService::initialize(uint8_t address, uint8_t brightness, bool rotated) {
  // SSD1306 128x64, internal charge pump, horizontal addressing.
  const uint8_t init[] = {0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0,0x40,0x8D,0x14,
    0x20,0,uint8_t(rotated ? 0xA0 : 0xA1),uint8_t(rotated ? 0xC0 : 0xC8),0xDA,0x12,0x81,brightness,0xD9,0xF1,0xDB,0x40,0xA4,0xA6,0xAF};
  return command(address, init, sizeof(init));
}
bool EyeService::display(uint8_t address, const EyeRaster& raster) {
  const uint8_t region[] = {0x21,0,127,0x22,0,7};
  if (!command(address, region, sizeof(region))) return false;
  for (size_t offset=0; offset<1024; offset+=32) {
    Wire.beginTransmission(address); Wire.write(uint8_t(0x40)); Wire.write(raster.pixels+offset,32);
    if (Wire.endTransmission() != 0) return false; // Fail fast, no 32 timeouts/frame.
  }
  return true;
}
void EyeService::task(void* argument) {
  auto& self = *static_cast<EyeService*>(argument);
  const bool bus = Wire.begin(Config::eyeSda, Config::eyeScl, Config::eyeBusHz);
  Wire.setTimeOut(10);
  portENTER_CRITICAL(&self.mux_); self.state_.taskReady = bus; portEXIT_CRITICAL(&self.mux_);
  EyeAnimator animator; EyeRaster raster;
  bool online[2] = {false,false};
  bool enabled[2] = {true,true}, rotated[2] = {false,false};
  uint8_t brightness[2] = {0,0};
  uint32_t retry[2] = {0,0}, frames=0, errors=0;
  EyeCheckSnapshot check;
  uint32_t testUntil = 0;
  const uint8_t addresses[] = {Config::eyeLeftAddress,Config::eyeRightAddress};
  TickType_t wake = xTaskGetTickCount();
  for (;;) {
    const uint32_t startedUs=micros();
    auto settings = self.snapshot().settings;
    const auto control = self.control_.snapshot();
    if(control.logic.demoOwned&&control.logic.demoEye) {
      settings.automatic=false;settings.playMode=EyePlayMode::Manual;
      settings.expression=control.logic.demoEye==1?Expression::Happy:control.logic.demoEye==2?Expression::WinkLeft:Expression::Curious;
    }
    const uint32_t now = millis();
    portENTER_CRITICAL(&self.mux_);
    const auto requestId=self.requested_;
    const auto requestMode=self.requestedMode_;
    const auto requestTarget=self.requestedTarget_;
    portEXIT_CRITICAL(&self.mux_);
    if (!control.logic.maintenance && requestId!=check.request) {
      check.request=requestId; check.mode=requestMode; check.target=requestTarget;
      testUntil=0;
      for(unsigned i=0;i<2;++i) { online[i]=false; retry[i]=now; }
      Wire.setClock(requestMode==EyeCheckMode::Scan ? 100000 : Config::eyeBusHz);
      if(requestMode==EyeCheckMode::Scan) {
        check.scanned=0; check.scanComplete=false; memset(check.found,0,sizeof(check.found));
        check.leftProbe=check.rightProbe=255;
      } else if(requestMode==EyeCheckMode::Reinit) check.mode=EyeCheckMode::Animate;
      else if(requestMode!=EyeCheckMode::Animate) testUntil=now+5000;
    }
    if(testUntil && int32_t(now-testUntil)>=0) {
      check.mode=EyeCheckMode::Animate; testUntil=0;
      for(unsigned i=0;i<2;++i) { online[i]=false; retry[i]=now; }
    }
    check.remainingMs=testUntil ? uint32_t(testUntil-now) : 0;
    // One 7-bit probe per iteration: no long scan in HTTP or actuator tasks.
    const bool scanning=check.mode==EyeCheckMode::Scan;
    if(bus && scanning && !control.logic.maintenance) {
      const uint8_t address=uint8_t(8+check.scanned);
      Wire.beginTransmission(address);
      const uint8_t result=Wire.endTransmission();
      if(result==0) check.found[address/8] |= uint8_t(1<<(address%8));
      if(address==Config::eyeLeftAddress) check.leftProbe=result;
      if(address==Config::eyeRightAddress) check.rightProbe=result;
      if(++check.scanned==112) {
        check.scanComplete=true; check.lastScanMs=now; check.mode=EyeCheckMode::Animate;
        Wire.setClock(Config::eyeBusHz);
      }
    }
    const auto frame = animator.update(settings, control.logic.output, control.logic.maintenance, now);
    for (unsigned i=0;bus && !scanning && !control.logic.maintenance && i<2;++i) {
      if (!online[i]) {
        if (int32_t(now-retry[i]) < 0) continue;
        rotated[i] = i==0 ? settings.rotateLeft : settings.rotateRight;
        online[i] = self.initialize(addresses[i], settings.brightness,rotated[i]);
        brightness[i] = settings.brightness; enabled[i] = true;
        if (!online[i]) { ++errors; retry[i]=now+5000; continue; }
      }
      const bool test = testUntil && (check.target==EyeTarget::Both ||
        (i==0 && check.target==EyeTarget::Left) || (i==1 && check.target==EyeTarget::Right));
      const bool wantedEnabled=settings.enabled || test;
      const uint8_t wantedBrightness=test ? 255 : settings.brightness;
      bool ok = true;
      const bool wantedRotation=i==0 ? settings.rotateLeft : settings.rotateRight;
      if(rotated[i]!=wantedRotation) {
        const uint8_t commands[]={uint8_t(wantedRotation ? 0xA0 : 0xA1),uint8_t(wantedRotation ? 0xC0 : 0xC8)};
        ok=self.command(addresses[i],commands,2);
        if(ok) rotated[i]=wantedRotation;
      }
      if (ok && enabled[i] != wantedEnabled) {
        const uint8_t cmd = wantedEnabled ? 0xAF : 0xAE;
        ok = self.command(addresses[i], &cmd, 1); enabled[i] = wantedEnabled;
      }
      if (ok && brightness[i] != wantedBrightness) {
        const uint8_t cmd[] = {0x81,wantedBrightness};
        ok = self.command(addresses[i],cmd,2); brightness[i]=wantedBrightness;
      }
      if (ok && wantedEnabled && !control.logic.maintenance) {
        if(test && check.mode==EyeCheckMode::White) memset(raster.pixels,255,sizeof(raster.pixels));
        else if(test && check.mode==EyeCheckMode::Checker) {
          raster.clear();
          for(int y=0;y<64;++y) for(int x=0;x<128;++x)
            if(((x/8+y/8+int((now/500)%2))%2)==0) raster.dot(x,y);
        } else if(test && check.mode==EyeCheckMode::Identity) {
          raster.clear(); raster.text(4,0,i==0 ? "L 3D" : "R 3C");
          raster.text(4,12,kFirmwareVersion);
          char date[11]; memcpy(date,kFirmwareBuildUtc,10); date[10]=0;
          raster.text(4,28,date); raster.text(4,42,kFirmwareBuildUtc+11);
          for(int x=0;x<128;++x) raster.dot(x,63);
        } else if (now < 4000) {
          raster.clear(); raster.text(4,8,kFirmwareVersion); raster.text(4,24,"UTC");
          char date[11]; memcpy(date,kFirmwareBuildUtc,10); date[10]=0;
          raster.text(4,36,date); raster.text(4,48,kFirmwareBuildUtc+11);
        } else {
          const bool logicalLeft = (i == 0) != settings.swap;
          raster.eye(logicalLeft ? frame.left : frame.right,frame.expression,logicalLeft);
        }
        ok = self.display(addresses[i],raster);
      }
      if (!ok) { online[i]=false; ++errors; retry[i]=now+5000; }
    }
    ++frames;
    const uint32_t workUs=micros()-startedUs;
    portENTER_CRITICAL(&self.mux_);
    self.state_.check=check;
    self.state_.frame=frame; self.state_.leftOnline=online[0]; self.state_.rightOnline=online[1];
    self.state_.errors=errors; self.state_.frames=frames;
    self.state_.workUs=workUs;
    self.state_.maxWorkUs=std::max(self.state_.maxWorkUs,workUs);
    if(workUs>=Config::eyeFrameMs*1000) ++self.state_.overruns;
    portEXIT_CRITICAL(&self.mux_);
    // Missed frames are dropped. Never replay a backlog against Wi-Fi on core0.
    if(workUs>=Config::eyeFrameMs*1000) wake=xTaskGetTickCount();
    vTaskDelayUntil(&wake,pdMS_TO_TICKS(Config::eyeFrameMs));
  }
}
