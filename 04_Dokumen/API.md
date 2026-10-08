# HTTP API v1

Base URL: `http://<device-ip>/api/v1`. All API routes require `Authorization: Bearer <device-token>`. JSON requests require `Content-Type: application/json`. Control bodies reject missing, extra, duplicate, wrongly typed, non-finite, or out-of-range fields. Maximum parsed JSON body is 1024 bytes; this is application validation, not a streaming HTTP memory limit. Root dashboard HTML is accessible without token.

## Acquire

`POST /control/acquire` with `{"owner":"manual"}` or `{"owner":"program"}` returns `{"session":123,"timeout_ms":500}`. Session is an opaque nonzero uint32. Only one lease exists. Manual can replace a program lease after outputs acknowledge stop. Other occupied acquisitions return 409. Acquiring alone does not emit servo pulses. A lease expires 500 ms after acquisition if no command follows.

## Command

`POST /command`:

```json
{"session":123,"sequence":1,"left":0.25,"right":0.25,"arm_left":45,"arm_right":135,"laser":false}
```

`session` and `sequence` are integers 1..4294967295. Sequence starts at 1 and must strictly increase for that lease; acquire a new lease before overflow. Wheels are finite numbers -1..1, arms 0..180, laser is a JSON boolean. Full command applies atomically; all accepted commands enable servo PWM. Response is `{"ok":true}`. Range/schema failure: 400; expired/wrong session, old sequence, or maintenance: 409. Rejected commands never refresh the deadline.

Refresh at 100 ms. At 500 ms without a valid command, motor PWM becomes zero, laser becomes off, servo holds last angle, and the lease is invalidated. Timeout is checked in the independent 10 ms output task and before command/acquire acceptance; timing still requires on-device verification. There is no automatic motion resumption.

## Stop and release

- `POST /stop` with `{}`: authenticated global stop, including a different owner's session. Idempotent; preserves last servo angle. Response 200 means the task acknowledged output application, not that physical motion has ceased.
- `POST /control/release` with `{"session":123}`: stops only the matching active lease; otherwise 409. Use this on background client errors to avoid stopping a newer manual session.

## Status

`GET /status` returns firmware/API version, uptime, owner (`none`, `manual`, `program`), session, latest sequence, stop reason, maintenance/ArduinoOTA flags, STA/AP state/IPs, and `commanded_output` containing `left`, `right`, `arm_left`, `arm_right`, `laser`, `servo_enabled`.

Output reflects software targets after ramping; GPIO application can lag by one task cycle. There is no measured position, wheel velocity, battery level, or obstacle status. Stop reasons include `boot`, `acquired`, `running`, `timeout`, `stop`, `released`, `wifi_config`, `ota`, `ota_finished`, `task_unresponsive`, `pwm_init_failed`.

## Settings

`POST /settings` accepts either `{"ssid":"router","password":"..."}` or `{"arduino_ota":true}`. SSID is 1..32 bytes and password 0..63 bytes. Wi-Fi is stored in NVS; connection changes stop the current lease. ArduinoOTA enablement is volatile and resets OFF on boot. Never return stored credentials in status.

## OTA upload

`POST /update`, authenticated multipart/form-data with one file named `firmware`: application `.bin` for this board/partition layout. Example:

```text
curl -H "Authorization: Bearer <token>" -F "firmware=@build/robot_controller.ino.bin" http://<device-ip>/api/v1/update
```

ArduinoOTA must be disabled. OTA locks control and waits for zero motor/laser plus disabled servo PWM before flash writes. Valid image returns `{"ok":true,"rebooting":true}` then reboots. Upload failure returns 400 and leaves idle with no lease; authentication failure is 401. Firmware image validation comes from ESP32 Update; there is no signed-image/product-ID enforcement, TLS, downgrade prevention, or automatic boot rollback.

General errors: 401 authentication, 400 invalid input, 409 control conflict, 503 output acknowledgement or resource failure, 404 unknown route. JSON error response: `{"error":"reason"}`. No CORS is enabled; Python is the intended laptop program interface.
