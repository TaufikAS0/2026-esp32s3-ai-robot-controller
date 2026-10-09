# Changelog

## v0.3.1 — 2026-10-09

- Disable Wi-Fi modem sleep for LAN control after device tests observed HTTP jitter close to the 500 ms lease. Expose Wi-Fi sleep state in status.


## v0.3.0 — 2026-10-09

- Connect dashboard status automatically, retry failures, and show an explicit connection state.
- Add explicit Auto/Manual modes; Manual boot default, mode changes stop and invalidate the session. Python clients explicitly select Auto before program acquisition.
- Acquire manual ownership on button/slider interaction; release only the dashboard session on blur, hide, and button release.
- Mirror left servo pulse angle centrally as 180 minus logical angle.
- Check LEDC write results and report driver duty/frequency readback; add actuator-boundary and mode/recovery/race tests.


## v0.2.0 â€” 2026-10-09

- Explicit open-LAN robot profile: remove API bearer-token checks and both OTA passwords.
- Remove dashboard token input and Python token requirement (legacy argument ignored); report open_lan access mode.
- Migrate NVS by removing only obsolete API/OTA authentication keys, retaining approved lab Wi-Fi and unrelated settings.
- Preserve sessions, sequence validation, producer expiry, command timeout, and OTA stop interlocks.
- Document the user-approved decision in Obsidian and publish under MIT.

## v0.1.1 â€” 2026-10-08

- Correct shared AP password to 12345678 and embed the user-approved HuaweiJIN lab STA profile.
- Migrate old STA/AP NVS values without erasing API/OTA secrets; reject other runtime Wi-Fi profiles in this lab build.
- Expose target/active SSID in status and serial info; require Obsidian credential-rule reading in agent entrypoints.
- Add a build/CI gate rejecting incorrect lab network defaults.

## v0.1.0 â€” 2026-10-08

- Initial ESP32-S3 robot motor, hand servo, and laser controller.
- Authenticated v1 HTTP API, manual dashboard, and Python client with producer expiry.
- Independent actuator task, expiring control sessions, motor ramp/reversal, and manual takeover.
- STA with recovery AP, NVS settings, browser OTA, and opt-in ArduinoOTA.
- Pinned build, two OTA slots, logic/client/browser tests, and GitHub Actions validation.

Pre-release baseline; physical device acceptance is pending.
