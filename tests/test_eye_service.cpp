#include <cassert>
#include <vector>
#include <array>
#include <algorithm>
#include "../02_Firmware/robot_controller/eye_service.h"
FakeWire Wire;
namespace {
struct EndTask {};
uint32_t clockMs = 0;
unsigned ticks = 0, limit = 0, snapshotCalls = 0;
void (*callback)(void*) = nullptr;
void* argument = nullptr;
bool busReady = true, dataFailure = false, maintenance = false;
bool changeRotation = false;
int failedAddress = -1, extraAddress = -1;
uint8_t activeAddress = 0;
std::vector<uint8_t> packet;
struct Transfer { uint8_t address; std::vector<uint8_t> data; };
std::vector<Transfer> transfers;
std::array<std::vector<uint8_t>, 128> images;
Output output;
void reset() {
  busReady = true; dataFailure = false; maintenance = false; failedAddress = extraAddress = -1;
  changeRotation = false;
  snapshotCalls = 0; transfers.clear(); for (auto& image : images) image.clear();
  output = Output{};
}
void run(EyeService& eyes, unsigned frames, uint32_t start = 5000) {
  clockMs = start; ticks = 0; limit = frames;
  assert(eyes.begin());
  try { callback(argument); } catch (const EndTask&) {}
}
}
bool FakeWire::begin(uint8_t sda, uint8_t scl, uint32_t hz) {
  assert(sda == 8 && scl == 9 && hz == 400000); return busReady;
}
void FakeWire::setClock(uint32_t hz) { assert(hz==100000 || hz==400000); }
void FakeWire::setTimeOut(uint16_t ms) { assert(ms == 10); }
void FakeWire::beginTransmission(uint8_t address) { activeAddress = address; packet.clear(); }
size_t FakeWire::write(uint8_t byte) { packet.push_back(byte); return 1; }
size_t FakeWire::write(const uint8_t* bytes, size_t count) {
  packet.insert(packet.end(), bytes, bytes+count); return count;
}
uint8_t FakeWire::endTransmission() {
  assert(packet.size() <= 33);
  if(packet.empty()) return (activeAddress==0x3C || activeAddress==0x3D || activeAddress==extraAddress) && activeAddress!=failedAddress ? 0 : 2;
  assert(activeAddress == 0x3C || activeAddress == 0x3D);
  transfers.push_back({activeAddress,packet});
  if (activeAddress == failedAddress || (dataFailure && activeAddress == 0x3D && packet[0] == 0x40)) return 2;
  if (packet[0] == 0 && packet.size() > 1 && packet[1] == 0x21) images[activeAddress].clear();
  if (packet[0] == 0x40) images[activeAddress].insert(images[activeAddress].end(),packet.begin()+1,packet.end());
  return 0;
}
uint32_t millis() { return clockMs; }
TickType_t xTaskGetTickCount() { return clockMs; }
int xTaskCreatePinnedToCore(void (*fn)(void*), const char*, uint32_t, void* arg, int priority, void*, int core) {
  assert(priority == 1 && core == 0); callback = fn; argument = arg; return pdPASS;
}
void vTaskDelayUntil(TickType_t* wake, TickType_t period) {
  *wake += period; clockMs = *wake;
  if (++ticks >= limit) throw EndTask{};
}
ControlSnapshot ControlService::snapshot() {
  ++snapshotCalls;
  if(changeRotation && snapshotCalls==2) {
    EyeSettings settings; settings.rotateLeft=false; settings.rotateRight=true;
    static_cast<EyeService*>(argument)->configure(settings);
  }
  ControlSnapshot s{}; s.logic.output = output; s.logic.maintenance = maintenance; return s;
}
int main() {
  ControlService control;
  reset(); output.left = -1; output.right = 1;
  EyeService normal(control); run(normal,2);
  assert(normal.snapshot().leftOnline && normal.snapshot().rightOnline);
  assert(images[0x3C].size() == 1024 && images[0x3D].size() == 1024);
  const auto left = images[0x3D], right = images[0x3C];
  assert(left != right); // Curious eye asymmetry.
  EyeService swapped(control); EyeSettings settings; settings.swap = true;
  assert(swapped.configure(settings)); run(swapped,2);
  assert(images[0x3D] == right && images[0x3C] == left);

  reset(); EyeService oriented(control); settings=EyeSettings{};
  settings.rotateLeft=true;settings.rotateRight=false;assert(oriented.configure(settings));run(oriented,2);
  bool leftRotation=false,rightNormal=false;
  for(const auto& tx:transfers) if(tx.data.size()>20 && tx.data[1]==0xAE) {
    if(tx.address==0x3D) leftRotation=std::find(tx.data.begin(),tx.data.end(),0xA0)!=tx.data.end() && std::find(tx.data.begin(),tx.data.end(),0xC0)!=tx.data.end();
    if(tx.address==0x3C) rightNormal=std::find(tx.data.begin(),tx.data.end(),0xA1)!=tx.data.end() && std::find(tx.data.begin(),tx.data.end(),0xC8)!=tx.data.end();
  }
  assert(leftRotation && rightNormal);
  reset(); changeRotation=true; EyeService runtimeRotation(control);run(runtimeRotation,4);
  unsigned leftChanges=0,rightChanges=0;
  for(const auto& tx:transfers) if(tx.data==std::vector<uint8_t>{0,0xA1,0xC8}) {
    if(tx.address==0x3D) ++leftChanges; else ++rightChanges;
  }
  assert(leftChanges==1 && rightChanges==0);
  reset(); failedAddress = 0x3D;
  EyeService absent(control); run(absent,100,0);
  assert(!absent.snapshot().leftOnline && absent.snapshot().rightOnline);
  assert(absent.snapshot().errors == 2); // Only boot and the 5-second retry.
  assert(snapshotCalls == 100 && absent.snapshot().frames == 100);

  reset(); dataFailure = true;
  EyeService brokenData(control); run(brokenData,2);
  unsigned failedDataChunks = 0;
  for (const auto& tx : transfers) if (tx.address == 0x3D && tx.data[0] == 0x40) ++failedDataChunks;
  assert(failedDataChunks == 1); // Abort immediately, not 32 repeated timeouts.
  assert(!brokenData.snapshot().leftOnline && brokenData.snapshot().rightOnline);

  reset(); EyeService scanner(control);
  assert(scanner.check(EyeCheckMode::Scan,EyeTarget::Both)); run(scanner,113,0);
  auto scan=scanner.snapshot().check;
  assert(scan.scanComplete && scan.scanned==112 && scan.leftProbe==0 && scan.rightProbe==0);
  assert(scan.found[0x3C/8] & (1<<(0x3C%8)));
  assert(scan.found[0x3D/8] & (1<<(0x3D%8)));
  assert(scan.mode==EyeCheckMode::Animate && snapshotCalls==113);
  reset(); extraAddress=0x42; EyeService unknown(control);
  unknown.check(EyeCheckMode::Scan,EyeTarget::Both); run(unknown,112,0);
  assert(unknown.snapshot().check.found[0x42/8] & (1<<(0x42%8)));
  assert(transfers.empty()); // Probe unknown devices; never send OLED initialization to them.
  reset(); failedAddress=0x3D; EyeService one(control);
  one.check(EyeCheckMode::Scan,EyeTarget::Both); run(one,112,0);
  assert(one.snapshot().check.leftProbe==2 && one.snapshot().check.rightProbe==0);
  reset(); EyeService white(control);
  white.check(EyeCheckMode::White,EyeTarget::Left); run(white,2);
  for(auto byte:images[0x3D]) assert(byte==255);
  assert(images[0x3C]!=images[0x3D]);
  reset(); EyeService timed(control);
  timed.check(EyeCheckMode::White,EyeTarget::Both); run(timed,90,0);
  assert(timed.snapshot().check.mode==EyeCheckMode::Animate && timed.snapshot().check.remainingMs==0);
  reset(); EyeService checker(control);
  checker.check(EyeCheckMode::Checker,EyeTarget::Both); run(checker,2);
  assert(images[0x3D][0]!=images[0x3D][8]);
  reset(); EyeService identity(control);
  identity.check(EyeCheckMode::Identity,EyeTarget::Both); run(identity,2);
  assert(images[0x3D]!=images[0x3C]);
  reset(); maintenance = true;
  EyeService ota(control); run(ota,2); assert(transfers.empty());
  reset(); busReady = false;
  EyeService noBus(control); run(noBus,2);
  assert(!noBus.snapshot().taskReady && transfers.empty());
  reset(); EyeService disabled(control); settings.enabled = false;
  assert(disabled.configure(settings)); run(disabled,2);
  for (const auto& tx : transfers) assert(tx.data[0] != 0x40);
}
