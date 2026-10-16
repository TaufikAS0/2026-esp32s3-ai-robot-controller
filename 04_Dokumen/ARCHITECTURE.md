# Architecture and boundaries

`robot_controller.ino` delegates bootstrap/runtime to App. App composes NetworkService, ControlService, OtaService, and WebService. Drivers contain GPIO/LEDC access; portable ControlLogic contains lease, range, expiry, output, maintenance, and motor-ramp behavior.

WebServer and ArduinoOTA execute in the Arduino loop. A priority-3 FreeRTOS task on core 1 owns all actuator writes and checks expiry every 10 ms. A short spinlock protects only in-memory state snapshots. GPIO and flash work occurs outside it. Stop/acquire/OTA operations await a task generation acknowledgement (maximum 100 ms) before returning success. No network callback writes GPIO.

Control: boot idle → acquire → complete commands → stop/expiry → idle. Maintenance rejects acquire/command and suppresses all outputs. Normal stop holds servo angle; OTA explicitly disables its pulses. Hardware initialization failure locks control. A watchdog-reset device boots idle; no active lease is stored in NVS.

Motor direction reversal first ramps the old polarity to zero, holds zero for 40 ms, then ramps the new polarity. Ramp defaults to 0.04 normalized power per 10 ms tick. Stop/expiry bypass ramp to command zero immediately. Driver clears inactive H-bridge leg before writing active PWM. Physical braking, stopping distance, and actual polarity are unverified.

Shared lab defaults are centralized in network_defaults.h: STA HuaweiJIN and AP password 12345678. User explicitly authorized the lab password in source. Boot migrates only STA/AP NVS values that differ, preserving unrelated settings and removing obsolete apiToken/otaPassword keys. This lab build rejects other runtime Wi-Fi profiles. Fallback AP starts after 15 seconds without STA and remains on until reboot. Network recovery never restores a control lease. A client producer must explicitly renew desired targets; timer heartbeats alone must not keep stale AI targets alive. Build verifies lab configuration and locally reads the actual Obsidian rules/profile; this is a configuration check, not proof of an AI's reading behavior.

Dashboard has no external assets or CDN. Python uses only standard library. Firmware JSON uses ESP-IDF cJSON bundled with the pinned core. Version is firmware_version.h; API prefix is independent. Build uses core 3.3.10 and custom 16 MiB partition map with two 6 MiB app slots.

This is a local prototype controller without motion feedback, collision avoidance, TLS, automatic rollback, or independent hardware power cutoff. These are capabilities not provided by v0.1.0, not prerequisites added to the requested scope.

Access profile v0.2.0: all HTTP API routes and both OTA transports have no authentication, as explicitly authorized for this robot lab. ArduinoOTA remains OFF by default. Sessions, command ordering, timeout, and OTA maintenance interlocks remain enforced.

## v0.3.0 modes and driver verification

Volatile boot mode Manual gates manual ownership; Auto gates program ownership. Mode changes stop before acknowledgment and invalidate the lease. Dashboard acquires manual control on actuator interaction, polls status on load, retries network failures, and releases only its own session on focus/release/errors. A generation guard discards late acquisition responses. The driver maps left servo angle to 180-angle, checks LEDC writes, and snapshots peripheral duty/frequency after each write from the single output task. PWM failure locks control and suppresses stop/acquire acknowledgment until writes succeed. No physical sensor measurement is implied.

## Dual eyes v0.5.0

App also composes EyeService. A priority-1 task on core0 exclusively owns Wire and the SSD1306 driver/framebuffer; it samples a short control snapshot, then renders outside all locks at a nominal60 ms interval. Wire timeout10 ms, fail-fast data chunks and per-panel5 s retry keep bus failures bounded. Control output/expiry remains priority3/core1/10 ms. This task separation is a software design; actual timing under disconnected/stuck bus and OTA still requires hardware verification.

EyeAnimator is portable, motor/arm/laser-reactive shape logic; EyeRaster converts poses into clipped SSD1306 page pixels, including boot build identity. WebServer changes one complete validated EyeSettings snapshot through a separate short spinlock. No eye operation updates ControlLogic or NVS. OTA maintenance suppresses further eye bus work after the current transfer; no eye acknowledgement is required for actuator stop. Status online fields indicate last successful protocol transfer, not physical pixel feedback. Eye task failure leaves actuator control independent and reports task_ready=false.

v0.8.0: arm_motion.h contains portable reference interpolation; ControlLogic owns two interpolators and RAM limits. ControlService serializes limit configuration under its existing lock. ActuatorDriver remains the only GPIO/LEDC writer and maps logical wheel sides centrally. Web settings never directly write hardware or renew leases. Stop/expiry cancels interpolation, OTA disables pulses. arm_motion telemetry explicitly separates requested destination from interpolated output. No new runtime library, NVS schema or partitions.
