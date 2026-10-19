# AI Robot Controller Rules

v0.12.0 user decision19 October2026 supersedes the fixed-SSID-only rules below:
read04_Dokumen/WIFI_PROFILES_V012.md. Up to5 user-saved ordered profiles in
robot/wifiProfilesV1 are authoritative after one-time legacy/default migration;
never overwrite them at boot with HuaweiJIN. Lab defaults remain first-install
fallback, AP12345678 stays, open-access unchanged. Scan/manual config only idle;
network_busy must atomically gate HTTP/TCP acquisition/commands and OTA. Lost
STA invalidates control; no automatic target/session resumption. No Wi-Fi
passwords in status/logs/Git; no NVS clear. Pin/partition mapping stays unchanged.

v0.11.0 adds TCP9000: read04_Dokumen/TCP_CONTROL_V011.md. Sessions are bound to
the socket generation and cannot be driven/released through HTTP. Keep explicit
Auto,500 ms valid-command expiry, bounded framing/send deadlines and no command
pipeline/replay. User confirmed unknown initial physical servo pose: do not enable
servo/laser during device verification until a known accepted pose is supplied.

v0.10.0 servo follow-up: read `04_Dokumen/SERVO_CONTROL_V010.md` and the latest
verification. Natural60/240, per-arm RAM limits and optional forward/reverse
dynamic hands; dynamic OFF at boot. Never let a dynamic gesture write wheels.
HTTP81 isolates control from dashboard/OTA80; do not extend500 ms expiry.
The old24-hour run failed partially; monitor PAUSED, no automatic replacement.

v0.9.0 continuation: read `04_Dokumen/SOAK.md` and the latest VERIFICATION entry.
One durable zero-wheel runner only; monitor checkpoints, never auto-restart an
actuator producer or flash unattended. First command after disabled pulses seeds
the supplied accepted arm reference directly; boot90 is not a physical position.
Sharingan has its own category;52 presets/22 box/30 special. Keep original IDs.

Before the next change, read `04_Dokumen/HANDOFF_CONNECTION_EYES.md`: open exhibition timeout/servo+laser disconnect, Kurama clipping and Sharingan work. User prohibits wheel motion during tests (all wheel commands zero); servo and laser test scope, 24-hour soak and continuation automation instructions are in that handoff. Publishing the handoff does not mean those fixes or the soak are complete.

Current eyes work v0.7.0: read 04_Dokumen/EYES_DESIGN.md and latest VERIFICATION.md before changing OLEDs. eye_catalog.h is the single preset catalog; web loads it via /eyes/catalog. Keep diagnostic I2C in EyeService and keep cosmetic requests separate from actuator sessions. Do not infer two physical devices from one scan ACK; no automatic address remap.


- Read README and 04_Dokumen/API.md before changing control behavior.
- Before planning, connectivity changes, build/upload, read the sibling vault's 01_Rules/Rules_Kredensial_WiFi.md and 04_Profiles/WiFi/WiFi_HuaweiJIN.md. Locate the vault if checkout is elsewhere; never infer network defaults from legacy baselines. Report the rule files read.
- Current lab policy: HuaweiJIN is the initial fallback; user profiles may select other SSIDs under the v0.12.0 decision above. AP password12345678 unchanged. Preserve unrelated keys; remove only obsolete apiToken/otaPassword auth keys. The updated build network-policy check must pass.
- Arduino-ESP32 core 3.3.10; generic ESP32-S3 N16R8. Build with scripts/build.py.
- Maintain thin sketch, separate modules, and a single firmware_version.h version source.
- Only the 10 ms control task initializes/writes actuator GPIO and LEDC.
- Never hold the control spinlock during GPIO, network, flash, or logging work.
- Invalid commands must not refresh the lease. Expired sessions cannot restart motion.
- Keep producer freshness separate from client HTTP heartbeat.
- No physical feedback exists. Never describe commanded output as measured position/speed.
- OTA must acknowledge stopped outputs before flash writes. No automatic rollback promise.
- No private credentials, API/OTA secrets, absolute machine paths, binaries, or machine profiles in tracked source. Shared lab STA/AP credentials in network_defaults.h are the explicit user-authorized exception.
- Gitflow: feature/* from develop; PR to develop; main reserved for reviewed releases.
- Run native control tests, Python client tests, dashboard tests, and pinned firmware build.
- Distinguish compile, simulation, and actual hardware verification in reports.

- Mandatory access decision: read the sibling vault 01_Rules/Rules_Akses_Robot_Lab.md before changing access behavior. This robot is explicitly open on the LAN: no API token or OTA password. Do not silently restore authentication. Sessions and timeout are required.

Mandatory vault read order (resolve the sibling vault before local firmware work):
1. 00_Start_Here/Firmware_AI_Vault_Home.md
2. 01_Rules/Rules_Nama_dan_Folder_Workspace.md
3. 01_Rules/Rules_Firmware_Standar.md
4. 01_Rules/Rules_GitHub_Gitflow.md
5. 01_Rules/Rules_Kredensial_WiFi.md
6. 04_Profiles/WiFi/WiFi_HuaweiJIN.md
7. 01_Rules/Rules_Akses_Robot_Lab.md

For a standalone checkout without the private sibling vault, the approved lab decisions are reproduced in README, API.md and this file for reproducible CI. Do not infer changed deployment requirements; local workspace work must locate and read the actual vault.

- Auto/Manual are explicit runtime modes, boot Manual. Mode changes stop/invalidate sessions; never silently switch mode during acquire. Python engines explicitly select Auto.
- Left hand commands are logical 0–180; driver pulse angle is 180 minus left angle. PWM status is LEDC readback, never measured wheel movement. Keep Wi-Fi modem sleep disabled for the lab latency profile.

- Wheel polarity is mapped only in the actuator driver: invertLeftMotor=false, invertRightMotor=true. Preserve logical forward signs in API/dashboard/ramp; choose PWM frequency readback from the mapped physical active input.

## Aturan identitas build dan pemberitahuan perubahan — 2026-10-12

Sebelum perubahan firmware, build/upload, atau push, AI wajib membaca `01_Rules/Rules_Versioning.md` dan `01_Rules/Rules_Change_Notifications.md` di vault (selain urutan baca yang sudah ada). Baca kedua file penuh pada awal tugas; sebutkan aturan yang dibaca dalam laporan. Semua perubahan firmware wajib dicatat di changelog/handoff dan diberitahukan kepada pengguna: versi sebelum/sesudah, timestamp build dengan zona waktu, perilaku/API/pin/NVS yang berubah, hasil tes, serta apakah perangkat sudah diupload. Jangan mengklaim timestamp sebagai jam perangkat atau uptime. Identitas versi + waktu build wajib terlihat pada serial, API, dashboard/display yang tersedia; metadata build dibuat ulang saat build, bukan ditulis manual. Build gate harus menolak metadata hilang/tidak cocok. AI tidak boleh melewati aturan ini karena mengingat sesi lama. Aturan baru/perubahan kebijakan wajib ditautkan dari halaman awal dan AGENTS/template agar terbaca oleh AI berikutnya.

## Dual eyes v0.5.0
- Read 04_Dokumen/EYES_DESIGN.md before modifying eye behavior, bus mapping, or the Eye Studio UI.
- SSD1306 confirmed, shared Wire SDA8/SCL9, physical left0x3D/right0x3C. Supplied 0x7A/0x78 are 8-bit write addresses, not Arduino Wire addresses.
- Only EyeService task owns I2C. Never perform OLED transfers inside the 10 ms actuator task or hold its spinlock during bus work.
- Eye settings are volatile, independent of motor ownership, and must never renew a motor lease or Python producer freshness. Preserve swap option and report missing OLEDs honestly.
- Run tests/test_eyes.cpp along with existing native/control/client/dashboard/build checks. OLED shape simulation and I2C ACK do not prove visible display correctness.

## Eye family and mounting decision — 2026-10-16
- User requires one consistent rounded-box eye character during Reactive/Playful. isBoxEye() in eye_catalog.h is authoritative; all idle and actuator reactions must satisfy it. Symbols/cartoon shapes belong to a separate chooser family and are explicit Manual/Showcase options.
- Never advertise41 different base shapes: several presets intentionally share a character and differ by movement/lids. Use actual rendered thumbnails and motion descriptions; do not replace them with repeated generic text icons.
- Physical OLED rotation is independent for each address, separate from logical swap. Installation defaults rotateLeft/rotateRight=true (both mounted upside down). All eye settings are volatile; retain rotation on legacy7/10-field requests. No NVS schema change in v0.7.0.
- Test runtime orientation packets, family-constrained reactions, UI12-field schema, preview rotation and producer freshness before build/OTA. ACK is transfer evidence, not an observed image. Record intermittent ACK honestly.

## v0.8.0 arm and wheel contract
- Read04_Dokumen/ARM_MOTION_DESIGN.md before modifying actuator trajectories. ArmMotion limits setpoint speed/acceleration in the single control task; no new output/timer owners. Do not claim jerk-limited planning or known physical boot position.
- Stop/expiry immediately freezes arm reference and cancels pending travel. Repeated valid targets cannot restart easing. Producer/lease freshness remains independent from settings requests. No autonomous idle arm gestures.
- Physical swapMotorSides=true corrects observed reversed turning without flipping equal-wheel forward/back signs. Polarity remains leftfalse/righttrue on the GPIO groups. Servo mapping remains left180-angle/rightangle. Verify both API and web use the same central driver mapping.
-47 eye presets,22 box/25 special; shinobi category is explicitly Manual/Showcase only through family filtering. Preserve both180-degree mounting defaults. No additional NVS keys.
