# AI Robot Controller

Firmware **v0.3.3** for ESP32-S3 N16R8: two DC motors, two servo hands, and a laser, controlled through a local Indonesian dashboard or HTTP JSON API. A laptop AI can call the included Python client; no AI engine is embedded in this firmware.

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

Motor sign convention: positive is the configured forward polarity. Left motor polarity is inverted in config.h to match the right wheel on this robot. Positive commands activate GPIO 13 (left) and GPIO 10 (right); negative commands activate GPIO 12 and GPIO 11. There is no speed measurement. PWM is 20 kHz / 10 bits; servo PWM is 50 Hz / 14 bits on separate LEDC timers.

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
python -m unittest discover -s tests -p test_client.py -v
python -m unittest discover -s tests -p test_network_policy.py -v
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
