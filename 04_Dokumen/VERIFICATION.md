# Verification evidence

Date: 2026-10-08. Baseline firmware: v0.1.0.

## Local results

- Native C++ control assertions: PASS under MSVC 14.51, C++17, /W4 /WX. Covers boot idle, command validation/ranges, NaN, old sequence, wrong session, expiry, manual takeover, maintenance lock, immediate stop, clock wrap, and reversal zero dwell.
- Python client: 6 tests PASS under Python 3.11. Covers fresh producer targets, expiry/release, no heartbeat on acquisition alone, atomic setter validation, network errors, and blocked HTTP I/O rejecting a stale target before transmission.
- Dashboard: 8 Chromium simulations PASS. Covers pointer release, blur, hidden tab, failed network, API rejection, stop/hand/laser controls, desktop and mobile layout. Tests serve the actual embedded HTML with simulated device responses, not a physical ESP32.
- Browser runtime: Playwright 1.58.0, locally available Chromium headless shell revision 1217 via PLAYWRIGHT_CHROMIUM_EXECUTABLE. CI installs the browser bundled with Playwright 1.58.0.
- Desktop (1100 px) and mobile (390 px) screenshots visually inspected; no horizontal overflow. Screenshots remain in ignored build/.
- Final Arduino CLI 1.4.1 / core 3.3.10 compile: PASS with 16 MiB flash / octal PSRAM configuration. Build parses the emitted partition binary and checks application size against two 6,291,456-byte OTA slots.
- Final application binary: 1,036,432 bytes. Program storage reported by CLI: 1,036,282 bytes (16% of slot). Global RAM: 51,400 bytes (15%); 276,280 bytes remain for runtime allocations.
- Binary contains `v0.1.0`. SHA256: `95965aed4f6b95fea76f358919fcdfE4ddf0e0e6a092dd0150da10be3bb06d61`.
- Tracked source has no absolute machine-specific Windows paths. Generated binaries, screenshots, native test output, and caches are ignored.

GitHub Actions is configured but has not run on GitHub because no remote/push is part of this task. Native and browser tests do not exercise actual ESP32 HTTP/OTA or GPIO; these require the physical acceptance below.

## USB upload and boot — verified

- Uploaded the existing v0.1.0 application through COM11 on 2026-10-08 using the pinned FQBN and build artifacts; esptool 5.3.0 verified written hashes for bootloader, partition table, OTA boot metadata, and application, then reset the board.
- ROM identification: ESP32-S3 revision v0.2, embedded PSRAM 8 MB; flash identification: 16 MB, quad flash, 3.3 V. Target matches N16R8.
- Serial at 115200 baud confirmed `AI Robot Controller v0.1.0`. No actuator-initialization failure or panic was observed in the 20-second boot capture.
- Fallback AP started with IP `192.168.4.1`; STA IP remained `0.0.0.0`, so router connectivity was not established. Serial `info` returned device credentials; secret values were withheld from tool output and tracked files.
- This confirms USB write, boot, and AP startup only. Dashboard/API over Wi-Fi, physical motion, timed stopping, and OTA transfer remain untested.

## Remaining physical device acceptance — not performed

- Physically verify idle outputs after the confirmed USB upload/boot; PSRAM initialization/allocation has not been measured despite ROM identifying 8 MB.
- Check both wheel polarities/PWM, left/right hand angles and pulse endpoints, laser on/off.
- Disconnect laptop/Wi-Fi, stop producer, send malformed/stale commands: verify outputs stop and old sessions cannot resume.
- Confirm manual takeover acknowledges stopped outputs and program commands cannot override manual.
- Load HTTP/status traffic while driving: measure expiry under 500 ms plus one 10 ms task period; timing cannot be certified from native tests.
- Upload valid OTA image; verify stopped outputs throughout upload, reboot idle, version, and reacquisition.
- Abort OTA, wrong image, oversized image, wrong authentication, unavailable router, and reboot during upload; verify recovery and no resumed motion.
- Confirm servo pulses remain disabled after failed OTA until a new valid complete command.
- Verify separate ArduinoOTA password, opt-in enablement, and reboot default OFF.

Compile/simulation success is not physical robot acceptance.
