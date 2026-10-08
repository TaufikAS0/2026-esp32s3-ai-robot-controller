# Verification evidence

Date: 2026-10-08. Current firmware: v0.1.1; earlier v0.1.0 evidence retained below.

## v0.1.1 lab profile and mandatory rule reading

- Build PASS with pinned Arduino CLI 1.4.1 / ESP32 core 3.3.10. Application: 1,037,024 bytes; program storage reported 1,036,874 bytes; global RAM 51,400 bytes. Both emitted OTA slots verified at 6,291,456 bytes each.
- Application SHA256: `6a11fb06b2e95f574ecb23f3f4e2a5a0f9acdee3e74d6be052cc040ce4d58114`.
- Seven network-policy tests PASS, six Python client tests PASS, eight Chromium dashboard simulations PASS. Control logic/drivers are unchanged from the native tests below.
- Build gate reads local Obsidian Rules_Kredensial_WiFi.md and WiFi_HuaweiJIN.md, checks approved constants and actual network source, and rejects random AP passwords or STA connection through an arbitrary old NVS profile. Standalone CI applies the approved source gate without requiring the sibling vault.
- Workspace AGENTS.md, vault home/AGENTS/template, and project AGENTS now explicitly require credential-rule/profile reading before firmware planning/reuse/changes/build/upload. This is an instruction and configuration gate, not proof of an AI's reading behavior. Other repositories are not retrofitted with this project's build gate.
- Uploaded v0.1.1 through COM11; esptool verified written flash hashes. Boot confirmed v0.1.1 and STA target HuaweiJIN.
- Serial verified AP password is 12345678. API token and OTA password fingerprints match their pre-upload values; no erase-all or secret rotation was performed.
- Station connection NOT verified: after boot, STA connected=false and IP 0.0.0.0; fallback AP active at 192.168.4.1. Laptop is connected to HuaweiJIN and its saved password matches the requested lab password. This does not prove ESP32 compatibility/association; cause remains undetermined. No router settings or laptop network association were changed.
- Vault/workspace rule updates are local; pre-existing dirty vault edits are preserved and are not included in the robot repository commit. No GitHub push was performed.

## Local results — v0.1.0

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
