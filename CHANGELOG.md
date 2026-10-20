# Changelog

## v0.14.1 — 2026-10-20 WIB

- Previous: v0.14.0 demo capped25% while operator found15% immobile and60–80%
  necessary for visible loaded motion. Raw-PWM linear model missed this behavior.
- Changes: optional min_pwm/deadband_pwm,80% upper limit, unchanged15/0/0 defaults;
  shared effective-effort integration/inversion in planner and calibration, explicit
  turn braking inside tolerance and minimum-speed readiness guard. Advanced UI
  percentages and guard explanation. Any PWM change invalidates all motion results.
- Unchanged: raw HTTP/TCP commands, pins/partitions, Wi-Fi/open-LAN policy, single
  GPIO writer,500 ms expiry and no automatic restart. Laser remains off in demo.
- Tests: expanded native/schema/random path/service and browser percentage/guard
  regressions; exact counts, build identity and OTA outcome in VERIFICATION.
- Limitations: measured position unavailable; operator floor observations were
  rough estimates. No completed calibration, autonomous physical path or soak proof.
- Upgrade: reads old15-field config/19-field stored data, writes new21-field data
  under same demoV1 key. Old firmware cannot read the extended object. No NVS erase;
  reboot clears hand confirmation/Zero/run, requires commissioning again.

## v0.14.0 — 2026-10-20 WIB

- Previous: v0.13.0 manual/external-engine control, no autonomous bounded-model demo.
- Changes: shared portable planner for no-output simulation and explicit onboard
  Auto demo: random goal, turn, forward, brake, stopped hand/box-eye gesture,
  repeat until Stop. Top-view map, table/robot footprint/clearance, operator Zero,
  wheel/hand calibration, bounded256-point trace, model/phase/stall cancellation.
- Add internal demo lease isolation, exclusive config-write gate, max25% PWM,
  max5 cm/s/30 deg/s modeled speeds, scoped run-ID Stop and existing global Stop.
  Dynamic hands suppressed only for demo; temporary eyes don't overwrite settings.
- New HTTP demo APIs and NVS robot/demoV1 for config/calibration. Zero/position/
  running state never persists; hand confirmation resets at boot. No auto resume.
- Unchanged: pins/partitions, Wi-Fi profiles/access, TCPv1 wire commands, single
  GPIO writer and500 ms valid-command expiry. Laser stays off throughout demo.
- Tests: native planner/service/ownership and eye overlay, Chromium wizard/map/
  simulation/readiness/Stop/reopen, full regression, sanitizers and pinned build.
  Exact counts/build identity and actual OTA/bench scope: VERIFICATION.md.
- Limitations: open-loop ideal estimate, not physical localization or edge
  detection. Fenced tabletop/operator measurements required for real movement;
  no claim of physical containment or24-hour endurance.
- Upgrade: app-only OTA, no unrelated NVS erase. Saved geometry/calibration stays;
  reboot requires physical hand confirmation and new Zero before real Start.

## v0.13.0 — 2026-10-19 WIB

- Previous: v0.12.0 attempted saved profiles by stored priority, including absent
  networks, with a Wi-Fi panel buried under OTA. User found access difficult and
  requested strongest-signal redundancy with a simpler interface.
- Changes: scan at boot/loss/retry; strongest visible saved SSID first, then next
  on15 s failure.30 s after candidate failures;120 s rescan when none visible or
  scan fails. Manual selection may try a hidden/absent profile first. Stay on
  healthy connections; no continuous strongest-signal roaming.
- Show active Wi-Fi, IP, live RSSI/bars at the top, separate hotspot access and
  last-scan signal, simplified saved Sambung/Hapus and one save/connect action.
  Keep existing APIs; GET/wifi adds nullable RSSI, selection/busy/retry/scan age.
  /wifi/order remains a signal-tie preference for legacy clients.
- Unchanged: pins, partitions, NVS wifiProfilesV1/profiles, shared lab AP/open LAN,
  HTTP/TCP ports, ownership and500 ms expiry. No automatic target/session resume.
- Tests:84 Python/browser tests and8 native suites; network bounds/ties/ranking,
  hidden manual selection, scan failure/timeout, missing-network120 s cooldown,
  ownership/OTA guards, no healthy roaming, and desktop/mobile visual review.
- Limitations: signal is not internet/latency proof. Shared AP/STA radio can
  interrupt requests. Hidden saved SSIDs need manual connect; successful failover
  between two routers and24-hour endurance are not established by host tests.
- Upgrade: app-only OTA retains user profiles with no NVS erase. Actual artifact,
  upload and device evidence are recorded separately in VERIFICATION.md.

## v0.12.0 — 2026-10-19 WIB

- User-approved replacement of fixed-SSID-only policy: asynchronous SSID scan,
  five ordered saved profiles, manual connect/update/delete/reorder, automatic
  15-second attempts and30-second cycle cooldown. Stay on a healthy connection.
- Keep recovery AP active from boot; add authoritative NVS wifiProfilesV1 with
  one-time legacy/default import, atomic save and no unrelated key erase. Never
  return profile passwords. New operator credentials are not placed in source.
- Add idle-only network reservation, stop/invalidate on link loss and gate new
  HTTP/TCP commands/acquisition plus OTA while transitioning. Servo reference
  holds, wheels/laser stop; no stale session/target resume. HTTP80 Wi-Fi APIs and
  dashboard scan/save/priority/connect controls; add network_busy/status phase.
- Keep pins/partitions, TCPv1 commands and open lab access. New NVS schema and
  persistent AP/profile behavior are documented in WIFI_PROFILES_V012.md.
- Add tests of actual network service using fake Wi-Fi/NVS, updated policy gate,
  and browser configuration/error/mobile behavior. Build/OTA/real failover
  evidence is recorded separately in VERIFICATION; host tests are not router tests.

## v0.11.0 — 2026-10-19 WIB

- Add open-LAN TCP9000 JSON-lines v1 server for an external engine: hello/status,
  explicit mode, program acquire, complete command, scoped release and global stop.
  One connection,1024-byte frames,250 ms framing/send deadlines, strict schema and
  bounded parser depth/UTF-8; command pipelining rejects instead of replaying a queue.
- Bind TCP leases to the originating socket generation; other sockets/HTTP cannot
  command or release them. Existing HTTP explicit mode changes/global stop retain
  authority. Single output task,500 ms expiry, arm limits/trajectory remain.
- Add synchronous dependency-free Python SDK with fresh producer timestamps,
  correlated ACKs, bounded I/O and no automatic heartbeat/retry/reconnect; read-only
  discovery example and backend contract. HTTP status advertises TCP readiness.
- Add shared native protocol/socket coverage and Python client integration tests.
  Build/OTA/device evidence is recorded separately in VERIFICATION. User confirmed
  unknown initial physical pose, so live tests must not enable servo/laser.
- No pin/partition/NVS/lab-network policy change; old24-hour soak stays partial and
  monitoring paused. Transport availability is not a stability certification.

## v0.10.0 — 2026-10-17 WIB

- Preserve integer slider/numeric drafts across polling/release; show separate0.1-degree reference, pulse state and stop reason. A stale draft is never silently resumed. Explain blur cancellation and keep fresh control alive across a failed cosmetic/status request.
- Natural profile60 degrees/s,240 degrees/s2; Lembut25/60 and Lincah90/360. Existing planner accelerates, cruises and brakes rather than jumping. The interrupted soak left5/20 while a foreign owner prevented cleanup; decimals were not the reason for that speed.
- Add atomic per-arm RAM min/max policy enforced in firmware command validation and intermediate/dynamic references. Reject invalid limits and limits excluding an enabled held reference. No configuration-only arm movement. UI Stop & Apply releases its own session.
- Optional bounded dynamic hands on fresh same-direction forward/reverse commands; idle and in-place turns do not gesture. No wheel generation, no extra actuator/timer owner. Simulated dynamic tests only; all physical wheel commands remain zero.
- Add bounded core ESP-IDF HTTP81 control/status service below actuator task priority, while HTTP80 retains dashboard/eyes/OTA/legacy APIs. Shared strict JSON validators, same-host browser CORS, advertised readiness, single-flight300 ms command requests, independent500 ms firmware expiry. Cross-task AP/OTA flags are atomic. This is queue isolation, not a proven RF/supply fix.
- No pin/partition/NVS/access-profile change. Limits/dynamic mode remain RAM-only and require operator calibration; dynamic resets OFF. Record interrupted v0.9.0 soak as PARTIAL/FAILED and pause its monitor; do not claim24-hour completion.

## v0.9.0 — 2026-10-16

- Clip and mirror animal-eye highlights inside their ellipse/lid silhouette in native and browser renderers; regression covers both Fox eyes across100 phases.
- Add separate Sharingan category: retain `sharingan` (3 tomoe), add1/2 tomoe and distinct Mangekyo pinwheel/flower/star.52 presets,22 box/30 special,13 category selectors. Catalog, Python and preview remain aligned.
- Precompute tomoe geometry/per-frame constants, bound native raster loops, avoid unused atan2, and drop overdue OLED work instead of catching up against core0 Wi-Fi. Add OLED work_us/max_work_us/overruns and network MAC/disconnect/reset telemetry. These changes are hypotheses to measure, not a proven RF/power fix.
- First complete command with disabled servo pulses now seeds its explicit requested reference, avoiding a transient path through assumed90 degrees after OTA. Subsequent targets remain speed/acceleration limited. A supplied target is not measured position; use an accepted setup pose.
- Add guarded standard-library Python24-hour runner, JSONL/CSV/checkpoints, seeded scenarios, bounded recovery, scoped cancellation, single-process lock and durable Windows launcher. Every wheel command is zero; includes small slow servo, laser and combined cases. Codex heartbeat monitors the actual process.
- Preserve500 ms expiry, sessions/sequences, single actuator writer, both mounting rotations, swap, lab network/open access, pins, partitions and selective NVS migration. Build supports bounded compiler jobs and explicit incremental library/core reuse while regenerating identity and rejecting source changes during compilation.
- Build, smoke, device and24-hour results are recorded separately in VERIFICATION;24-hour completion is never inferred from launch.

## v0.8.0 — 2026-10-16

- Smooth arm reference motion in the existing control task: speed/acceleration limits with braking, retargeting without heartbeat restarts, and stop/timeout hold at the current setpoint. Default45°/s and120°/s²; no jerk-limit or measured-position claim.
- Add RAM-only /api/v1/arms/settings and web Gentle/Natural/Lively profiles, plus Python set_arm_motion(). Reject settings during an active lease/maintenance. Status exposes settings, requested destinations and reference-moving flag.
- Correct reported reversed left/right turning centrally by swapping wheel targets before existing channel polarities. Equal-wheel forward/reverse and servo mirroring remain unchanged.
- Add Sharingan, Rinnegan, cat, fox/Kurama, snake and toad eye styles in a separate Hewan & shinobi category.47 presets total:22 box and25 special. Reactive/Playful remain box-only.
- Preserve both180-degree OLED mounting defaults, access/Wi-Fi/NVS policy and OTA partitions. Physical motion/appearance and OTA acceptance recorded separately.


## v0.7.0 — 2026-10-16

- Keep Reactive/Playful in one rounded-box eye family:22 compatible expressions. Separate19 symbol/cartoon presets in the chooser; Manual and Showcase retain access to all41 IDs. Low-angle hand reaction uses shy box eyes instead of switching to hearts.
- Replace repeated text icons with rendered two-eye thumbnails, motion captions and hover/focus animation. Main preview now animates continuously instead of holding one-second pose snapshots. Strengthen laughter/giggle/skeptical/thinking/bored and other pose differences.
- Add independent physical left/right OLED180-degree mounting rotation, separate from logical swap, applied at initialization and runtime, including splash and diagnostics. Both rotation defaults are true for the user's upside-down installation.
- Extend /eyes to accept12-field settings while preserving7/10-field clients and their existing orientation; add status rotate_left/rotate_right and catalog family metadata. Python accepts optional rotation booleans.
- No actuator, pin, network, partition or NVS migration change. Eye settings remain volatile; rotation resets to the installation default (both180 degrees) on reboot.


## v0.6.0 — 2026-10-16

- Expand to 41 expressions in 10 categories, including separate left/right winks, double/alternating blink, Googly pupils, stars, hearts, spirals, dollars, glitch and comic error/loading styles.
- Add reactive, playful, showcase and manual animation policies; category rotation and three energy levels. Idle now rotates expressions instead of becoming permanently sleepy.
- Add firmware-owned GET /api/v1/eyes/catalog; grouped Eye Studio, local draft preview and extended cosmetic settings. Legacy seven-field clients remain accepted.
- Add queued OLED scan/reinit/white/checker/identity diagnostics; 100 kHz scan of 7-bit addresses8..119, raw expected-address probe codes and five-second test expiry. All I2C stays outside HTTP and actuator tasks.
- Preserve motor/servo polarity, pins, control sessions, 500 ms timeout, Wi-Fi, open LAN access, partitions and NVS migration. Eyes remain volatile. Build/OTA evidence is recorded separately in VERIFICATION.md.


## v0.5.0 — 2026-10-16

- Correct reversed forward/back by flipping both motor driver polarities; forward now uses GPIO12/11, reverse13/10. Logical command signs, ramp and steering stay consistent.
- Add dual SSD1306 128x64 eyes on SDA8/SCL9, left0x3D/right0x3C, with logical display swap.
- Add independent eye animation task, seven expressions, motion/arm/laser reactions, natural blink, idle glances/winks/sleepiness, boot version/build identity, missing-display retry and OTA pause.
- Add Eye Studio preview/settings and POST /api/v1/eyes plus additive eyes status and Python set_eyes(). Cosmetic requests do not renew motor control or producer freshness.
- Eye settings are volatile; network/access/NVS schema remain unchanged.

## v0.4.0 — 2026-10-12

- Adapt supplied Stitch design into a bundled responsive dark/cyan dashboard with sidebar navigation, robot banner, control/arm panels, system telemetry, JSON status and OTA.
- Add hardware status: chip/CPU frequency, internal heap/free minimum, PSRAM, flash, app/OTA-slot sizes, STA RSSI. Cache immutable image/partition sizes at boot.
- Show real telemetry and browser HTTP response latency, without mock CPU load, temperatures, filesystem or WebSocket claims.
- Preserve generated build UTC, mode/session/stop behavior, GPIO mappings, lab credentials, and NVS migration.

## v0.3.4 — 2026-10-12

- Show generated build-start UTC timestamp in serial, dashboard and API status.
- Clean build and reject stale/missing metadata in final application; publish SHA-256 build manifest with CI binaries.
- Require versioning/change-notification rules in AI entrypoints and record changes across handoffs.
- Preserve control, wheel polarity, lab STA/AP and NVS behavior.

## v0.3.3 — 2026-10-09

- Retry stalled STA connection attempts every 30 seconds with recovery AP retained.
- Log Wi-Fi disconnect reason/count, connection status, RSSI, heap, uptime and boot reset reason for USB diagnosis.
- Preserve left wheel inversion and the approved lab/NVS profile.

## v0.3.2 — 2026-10-09

- Invert left wheel polarity centrally for dashboard and API commands to match the installed right wheel direction.
- Read frequency from the mapped active pin; preserve logical signs, ramp, reversal pause, and timeout.

## v0.3.1 — 2026-10-09

- Disable Wi-Fi modem sleep for LAN control after device tests observed HTTP jitter close to the 500 ms lease. Expose Wi-Fi sleep state in status.


## v0.3.0 — 2026-10-09

- Connect dashboard status automatically, retry failures, and show an explicit connection state.
- Add explicit Auto/Manual modes; Manual boot default, mode changes stop and invalidate the session. Python clients explicitly select Auto before program acquisition.
- Acquire manual ownership on button/slider interaction; release only the dashboard session on blur, hide, and button release.
- Mirror left servo pulse angle centrally as 180 minus logical angle.
- Check LEDC write results and report driver duty/frequency readback; add actuator-boundary and mode/recovery/race tests.


## v0.2.0 â€” 2026-10-09

- Explicit open-LAN robot profile: remove API bearer-token checks and both OTA passwords.
- Remove dashboard token input and Python token requirement (legacy argument ignored); report open_lan access mode.
- Migrate NVS by removing only obsolete API/OTA authentication keys, retaining approved lab Wi-Fi and unrelated settings.
- Preserve sessions, sequence validation, producer expiry, command timeout, and OTA stop interlocks.
- Document the user-approved decision in Obsidian and publish under MIT.

## v0.1.1 â€” 2026-10-08

- Correct shared AP password to 12345678 and embed the user-approved HuaweiJIN lab STA profile.
- Migrate old STA/AP NVS values without erasing API/OTA secrets; reject other runtime Wi-Fi profiles in this lab build.
- Expose target/active SSID in status and serial info; require Obsidian credential-rule reading in agent entrypoints.
- Add a build/CI gate rejecting incorrect lab network defaults.

## v0.1.0 â€” 2026-10-08

- Initial ESP32-S3 robot motor, hand servo, and laser controller.
- Authenticated v1 HTTP API, manual dashboard, and Python client with producer expiry.
- Independent actuator task, expiring control sessions, motor ramp/reversal, and manual takeover.
- STA with recovery AP, NVS settings, browser OTA, and opt-in ArduinoOTA.
- Pinned build, two OTA slots, logic/client/browser tests, and GitHub Actions validation.

Pre-release baseline; physical device acceptance is pending.
