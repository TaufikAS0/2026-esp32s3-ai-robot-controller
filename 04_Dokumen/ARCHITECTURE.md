# Architecture and boundaries

`robot_controller.ino` delegates bootstrap/runtime to App. App composes NetworkService, ControlService, OtaService, and WebService. Drivers contain GPIO/LEDC access; portable ControlLogic contains lease, range, expiry, output, maintenance, and motor-ramp behavior.

WebServer and ArduinoOTA execute in the Arduino loop. A priority-3 FreeRTOS task on core 1 owns all actuator writes and checks expiry every 10 ms. A short spinlock protects only in-memory state snapshots. GPIO and flash work occurs outside it. Stop/acquire/OTA operations await a task generation acknowledgement (maximum 100 ms) before returning success. No network callback writes GPIO.

Control: boot idle → acquire → complete commands → stop/expiry → idle. Maintenance rejects acquire/command and suppresses all outputs. Normal stop holds servo angle; OTA explicitly disables its pulses. Hardware initialization failure locks control. A watchdog-reset device boots idle; no active lease is stored in NVS.

Motor direction reversal first ramps the old polarity to zero, holds zero for 40 ms, then ramps the new polarity. Ramp defaults to 0.04 normalized power per 10 ms tick. Stop/expiry bypass ramp to command zero immediately. Driver clears inactive H-bridge leg before writing active PWM. Physical braking, stopping distance, and actual polarity are unverified.

Shared lab defaults are centralized in network_defaults.h: STA HuaweiJIN and AP password 12345678. User explicitly authorized the lab password in source. Boot migrates only STA/AP NVS values that differ, preserving unrelated settings and removing obsolete apiToken/otaPassword keys. This lab build rejects other runtime Wi-Fi profiles. Fallback AP starts after 15 seconds without STA and remains on until reboot. Network recovery never restores a control lease. A client producer must explicitly renew desired targets; timer heartbeats alone must not keep stale AI targets alive. Build verifies lab configuration and locally reads the actual Obsidian rules/profile; this is a configuration check, not proof of an AI's reading behavior.

Dashboard has no external assets or CDN. Python uses only standard library. Firmware JSON uses ESP-IDF cJSON bundled with the pinned core. Version is firmware_version.h; API prefix is independent. Build uses core 3.3.10 and custom 16 MiB partition map with two 6 MiB app slots.

This is a local prototype controller without motion feedback, collision avoidance, TLS, automatic rollback, or independent hardware power cutoff. These are capabilities not provided by v0.1.0, not prerequisites added to the requested scope.

Access profile v0.2.0: all HTTP API routes and both OTA transports have no authentication, as explicitly authorized for this robot lab. ArduinoOTA remains OFF by default. Sessions, command ordering, timeout, and OTA maintenance interlocks remain enforced.
