# Changelog

## v0.1.1 — 2026-10-08

- Correct shared AP password to 12345678 and embed the user-approved HuaweiJIN lab STA profile.
- Migrate old STA/AP NVS values without erasing API/OTA secrets; reject other runtime Wi-Fi profiles in this lab build.
- Expose target/active SSID in status and serial info; require Obsidian credential-rule reading in agent entrypoints.
- Add a build/CI gate rejecting incorrect lab network defaults.

## v0.1.0 — 2026-10-08

- Initial ESP32-S3 robot motor, hand servo, and laser controller.
- Authenticated v1 HTTP API, manual dashboard, and Python client with producer expiry.
- Independent actuator task, expiring control sessions, motor ramp/reversal, and manual takeover.
- STA with recovery AP, NVS settings, browser OTA, and opt-in ArduinoOTA.
- Pinned build, two OTA slots, logic/client/browser tests, and GitHub Actions validation.

Pre-release baseline; physical device acceptance is pending.
