# AI Robot Controller

Source **v0.13.0** simplifies Wi-Fi: the top dashboard panel shows the active
network and live signal, **Cari Wi-Fi → pilih jaringan → password → Simpan &
sambungkan**, plus Sambung/Hapus for saved networks. Boot/loss/retry select the
strongest detected saved SSID, then the next on failure; no roaming while healthy.
Absent saved SSIDs are skipped automatically, with120 s idle rescan when none are
visible to reduce repeated association disruption on the recovery AP. Read the
[current Wi-Fi guide](04_Dokumen/WIFI_V013.md) and latest
[verification](04_Dokumen/VERIFICATION.md). Prior v0.12 behavior follows as history.

Source **v0.12.0** adds detected Wi-Fi selection, up to5 saved profiles with
priority/failover, and persistent recovery hotspot. Open **Pengaturan Wi-Fi &
jaringan cadangan**: scan, save credentials, reorder, then connect when idle.
The new user decision replaces the fixed-HuaweiJIN-only runtime policy described
in historical sections below. See [Wi-Fi guide and API](04_Dokumen/WIFI_PROFILES_V012.md)
and [verification](04_Dokumen/VERIFICATION.md) for build and actual upload state.
IP can change between networks; old control sessions never resume automatically.

Earlier v0.11.0 behavior:

Source **v0.11.0** adds a LAN **TCP server on port9000** for an external control
engine. [Protocol and backend guide](04_Dokumen/TCP_CONTROL_V011.md),
[Python client](04_Dokumen/python/robot_tcp_client.py) and a read-only
[discovery example](04_Dokumen/python/tcp_example.py) are included. JSON lines,
one connected client, explicit Auto/program session and100 ms fresh commands;
500 ms expiry still stops wheels/laser and freezes arm travel. Build/OTA/live
results are recorded separately in [VERIFICATION](04_Dokumen/VERIFICATION.md).

Earlier v0.10.0 behavior:

Source **v0.10.0** adds faster accelerate/cruise/brake servo motion, configurable
per-arm degree limits and optional dynamic hands during forward/reverse. Slider
targets stay separate from held references; a dedicated HTTP81 control channel
isolates commands from HTTP80 dashboard stalls. See [servo controls](04_Dokumen/SERVO_CONTROL_V010.md)
and latest verification for actual build/OTA evidence. Limits need operator
calibration and reset at reboot. The first24-hour run ended partially after94
minutes; [full result](04_Dokumen/SOAK_RESULT_2026-10-16.md). It did not PASS.

The sections below describe earlier versions and their original launch context.

Earlier **v0.9.0** fixed Fox/Kurama highlight clipping, adds six distinct
Sharingan designs in their own category (52 total eye presets), reduces repeated
native raster work and reports network/reset/OLED budgets. It adds a guarded
[zero-wheel24-hour soak runner](04_Dokumen/SOAK.md). Connection stability remains
under investigation; see the latest [verification](04_Dokumen/VERIFICATION.md)
for actual build/upload/smoke/soak state. Launch is not24-hour completion.
The older v0.8.0 handoff below records the original unresolved baseline.

**Open issues / next AI:** read the [exhibition connection and eyes handoff](04_Dokumen/HANDOFF_CONNECTION_EYES.md). v0.8.0 has reported intermittent timeout (especially servo/laser), a Kurama highlight artifact and only one Sharingan design. Those fixes and the requested automated24-hour zero-wheel soak are pending; this is not an exhibition stability certification.

v0.8.0 adds smoother arm setpoints with **Lembut/Natural/Lincah** profiles (default45°/s,120°/s²), corrects reported reversed wheel steering, and expands eyes to47 presets including Sharingan/Rinnegan and animal pupils. See [arm motion design and research](04_Dokumen/ARM_MOTION_DESIGN.md). This controls commanded trajectories; physical smoothness/direction need observation. Stop/timeout cancels arm travel at its current reference. Both OLED180° mounting defaults retained.


v0.7.0 keeps **Reactive/Playful as one rounded-box character** and separates22 box presets from19 symbol/cartoon presets. Eye Studio now shows rendered thumbnails and motion notes. **Putar OLED kiri/kanan180°** corrects upside-down mounting independently of swap; both are enabled by default for this robot. Settings remain per boot. Existing7/10-field eye clients still work; new clients can set both rotation booleans.


v0.6.0 adds **41 eye expressions / 10 categories**, Reactive/Playful/Showcase/Manual policies, energy selection and separate winks. Open **Mata & kepribadian**, choose category/preset or animation policy, then **Terapkan ke robot**. **Periksa LCD / OLED** provides a full I2C scan and five-second visual tests. The supplied0x78/0x7A write addresses map to Wire0x3C/0x3D; use scanner results rather than assuming both modules have different addresses. See [eye design](04_Dokumen/EYES_DESIGN.md) and [API](04_Dokumen/API.md).


Firmware **v0.10.0** for ESP32-S3 N16R8: two DC motors, two servo hands, and a laser, controlled through a local Indonesian dashboard or HTTP JSON API. A laptop AI can call the included Python client; no AI engine is embedded in this firmware.

**No encoder or other feedback:** output status is commanded PWM/angle, never measured motion. Hardware power design and autonomous navigation are outside this version.

## Layout

- `01_Desain`: pin map.
- `02_Firmware/robot_controller`: active Arduino sketch and modules.
- `04_Dokumen`: API, architecture, verification checklist, and Python client.
- `scripts`, `tests`, `.github`: reproducible build, behavioral tests, and CI.

## Build (Windows, Linux, macOS)

Install Arduino CLI **1.4.1** and Python **3.11+**. Use the official ESP32 package index:

```text
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.10 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
python scripts/build.py
```

If CLI is not in PATH, use `python scripts/build.py --cli <path-to-arduino-cli>`. No third-party firmware library installation is required; JSON uses core-bundled ESP-IDF cJSON.

Pinned FQBN:

```text
esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=default,USBMode=hwcdc
```

The sketch's custom `partitions.csv` overrides the menu partition table. The build script overrides the menu application-size ceiling to match the actual 6 MiB slot and validates the emitted table and binary. `CDCOnBoot=default` means native USB CDC is disabled; `Serial` uses UART0 through the board's USB-to-UART bridge. Upload and serial boot were verified through COM11. First upload (replace `<PORT>` with the verified port):

```text
arduino-cli upload --fqbn esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=default,USBMode=hwcdc --port <PORT> --input-dir build 02_Firmware/robot_controller
```

## Setup and use

1. Open USB serial at 115200 baud. Boot prints version and open-LAN access mode. Send `info` followed by newline to see connection information. Boot removes only legacy `apiToken` and `otaPassword` NVS keys; unrelated settings are preserved.
2. This lab build automatically connects to SSID **HuaweiJIN**, password **jayaabadi100**, carried in `network_defaults.h` with explicit user approval. These shared lab defaults override old STA/AP NVS values on boot without erasing other keys. Web/serial Wi-Fi settings accept only this lab profile; other profiles require an explicitly approved source/policy change.
3. If STA is unavailable for 15 seconds, join the device-specific `ai-robot-<id>-setup` AP with password **12345678** and browse `http://192.168.4.1/`. AP remains available until reboot once started.
4. On the router, use the assigned device IP. The dashboard connects and retries automatically. Select Manual to use buttons/sliders; pressing an actuator control acquires the manual session automatically. Release stops motion and invalidates the session. Auto is for the laptop engine/API and disables dashboard actuator controls. Changing mode stops output and invalidates the session. Boot mode is Manual.
5. Servo sliders use 0–180 degrees. Initial commanded angle is 90 degrees on the first complete command; boot itself generates no servo pulses. Servo pulse defaults are 500–2500 microseconds and are centralized in config.h.

Motor sign convention: positive is the configured forward polarity. Forward/back was corrected in v0.5.0 by flipping both physical motor polarities: invertLeftMotor=false and invertRightMotor=true. Positive commands activate GPIO 12 (left) and GPIO 11 (right); negative commands activate GPIO 13 and GPIO 10. Logical API signs and differential steering are unchanged. There is no speed measurement. PWM is 20 kHz / 10 bits; servo PWM is 50 Hz / 14 bits on separate LEDC timers.

## Laptop client

Set `ROBOT_URL` outside Git, then run `python 04_Dokumen/python/example.py`. The example moves both wheels at 20% commanded power for one second. Review this behavior before running it on a device.

```python
from robot_client import RobotClient
with RobotClient(base_url) as robot:
    robot.set_mode("auto")  # Explicit mode change stops and invalidates prior control.
    robot.acquire("program")
    robot.drive(0.2, 0.2)
    robot.set_arms(45, 135)
    robot.set_laser(False)
    # Producer must refresh targets more often than target_ttl (default 300 ms).
    robot.stop()
```

The client heartbeat sends every 100 ms, but a stale producer target expires after 300 ms. Firmware independently expires control after 500 ms without valid commands. A resumed producer must explicitly acquire again. Setter calls update one shared complete target; refreshing a hand/laser setting also refreshes that target, including its wheel values. Use one producer per client instance; lifecycle calls are made from that producer thread.

## OTA

- Rebuild and upload **`build/robot_controller.ino.bin`** through the dashboard firmware panel or multipart API without authentication. Do not upload merged, bootloader, or partition binaries through OTA.
- Web OTA and ArduinoOTA require no password or token. ArduinoOTA is OFF each boot; enable it from dashboard settings when needed.
- Disable ArduinoOTA before web upload. During OTA, motor and laser turn off, servo pulses stop, and new control sessions are rejected.
- Failed upload leaves idle output with no active session. Successful upload reboots idle. If a connection disappears after image validation, reboot manually if necessary.
- USB is required for initial installation and partition-layout changes. Automatic boot rollback is not implemented.
- HTTP and OTA are intended for a trusted local network; there is no TLS or public-internet deployment configuration. Never forward these ports to the internet.

## Validation

```text
g++ -std=c++17 -Wall -Wextra -Werror tests/test_control.cpp -o test-control
./test-control
g++ -std=c++17 -Wall -Wextra -Werror tests/test_eyes.cpp -o build/test-eyes
./build/test-eyes
python -m unittest discover -s tests -p test_client.py -v
python -m unittest discover -s tests -p test_network_policy.py -v
python -m unittest discover -s tests -p test_build_metadata.py -v
python -m pip install playwright==1.58.0
python -m playwright install chromium
python tests/test_dashboard.py
python scripts/build.py
```

On Windows, compile the C++ test from a Visual Studio developer command prompt with `cl /std:c++17 /EHsc /W4 /WX tests\test_control.cpp /Fe:test-control.exe`, then run it. CI runs the same logic tests under g++ and dashboard tests under Chromium.

See `04_Dokumen/VERIFICATION.md` for current evidence and pending device tests. Firmware version comes only from `firmware_version.h`; API version is independently `/api/v1`. Every build runs the lab-profile gate before compile. Local builds read the sibling Obsidian credential rule/profile; standalone CI checks the approved profile without needing the vault checkout. This gate checks source configuration, not whether an AI read the prose; AGENTS.md supplies the mandatory reading workflow.

## GitHub workflow

Local work branch: `feature/initial-robot-controller`, based on `develop`. Push this feature branch and open a reviewed PR to `develop` when publication is requested. `main` is for approved releases through `release/*` with semantic tags and back-merge. Remote publication is through the feature branch and PR to develop. No release binaries are tracked.

## License and LAN access

MIT license. API, settings, stop, and firmware upload are accessible to anyone who can reach the device on its LAN or recovery AP. Open-source licensing and network access are separate choices; both are explicitly authorized for this robot lab profile. Control sessions arbitrate ownership and are not authentication. Customer/site deployments require their own explicit access policy.

## Hands and PWM status

Hand angles in UI/API are logical 0–180 degrees. The left servo pulse angle is `180 - arm_left`; the right is `arm_right`. Mirroring is configured centrally in config.h. `pwm` status contains LEDC duty readback for GPIO 12/13 (left) and 10/11 (right), initialization/write status, and active frequency. Raw motor duty is 0–1024; core 3.3.10 uses 1024 for full-on. An idle active-frequency read is zero because duty is zero. This reports peripheral configuration/readback, not voltage or physical wheel movement.

Wi-Fi modem sleep is disabled for responsive local control. Status reports `wifi_sleep: false`. Network delays can still exceed the lease; timeout never automatically resumes motion.

Wi-Fi recovery retries a stalled/disconnected STA attempt every 30 seconds while retaining the recovery AP. USB serial `info` includes last disconnect reason/count, RSSI, heap, and uptime; boot logs reset reason. A retry does not resume expired control.

## Build identity v0.3.4

`scripts/build.py` generates ignored `firmware_build.h` at build start using the build machine UTC clock, then compiles with `--clean` and checks that the application contains the current version and timestamp. Direct sketch compile requires this generated header; use the build script to avoid stale metadata. The serial boot log, dashboard header and status `build_time_utc` expose ISO 8601 UTC (for example `2026-10-12T01:02:03Z`); this is neither uptime nor upload time. `build/build_manifest.json` records the version, UTC timestamp and application SHA-256 and accompanies CI binaries. Different build timestamps intentionally produce different images. Before publishing, read the vault versioning/change-notification rules and report changes plus whether hardware was actually uploaded.

## Dashboard v0.4.0

The embedded dark/cyan dashboard adapts the supplied Stitch ESP32 web dashboard export. Navigation anchors lead to control, real system telemetry, OTA, Wi-Fi settings and JSON status. All CSS, icons and scripts ship in firmware; no Tailwind CDN or external fonts are required. CPU frequency is configuration, HTTP response time is browser-to-status latency, and memory values come from ESP32 APIs. No CPU-load/temperature/LittleFS sensor data is invented. Status remains HTTP polling every 1 second; actuator command refresh remains 100 ms with the same 500 ms device timeout. The wheels use MX1508 DC drive, not steppers. Available telemetry is separate from unmeasured wheel motion/arm position.

## Dual OLED eyes v0.7.0

Confirmed SSD1306 128x64 on shared I2C: SDA GPIO8, SCL GPIO9. Right address 0x3C, left 0x3D (the supplied 8-bit write addresses 0x78/0x7A shifted right once). Both modules must actually have different hardware addresses; firmware cannot change a fixed-address module. Connect both SDA/SCL lines to the shared bus and common ground; use 3.3 V I2C logic. Eye Studio can swap logical eye images without rewiring.

The dashboard offers reactive/playful/showcase/manual expression modes, 41 presets in 10 categories, brightness, tempo, natural blink and display enable. These eye modes are independent of Manual/Auto motor ownership. Select a preset to preview locally, then **Terapkan ke robot**. Settings are volatile and reset at reboot; NVS is unchanged. Eye settings never acquire/refresh a motor lease. `RobotClient.set_eyes(automatic=False, expression="love", swap=True)` uses the same API without extending producer freshness.

Eyes react to configured forward/reverse, turning, logical arm angles and laser state; idle behavior includes glances, blinks, winks and category-based rotation. They reflect commanded output, not sensed motion. A separate low-priority task owns Wire and SSD1306 framebuffers; I2C failures do not run inside the 10 ms actuator task. Missing displays retry every 5 seconds, fail fast per frame and appear in status. OLEDs show firmware version/build UTC for the first four seconds after boot, then animate. During OTA, eye bus work pauses. Preview is illustrative (1-second pose status and animated local draft), not a physical screen readback.

See [eye design and wiring](04_Dokumen/EYES_DESIGN.md). No additional firmware libraries are required.

The CI also compiles the real EyeService with fake Wire/RTOS boundaries (`tests/test_eye_service.cpp`) to check full 1024-byte panels, image swap, failed-transfer early abort, retry timing, bus init failure and maintenance suppression. Generate build metadata (or run scripts/build.py) before that boundary test; CI generates it explicitly before testing and regenerates it at actual firmware build start. These fakes do not establish physical timing or pixel correctness.
