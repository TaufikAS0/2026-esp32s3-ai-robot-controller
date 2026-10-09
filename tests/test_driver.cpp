#include <cassert>
#include <vector>
#include <utility>
#include "../02_Firmware/robot_controller/actuator_driver.h"
struct Pin { uint32_t hz = 0, duty = 0; uint8_t channel = 0; bool attached = false; };
Pin pins[64];
std::vector<std::pair<uint8_t, uint32_t>> writes;
int failPin = -1, laser = 0;
void pinMode(uint8_t, int) {}
void digitalWrite(uint8_t p, int v) { if (p == Config::laser) laser = v; }
bool ledcAttachChannel(uint8_t p, uint32_t hz, uint8_t, uint8_t channel) {
  pins[p] = {hz, 0, channel, true}; return true;
}
bool ledcWrite(uint8_t p, uint32_t duty) {
  writes.emplace_back(p, duty);
  if (p == failPin || !pins[p].attached) return false;
  pins[p].duty = (duty == 1023 && pins[p].hz == Config::motorHz) ? 1024 : duty; return true;
}
uint32_t ledcRead(uint8_t p) { return pins[p].duty; }
uint32_t ledcReadFreq(uint8_t p) { return pins[p].duty ? pins[p].hz : 0; }
int main() {
  ActuatorDriver d; assert(d.begin()); Output out;
  assert(d.write(out)); assert(pins[5].duty == 0 && pins[6].duty == 0);
  assert(pins[12].hz == 20000 && pins[5].hz == 50);
  assert(pins[12].channel / 2 != pins[5].channel / 2);
  out.left = .25f; out.right = -.5f; out.servoEnabled = true;
  out.armLeft = 0; out.armRight = 0; writes.clear(); assert(d.write(out));
  assert(writes[0].first == 13 && writes[0].second == 0);
  assert(writes[1].first == 12 && writes[1].second == 256);
  assert(pins[10].duty == 0 && pins[11].duty == 512);
  assert(pins[5].duty == 2048 && pins[6].duty == 410);
  auto state = d.snapshot(); assert(state.ready && state.writeOk);
  assert(state.left1 == 256 && state.right2 == 512 && state.leftHz == 20000);
  assert(state.armLeftPulseAngle == 180 && state.armRightPulseAngle == 0);
  out.armLeft = 180; out.armRight = 180; assert(d.write(out));
  assert(pins[5].duty == 410 && pins[6].duty == 2048);
  out.armLeft = out.armRight = 90; assert(d.write(out));
  assert(pins[5].duty == pins[6].duty);
  out.left = out.right = 0; out.servoEnabled = false; assert(d.write(out));
  assert(pins[10].duty == 0 && pins[11].duty == 0 && pins[12].duty == 0 && pins[13].duty == 0);
  assert(pins[5].duty == 0 && pins[6].duty == 0);
  out.left = out.right = 1; assert(d.write(out));
  assert(d.snapshot().left1 == 1024 && d.snapshot().right1 == 1024);
  out.left = out.right = 0; assert(d.write(out));
  failPin = 13; out.left = 1; assert(!d.write(out));
  assert(pins[12].duty == 0 && !d.snapshot().writeOk);
}
