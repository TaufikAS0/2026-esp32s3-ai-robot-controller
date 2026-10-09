# HTTP API v1

Base URL: `http://<device-ip>/api/v1`. All API routes are open on the local network: no token or OTA password. Control sessions arbitrate ownership; they do not authenticate a person. Status reports `access_mode: "open_lan"`. JSON requests require `Content-Type: application/json`. Control bodies reject missing, extra, duplicate, wrongly typed, non-finite, or out-of-range fields. Maximum parsed JSON body is 1024 bytes; this is application validation, not a streaming HTTP memory limit. The dashboard also requires no token.

## Mode

`POST /control/mode` with `{"mode":"manual"}` or `{"mode":"auto"}` selects the explicit runtime mode. Boot defaults to Manual; mode is not persisted in NVS. Changing mode stops output and invalidates the active session. Manual permits `owner: manual`; Auto permits `owner: program`. Auto awaits the laptop engine and never generates commands on its own. The engine should explicitly call `RobotClient.set_mode("auto")` before acquisition; this changes control policy and is not an implicit acquisition side effect.

## Acquire

`POST /control/acquire` with `{"owner":"manual"}` or `{"owner":"program"}` returns `{"session":123,"timeout_ms":500}`. Session is an opaque nonzero uint32. Only one lease exists. Only the owner matching the selected mode is accepted. Switching Auto to Manual invalidates the program lease after stopping output. Occupied acquisitions or mode mismatch return 409. Acquiring alone does not emit servo pulses. A lease expires 500 ms after acquisition if no command follows.

## Command

`POST /command`:

```json
{"session":123,"sequence":1,"left":0.25,"right":0.25,"arm_left":45,"arm_right":135,"laser":false}
```

`session` and `sequence` are integers 1..4294967295. Sequence starts at 1 and must strictly increase for that lease; acquire a new lease before overflow. Wheels are finite numbers -1..1, arms 0..180, laser is a JSON boolean. Full command applies atomically; all accepted commands enable servo PWM. Response is `{"ok":true}`. Range/schema failure: 400; expired/wrong session, old sequence, or maintenance: 409. Rejected commands never refresh the deadline.

Refresh at 100 ms. At 500 ms without a valid command, motor PWM becomes zero, laser becomes off, servo holds last angle, and the lease is invalidated. Timeout is checked in the independent 10 ms output task and before command/acquire acceptance; timing still requires on-device verification. There is no automatic motion resumption.

## Stop and release

- `POST /stop` with `{}`: global stop, including a different owner's session. Idempotent; preserves last servo angle. Response 200 means the task acknowledged output application, not that physical motion has ceased.
- `POST /control/release` with `{"session":123}`: stops only the matching active lease; otherwise 409. Use this on background client errors to avoid stopping a newer manual session.

## Status

`GET /status` returns firmware/API version, uptime, owner (`none`, `manual`, `program`), session, latest sequence, stop reason, maintenance/ArduinoOTA flags, STA/AP state/IPs, and `commanded_output` containing `left`, `right`, `arm_left`, `arm_right`, `laser`, `servo_enabled`.

Status includes `wifi_sleep` (false in the latency profile), `mode` plus `pwm`: `ready`, `write_ok`, `left1_duty`/`left2_duty` (GPIO 12/13), `right1_duty`/`right2_duty` (GPIO 10/11), `left_active_hz`/`right_active_hz`, and mapped servo `arm_left_pulse_angle`/`arm_right_pulse_angle`. Motor raw duty is 0–1024 (1024 is full-on in the pinned core); active frequency returns 0 when duty is zero. Logical arm commands remain 0–180 degrees, mapped to left `180 - angle` and right `angle` in the driver. These fields report LEDC state and mapped pulse targets, not measured voltage, position, or motion.

Output reflects software targets after ramping; GPIO application can lag by one task cycle. There is no measured position, wheel velocity, battery level, or obstacle status. Stop reasons include `boot`, `acquired`, `running`, `timeout`, `stop`, `released`, `wifi_config`, `ota`, `ota_finished`, `task_unresponsive`, `pwm_init_failed`, `pwm_write_failed`, `mode_changed`.

## Settings

`POST /settings` accepts either `{"ssid":"HuaweiJIN","password":"jayaabadi100"}` or `{"arduino_ota":true}`. This lab build accepts only the approved HuaweiJIN profile; other values return 400. Reconnecting stops the current lease. On boot, old STA/AP NVS values are migrated to the centralized lab defaults, removing obsolete `apiToken`/`otaPassword` keys while preserving unrelated settings. Status also includes `sta_target_ssid` and `sta_ssid`; connection success is determined by `sta_connected` and a nonzero IP. ArduinoOTA enablement is volatile and resets OFF on boot. Never return stored passwords or tokens in status.

## OTA upload

`POST /update`, multipart/form-data with one file named `firmware`: application `.bin` for this board/partition layout. Example:

```text
curl -F "firmware=@build/robot_controller.ino.bin" http://<device-ip>/api/v1/update
```

ArduinoOTA must be disabled. OTA locks control and waits for zero motor/laser plus disabled servo PWM before flash writes. Valid image returns `{"ok":true,"rebooting":true}` then reboots. Upload failure returns 400 and leaves idle with no lease. Firmware image validation comes from ESP32 Update; there is no signed-image/product-ID enforcement, TLS, downgrade prevention, or automatic boot rollback.

General errors: 400 invalid input, 409 control conflict, 503 output acknowledgement or resource failure, 404 unknown route. JSON error response: `{"error":"reason"}`. No CORS is enabled; Python is the intended laptop program interface.

## Wheel polarity v0.3.2

Logical positive means configured forward for both wheels. The driver inverts the left motor only (config.h): forward uses GPIO 13/10, reverse uses GPIO 12/11; the paired input remains zero. Status commanded_output retains logical signs; PWM duty fields retain physical GPIO identities. Frequency readback follows the mapped active pin.
