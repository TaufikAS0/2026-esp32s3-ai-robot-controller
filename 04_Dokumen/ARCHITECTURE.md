# Architecture and boundaries

`robot_controller.ino` delegates bootstrap/runtime to App. App composes NetworkService, ControlService, OtaService, and WebService. Drivers contain GPIO/LEDC access; portable ControlLogic contains lease, range, expiry, output, maintenance, and motor-ramp behavior.

WebServer and ArduinoOTA execute in the Arduino loop. A priority-3 FreeRTOS task on core 1 owns all actuator writes and checks expiry every 10 ms. A short spinlock protects only in-memory state snapshots. GPIO and flash work occurs outside it. Stop/acquire/OTA operations await a task generation acknowledgement (maximum 100 ms) before returning success. No network callback writes GPIO.

Control: boot idle → acquire → complete commands → stop/expiry → idle. Maintenance rejects acquire/command and suppresses all outputs. Normal stop holds servo angle; OTA explicitly disables its pulses. Hardware initialization failure locks control. A watchdog-reset device boots idle; no active lease is stored in NVS.

Motor direction reversal first ramps the old polarity to zero, holds zero for 40 ms, then ramps the new polarity. Ramp defaults to 0.04 normalized power per 10 ms tick. Stop/expiry bypass ramp to command zero immediately. Driver clears inactive H-bridge leg before writing active PWM. Physical braking, stopping distance, and actual polarity are unverified.

Wi-Fi credentials and independent generated API/AP/OTA secrets are in NVS. Fallback AP starts after 15 seconds without STA and remains on until reboot. Network recovery never restores a control lease. A client producer must explicitly renew desired targets; timer heartbeats alone must not keep stale AI targets alive.

Dashboard has no external assets or CDN. Python uses only standard library. Firmware JSON uses ESP-IDF cJSON bundled with the pinned core. Version is firmware_version.h; API prefix is independent. Build uses core 3.3.10 and custom 16 MiB partition map with two 6 MiB app slots.

This is a local prototype controller without motion feedback, collision avoidance, TLS, automatic rollback, or independent hardware power cutoff. These are capabilities not provided by v0.1.0, not prerequisites added to the requested scope.
