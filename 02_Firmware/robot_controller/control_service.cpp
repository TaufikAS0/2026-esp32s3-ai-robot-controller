#include "control_service.h"
#include <esp_system.h>
#include <esp_random.h>
bool ControlService::begin() {
  if (xTaskCreatePinnedToCore(task, "actuators", 4096, this, 3, nullptr, 1) != pdPASS)
    return false;
  uint32_t start = millis();
  while (!initialized_ && millis() - start < 2000) delay(1);
  return initialized_ && hardwareReady_;
}
void ControlService::task(void* argument) {
  auto& self = *static_cast<ControlService*>(argument);
  self.hardwareReady_ = self.driver_.begin();
  if (!self.hardwareReady_) {
    self.logic_.maintenance = true;
    self.logic_.stop("pwm_init_failed", true);
  }
  self.initialized_ = true;
  TickType_t wake = xTaskGetTickCount();
  for (;;) {
    portENTER_CRITICAL(&self.mux_);
    self.logic_.tick(millis());
    Output out = self.logic_.output;
    uint32_t generation = self.generation_;
    portEXIT_CRITICAL(&self.mux_);
    bool writeOk = self.driver_.write(out);
    PwmSnapshot pwm = self.driver_.snapshot();
    portENTER_CRITICAL(&self.mux_);
    self.pwm_ = pwm;
    if (!writeOk && self.hardwareReady_) {
      self.logic_.maintenance = true;
      self.logic_.stop("pwm_write_failed", true);
    }
    self.appliedGeneration_ = generation;
    portEXIT_CRITICAL(&self.mux_);
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(Config::tickMs));
  }
}
bool ControlService::awaitApplied(uint32_t generation) {
  uint32_t start = millis();
  while (millis() - start < 100) {
    auto s = snapshot();
    if (s.appliedGeneration == generation) return s.pwm.writeOk;
    delay(1);
  }
  return false;
}
ControlSnapshot ControlService::snapshot() {
  portENTER_CRITICAL(&mux_);
  ControlSnapshot s{logic_, appliedGeneration_, pwm_};
  portEXIT_CRITICAL(&mux_);
  return s;
}
bool ControlService::acquire(Owner owner, uint32_t& session) {
  uint32_t id; do { id = esp_random(); } while (!id);
  portENTER_CRITICAL(&mux_);
  bool ok = logic_.acquire(owner, id, millis());
  uint32_t gen = ok ? ++generation_ : generation_;
  portEXIT_CRITICAL(&mux_);
  if (!ok) return false;
  if (!awaitApplied(gen)) { stop("task_unresponsive"); return false; }
  session = id; return true;
}
bool ControlService::setMode(ControlMode mode) {
  portENTER_CRITICAL(&mux_);
  bool ok = logic_.setMode(mode);
  uint32_t gen = ok ? ++generation_ : generation_;
  portEXIT_CRITICAL(&mux_);
  return ok && awaitApplied(gen);
}
bool ControlService::command(const Command& c) {
  portENTER_CRITICAL(&mux_);
  bool ok = logic_.command(c, millis());
  portEXIT_CRITICAL(&mux_);
  return ok;
}
bool ControlService::stop(const char* reason, bool disableServo) {
  portENTER_CRITICAL(&mux_);
  logic_.stop(reason, disableServo); uint32_t gen = ++generation_;
  portEXIT_CRITICAL(&mux_);
  return awaitApplied(gen);
}
bool ControlService::release(uint32_t session) {
  portENTER_CRITICAL(&mux_);
  bool ok = logic_.owner != Owner::None && logic_.session == session;
  if (ok) logic_.stop("released");
  uint32_t gen = ok ? ++generation_ : generation_;
  portEXIT_CRITICAL(&mux_);
  return ok && awaitApplied(gen);
}
bool ControlService::beginMaintenance() {
  portENTER_CRITICAL(&mux_);
  logic_.beginMaintenance(); uint32_t gen = ++generation_;
  portEXIT_CRITICAL(&mux_);
  return awaitApplied(gen);
}
void ControlService::endMaintenance() {
  portENTER_CRITICAL(&mux_);
  if (hardwareReady_) logic_.endMaintenance();
  ++generation_;
  portEXIT_CRITICAL(&mux_);
}
