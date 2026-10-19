# Continuation status — v0.14.0 editable table demo

v0.14.0 implemented and app-only OTA verified at192.168.1.22, expected
MACe0:72:a1:fd:0d:88/buildUTC2026-10-19T20:42:42Z (20 October03:42:42 WIB).
Read DEMO_V014.md and current VERIFICATION entry before continuing. User wants
table dimensions/reach editable in web and will enter estimates later; fields
remain blank, no fabricated physical geometry or calibration was persisted.
Simulation uses private virtual control; real onboard demo requires explicit
Auto, valid measured configuration/calibration, fresh hand confirmation and Zero.
Closing browser does not stop real demo. Global Stop remains unconditional;
demo Stop requires current run_id. Never resume a session/pose after reboot/fault.

90 Python/browser and10 native suites PASS; planner/service sanitizers PASS;
clean core3.3.10 build/app1,230,672 bytes/source hashes/OTA slots PASS. Live no-motion
API:35 normal samples/0 failure,p95~62 ms,p99/worst~63 ms; six expected rejection
cases. Two postboot timeouts before recovery retained. HTTP80/81 demo and read-only
TCPhello/status work. Live desktop/mobile UI inspected, no POST/JS error/overflow.
Final checked Manual/unowned, all wheel duties0/laserOFF/pulsesdisabled, demoidle,
Zeroinvalid and realStartdisabled. No actual actuator command/calibration/demo
run this delivery. User fills size, establishes safe hand pose/limits, measures
forward/CW/CCW one-second tests on fenced table, then repositions/Set Zero before
real Start. Physical drift/containment remain unverified. Old soak stays partial,
monitor paused, no automatic restart. Build/time is compiler identity, not clock.

## Historical v0.12.0 saved Wi-Fi and failover

Latest source adds scan/5 saved priority profiles/failover and always-active
recovery AP, under new explicit user policy replacing fixed-SSID-only behavior.
Read WIFI_PROFILES_V012.md and latest VERIFICATION. NVS wifiProfilesV1 is now
authoritative; no boot overwrite of saved credentials. Preserve no-wheel/unknown
servo pose constraint. Network changes invalidate control, do not resume targets.
Build/upload/physical failover evidence is reported separately; old soak stays
partial/paused, no new automation started.

v0.12.0 app OTA subsequently verified over recovery AP192.168.4.1, exact
buildUTC2026-10-19T12:46:30Z (19:46:30 WIB), app1,199,472 bytes;82 Python/browser
tests and8 native suites PASS. Initial postboot HTTP check failed until laptop
re-associated to the same saved AP. Actual scan/CRUD/network guard worked;
follow-up trace observed temporary-profile attempt -> HuaweiJIN at15.594 s ->
waiting30.625 s, with8 failed requests retained. Initial live test also timed out;
all temporary profiles removed, sole original HuaweiJIN restored. No successful
second-router association claimed. Real UI read-only inspected. Final wheels0,
laserOFF/pulsesdisabled/no owner; no actuator command sent. See latest VERIFICATION
for exact failure/recovery evidence and limits. No new soak; old monitor paused.

Latest source adds TCP9000 for an external LAN engine: read TCP_CONTROL_V011.md
and the latest VERIFICATION entry. JSON lines, explicit Auto/program, one socket,
bound sessions,500 ms expiry and no command replay. User confirmed unknown
physical initial servo pose; live verification must send no command and must not
enable servo/laser until an accepted pose is supplied. Old soak remains FAILED/
PARTIAL, monitor PAUSED. Build/OTA/live outcome is recorded separately below.

v0.11.0 app OTA succeeded: exact buildUTC2026-10-19T03:04:54Z (10:04:54 WIB),
MACe0:72:a1:fd:0d:88, app1,185,408 bytes.75 Python/browser tests +7 native suites
PASS (TCP suite also ASan/UBSan). Actual no-actuator suite:50 status requests,
0 normal failures,p95=46 ms,p99/worst47 ms; session arbitration/expiry, framing
and slow-client faults verified. First slow-reader setup failed to establish
closure; original evidence retained and corrected setup/probe recorded in
VERIFICATION. Final Manual/unowned, all wheel duties0, laserOFF, pulsesdisabled;
no command requests or physical actuator tests. Backend's real engine remains
to be integrated against the documented contract; no24-hour acceptance claim.

## Latest source — v0.10.0

App OTA and bounded live verification are complete: exact v0.10.0/buildUTC2026-10-16T17:04:24Z/MAC confirmed,34 zero-wheel commands with no unexpected request failures (p95~94 ms,p99/worst~109 ms), reference+2 degrees/return, HTTP80 partial-header overlap with HTTP81 control, CORS/policy checks and final scoped release. Laser off, wheels0, dynamicOFF, reference held99.0164/102.635. Real desktop/mobile dashboard inspection passed with no POST requests. Raw evidence stays ignored underbuild/{ota,live,browser}-v010-result.json; see VERIFICATION for exact limits. This is short-run evidence, not24-hour acceptance or physical angle feedback.

Servo/UI/HTTP81 fixes, per-arm RAM min/max and optional bounded dynamic hands are implemented. Read SERVO_CONTROL_V010.md and the latest VERIFICATION entry for build/OTA/live evidence. Natural60/240; dynamic OFF at boot. Tests: six native suites and64 Python/browser tests passed. Firmware buildUTC2026-10-16T17:04:24Z (2026-10-17 00:04:24 WIB), app1,179,616 bytes. No physical wheel test is authorized; do not restart the failed old soak automatically.

## Interrupted run, servo follow-up

The original24-hour runner is **FAILED/PARTIAL**, not running: ended2026-10-16 21:51:33 WIB after1 h34 min15.719 s on conflicting controller ownership. Monitor is PAUSED. See [the full partial result](SOAK_RESULT_2026-10-16.md):13,609 requests/130 failures,8,202 zero-wheel command attempts/32 command failures, no recorded reboot/Wi-Fi disconnect. Final idle verification failed because of ownership conflict; do not label the last older idle snapshot a verified final state.

User now requests fixing slider/control offline behavior, visible angles, configurable per-arm limits, faster accelerate/cruise/brake motion and optional dynamic arm movement during forward/reverse; then tested source push and app OTA. Do not restart the old soak. All physical wheel test commands still must remain zero. Dynamic wheel reactions are tested in simulation. The previous body below is historical launch evidence and must not be treated as current process status.

Issue#2 has been consumed on the testing laptop. Feature branch remains feature/initial-robot-controller; PR#1 targets develop and must remain unmerged. Read the latest VERIFICATION entry and SOAK.md before further work. The original handoff is preserved below as historical problem definition.

Implemented and tested: Fox clipping/mirroring, six distinct Sharingan designs/category (52 presets), native raster precomputation/clipping, dropped OLED catch-up work, network/reset/OLED telemetry and guarded durable soak runner. First command after disabled pulses seeds an explicitly accepted reference instead of traversing assumed90 degrees; subsequent rate limits remain. No wheel tests, pin/NVS/partition/profile/access change or500 ms timeout extension. Five native suites,12 client/7 policy/2 identity/10 soak tests and28 Chromium simulations passed. Native/browser contact sheets inspected; physical pixels remain unobserved.

App-only OTA verifiedv0.9.0/buildUTC2026-10-16T13:10:42Z (20:10:42 WIB), application1,138,640 bytes, SHA256314a75148b86bb16051a2a414a259cdb9b4e22e0b0cdbeb518f6bbc138c133c4. Boot identity/idle outputs/HTML verified at192.168.1.66, MACe0:72:a1:fd:0d:88 via Ethernet192.168.1.122. Source snapshot and regenerated binary identity matched; final build reused core/library objects (see VERIFICATION for exact build evidence).

Smoke144.188 s completed all10 scenarios:527 zero-wheel commands,0 command failures,commandp95=48 ms/p99=78 ms,worst ACK gap234 ms. Normal non-slow-client requests721/0 failures. Intentional partial-header slow client caused2 status/recovery timeouts; recovered without radio disconnect/reboot. Producer expiry verified laser off/session invalidation. Thus normal short-run improved, but synchronous slow-client stalls remain a known limitation and causation of the original intermittent outages is still unproven. No exhibition-ready claim.

**Historical launch below: this process subsequently failed; see latest status above.**

- StartUTC2026-10-16T13:17:17.4144055Z =20:17:17 WIB; planned deadline around2026-10-17 20:17:18 WIB, plus cleanup, if uninterrupted.
- Initial PID1620; verify command line/start time in ignoredbuild/soak-24h-v090.launch.json, never trust PID alone.
- Durable state: build/soak-24h-v090/checkpoint.json, events.jsonl and requests.csv; stdout/stderr use same output prefix.500 ms expiry and zero-wheel boundary guard remain enforced.
- Codex heartbeat pantau-soak-robot-24-jam-tanpa-roda is ACTIVE every30 minutes on the continuation chat. It only supervises the existing job, reports actionable failure/completion, and must pause on terminal state. No auto-restart, target resumption or unattended flashing.
- Cancel by creatingbuild/soak-24h-v090/stop.flag. Runner releases its own session, restores settings when unowned, and verifies final zero wheels/laser off; loss of connectivity leaves final physical state unverified.

Next: inspect durable evidence after24 actual hours; report full/partial duration, failures including induced ones, per-scenario and command percentiles/worst gaps, RF/reset/heap/OLED telemetry and recovery. Then pause monitoring and update report/PR. Do not interrupt a healthy soak to flash a speculative fix. Physical pixels, mechanical response, supply/current/brownout measurements and exhibition RF conditions remain unknown.

---

# Handoff: exhibition connection stability and eye rendering

Updated 2026-10-16 (UTC+7). This is an **open problem**, not a completed fix.

User confirmed: execute on **the other laptop**, already powered on24/7; do not start a duplicate runner or automation on the laptop publishing this handoff. The Codex on that other laptop must consume this GitHub prompt, verify it can actually reach the robot's lab network, then prepare/run the automation itself. Merely staying powered on does not establish LAN reachability.

## Copy-ready prompt for the next Codex on the testing laptop

Continue this repository on `feature/initial-robot-controller`, PR to `develop`; do not merge or touch `main`. Read `AGENTS.md`, README, API.md, VERIFICATION.md, EYES_DESIGN.md, ARM_MOTION_DESIGN.md and this handoff first. Locate/read the actual firmware vault when available using AGENTS' mandatory sequence; standalone approved lab policies are documented in the repository. Audit current branch, remote, working tree and live firmware identity before changes. Preserve existing work.

The user needs this robot for an exhibition. Implement and verify the following, then push reviewed/tested changes to the feature branch and update its PR. Be explicit about what passed and what remains unknown. Do not claim an exhibition-ready connection from compilation or a few successful requests.

### Non-negotiable actuation scope

- **Do not move the robot's wheels. Every test command must have left=0 and right=0.** Never run the existing moving `example.py`. Add an enforced zero-wheel assertion to the soak runner; no nonzero-wheel scenario or wheel-direction physical test is authorized in this handoff.
- Servo tests are authorized. Use small, slow movements near the current accepted pose, within configured limits; avoid unsolicited full-travel sweeps. The boot reference is not a measured physical position. Begin with a short supervised setup where possible.
- User specifically reports disconnections when servo and laser operate. Include laser-only and servo-plus-laser scenarios, keep wheels zero, and end each scenario with laser off. Do not add autonomous aiming or wheel motion.
- On failure, conflicting owner, maintenance, loss of freshness or cancellation: stop producing commands, attempt stop/release, and leave wheels zero/laser off. Firmware's 500 ms expiry remains mandatory; no reconnect may silently resume the previous motion/laser target.
- Keep single actuator output ownership, producer freshness, session/sequence validation, both OLED mounting rotations and logical/physical eye swap semantics intact. Do not mask failures by extending the stale-command timeout.

### 1. Diagnose and improve connection stability

Facts: the installed baseline is v0.8.0, build `2026-10-16T12:13:15Z` (19:13:15 UTC+7), last verified at `http://192.168.1.66/`. IP can change; rediscover/verify device identity instead of assuming. Robot is ESP32-S3 N16R8, two SSD1306 displays, SDA8/SCL9; Wire addresses left0x3D/right0x3C. Device MAC last verified `e0:72:a1:fd:0d:88`.

On 2026-10-16, 16 sequential status requests with a 2-second deadline yielded15 successes/1 timeout. Observed latencies in ms:94,1078,94,1969,219,219,31,94,63,79,63,1984,109,2047,94,78 (2047 was the timed-out attempt). A successful status sample reported RSSI-72 dBm, modem sleep disabled, free heap232572, uptime930074 ms, both OLEDs online/errors0, Shinobi Showcase active. User separately reports intermittent disappearance and more trouble while servos/laser run. Raw logs are local ignored artifacts, not shipped source.

These observations establish latency failures, **not** the cause of radio disconnects. Compare idle/OLED disabled/reactive/Sharingan-heavy rendering with actuator/laser cases. Measure before changing code and record RSSI, uptime/reset, reconnect/disconnect reason/count, heap minima, request latency/loss, HTTP concurrency and OLED work duration. Add useful bounded telemetry if missing; avoid heavy per-request flash work or log flooding.

Source review leads, not proven root causes:

- `eye_raster.h` recomputes expensive per-pixel floats/trigonometry; Sharingan computes tomoe positions inside the pixel loop, and radial detection currently invokes atan2 even for shapes that do not use angle. Reduce repeated work, clip raster bounds and precompute per-frame constants, preserving rendering correctness.
- Eye task is priority1/core0; Wi-Fi also uses core0. Measure budget/overruns and avoid catch-up loops monopolizing CPU. Keep I2C off the actuator task.
- Dashboard runs status polling, catalog loads and command fetches concurrently against synchronous WebServer. Study the **pinned core3.3.10 implementation**, actual browser traffic and slow clients before blaming keep-alive. Prioritize command freshness without letting queued old commands replay later.
- Laptop previously had simultaneous Ethernet/Wi-Fi routes in the same subnet; this is a possible test confounder. Record interface/route used, avoid attributing every timeout to firmware, and do not change routers/routes/security without scope.
- Servo/laser load may reveal a supply/reset problem; that is a hypothesis. A healthy uptime across an outage differs from reboot/brownout. Record reset reason on boot and distinguish confirmed evidence from assumptions. Software cannot prove supply stability.

Lab STA remains HuaweiJIN and AP12345678; authorized source defaults already exist. Open LAN API/OTA is intentional. Preserve NVS migration: update differing lab STA/AP values; remove only apiToken/otaPassword; keep other keys. No arbitrary network/profile/partition changes.

### 2. Fix Kurama and add real Sharingan choices

Kurama/Fox has an identified raster defect: the highlight is unconditionally painted after ellipse/lid clipping in both C++ and JS. It can create a disconnected pixel island above the right lid. Fix the clipping/mirroring coherently in native renderer and preview. Regression-test both eyes across phases, verify no lit pixels outside the intended silhouette, and visually inspect actual native bitmap and browser output. ACK does not prove the physical OLED image.

Sharingan currently has just one3-tomoe preset. Add a tidy separate Sharingan category/subgroup with **visibly different** designs:1/2/3-tomoe and several distinct stylized Mangekyo pinwheel/flower/star variants, for example. Existing sharingan ID stays compatible. Do not merely rename identical thumbnails or vary speed alone. Monochrome128x64 readability matters; compare pairs across animation phases. Keep reactive/playful on the existing rounded-box character; these special eyes belong to explicit Manual/Showcase. Catalog is the source of truth; update validation, Python accepted IDs, previews, thumbnails, native/browser tests and docs together.

### 3. Prepare and actually run an automated 24-hour soak

User explicitly wants the next AI to design its own scenarios and use automation for an all-day test. **Do not claim it ran until an accessible lab laptop actually executes it.** Remote GitHub CI cannot reach this private LAN and is not a substitute for the device soak. If the testing laptop has left the lab, prepare the runner/automation and report blocked device execution honestly; do not hammer guessed IPs all day.

Build a reproducible Python runner with URL/config arguments (no machine-specific paths), scenario seed/config, precise monotonic timing, JSONL/CSV logs, summary percentiles and checkpoints. Enforce the zero-wheel guard before every command. Log version/build identity and network/device uptime for each scenario. Log failures as failures, not discarded retries. A crashed/stalled producer must expire, rather than let a background heartbeat perpetuate stale targets. Do not fight a human/controller session. Provide clean cancellation and final stop/off behavior.

Design a bounded24-hour matrix including idle, reactive OLEDs, heavy Sharingan/Showcase, HTTP/browser-like concurrency, servo-only, laser-only, and combined servo+laser. Include repetition, recovery after request loss and slow-client tests. Test intentional network outages only with an isolated/explicit method; no router reboot or disruptive shared-network changes. OTA interruptions belong in a controlled maintenance scenario, not random unattended flashing. Start with a short smoke run and inspect output before the full soak.

On the actual testing laptop, use its Codex automation capability: prefer a heartbeat attached to its continuation chat, inspecting durable runner progress/checkpoints periodically (for example every30 minutes) and notifying only actionable failures or completion. Launch one durable runner; automation monitors/resumes supervision rather than spawning duplicate24-hour jobs. Keep messages quiet while unchanged. End the soak after24 hours of real execution, summarize complete/partial duration and stop/disable its monitoring schedule. If automation tools are unavailable, state that and provide an explicit local scheduler/launch alternative instead of claiming a schedule exists.

Suggested provisional exhibition target: normal loaded-control p95 response<150 ms and p99<300 ms, no command gap reaching500 ms, no unexpected reboot or stale-target resume, and24 hours without unexplained disconnects. These are targets to assess with the user and evidence, not a guarantee. Report sample counts, worst gaps, scenario durations, failures, retries, disconnect/reboot counts and recovery times; show both percentile and worst-case failures. RF conditions at the exhibition can differ from the lab. Do not call the result universally reliable.

### Delivery and acceptance

Use a new semver for firmware behavior/new presets, generate actual UTC build metadata, run native control/driver/eye/service tests, Python client/expiry/network/identity tests, Chromium dashboard simulations and pinned ESP32-S3 build/size/partition gates. OTA only the app binary when the verified robot is reachable; stop/maintenance/pulses-off first and verify identity plus idle outputs after reboot. Record separate build, simulation, short live test and24-hour soak results. Push feature branch; keep PR to develop unmerged. Update this handoff, changelog and verification report with exact next steps, artifact identity and honest limits.

## Handoff publication status

The current publication ships the previously built/tested v0.8.0 source plus this handoff. **No new firmware fix for the above timeout, Kurama or expanded Sharingan work has been implemented in this publication. No24-hour runner/automation is running as a result of publishing this prompt.** This task must be started on the continuation laptop/chat; creating a GitHub issue does not launch Codex by itself.


## v0.13.0 strongest saved Wi-Fi and simple dashboard — 2026-10-19 WIB

Previous: v0.12.0. User explicitly replaced manual priority ordering with strongest
detected saved SSID first, fallback on failure, visible active Wi-Fi/signal and
simple save/connect/delete UI. Read current AGENTS/README/API/handoff/verification
and actual sibling vault home, folder, firmware, Gitflow, Rules_Kredensial_WiFi,
WiFi_HuaweiJIN, Rules_Akses_Robot_Lab, Rules_Versioning, Rules_Change_Notifications.
See WIFI_V013.md for the current contract; v0.12 ordered-attempt notes are history.

Changes: asynchronous scan at boot/loss/retry; strongest VISIBLE SAVED candidate
first,15 s each, then30 s cooldown. If none seen or scan fails, no blind automatic
association attempts; wait120 s before scanning again. Explicit manual connect
can attempt hidden/absent SSID first. Healthy link never roams/scans automatically.
Rank all5 saved SSIDs even beyond24 public scan rows; duplicate BSSID RSSI uses
strongest, equal RSSI keeps stored order. /wifi/order retained as tie preference.
Active SSID/live RSSI/bars/IP now top-level, distinct from last-scan signals and
browser hotspot access. One existing POST/settings saves/connects; simple saved
Sambung/Hapus. GET/wifi adds selection/busy/retry/scan age and nullable RSSI.
Hostname is set before Wi-Fi start per core API; timed-out driver scan is stopped.

Unchanged: pins, two6 MiB slots, core3.3.10, Wi-Fi profile NVS schema/list,
AP password/open LAN, HTTP80/81/TCP9000, ownership/500 ms expiry, no stale resume.
Upgrade: app-only OTA preserves NVS profiles, no erase/unrelated key modifications.

Tests:84 Python/browser tests PASS (39 dashboard,13 HTTP,11 TCP,9 policy,2 identity,
10 soak simulation);8 GCC C++17 Wall/Wextra/Werror suites PASS. Expanded network
test (actual service/fake SDK) also ASan/UBSan PASS including full5-candidate array,
RSSI ties/duplicate APs, profile deletion/cache correspondence, >24 results,
scan failure/timeout, absent-network120 s wait, preferred manual override and
strongest fallback. Existing control/TCP/eyes/driver/OTA reservation tests passed.
Mock desktop/mobile visual review passed. Native evidence remains simulation.

Pinned incremental firmware build PASS: v0.13.0,
buildUTC 2026-10-19T13:19:03Z =2026-10-19 20:19:03 WIB.
App1,203,456 bytes; program1,203,306/staticRAM55,008; two6,291,456-byte slots verified.
App SHA256 65d70241a133c930e017876a1683f18006ec8243fa400425ce2b1107c92f3192.
Source-input hashes passed and were checked again before upload; clean_build=false.

Actual AP192.168.4.1 matched MACe0:72:a1:fd:0d:88, v0.12.0/build12:46:30Z,
no owner/session, wheels0/laserOFF/pulsesdisabled before OTA. FIRST upload failed
HTTP400; subsequent polling confirmed old version, not a successful flash. Exact
cause not proven; busy network is plausible. Evidence ota-v013-result.json.
Waited for observed connecting -> waiting transition (ota-v013-idle-window.json)
then performed ONE retry; upload acknowledged ok/rebooting. Windows remained
associated to same AP/BSSID but ARP unresolved after reboot; disconnect/reconnect
the SAME saved hotspot restored access. No router/route/firewall change. Final
readback matches exact v0.13.0/build/MAC, idle wheels/PWM0, laserOFF/pulsesdisabled,
HTTP81/TCP ready and one original saved profile. Evidence ota-v013-retry-result.json.
Do not hide first rejection or claim uninterrupted reboot/reassociation.

Actual device test PARTIAL: 53.031 s, 47 requests,
1 failure retained. Manual async scan returned 10 SSIDs,
sorted RSSI, offline current_ssid empty/RSSI null, passwords absent, profiles
unchanged. The failure was the first GET/wifi at1.22 s during manual scan,
timeout3 s; later polling completed. All42 status reads succeeded; p95
250.0 ms, p99 1859.0 ms, worst
2328.0 ms (successful samples; timeout is separate). Whole
suite passed=false, not relabeled PASS. Final uptime119917 ms, Wi-Fi waiting,
STAconnect0/disconnect4/reason203; these counts include connection attempts,
not4 proven RF outages. No unexpected reboot observed in bounded check. Reason203
does not establish password/RF/root cause here. Evidence live-wifi-v013-result.json.
Final manual/unowned, network_busyfalse, all wheel duties0, laserOFF/pulsesdisabled.
No actuator command and no profile save/delete was sent during this live check.

Actual desktop/mobile dashboard read-only PASS, no JS error/overflow or actuator
POST, correct v0.13.0 and one saved profile. Screenshots wifi-v013-live-desktop.png,
wifi-v013-live-mobile.png and browser-wifi-v013-live.json under ignored build/.

Limitations: scan/AP association still can interrupt HTTP; successful status can
exceed2 s. Not seamless failover or TCP200 ms stability certification. Only one
saved network on device; successful strongest selection/fallback between two
credentialed routers remains native simulation. Actual saved profile survived
OTA; new user credential persistence is not separately reboot-tested. Unknown
servo pose restriction maintained; no wheels/servo/laser activation, new soak or
automation restart. Old24-hour run remains partial; keep feature PR#1 to develop
open/unmerged. Next tests: two real saved networks with operator credentials,
association failure diagnosis and AP/RF latency observation, without moving wheels.
