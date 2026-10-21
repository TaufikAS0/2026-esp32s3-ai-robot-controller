# HTTP API v1

### v0.18.0 dedicated Operations viewport

[OPERATIONS_V018.md](OPERATIONS_V018.md) adds dashboard route `/#operation-panel`.
No new endpoint/schema/GPIO: existing Manual commands drive left joystick,
two right-side arm sliders and the existing laser switch. Navigation/load/power
do not acquire; entering/leaving releases only this browser's lease. Global Stop
and 100ms command producer / 500ms firmware expiry remain.

### v0.17.0 Manual joystick and laser release

[JOYSTICK_LASER_V017.md](JOYSTICK_LASER_V017.md) changes dashboard input only.
No endpoint/schema changes. Normal drive release with laser selected sends wheel
zero within the active Manual lease; full release/Stop still switches laser off.
Existing100ms full commands/500ms timeout, Auto/demo laserOFF and arms limits stay.

### v0.16.0 supervised floor exhibition

[EXHIBITION_V016.md](EXHIBITION_V016.md) adds strict POST/demo/exhibition/config,
/center and/start (robot or isolated simulation), plus GET/demo.exhibition.
Runtime-only virtual axle area and operator estimates are separate from stored
table calibration. Reset center is idle-only/no GPIO/no acquisition/no auto start;
physical repositioning and explicit Auto/pose confirmations required. Existing
scoped Stop/HTTP81 status and global Stop remain; no automatic resume after
mode/network/OTA/Stop/reboot. Internal Happy override while waving left130–150.

### v0.15.0 guided Auto and pending results

[AUTO_GUIDE_V015.md](AUTO_GUIDE_V015.md) defines same-boot pending sample lifetime
and invalidation. Existing endpoint request bodies unchanged. GET/demo adds
sample_pending/sample_reason plus config_valid/calibration_forward_fits. Existing
sample_id/sample_kind/sample_integral describe the pending sample. Remove the old
120-second expiry, not500 ms actuator expiry. Pending results hold no control lease,
never resume actuators and clear on control/network/config/OTA/new run/reboot.

### v0.14.1 demo PWM deadband

[DEMO_V0141_PWM.md](DEMO_V0141_PWM.md) extends demo config with optional paired
`min_pwm,deadband_pwm` (normalized0..1), raises demo/calibration `pwm` maximum
to0.80 while retaining15% default, and adds GET/demo `minimum_speed_allowed`.
Exact legacy request/storage schemas remain readable. Any PWM change resets
motion calibration; gains now use applied effort above the configured deadband.
No change to raw HTTP/TCP actuator command semantics or automatic startup.

### v0.14.0 onboard demo

[DEMO_V014.md](DEMO_V014.md) defines GET/demo, POST/demo/config, /zero, /start,
/stop, /calibration/start and /calibration/result. Config/Zero do not move the
robot; simulation never takes a real lease. Real demo/calibration explicitly use
Auto with internal transport `demo`; external HTTP/TCP cannot drive/release that
lease. Existing global Stop and mode/network/OTA cancellation remain. HTTP81
offers GET/demo and POST/demo/stop; TCPv1 command bodies unchanged. New NVS demoV1
stores config/gains only. Pose is estimated, not measured; real tests need a
fenced table, operator-provided geometry and accepted hand calibration.

### v0.13.0 strongest saved Wi-Fi and clear connection status

[WIFI_V013.md](WIFI_V013.md) supersedes v0.12 stored-order selection. GET/wifi adds
selection=strongest_saved, busy, retry_in_ms, scan_age_ms, live rssi (null offline),
per-profile scanned rssi (nullable); current_ssid is empty offline. Existing
endpoints/NVS schema remain. /wifi/order now only breaks RSSI ties. Boot/loss/retry
scan then attempt visible saved SSIDs strongest first,15 s each;30 s after all
fail,120 s if no candidates/scan failed. Manual connect tries chosen profile first,
then other visible saved networks. No automatic attempts of hidden SSIDs; use
explicit connect. No background scan/roaming while healthy. Dashboard uses existing
POST/settings to save/connect together. Same ownership/OTA/500 ms lease protections.

### v0.12.0 saved Wi-Fi profiles

[WIFI_PROFILES_V012.md](WIFI_PROFILES_V012.md) defines GET/wifi and POST
/wifi/scan, /wifi/save, /wifi/delete, /wifi/order, /wifi/connect on port80.
User-approved saved SSIDs replace fixed-SSID-only configuration; existing legacy
POST/settings {ssid,password} now saves/connects any valid profile while idle.
New NVS wifiProfilesV1 preserves user list across boot. HTTP status adds
network_busy/wifi_state and reports actual targetSSID; TCP status adds network_busy.
Scan/config/failover invalidate stale control and gate acquire/command; no secret
profile password returned. The old fixed-profile descriptions below are historical.

### v0.11.0 TCP control

See [TCP_CONTROL_V011.md](TCP_CONTROL_V011.md) for TCP9000 JSON-lines protocol,
SDK, explicit Auto/program flow and socket-bound ownership. HTTP status adds
`tcp_control:{ready,port:9000,protocol_version:1}` and
`control_transport:"none"|"http"|"tcp"`. HTTP command/release cannot operate a
TCP-bound session; HTTP leases continue to work normally. Explicit HTTP mode
changes/global stop still stop/invalidate ownership. Existing HTTP schemas,
arm policy/settings and eye endpoints remain available. No network/NVS changes.

### v0.10.0 servo and isolated control

Read [SERVO_CONTROL_V010.md](SERVO_CONTROL_V010.md) for the strict7-field
`POST /arms/policy` schema, per-arm min/max, optional forward/reverse hand motion,
accepted-pose requirements and RAM reset behavior. Natural defaults are60/240.
Status `arm_motion` adds min_left/max_left/min_right/max_right, dynamic,
dynamic_active, amplitude_deg, period_ms and effective_left/right (bounded eased
destinations before trajectory stepping). `moving` compares the effective target
to reference; all angles remain commanded rather than measured.

`control_channel:{ready:true,port:81}` advertises an additional HTTP service with
the same status, command, acquire/release/mode, stop, arms/settings and arms/policy
routes. Dashboard chooses81 when ready;80 retains legacy API, eyes, UI and OTA.
Same-host dashboard CORS and OPTIONS are supported on81; non-browser LAN access
remains open. Max1024-byte complete JSON/exact schemas. Out-of-limit targets reject
without lease renewal; acquired sessions still expire500 ms after last valid
command. Socket timeout/queue isolation does not guarantee radio latency.

### v0.9.0 additive telemetry and presets

Status adds `sta_mac`, `wifi_connect_count` (GOT_IP events, including first boot
connection), `wifi_disconnect_count`, `wifi_disconnect_reason` (raw
ESP-IDF Wi-Fi reason), and `reset_reason` (esp_reset_reason numeric enum). Counts
and minima are since boot. `eyes.work_us` is the last eye iteration's elapsed
microseconds including I2C, `max_work_us` its maximum and `overruns` the number
reaching the60 ms frame budget. These are work durations, not CPU utilization or
physical pixel measurements. Overdue frames are dropped rather than replayed.

Catalog now has52 presets:22 box/30 special and13 selectors including All.
Category `sharingan` contains existing `sharingan` (3 tomoe), `sharingan_1`,
`sharingan_2`, `mangekyo_pinwheel`, `mangekyo_flower`, `mangekyo_star`.
Original IDs and7/10/12-field schemas remain compatible. Reactive/Playful stay box.
Both180-degree mounting defaults and logical swap are preserved.

When servo pulses are disabled (boot/maintenance), the first accepted complete
command seeds its requested arm reference directly; it no longer begins from
assumed90 degrees. Callers must supply a previously accepted/setup pose. This is
not homing or position measurement and may jump if the physical pose is different.
After enable, ordinary targets remain speed/acceleration limited. Stop and500 ms
expiry still cancel travel and invalidate the session. No NVS/pin/network changes.

### v0.8.0 arms, steering and eyes

`POST /api/v1/arms/settings` requires exactly `{"speed_dps":45,"acceleration_dps2":120}`. Finite numeric speed5..120 and acceleration20..600.400 rejects malformed/range/type errors;409 rejects active control ownership or maintenance. RAM-only; boot45/120. No control session acquired/renewed, no motion from configuration alone. Python `set_arm_motion(25,60)` uses the endpoint before acquire.

Status adds `arm_motion`: speed_dps, acceleration_dps2, target_left/right (requested logical angles), moving (reference trajectory still unfinished), wheel_channels_swapped=true. `commanded_output.arm_left/right` now reports the interpolated reference, not the final requested destination. No physical feedback exists. Command JSON shape remains unchanged; repeated targets do not restart interpolation. Keep a fresh valid command every100 ms while a gesture is intended. Stop/release/timeout holds current reference and cancels remaining travel. OTA disables pulses. First enable can still jump from an unknown physical startup angle.

Logical wheel commands remain left/right in robot coordinates. Driver now swaps target sides before polarity to correct user's reported reversed turning. +/+ and -/- unchanged. Raw pwm.left* fields still refer to physical GPIO12/13 and right* GPIO10/11; do not interpret them as a measured logical wheel location. Existing servo mirroring unchanged.

Eye catalog adds category shinobi (Hewan & shinobi) and six IDs: sharingan,rinnegan,cat,fox,snake,toad.47 presets/22 box/25 special;11 non-All categories and12 total category selectors. Manual/Showcase can select them; Reactive/Playful remain box-only. OLEDs are monochrome; symbols are stylized graphics, not sensor values. Existing eye schemas7/10/12 fields preserved.

See ARM_MOTION_DESIGN.md for trajectory bounds, timing, startup assumptions and research sources.


### Eyes additions in v0.7.0

Catalog entries add `family`: `box` or `special` (22/19 presets). Reactive and Playful always choose box-family expressions, regardless of the saved manual preset. Motor and hand reactions also remain box-family; hand-low is now shy. Showcase cycles the selected category across both families; Manual can select any ID. Family tabs filter browsing locally, independently of category rotation; selecting an expression switches to Manual. Main web preview is a local animation informed by1 Hz status, not a recording of physical OLED frames.

POST /eyes accepts exactly7 legacy fields,10 v0.6 fields, or12 fields: the10-field form plus boolean `rotate_left` and `rotate_right`. Both must be supplied in the12-field form. Rejected schema/types never partially apply. Seven/ten-field clients retain the current rotation. Status exposes both booleans. Defaults are true/true for the current upside-down mounting; these are RAM settings and reset to those defaults on reboot. No NVS keys added.

Rotation applies to physical addresses left0x3D/right0x3C, after logical swap. It is a true180-degree turn (both axes), not only a vertical mirror. It also affects boot identity and all diagnostic patterns. The controller changes segment/COM mapping: normal A1/C8, rotated A0/C0; framebuffer commands remain unchanged. UI previews rotate panel data too; physically mounting the module upside down restores the upright view.

Python: `robot.set_eyes(play_mode="reactive", rotate_left=True, rotate_right=True)`. If only one rotation is supplied, the client reads status to preserve the other. Cosmetic settings still do not acquire/extend a motor session or producer target freshness.


### Eyes additions in v0.6.0

`GET /api/v1/eyes/catalog` returns `presets` (`id,label,category,shape,width,height`) and `categories` (`id,label`), sourced from the firmware catalog. There are41 presets and10 categories plus `all`.

`POST /api/v1/eyes` accepts the original seven fields or exactly ten fields with these additions:

```json
{"enabled":true,"automatic":true,"blink":true,"swap":false,"expression":"googly","brightness":160,"tempo":1,"play_mode":"playful","category":"wild","energy":3}
```

`play_mode`: reactive/playful/showcase/manual; `automatic` must be false only for manual. `category`: all/happy/love/curious/focus/surprise/rest/sad/wink/wild/digital. `energy`: integer1..3. Invalid schemas, duplicates, ranges or inconsistent mode reject atomically with400; maintenance409. Legacy seven-field requests retain category/energy and select manual when automatic=false; switching automatic back on restores reactive if previously manual. Settings are RAM-only. Python `set_eyes(play_mode="showcase",category="wink",energy=3)` derives automatic from the mode. Supplying a nondefault category/energy without play_mode infers reactive or manual from automatic; default seven-field calls stay compatible.

`POST /api/v1/eyes/check` requires exactly `{"mode":"scan","target":"both"}`. Modes scan/reinit/white/checker/identity/animate; targets both/left/right. Returns202 queued,400 invalid,409 maintenance,503 bus task unavailable. Scan always covers the full bus; target selects the timed pattern destination. Latest request wins. Patterns expire after5 seconds, with settings restored. No actuator session is acquired or renewed.

Status `eyes` adds play_mode/category/energy and pose pupilX/pupilY/phase. `eyes.check` reports mode,target,request,remaining_ms,scan_complete,scanned,last_scan_ms,left_probe_code,right_probe_code,found_addresses. Addresses are decimal7-bit values; last_scan_ms is uptime, not a wall clock. Raw probe codes0 ACK,2 address NACK,5 timeout,255 untested. See EYES_DESIGN.md for address collision and physical-verification limits.


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

## Build identity v0.3.4

`GET /status` adds `build_time_utc`, ISO 8601 UTC build-start time (`YYYY-MM-DDTHH:MM:SSZ`) from the compiled image. It is independent of `uptime_ms` and firmware version; clients must not infer clock synchronization or upload time from it.

## Hardware telemetry v0.4.0

`GET /status` adds a `hardware` object (API v1 remains compatible): `chip_model` string, `cpu_freq_mhz`, `heap_size_bytes`, `free_heap_bytes`, `min_free_heap_bytes` (since boot), `psram_size_bytes`, `free_psram_bytes`, `flash_size_bytes`, `application_bytes`, `ota_slot_bytes`, and `wifi_rssi_dbm` (null when STA is disconnected). Memory sizes are allocator-reported bytes; zero PSRAM means unavailable. App size and running OTA slot size are cached at boot, with app image verification done only once. CPU frequency is not CPU utilization. No CPU usage, measured wheel motion, temperature or filesystem telemetry is provided. Dashboard HTTP latency measures a status request round trip including parsing; it is not ICMP ping.

## Dual OLED eyes (legacy seven-field form)

`POST /eyes` accepts this complete legacy object or the ten-field form documented above:

```json
{"enabled":true,"automatic":true,"blink":true,"swap":false,"expression":"happy","brightness":160,"tempo":1.0}
```

Four switches are JSON booleans; expression is a catalog ID from GET /eyes/catalog (the original seven IDs remain valid); brightness integer1..255; tempo finite0.5..2.0. Unknown/missing/extra/duplicate fields or invalid values return400. Maintenance returns409. Accepted response200 means settings stored in RAM, not confirmed display pixels. Settings are volatile and independent of motor Manual/Auto mode, session and heartbeat. No lease required; this endpoint must never renew motor or client producer freshness. Other clients on the lab LAN can change cosmetic settings. Manual expressions still blink if enabled. Automatic reactions derive from ramped commanded output; arm angles are logical, not pulse-mirrored.

GET /status adds `eyes`: the seven settings, active_expression, reaction, task_ready, left_online/right_online (physical panels' last successful I2C transfer), errors (failed transfers), frames (animation iterations, not successful physical frames), sda/scl, left_address61/right_address60 (7-bit), and logical left/right poses with x/y/width/height in128x64 coordinates. `swap` exchanges which logical image is drawn on each physical address. Pose preview is illustrative; polling is slower than device animation. GPIO/LEDC control and APIv1 command schema remain unchanged.

Wheel polarity correction: v0.5.0 uses invertLeftMotor=false, invertRightMotor=true. Logical +/+ forward drives GPIO12/11; -/- reverse drives13/10. The v0.3.2 mapping above is historical.

Python: `robot.set_eyes(automatic=False, expression="love", brightness=160, tempo=1.0, blink=True, swap=False, enabled=True)` works without acquiring motor control. Eye changes never extend an old drive target.
