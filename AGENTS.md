# AI Robot Controller Rules

- Read README and 04_Dokumen/API.md before changing control behavior.
- Before planning, connectivity changes, build/upload, read the sibling vault's 01_Rules/Rules_Kredensial_WiFi.md and 04_Profiles/WiFi/WiFi_HuaweiJIN.md. Locate the vault if checkout is elsewhere; never infer network defaults from legacy baselines. Report the rule files read.
- Internal lab policy: STA HuaweiJIN, AP password 12345678. Centralized source lab defaults are explicitly authorized; migrate old STA/AP NVS values preserving unrelated keys; the approved open-access migration removes only obsolete apiToken/otaPassword keys. The build's network-policy check must pass.
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

- Wheel polarity is mapped only in the actuator driver: invertLeftMotor=true, invertRightMotor=false. Preserve logical forward signs in API/dashboard/ramp; choose PWM frequency readback from the mapped physical active input.

## Aturan identitas build dan pemberitahuan perubahan — 2026-10-12

Sebelum perubahan firmware, build/upload, atau push, AI wajib membaca `01_Rules/Rules_Versioning.md` dan `01_Rules/Rules_Change_Notifications.md` di vault (selain urutan baca yang sudah ada). Baca kedua file penuh pada awal tugas; sebutkan aturan yang dibaca dalam laporan. Semua perubahan firmware wajib dicatat di changelog/handoff dan diberitahukan kepada pengguna: versi sebelum/sesudah, timestamp build dengan zona waktu, perilaku/API/pin/NVS yang berubah, hasil tes, serta apakah perangkat sudah diupload. Jangan mengklaim timestamp sebagai jam perangkat atau uptime. Identitas versi + waktu build wajib terlihat pada serial, API, dashboard/display yang tersedia; metadata build dibuat ulang saat build, bukan ditulis manual. Build gate harus menolak metadata hilang/tidak cocok. AI tidak boleh melewati aturan ini karena mengingat sesi lama. Aturan baru/perubahan kebijakan wajib ditautkan dari halaman awal dan AGENTS/template agar terbaca oleh AI berikutnya.
