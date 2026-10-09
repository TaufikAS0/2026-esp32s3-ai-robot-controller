# Verification evidence

Current firmware: v0.3.1 (2026-10-09). Earlier version evidence is historical.

## v0.3.1 dashboard, modes, mirrored hand and PWM — 2026-10-09

- Local pinned build PASS: ESP32 core 3.3.10 / Arduino CLI 1.4.1. Application 1,040,896 bytes; program storage 1,040,754 bytes; global RAM 51,440 bytes. Both emitted 6,291,456-byte OTA slots verified. Application SHA256 `ee53fff22e6c7452a3d7457589b4a562d00c5717bd464e50558396bdf764d2d2`.
- Native control and actual actuator-driver boundary assertions PASS with MSVC C++17 /W4 /WX. Eight Python client tests, seven network-policy tests, and fourteen Chromium dashboard tests PASS. Covers mode gates/stop, reversal, inactive-leg-first writes, mirroring endpoints/centre, zero boot PWM, full-on duty, write failure, automatic connection/recovery, mode controls, automatic acquisition, and release before a late acquisition response.
- OTA v0.3.0 and v0.3.1 through http://192.168.1.34/api/v1/update both returned success/rebooting without authentication. Post-reboot API confirmed each version, Manual mode, no lease, wheel duty zero, laser off, servo pulses disabled, ArduinoOTA OFF, and HuaweiJIN association at 192.168.1.34. API v0.3.1 reports wifi_sleep=false.
- Direct device Manual forward/reverse at 25% passed: active legs GPIO12/10 then GPIO13/11 read duty 256; inactive legs read zero; active frequency read 20,000 Hz. Switching to Auto read all wheel duties zero and invalidated the lease. Program acquisition in Manual and manual acquisition in Auto returned 409 as intended.
- Auto/Python live client passed: right-only wheel duty 256 and left duty zero; logical hands 60/60 mapped to left pulse-angle target 120 and right 60. Producer expiry released its lease and zeroed PWM. Separate raw commands without a client stop timed out and zeroed PWM. These are LEDC peripheral readbacks, not external voltage, waveforms, or measured motion.
- An initial v0.3.0 Auto test received 409; repeat passed. HTTP jitter reached 625 ms in observed requests. This did not establish the sole cause of that rejection. The v0.3.1 latency profile disables modem sleep; 28 requests in the final test measured 32–266 ms. This sample is not a latency guarantee.
- Real Chrome dashboard automatically showed Connected and v0.3.1 on page load. Auto disabled manual actuators; Manual re-enabled them. A left-slider keyboard step from 90 to 89 automatically acquired a manual lease and device readback confirmed mapped pulse-angle target 91. STOP invalidated the lease and zeroed wheel duty. Hands were returned to 90 and laser stayed off. Live screenshot and detailed device JSON are in ignored build/.
- Rules read: Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md (paths listed below for the previous policy rollout). STA/AP remain HuaweiJIN/12345678. Existing targeted NVS migration remains unchanged: only obsolete apiToken/otaPassword keys are removed, differing STA/AP keys are corrected, unrelated keys are retained; no full NVS erase.
- External GPIO waveforms, actual wheel movement/direction, actual hand alignment, failed/aborted OTA recovery, and endurance are not independently verified. No physical feedback sensor exists. A user observation has been requested separately.

## v0.2.0 open-LAN profile — 2026-10-09

- Pinned local build PASS: Arduino CLI 1.4.1 / ESP32 core 3.3.10, 16 MB flash / octal PSRAM. Application 1,035,408 bytes; program storage 1,035,262 bytes; global RAM 51,360 bytes. Both emitted OTA slots verified at 6,291,456 bytes. Application SHA256: `5cfd3e8701ed33400b89b8ce79c3522031f37797be87750b140bb1611603b474`.
- User explicitly authorizes anyone on the same reachable network to access control, settings, stop, and OTA without API token or OTA password. Sessions, sequence checks, command timeout, and maintenance stop interlocks remain enforced.
- Native C++ control assertions PASS (MSVC C++17 /W4 /WX). Seven Python client tests, seven network-policy tests, and nine actual-dashboard Chromium simulations PASS. HTTP client and browser requests contain no Authorization header; simulated dashboard OTA stops control before uploading. Actual ESP32 HTTP/OTA transport remains untested.
- Mobile dashboard screenshot visually reviewed after token removal; version v0.2.0 visible and controls fit the viewport.
- COM11 is absent in Arduino CLI enumeration and present-only Windows Ports inventory on this date. v0.2.0 is not uploaded to hardware; earlier COM11 results below belong to older versions.
- Mandatory vault files read: 00_Start_Here/Firmware_AI_Vault_Home.md; 01_Rules/Rules_Nama_dan_Folder_Workspace.md; 01_Rules/Rules_Firmware_Standar.md; 01_Rules/Rules_GitHub_Gitflow.md; 01_Rules/Rules_Kredensial_WiFi.md; 04_Profiles/WiFi/WiFi_HuaweiJIN.md; new decision 01_Rules/Rules_Akses_Robot_Lab.md.
- STA target remains HuaweiJIN with the approved shared lab password; fallback AP password remains 12345678. Boot migrates differing STA/AP keys and removes only obsolete apiToken/otaPassword keys; no full NVS erase. Physical migration and STA association for this version are not yet verified.
- Decision recorded in the local Obsidian vault and workspace/vault/template/project AGENTS entrypoints. Existing unrelated vault edits remain intact; the vault is not included in the public robot repository.

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
- Abort OTA, wrong image, oversized image, unauthenticated LAN access, unavailable router, and reboot during upload; verify recovery and no resumed motion.
- Confirm servo pulses remain disabled after failed OTA until a new valid complete command.
- Verify ArduinoOTA without a password, opt-in enablement, and reboot default OFF.

Compile/simulation success is not physical robot acceptance.
