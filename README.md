# AI Robot Controller

Firmware **v0.1.0** for ESP32-S3 N16R8: two DC motors, two servo hands, and a laser, controlled through a local Indonesian dashboard or HTTP JSON API. A laptop AI can call the included Python client; no AI engine is embedded in this firmware.

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

1. Open USB serial at 115200 baud. Boot prints version plus generated API token, AP password, and separate ArduinoOTA password. Send `info` followed by newline if the initial USB boot output was missed. These persist in NVS; erase NVS to regenerate credentials.
2. Configure router: `wifi <ssid>|<password>` followed by newline. The first `|` separates SSID/password; use the web form for SSIDs containing `|`.
3. If STA is unavailable for 15 seconds, join the device-specific `ai-robot-<id>-setup` AP with its generated password and browse `http://192.168.4.1/`. AP remains available until reboot once started.
4. On the router, use the assigned device IP. Enter API token, connect, then take manual control. Press and hold direction buttons. Release stops motion and invalidates the session; acquire again for the next movement.
5. Servo sliders use 0–180 degrees. Initial commanded angle is 90 degrees on the first complete command; boot itself generates no servo pulses. Servo pulse defaults are 500–2500 microseconds and are centralized in config.h.

Motor sign convention: positive is the configured forward polarity. There is no physical polarity calibration or speed measurement. PWM is 20 kHz / 10 bits; servo PWM is 50 Hz / 14 bits on separate LEDC timers.

## Laptop client

Set `ROBOT_URL` and `ROBOT_TOKEN` outside Git, then run `python 04_Dokumen/python/example.py`. The example moves both wheels at 20% commanded power for one second. Review this behavior before running it on a device.

```python
from robot_client import RobotClient
with RobotClient(base_url, token) as robot:
    robot.acquire("program")
    robot.drive(0.2, 0.2)
    robot.set_arms(45, 135)
    robot.set_laser(False)
    # Producer must refresh targets more often than target_ttl (default 300 ms).
    robot.stop()
```

The client heartbeat sends every 100 ms, but a stale producer target expires after 300 ms. Firmware independently expires control after 500 ms without valid commands. A resumed producer must explicitly acquire again. Setter calls update one shared complete target; refreshing a hand/laser setting also refreshes that target, including its wheel values. Use one producer per client instance; lifecycle calls are made from that producer thread.

## OTA

- Rebuild and upload **`build/robot_controller.ino.bin`** through the dashboard firmware panel or authenticated multipart API. Do not upload merged, bootloader, or partition binaries through OTA.
- Web OTA uses API token. ArduinoOTA is OFF each boot; enable it from dashboard settings and supply the separate generated OTA password to Arduino IDE/espota.
- Disable ArduinoOTA before web upload. During OTA, motor and laser turn off, servo pulses stop, and new control sessions are rejected.
- Failed upload leaves idle output with no active session. Successful upload reboots idle. If a connection disappears after image validation, reboot manually if necessary.
- USB is required for initial installation and partition-layout changes. Automatic boot rollback is not implemented.
- HTTP and OTA are intended for a trusted local network; there is no TLS or public-internet deployment configuration. Never forward these ports to the internet.

## Validation

```text
g++ -std=c++17 -Wall -Wextra -Werror tests/test_control.cpp -o test-control
./test-control
python -m unittest discover -s tests -p test_client.py -v
python -m pip install playwright==1.58.0
python -m playwright install chromium
python tests/test_dashboard.py
python scripts/build.py
```

On Windows, compile the C++ test from a Visual Studio developer command prompt with `cl /std:c++17 /EHsc /W4 /WX tests\test_control.cpp /Fe:test-control.exe`, then run it. CI runs the same logic tests under g++ and dashboard tests under Chromium.

See `04_Dokumen/VERIFICATION.md` for current evidence and pending device tests. Firmware version comes only from `firmware_version.h`; API version is independently `/api/v1`.

## GitHub workflow

Local work branch: `feature/initial-robot-controller`, based on `develop`. Push this feature branch and open a reviewed PR to `develop` when publication is requested. `main` is for approved releases through `release/*` with semantic tags and back-merge. No remote is configured by this implementation. No release binaries are tracked.
