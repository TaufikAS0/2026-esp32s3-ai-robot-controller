# Verification evidence

## v0.16.0 build, OTA and live checks —20 October2026 WIB

- Incremental pinned ESP32 core3.3.10 build PASS; app1,250,016 bytes, storage
  usage1,249,862/globalRAM57,592; emitted6,291,456-byte slots and source-hash
  stability/identity gate PASS. BuildUTC2026-10-20T10:10:18Z =17:10:18WIB.
  SHA256 f9583e4dc5f9c151c89f5a3e06f0fca00e4ecba9a87a9bf963d5e2c1421290d4.
- App-only OTA success acknowledged;3 postboot status timeouts retained before
  recovery on fourth probe. Exact version/build/MAC verified at192.168.1.14.
  Boot Manual/no owner/wheel duties0/laserOFF/servo pulsesdisabled; table config
  and calibration mask0/profile order preserved. New exhibition center invalid.
- Actual no-motion API suite PASS:34 normal GETs/0 unexpected failures, p95~78,
  p99/worst~94ms. Expected stale result, unready starts and invalid/false floor
  confirmations rejected. No actuator commands. Read-only live desktop/mobile
  browser PASS: no JS errors/POST/overflow; screenshots inspected.
- First operator-observed floor trial: center reset verified NO motion and
  trace empty; start202; reset DURING motion409.18 status/demo samples through
  about4.64s (lastuptime41,326ms) captured turn/drive. Then GET/demo timeout,
  scopedStop timeout and finalGET timeout retained. A recovery globalStop on80
  acknowledged; newuptime14,053ms, all wheel duties0/laserOFF/pulsesdisabled.
  User explicitly reports manually switching off/resetting robot during this
  trial. Classify as OPERATOR-INTERRUPTED, not complete and not proof of a
  spontaneous firmware reset. Raw reset_reason4 retained; no crash trace captured
  at the interruption, so its low-level reset path is not independently proven.
- Diagnostic onboard ISOLATED simulation30s PASS: one goal/gesture, bounded trace,
  no real owner/wheel/laser command, no reboot (uptime63,654 to~94,000ms). UART
  COM11 opened with DTR/RTSfalse without reboot; no serial bytes during this
  quiet simulation. Scoped simulationStop acknowledged and center invalid.
- Recovery idle verified; no automatic physical retry. Await fresh operator
  center/front/pose setup before any next motion trial. Ignored build/ota-v016,
  live-v016, browser-v016 and exhibition-v016*.json preserve raw evidence.
  Old soak remains partial and monitor paused. No endurance/containment PASS.

## v0.16.0 exhibition — source and host checks

- Read vault home, workspace naming, firmware standard, Gitflow, credential/profile,
  robot LAN access, Versioning and Change Notifications. Existing saved Wi-Fi
  selection/AP/access policy preserved. No network changes or NVS erase.
-101 Python/browser tests PASS (56 Chromium),10 native C++17 Wall/Wextra/Werror
  suites PASS; planner/service ASan/UBSan PASS. Exhibition model400k20ms steps
  over10 seeds including timer wrap stays within±15cm axle bounds; left90–150,
  right90, no wave while wheel output nonzero, Happy and return90 checks.
- Source/service checks cover no NVS change/calibration promotion, RAM reset,
  simulated run no real lease/commands, scoped stop during every phase, invalid
  configs/duplicate fields, reset no GPIO/acquire, foreign owner/new session,
  invalid center after Stop/replacement, network/OTA/stall and reboot.
- Four new Chromium tests PASS: explicit reset/start/stop, reload no replay,
  simulation/config paths, foreign owner and mobile. Entire101-test regression
  PASS. Mobile screenshot visually inspected/no overflow. Initial new planner
  assertion expected Happy on the single transition INTO Gesture; corrected
  test to require Happy once wave starts (existing transition has no eye override).
- Device at192.168.1.14 verified v0.15.0/build07:40:11Z/MAC before upload; idle,
  no owner. Post-build/OTA/live results recorded below when actually complete.

Current source: v0.16.0 (2026-10-20). Entries below distinguish firmware versions, host/model tests and operator-observed hardware trials.

## v0.15.0 guided Auto — 2026-10-20 WIB

- Previous v0.14.1. Implemented approved AUTO_GUIDE_V015.md: explicit pose check,
  sequential forward/right/left test/result steps, units/direction, discard invalid
  motion observations, confirmed Auto/Zero before EACH operator-started test,
  separate final Start, advanced legacy controls collapsed and global Stop above
  the map. No acquisition/actuator commands from load/reload/Save/preparation.
- Pending completed calibration result stays in RAM for the boot without120 s
  expiry. It holds no lease. Invalidates on Stop/foreign controller (including
  whole quick sessions between ticks), network/config/OTA/new run/reboot. Own
  successful release is excluded; ACK and expected epoch checked. Result Save
  rechecks epoch under exclusive idle flash gate. Old/duplicate IDs reject.
  GET/demo adds sample_pending/sample_reason/config_valid/calibration_forward_fits.
  NVS demoV1 format/gains preserved, not stored pending state. No pin/partition/
  network/access/raw HTTP-TCP/500 ms expiry change or auto resume.
- Actual vault files read: Firmware_AI_Vault_Home; Rules_Nama_dan_Folder_Workspace;
  Rules_Firmware_Standar; Rules_GitHub_Gitflow; Rules_Kredensial_WiFi;
  WiFi_HuaweiJIN; Rules_Akses_Robot_Lab; Rules_Versioning;
  Rules_Change_Notifications. User-saved STA remains authoritative, no Wi-Fi change.
- Local97 Python/browser tests PASS, including52 Chromium dashboard tests. All13
  demo tests repeated PASS after final UI placement. Ten native C++17 suites
  Wall/Wextra/Werror PASS; planner/service ASan/UBSan PASS after final control fix.
  Coverage includes130 s waiting without lease, all invalidation events, quick
  acquire/release, new sample IDs, duplicate results, failed stop ACK, reboot/gains,
  full wizard sequence/reload/direction/units/discard/mobile/space explanations.
  Initial wizard tests failed because mock status omitted owner; corrected fixture
  and full regressions passed. No production guard weakened to satisfy tests.
- Pinned core3.3.10 incremental ESP32-S3 N16R8 build PASS, source hashes/binary
  identity/emitted two6,291,456-byte slots verified. Final v0.15.0/buildUTC
  2026-10-20T07:40:11Z =20 October2026 14:40:11 WIB. App1,241,840 bytes;
  program storage1,241,698/global RAM57,576. SHA256
  `0ef348562bb3a7b077e54f8508784998265a46bcd71a40feaf6dc7a2c774b7c1`.
  Incremental reuses core/library objects, not labeled clean. One intermediate
  build was rejected by the source-change gate during final ACK-guard work; not
  uploaded. The initial deployable07:35:25Z artifact was subsequently superseded
  only to move mobile sticky Stop from bottom to top (bottom overlapped guide).
- Both app-only OTA uploads acknowledged success, no failed upload retry. First
  reboot had3 status timeouts before recovery. Final reboot had5 failed probes
  before recovery, all retained. Final exact version/build/MACe0:72:a1:fd:0d:88
  verified at192.168.1.22; saved Wi-Fi profiles, geometry and calibration mask
  preserved. Boot Manual/unowned, all wheel duties0/laserOFF/pulsesdisabled,
  demo not running/Zero invalid/hand confirmationfalse/sample_pendingfalse.
- Final no-motion API PASS:34 normal GET samples/0 failure,p95~63ms,
  p99/worst~109ms; uptime31,761→35,964ms, Wi-Fi disconnect0→0. No additional
  reboot observed during this4.203s idle sample. Expected rejection of stale
  result ID and real Start retained. No configuration/actuator commands sent by
  the verification suite. Initial deployed UI suite also passed34 reads/0fail,
  p95~62/p99/worst~63ms; these brief samples are not sustained latency guarantees.
- Final actual desktop/mobile browser PASS: Siapkan Auto opens preparation,
  advanced controls collapsed, no old Start/Zero duplicated in main flow, no
  JS error/overflow/POST. Mock pending-result and live mobile images visually
  inspected; final Stop position corrected. Actual pending-result persistence
  across a REAL motion trial/reload has not been exercised; native/browser tests
  cover the behavior without actuators. Ignored build/{ota,live,browser}-v015*.json,
  wizard-v015-*.png and build_manifest retain evidence, including initial artifact.
- LIMITS / remaining physical step: stored75x75/reach15/margin10/coast0/speed5
  geometry is a user-authorized temporary MODEL. Its10cm half-goal extent is
  smaller than initial forward prediction12cm, so calibration_forward_fits=false.
  The guide explicitly explains the guard; no fabricated larger table/gains or
  old45°/3–5cm observation is accepted automatically. Asked operator for actual
  table size and current physical pose before movement; no answer yet at this
  record. No physical actuator command or new real demo/calibration in this task.
  No edge/containment/endurance proof. Old soak partial/monitor paused unchanged.


## v0.14.1 motor deadband and operator commissioning — 2026-10-20 WIB

- Previous v0.14.0. Operator requested physical Auto testing and delegated temporary
  model values, then confirmed hands90 safe, small88→92→90 safe, table centered/
  fenced, stable wheels-lifted support, and finally a clear floor lane. The chosen
  MODEL75x75 cm/reach15/margin10 generated20x20 cm goals; dimensions were not measured.
  No fake wheel gains/calibration flags were written. Pose/angle remains open loop.
- Physical observations on v0.14.0:15% one-second internal demo calibration produced
  0 cm (operator); result not accepted. Lifted25/40/60/80% stages: operator thought
  40% stationary,60% very slow,80% stronger. Descending80→60→40→25 answer "40%"
  interpreted as stopping at40, not a precise threshold. Floor60%1 s: approximately
  1 cm straight;80%1 s: approximately3–5 cm, direction/coast not explicitly reported.
  These show the25% cap is unsuitable here, not the electrical root cause or an
  exact speed curve. No random physical exploration was started.
- Six direct HTTP trial logs contain150 acknowledged commands/0 request failures;
  separate15% internal calibration has its own log. Commanded wheel/laser zero and
  scoped release verified after each trial, arms held90. Command p95/p99/worst
  ~79/94/109 ms. Per-scenario ALL-request p95/p99/worst ms: pose94/94/94,
  hands78/94/94, ascending79/109/109, descending78/79/79,
  floor60=63/78/78, floor80=78/79/79. Command scheduling gaps were not timestamped
  for every scenario; floor intent duration was1.000 s before scoped release.
  Distances/turns are not sensor measurements.
  Bench records: ignored build/commission-{pose,hands,forward,wheels-lifted,
  wheels-sustain,floor-60,floor-80}-v014.json. No replay or failed-request retry.
- Changes: DEMO_V0141_PWM.md documents optional paired minimum/deadband PWM,
  max80% but unchanged15/0/0 defaults, effective-applied-effort calibration/model,
  explicit turn braking and minimum-speed gate. Changing any PWM setting clears
  calibration. Raw external HTTP/TCP commands remain raw PWM; no GPIO/pin/partition/
  Wi-Fi/access or expiry change. Old storage is read before extended save.
- Read actual sibling-vault Firmware_AI_Vault_Home, Rules_Nama_dan_Folder_Workspace,
  Rules_Firmware_Standar, Rules_GitHub_Gitflow, Rules_Kredensial_WiFi, WiFi_HuaweiJIN,
  Rules_Akses_Robot_Lab, Rules_Versioning and Rules_Change_Notifications. Current
  user-saved STA remains authoritative; no profile/default/access changes or erase.
- Local PASS:92 Python/browser tests (47 Chromium);10 native C++17 suites
  Wall/Wextra/Werror. New/legacy schema, duplicates/ranges, model deadband and
  minimum target,900,000 combined random modeled steps, effective calibration
  integral/invalidation/storage and minimum-speed guard. Planner/service ASan/UBSan
  PASS. Mock advanced desktop/mobile inspected, no overflow/errors.
- Pinned core3.3.10 incremental build PASS (core/library objects reused, source
  hashes/binary identity verified; not labeled clean). v0.14.1/buildUTC
  2026-10-20T03:03:42Z =20 October2026 10:03:42 WIB. App1,232,496 bytes, storage
  1,232,342/global RAM57,576; two6,291,456-byte slots verified. App SHA256
  `b8880597da2fea658533c77ddd36ad98a50317084b54af6b899ee82ec6eba1f3`.
- App-only OTA acknowledged success. Three postboot GET timeouts retained before
  recovery; no automatic reflash. Exact version/build/MACe0:72:a1:fd:0d:88 verified
  at192.168.1.22, saved profiles unchanged. Boot Manual/unowned, wheel duties0,
  laserOFF/pulsesdisabled, demo not running/Zero invalid/hand confirmation false.
- v0.14.1 no-motion API PASS:36 normal reads/0 failures,p95/p99/worst~78 ms,
  uptime56,733→60,918 ms; Wi-Fi disconnect count0→0. Old19-field storage loaded;
  invalid PWM>80/minimum<=deadband/unpaired fields rejected; valid80/60/50 saved
  without actuator activation. Real start rejected pending calibration/Zero/pose.
  Actual desktop/mobile browser PASS: percentage values80/60/50, Startdisabled,
  no JS error/overflow/POST. Ignored build/{ota,live,browser}-v0141-result.json
  and demo-v0141-live-*.png retain evidence. This initial OTA verification sent
  no actuator command; the subsequent commissioning trial is recorded below.
- Limitations: motor supply/voltage/current/friction not measured, left/right
  thresholds not independently measured, coast and CW/CCW calibration missing.
  50% is an editable initial model estimate, not certified breakaway PWM. No
  accepted3-axis wheel calibration, measured physical containment, autonomous
  physical trajectory, recovery-under-load or24-hour proof. Old soak stays partial/
  paused; no automation/runner restarted. Fresh physical commissioning remains.


### Subsequent v0.14.1 clockwise commissioning

- Operator freshly confirmed centered fenced tabletop/front heading and safe90
  pose after OTA. Selected Auto explicitly, confirmed previously tested88–92
  limits, set new Zero and ran one80%/one-second internal CW calibration trial.
  Run/sample1 completed; effective-effort integral0.25719994. All request calls
  succeeded. No physical angle/direction response received before120 s expiry;
  no result was saved, no replay/retry, no completed wheel calibration claimed.
- Final follow-up check165 s after trial start: Auto/unowned, wheels0/laserOFF,
  hands90 references/pulsesenabled, demo not running, Zero invalid, calibration0.
  Log: ignored build/calibration-cw-v0141.json. Need a fresh operator-observed
  sample/recenter before saving a CW result. Do not use an expired sample ID or
  silently assert a new centered heading after this rotation. Previous floor
  pose/location notes below are historical. No autonomous path/old soak started.

## v0.14.0 editable table demo — 2026-10-20 WIB

- Previous v0.13.0; changes and contract: DEMO_V014.md. User explicitly requested
  editable dimensions to enter estimates later. Geometry remains unconfigured on
  the device; do not invent table dimensions, physical hand pose or calibration.
  Estimates are suitable for simulation, not evidence of physical containment.
- Mandatory sibling-vault rules read: Firmware_AI_Vault_Home; Workspace naming;
  Firmware standards; GitHub Gitflow; Wi-Fi credentials; HuaweiJIN profile; Robot
  lab access; Versioning; Change Notifications. Existing pins, partitions, saved
  Wi-Fi profiles/access and TCPv1 remain. New NVS robot/demoV1 stores only demo
  configuration/calibration; no running session/pose/Zero is restored.
- Local PASS: 90 Python/browser tests, including45 Chromium dashboard tests;
  10 native C++17 suites with Wall/Wextra/Werror. New planner covers12 seeds x
  60,000 ticks (720,000 modeled steps, not physical endurance), model bounds,
  turns, tiny geometry, timer wrap/stall, slow hands and gesture limits. Actual
  DemoService with fake SDK/real NetworkService covers simulation isolation,
  calibration IDs/results, bounded trace/JSON, HTTP/TCP replacement ownership,
  stop phases, OTA/network loss, stale Zero, corrupt storage/reboot and fresh
  hand confirmation. Planner and service also PASS ASan/UBSan. Eye-service tests
  verify temporary box-family overlay and restoration of user settings.
- Clean pinned ESP32-S3 N16R8/core3.3.10 build PASS. Firmware v0.14.0,
  buildUTC2026-10-19T20:42:42Z =20 October2026 03:42:42 WIB. App1,230,672 bytes;
  program storage1,230,518; global RAM57,560. Two6,291,456-byte OTA slots verified.
  App SHA256 `8ba6eae43ff13d9fc538885cb30b57c15637949790beeccb477b4819d98af8a4`.
  Source-hash gate passed and was rechecked before upload. Intermediate artifacts
  were superseded, including one source-change gate rejection and one deliberately
  cancelled build; none was uploaded. Only the final clean artifact was deployed.
- App-only OTA at192.168.1.22 acknowledged ok/rebooting. Expected
  MACe0:72:a1:fd:0d:88 and exact new firmware/build verified. Two postboot GET
  timeouts retained before the third probe recovered; these are reboot recovery
  probes, not discarded normal-request failures. Wi-Fi profile list unchanged.
  No owner/session, Manual, wheel duties0, laserOFF, servo pulsesdisabled.
  Demo task/storage ready; runningfalse, Zeroinvalid, calibrationnotready,
  can_start_robotfalse. No automatic demo/target resumed.
- Actual no-motion API PASS:35 normal GET samples,0 normal failures,
  p95~62 ms,p99/worst~63 ms. Observation uptime16,401→22,168 ms (5.767 s),
  disconnect_count0/connect_count1, no additional reboot observed. Six expected
  rejection cases retained: unconfigured real start/calibration, unconfirmed Zero,
  invalid geometry, injected calibration mask, stale run-ID Stop. No configuration
  persisted and no acquire/command/actuator request was sent. HTTP80/81 demo/status
  and TCP9000 hello/status worked. Command/scenario p95/p99 are not applicable:
  no motion scenario ran. This brief idle sample does not prove sustained latency.
- Actual read-only Chromium desktop/mobile PASS: exact version, editable blank
  length/width/reach fields, Startdisabled, no JS error/horizontal overflow/POST.
  Live mobile screenshot visually inspected; host simulation screenshots also
  inspected. Evidence remains ignored under build/{ota,live,browser}-v014*.json,
  tcp-v014-readonly.json and demo-v014-live-*.png; build_manifest.json records hashes.
- Limitations: no actual wheel calibration, servo/laser movement, real exploration,
  physical drift measurement or on-device simulated run was performed. Geometry
  awaits operator entry; simulation/model and fault tests above are host evidence.
  Operator must measure/calibrate on the fenced table and Set Zero before real
  Start. No claim of edge detection, physical containment or24-hour stability.
  Old soak stays failed/partial and its automation paused; no runner restarted.


## v0.3.3 Wi-Fi investigation — 2026-10-09

- Pinned build PASS: application 1,042,384 bytes, program storage 1,042,230, RAM 51,448; both 6 MiB OTA slots verified. SHA256 `a794f1fe590027f7d13388fda28132960afe6622ffed4c3c7bf053c2ca8a05de`. USB COM11 upload succeeded with esptool hash verification; NVS was not erased.
- v0.3.3 serial observed disconnect reason 2 (AUTH_EXPIRE), subsequent reason 8 (explicit retry disconnect), RSSI -49 to -57 dBm and STA state 0 without an IPv4 address. Periodic retry executed. No continuing reset loop was seen. Firmware-side recovery does not prove original cause or solve persistent router authentication/DHCP failures.
- Laptop temporarily joined robot AP with a separate temporary profile; actual HTTP status and dashboard both returned 200 at 192.168.4.1. API confirmed v0.3.3, uptime 144674 ms, idle Manual, no session, motor/laser off, servo pulses disabled; STA still disconnected/IP0. The laptop was restored to HuaweiJIN and temporary profile deleted. An existing same-name AP profile was preserved after a scope conflict; its authentication attempt did not establish AP failure.
- Former LAN IP 192.168.1.34 failed all 15 HTTP probes. LAN stability is NOT fixed/verified. Recovery AP is verified working. Further router-side authentication/DHCP and 2.4 GHz diagnosis is needed; no speculative router modification was made. Evidence stored in ignored build/v0.3.3-* diagnostic files.
- User reported intermittent then unavailable web access. COM12 (CP210x) was silent; COM11 (CH343) identified AI Robot Controller v0.3.2. User confirmed use of COM11. Esptool identified the original ESP32-S3 revision 0.2, embedded 8 MB PSRAM and MAC e0:72:a1:d6:1b:48.
- Old firmware serial repeatedly showed HuaweiJIN target, STA IP 0.0.0.0, disconnected, recovery AP 192.168.4.1. Existing serial Wi-Fi reconnect did not obtain an IP. HTTP at the former IP timed out. No continuing reboot loop was observed during serial sampling; reset observed when opening serial cannot establish prior reset history.
- Laptop associated to HuaweiJIN and detected its 2.4 GHz AP. AP-advertised channel utilization samples were 81-95%; this suggests congestion, not proof of cause. No router changes were made.
- Source v0.3.2 changed only wheel polarity/version/tests/docs; Wi-Fi was unchanged from v0.3.1. Existing network service lacked periodic recovery of a stalled attempt and disconnect-reason logging. v0.3.3 adds a 30-second retry and USB event/status/reset/RSSI/heap diagnostics. Left wheel inversion remains enabled.
- Local native control/driver suites, 8 Python client tests, 7 policy tests and 14 Chromium simulations PASS. These tests do not establish radio stability.
- All seven mandatory vault notes listed in the v0.3.2 section were read again. STA HuaweiJIN and AP 12345678 remain approved; no NVS erase, credentials change, or motor commands are part of this diagnosis. Existing migration preserves unrelated keys.

## v0.3.2 left wheel polarity — 2026-10-09

- Pinned local build PASS: application 1,040,928 bytes, storage 1,040,774 bytes, RAM 51,440 bytes; emitted two 6,291,456-byte OTA slots verified. SHA256 `abeb7fbcea402bc091e777901d9d0e258735b85fb2873d4b12dfffc8c8945562`.
- Web OTA returned success/rebooting at 192.168.1.34. API after reboot confirmed v0.3.2, Manual, no session, motor duties zero, laser off, servo pulses disabled, HuaweiJIN connected, ArduinoOTA OFF.
- Actual device +/-25% logical commands: forward read GPIO13/10 duty 256 and GPIO12/11 zero; reverse read GPIO12/11 duty 256 and GPIO13/10 zero. Both active frequency readbacks 20,000 Hz. Final state stopped, no session, laser off, arms 90 degrees held. Evidence in ignored build/v0.3.2-device-verification.json.
- This verifies polarity mapping in the ESP32 peripheral, not measured wheel direction or voltage. Independent physical observation remains pending.
- User requested inversion of the installed left wheel to match the right wheel. Driver maps left sign once after ramping; right polarity is unchanged. Logical forward activates GPIO 13/10; reverse activates GPIO 12/11. Frequency readback uses the mapped input. Stop, reversal dwell, sessions, and timeout remain unchanged.
- Local validation PASS: native control and driver suites (MSVC C++17 /W4 /WX), 8 Python client tests, 7 network-policy tests, and 14 Chromium dashboard simulations.
- Mandatory vault notes read in order: 00_Start_Here/Firmware_AI_Vault_Home.md; 01_Rules/Rules_Nama_dan_Folder_Workspace.md; Rules_Firmware_Standar.md; Rules_GitHub_Gitflow.md; Rules_Kredensial_WiFi.md; 04_Profiles/WiFi/WiFi_HuaweiJIN.md; 01_Rules/Rules_Akses_Robot_Lab.md. STA remains HuaweiJIN, shared AP password 12345678, open API/OTA. Existing NVS migration updates differing STA/AP keys and removes only legacy apiToken/otaPassword, retaining unrelated keys; this patch adds no NVS change or erase.

## v0.3.1 dashboard, modes, mirrored hand and PWM — 2026-10-09

- Local pinned build PASS: ESP32 core 3.3.10 / Arduino CLI 1.4.1. Application 1,040,896 bytes; program storage 1,040,754 bytes; global RAM 51,440 bytes. Both emitted 6,291,456-byte OTA slots verified. Application SHA256 `ee53fff22e6c7452a3d7457589b4a562d00c5717bd464e50558396bdf764d2d2`.
- Native control and actual actuator-driver boundary assertions PASS with MSVC C++17 /W4 /WX. Eight Python client tests, seven network-policy tests, and fourteen Chromium dashboard tests PASS. Covers mode gates/stop, reversal, inactive-leg-first writes, mirroring endpoints/centre, zero boot PWM, full-on duty, write failure, automatic connection/recovery, mode controls, automatic acquisition, and release before a late acquisition response.
- OTA v0.3.0 and v0.3.1 through http://192.168.1.34/api/v1/update both returned success/rebooting without authentication. Post-reboot API confirmed each version, Manual mode, no lease, wheel duty zero, laser off, servo pulses disabled, ArduinoOTA OFF, and HuaweiJIN association at 192.168.1.34. API v0.3.1 reports wifi_sleep=false.
- Direct device Manual forward/reverse at 25% passed: active legs GPIO12/10 then GPIO13/11 read duty 256; inactive legs read zero; active frequency read 20,000 Hz. Switching to Auto read all wheel duties zero and invalidated the lease. Program acquisition in Manual and manual acquisition in Auto returned 409 as intended.
- Auto/Python live client passed: right-only wheel duty 256 and left duty zero; logical hands 60/60 mapped to left pulse-angle target 120 and right 60. Producer expiry released its lease and zeroed PWM. Separate raw commands without a client stop timed out and zeroed PWM. These are LEDC peripheral readbacks, not external voltage, waveforms, or measured motion.
- An initial v0.3.0 Auto test received 409; repeat passed. HTTP jitter reached 625 ms in observed requests. This did not establish the sole cause of that rejection. The v0.3.1 latency profile disables modem sleep; 28 requests in the final test measured 32–266 ms. This sample is not a latency guarantee.
- Real Chrome dashboard automatically showed Connected and v0.3.1 on page load. Auto disabled manual actuators; Manual re-enabled them. A left-slider keyboard step from 90 to 89 automatically acquired a manual lease and device readback confirmed mapped pulse-angle target 91. STOP invalidated the lease and zeroed wheel duty. Hands were returned to 90 and laser stayed off. Live screenshot and detailed device JSON are in ignored build/.
- Rules read: Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md (paths listed below for the previous policy rollout). STA/AP remain HuaweiJIN/12345678. Existing targeted NVS migration remains unchanged: only obsolete apiToken/otaPassword keys are removed, differing STA/AP keys are corrected, unrelated keys are retained; no full NVS erase.
- External GPIO waveforms, actual wheel movement/direction, actual hand alignment, failed/aborted OTA recovery, and endurance are not independently verified. No physical feedback sensor exists. A user observation has been requested separately.

## v0.2.0 open-LAN profile — 2026-10-09

- Pinned local build PASS: Arduino CLI 1.4.1 / ESP32 core 3.3.10, 16 MB flash / octal PSRAM. Application 1,035,408 bytes; program storage 1,035,262 bytes; global RAM 51,360 bytes. Both emitted OTA slots verified at 6,291,456 bytes. Application SHA256: `5cfd3e8701ed33400b89b8ce79c3522031f37797be87750b140bb1611603b474`.
- User explicitly authorizes anyone on the same reachable network to access control, settings, stop, and OTA without API token or OTA password. Sessions, sequence checks, command timeout, and maintenance stop interlocks remain enforced.
- Native C++ control assertions PASS (MSVC C++17 /W4 /WX). Seven Python client tests, seven network-policy tests, and nine actual-dashboard Chromium simulations PASS. HTTP client and browser requests contain no Authorization header; simulated dashboard OTA stops control before uploading. Actual ESP32 HTTP/OTA transport remains untested.
- Mobile dashboard screenshot visually reviewed after token removal; version v0.2.0 visible and controls fit the viewport.
- COM11 is absent in Arduino CLI enumeration and present-only Windows Ports inventory on this date. v0.2.0 is not uploaded to hardware; earlier COM11 results below belong to older versions.
- Mandatory vault files read: 00_Start_Here/Firmware_AI_Vault_Home.md; 01_Rules/Rules_Nama_dan_Folder_Workspace.md; 01_Rules/Rules_Firmware_Standar.md; 01_Rules/Rules_GitHub_Gitflow.md; 01_Rules/Rules_Kredensial_WiFi.md; 04_Profiles/WiFi/WiFi_HuaweiJIN.md; new decision 01_Rules/Rules_Akses_Robot_Lab.md.
- STA target remains HuaweiJIN with the approved shared lab password; fallback AP password remains 12345678. Boot migrates differing STA/AP keys and removes only obsolete apiToken/otaPassword keys; no full NVS erase. Physical migration and STA association for this version are not yet verified.
- Decision recorded in the local Obsidian vault and workspace/vault/template/project AGENTS entrypoints. Existing unrelated vault edits remain intact; the vault is not included in the public robot repository.

## v0.1.1 lab profile and mandatory rule reading

- Build PASS with pinned Arduino CLI 1.4.1 / ESP32 core 3.3.10. Application: 1,037,024 bytes; program storage reported 1,036,874 bytes; global RAM 51,400 bytes. Both emitted OTA slots verified at 6,291,456 bytes each.
- Application SHA256: `6a11fb06b2e95f574ecb23f3f4e2a5a0f9acdee3e74d6be052cc040ce4d58114`.
- Seven network-policy tests PASS, six Python client tests PASS, eight Chromium dashboard simulations PASS. Control logic/drivers are unchanged from the native tests below.
- Build gate reads local Obsidian Rules_Kredensial_WiFi.md and WiFi_HuaweiJIN.md, checks approved constants and actual network source, and rejects random AP passwords or STA connection through an arbitrary old NVS profile. Standalone CI applies the approved source gate without requiring the sibling vault.
- Workspace AGENTS.md, vault home/AGENTS/template, and project AGENTS now explicitly require credential-rule/profile reading before firmware planning/reuse/changes/build/upload. This is an instruction and configuration gate, not proof of an AI's reading behavior. Other repositories are not retrofitted with this project's build gate.
- Uploaded v0.1.1 through COM11; esptool verified written flash hashes. Boot confirmed v0.1.1 and STA target HuaweiJIN.
- Serial verified AP password is 12345678. API token and OTA password fingerprints match their pre-upload values; no erase-all or secret rotation was performed.
- Station connection NOT verified: after boot, STA connected=false and IP 0.0.0.0; fallback AP active at 192.168.4.1. Laptop is connected to HuaweiJIN and its saved password matches the requested lab password. This does not prove ESP32 compatibility/association; cause remains undetermined. No router settings or laptop network association were changed.
- Vault/workspace rule updates are local; pre-existing dirty vault edits are preserved and are not included in the robot repository commit. No GitHub push was performed.

## Local results — v0.1.0

- Native C++ control assertions: PASS under MSVC 14.51, C++17, /W4 /WX. Covers boot idle, command validation/ranges, NaN, old sequence, wrong session, expiry, manual takeover, maintenance lock, immediate stop, clock wrap, and reversal zero dwell.
- Python client: 6 tests PASS under Python 3.11. Covers fresh producer targets, expiry/release, no heartbeat on acquisition alone, atomic setter validation, network errors, and blocked HTTP I/O rejecting a stale target before transmission.
- Dashboard: 8 Chromium simulations PASS. Covers pointer release, blur, hidden tab, failed network, API rejection, stop/hand/laser controls, desktop and mobile layout. Tests serve the actual embedded HTML with simulated device responses, not a physical ESP32.
- Browser runtime: Playwright 1.58.0, locally available Chromium headless shell revision 1217 via PLAYWRIGHT_CHROMIUM_EXECUTABLE. CI installs the browser bundled with Playwright 1.58.0.
- Desktop (1100 px) and mobile (390 px) screenshots visually inspected; no horizontal overflow. Screenshots remain in ignored build/.
- Final Arduino CLI 1.4.1 / core 3.3.10 compile: PASS with 16 MiB flash / octal PSRAM configuration. Build parses the emitted partition binary and checks application size against two 6,291,456-byte OTA slots.
- Final application binary: 1,036,432 bytes. Program storage reported by CLI: 1,036,282 bytes (16% of slot). Global RAM: 51,400 bytes (15%); 276,280 bytes remain for runtime allocations.
- Binary contains `v0.1.0`. SHA256: `95965aed4f6b95fea76f358919fcdfE4ddf0e0e6a092dd0150da10be3bb06d61`.
- Tracked source has no absolute machine-specific Windows paths. Generated binaries, screenshots, native test output, and caches are ignored.

GitHub Actions is configured but has not run on GitHub because no remote/push is part of this task. Native and browser tests do not exercise actual ESP32 HTTP/OTA or GPIO; these require the physical acceptance below.

## USB upload and boot — verified

- Uploaded the existing v0.1.0 application through COM11 on 2026-10-08 using the pinned FQBN and build artifacts; esptool 5.3.0 verified written hashes for bootloader, partition table, OTA boot metadata, and application, then reset the board.
- ROM identification: ESP32-S3 revision v0.2, embedded PSRAM 8 MB; flash identification: 16 MB, quad flash, 3.3 V. Target matches N16R8.
- Serial at 115200 baud confirmed `AI Robot Controller v0.1.0`. No actuator-initialization failure or panic was observed in the 20-second boot capture.
- Fallback AP started with IP `192.168.4.1`; STA IP remained `0.0.0.0`, so router connectivity was not established. Serial `info` returned device credentials; secret values were withheld from tool output and tracked files.
- This confirms USB write, boot, and AP startup only. Dashboard/API over Wi-Fi, physical motion, timed stopping, and OTA transfer remain untested.

## Remaining physical device acceptance — not performed

- Physically verify idle outputs after the confirmed USB upload/boot; PSRAM initialization/allocation has not been measured despite ROM identifying 8 MB.
- Check both wheel polarities/PWM, left/right hand angles and pulse endpoints, laser on/off.
- Disconnect laptop/Wi-Fi, stop producer, send malformed/stale commands: verify outputs stop and old sessions cannot resume.
- Confirm manual takeover acknowledges stopped outputs and program commands cannot override manual.
- Load HTTP/status traffic while driving: measure expiry under 500 ms plus one 10 ms task period; timing cannot be certified from native tests.
- Upload valid OTA image; verify stopped outputs throughout upload, reboot idle, version, and reacquisition.
- Abort OTA, wrong image, oversized image, unauthenticated LAN access, unavailable router, and reboot during upload; verify recovery and no resumed motion.
- Confirm servo pulses remain disabled after failed OTA until a new valid complete command.
- Verify ArduinoOTA without a password, opt-in enablement, and reboot default OFF.

Compile/simulation success is not physical robot acceptance.

## Build identity v0.3.4 — 2026-10-12

- PATCH from v0.3.3: generated build-start UTC on serial boot, status API and dashboard; no control/network/NVS behavior changes.
- Read vault home, workspace naming, firmware standard, Gitflow, credential rules, HuaweiJIN profile, robot access, Rules_Versioning and Rules_Change_Notifications. STA HuaweiJIN, AP 12345678; existing selective NVS migration is unchanged.
- Native control and actuator suites PASS; 8 Python client, 7 network policy, 2 build identity tests and 15 dashboard simulations PASS. Metadata tests reject stale binaries, naive time and missing mandatory rule entrypoints.
- Local pinned core 3.3.10 ESP32-S3 N16R8 clean compile PASS; emitted two 6,291,456-byte OTA slots verified. Application 1,042,688 bytes; version and current timestamp verified inside binary. Build UTC 2026-10-12T03:19:37Z; SHA-256 7d5d730b28e4aaa6fdf8fef7ad0f5d3e3d20f33a4681a8d945f30d57df637083. Manifest retained in ignored build/build_manifest.json; CI publishes its own build timestamp/manifest.
- Hardware: v0.3.3 was verified on COM11 and LAN 192.168.1.43 earlier today. v0.3.4 has NOT been uploaded; new serial/API identity and runtime stability on the physical board remain unverified.
- Vault policy published separately, commit aa0bb9e on feature/v1-firmware-ai-vault-bootstrap; unrelated dirty vault work preserved. Robot publication uses feature/initial-robot-controller and existing PR to develop, without merge.

### OTA device acceptance v0.3.4 — 2026-10-12

After the user explicitly requested upload to 192.168.1.43, STOP returned 200 and multipart application OTA returned 200 with rebooting=true. Device status after reboot confirmed v0.3.4 and build_time_utc=2026-10-12T03:19:37Z, HuaweiJIN STA at 192.168.1.43, boot Manual, owner none/session zero, wheel outputs and duty zero, laser off, servo pulses disabled, ArduinoOTA off. Served dashboard HTML contains the build-time UI and status binding; dashboard rendering was tested separately in 15 simulations. The physical timestamp was verified through API; serial build-time code compiled but this update did not separately capture serial boot. This proves successful OTA and idle reboot, not long-term Wi-Fi endurance or measured physical motion.

## Dashboard redesign v0.4.0 — 2026-10-12

- MINOR from v0.3.4: supplied Stitch HTML/screenshot adapted to embedded dark/cyan responsive UI. Sidebar anchors, robot banner, mode/command controls, arm/laser, system telemetry, JSON and OTA all use existing device routes. No CDN, hosted assets or external fonts.
- Additive hardware object reports actual chip/CPU frequency, internal heap/free minimum, PSRAM, flash, cached app/slot sizes and connected STA RSSI. Browser measures HTTP status response time. Removed mock CPU usage, temperature, filesystem, stepper and WebSocket claims from the reference.
- Read mandatory vault home, workspace naming, firmware standard, Gitflow, credential rules, HuaweiJIN profile, lab access, versioning and change notifications. STA HuaweiJIN; AP 12345678. Existing NVS migration unchanged (only old apiToken/otaPassword removed, lab STA/AP updated if different, unrelated keys retained).
- Local software PASS: native control/driver suites, 8 client, 7 network policy, 2 build identity and 18 dashboard tests. Desktop/mobile screenshots inspected; mobile horizontal overflow found and corrected. Test coverage includes actual telemetry rendering, absent telemetry, stale/offline labels, Wi-Fi anchor expansion, no external dependencies, release/blur/hidden/failed-network/rejected-command stops, mode ownership, OTA and build UTC.
- Pinned core 3.3.10 ESP32-S3 N16R8 clean build PASS: application 1,061,888 bytes; actual two 6,291,456-byte OTA slots and binary version/timestamp verified. Build-start UTC 2026-10-12T03:56:29Z; application SHA-256 fb3a3bcd3cecee481f57eb49374897d6463f39622004bbaebdda76dcd95ba9ff. Visual screenshots are from simulation; direct browser inspection was stopped by Computer Use because active URL could not be verified. No further app UI input was attempted. After OTA, verify served HTML equals simulated dashboard source and verify API hardware/build/idle state; this does not replace direct browser inspection or long-term endurance.

### Physical OTA / API / served UI v0.4.0

STOP and OTA each returned HTTP 200. Reboot API confirmed v0.4.0, build_time_utc=2026-10-12T03:56:29Z, HuaweiJIN STA 192.168.1.43, Manual, no session, motor duty/output zero, laser off, servo pulses disabled and ArduinoOTA off. New hardware object: ESP32-S3, CPU 240 MHz, PSRAM 8,388,608 bytes, flash 16,777,216 bytes, app 1,061,888 bytes and OTA slot 6,291,456 bytes. Served HTML equals tested embedded UI byte for byte. Four status requests over approximately 15 seconds passed with increasing uptime and no motion commands. This verifies OTA, boot, API telemetry and served assets; direct real-browser visual inspection was blocked by the tool, so visual evidence remains separately labeled simulation. No endurance, electrical PWM measurement or physical motor/servo test was claimed.

Rules read: Firmware_AI_Vault_Home.md; Rules_Nama_dan_Folder_Workspace.md; Rules_Firmware_Standar.md; Rules_GitHub_Gitflow.md; Rules_Kredensial_WiFi.md; WiFi_HuaweiJIN.md; Rules_Akses_Robot_Lab.md; Rules_Versioning.md; Rules_Change_Notifications.md. Skills: firmware-versioning-guardian, gitflow-guardian and computer-use (read-only visual verification attempted, then stopped). Source publication stays on feature/initial-robot-controller / PR #1 to develop; no merge or vault-wide changes.


## v0.5.0 — dual OLED eyes and corrected wheel polarity (2026-10-16)

Baseline source v0.4.0 -> v0.5.0. Read the nine mandatory vault entrypoint/rule/profile files listed in EYES_DESIGN.md before implementation. Confirmed with user: SSD1306, SDA8/SCL9; request includes a logical eye swap option. Driver now uses left inversion=false/right inversion=true: forward GPIO12/11, reverse13/10. API logical signs, 10 ms output writer, 500 ms timeout and servo mirroring unchanged.

New EyeAnimator/EyeRaster/EyeService provide seven expressions and motor/arm/laser/idle reactions. A separate priority1/core0 task owns I2C with10 ms transaction timeout, fail-fast32-byte packets and5 s missing-panel retry. Full framebuffer/identity and physical swap are independently driven for0x3D/0x3C. Eye Studio shows a local preview and actual last-transfer health; it handles drafts, rejected requests, missing firmware support and mobile layouts. POST /eyes and Python set_eyes are cosmetic and do not acquire/renew motor control. Eye settings remain volatile; no NVS key/schema/migration change. HuaweiJIN STA, recovery AP12345678 and open-LAN access are preserved. Existing boot migration updates differing lab STA/AP values, removes only obsolete apiToken/otaPassword and retains other keys.

Local software validation:
- Native MSVC /W4 /WX control, real actuator driver, pure eye animation/raster and real EyeService with fake Wire/RTOS: PASS. Boundary cases include complete1024-byte transfers, byte-level logical eye swap, missing panel/retry, failed data early abort, maintenance suppressing bus activity and bus initialization failure. Fakes do not measure physical timing.
- Python client9 tests, network-policy7, build-metadata2: PASS. New client test confirms repeating cosmetic settings does not refresh a stale drive producer.
- Chromium dashboard21 tests: PASS, including eye settings/swap, no motor-session side effects, rejected request preserving draft, draft surviving status poll, missing panel/unsupported firmware and390 px layout. Actual embedded HTML was used with mocked device HTTP. Cropped previews in ignored build/eye-studio-*.png are simulation only.
- Clean pinned Arduino-ESP32 3.3.10 ESP32-S3 N16R8 build: PASS. Application1,105,840 bytes, each emitted OTA slot6,291,456 bytes. Version/time verification and emitted partition checks passed. All22 copied .h/.cpp compiler inputs match final source after removing the CLI-added leading #line directive and normalizing line endings.

Device boundary: previous IP192.168.1.43 timed out on read-only status request this turn. No v0.5.0 upload or physical wheel/OLED test performed at this point. User was asked for current IP/USB target. Physical checks pending: actual direction, pixels/splash/contrast/swap, missing/stuck bus under actuator timeout, OTA/load/endurance. No new CI run has been claimed from local tests alone. Branch remains feature/initial-robot-controller; local source changes not yet published at this point.

Final v0.5.0 image identity: build UTC `2026-10-16T07:57:21Z` (2026-10-16 14:57:21 UTC+7), SHA-256 `8079a2218b0e3b7d9fbe5db1437b30b2e070d6bd083ac9adf43bc0d568025e15`. This is build-start time, not upload time/device clock. Manifest/binary and simulation previews remain ignored in build/. No v0.5.0 device upload or GitHub push was performed this turn; actual branch feature/initial-robot-controller remains locally modified, based on last published65be52b.


### COM11 USB upload attempt — 2026-10-16

User explicitly selected COM11 for v0.5.0 upload. Re-read mandatory vault home/folder/firmware/Gitflow/credential/profile/access/versioning/change-notification files and local AGENTS before upload. Existing image SHA-256, embedded v0.5.0 and build UTC2026-10-16T07:57:21Z were revalidated; no rebuild or firmware behavior change this turn. Shared HuaweiJIN/AP12345678 profile and existing selective NVS migration remain unchanged.

Windows reports COM11 as USB-Enhanced-SERIAL CH343, VID1A86/PID55D3, instance5B90012690. esptool5.3.0 chip-id failed with no serial data. A serial info request and RTS reset/10-second boot capture received0 bytes. An explicit ESP32-S3 flash-id retry at115200 also failed with no serial data. USB bridge detection does not establish ESP chip identity, flash capacity or application health. No write-flash/upload was attempted and no flash/NVS erase occurred. User was asked to enter BOOT download mode manually, then retry chip/flash/MAC/partition verification before writing. Source remains locally modified on feature/initial-robot-controller; no commit/push performed.


### COM16 identity mismatch — 2026-10-16

After user selected COM16, Windows reported CH343 VID1A86/PID55D3, serial5C37250654. Port was initially busy; a serial-monitor helper was stopped, and user subsequently closed the monitor. esptool5.3.0 then identified ESP32-S3 rev0.2, embedded8 MB PSRAM,16 MB quad flash, MACe0:72:a1:fd:0d:88. This differs from the earlier robot MACe0:72:a1:d6:1b:48. Read-only partition capture shows NVS0x9000/0x5000, otadata0xe000/0x2000, two3 MiB application slots at0x10000 and0x310000, and coredump0x610000/0x10000.

Serial boot positively identified the installed application as ESP32 ADS1115 Ground Checker Research, firmwarev0.11.1-research.1, releasev0.11.1-research.1+20260909.1341-wib, with GroundChecker-Setup recovery AP. This is not the previously verified robot application. User was asked whether this board is intentionally being repurposed before replacing its firmware/partition table. No robot firmware or partition writes, no erase, and no NVS migration were performed. The old Ground Checker application was booted and left running. Flash reads and boot capture remain ignored in build/.

Mandatory vault home/folder/firmware/Gitflow/credential/HuaweiJIN profile/access/versioning/change-notification rules were re-read. Robot image remainsv0.5.0, build2026-10-16T07:57:21Z (14:57:21 UTC+7), SHA-2568079a2218b0e3b7d9fbe5db1437b30b2e070d6bd083ac9adf43bc0d568025e15 and revalidated before the attempted upload. Robot profile, if this is confirmed as the target, remains HuaweiJIN/AP12345678 with selective existing NVS migration; the different board's current stored profile was not modified. Source changes remain local on feature/initial-robot-controller; no commit/push performed.


### COM16 repurposed to robot — USB success, 2026-10-16

User explicitly confirmed replacing the Ground Checker application on COM16 with robot firmware. Re-read Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md, Rules_Versioning.md and Rules_Change_Notifications.md before upload. Target confirmed ESP32-S3 rev0.2,16 MB quad flash,8 MB embedded octal PSRAM, MACe0:72:a1:fd:0d:88. Port CH343 COM16 was released after user closed Serial Monitor.

A full16,777,216-byte preflash backup was read successfully to ignored build/com16-ground-checker-full-backup.bin. SHA-256 `5d598477444d41b33dd7d750402f9d4a225da470452de4571c32dd27ab061774`; captured partition bytes match the preceding read-only partition dump. This backup includes old device configuration and stays local/ignored; it is not a publication artifact. Ground Checker source/repository was not modified.

Revalidated robot application v0.5.0, build2026-10-16T07:57:21Z (14:57:21 UTC+7), SHA-2568079a2218b0e3b7d9fbe5db1437b30b2e070d6bd083ac9adf43bc0d568025e15,1,105,840 bytes. Uploaded existing tested image via Arduino CLI with the pinned ESP32-S3 N16R8 FQBN; no rebuild/version change this turn. Upload start `2026-10-16T08:33:57.465092+00:00`, end `2026-10-16T08:34:15.278855+00:00`, exit0. esptool verified bootloader and written partition/OTA-data/application hashes. New table has two6 MiB slots replacing old3 MiB slots; NVS0x9000..0xdfff is outside the write ranges. No full-flash/NVS erase was performed. Boot's existing robot namespace migration removes only apiToken/otaPassword and updates differing approved lab STA/AP keys, preserving unrelated keys by design; unrelated Ground Checker NVS keys were not independently decoded after boot.

Serial after RTS reset confirmed AI Robot Controller v0.5.0 and matching build UTC, open-LAN access and AP password12345678. STA connected to HuaweiJIN at192.168.1.66. GET /api/v1/status confirmed Manual, owner none/session0, all motor duties0, laserfalse, servo_enabledfalse, ArduinoOTAfalse, flash16 MiB, PSRAM8 MiB, application1,105,840 bytes and running OTA slot6,291,456 bytes. Served dashboard HTML matches current embedded source exactly. Initial serial core_dump messages report no core-dump partition in this robot layout; there was no crash/reboot in the subsequent short polling interval.

Actual-device cosmetic API tests PASS: all seven manual expressions become active, swap setting accepted, brightness0 rejected with400 without changing accepted settings, and motor session/output/servo pulse state remain idle. Restored enabled=true,automatic=true,blink=true,swap=false,expression=happy,brightness160,tempo1. Three subsequent status samples increased uptime81446->84475->87514 ms and animation iterations1358->1408->1459 without reboot. Free heap approximately235 KB. This is short verification, not endurance proof.

OLED task_ready=true, but both physical left0x3D and right0x3C report offline/no successful I2C transfer on SDA8/SCL9. No physical OLED pixels or direction of wheel movement was observed or claimed. No motor/servo/laser actuation commands were sent. OLED wiring/address/display behavior, physical forward direction, stuck-bus timeout, OTA upload on this repurposed board and endurance remain pending. Ignored evidence: com16-robot-v0.5.0-upload.log, com16-robot-v0.5.0-boot.log, com16-robot-v0.5.0-status.json and com16-v0.5.0-eye-api-tests.json in build/. Source remains locally modified on feature/initial-robot-controller; no commit or GitHub push this turn.

### v0.6.0 expressions and OLED diagnostics — local validation, 2026-10-16

Change: v0.5.0 -> v0.6.0. Added41 expressions/10 categories, reactive/playful/showcase/manual policies, energy1..3 and separate winks. New GET /api/v1/eyes/catalog and POST /api/v1/eyes/check; POST /eyes accepts legacy7 or extended10 fields, with atomic rejection. Eye settings/diagnostics are volatile. Actuator GPIO/polarity/API command/session/timeout, flash partition table, Wi-Fi/open access and NVS migration unchanged.

Re-read actual sibling vault Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md, Rules_Versioning.md and Rules_Change_Notifications.md, plus repository AGENTS/README/API and eye design. STA profile remains HuaweiJIN, recovery AP password12345678. Existing boot migration updates differing STA/AP keys and removes only apiToken/otaPassword, preserving other NVS keys; this change introduces no migration or new keys.

Local PASS: four MSVC C++ units (control, actual actuator driver, eye animator/raster, actual EyeService against fake Wire/RTOS),10 Python client tests,7 network-policy tests,2 build-identity tests and24 Chromium dashboard simulations. Added raw scan/address discovery including unknown0x42 without OLED commands to it, target-only white, checker/identity, expiry, all41 raster outputs, separate winks and category-constrained showcase. Client catalog parity checks all41 IDs; producer expiry remains independent of cosmetic requests. Dashboard tests verify category filtering, modes/energy, no cosmetic motor lease, diagnostic queue/rejection, existing drive stop/race/offline/OTA flows. GCC warning-as-error compilation of eye units found and corrected two helper indentation warnings missed by MSVC; affected files then compiled cleanly with GCC flags. This is not an observed remote GitHub Actions run.

Desktop1440 and mobile390 Eye Studio snapshots inspected: category pills wrap, preset grid scrolls, Googly pupils render and mobile has no horizontal overflow. Snapshots are ignored in build/eye-studio-v6-desktop.png and eye-studio-v6-mobile.png. They use a mocked device and do not prove physical pixels.

Pre-OTA live status192.168.1.66 confirms v0.5.0 on HuaweiJIN, ownernone/session0, motors0, laserfalse, servo_enabledfalse, eye task ready but both panels offline. User suspicion of wrong address remains unproven at this stage; supplied8-bit0x78/0x7A conversion to7-bit0x3C/0x3D is correct. Actual scanner results and upload identity follow after build completion. All23 source .h/.cpp files match Arduino's copied sketch after removing its #line marker and normalizing line endings.

### v0.6.0 build and OTA verified — 2026-10-16

Pinned Arduino-ESP32 core3.3.10 clean build PASS with the same generic ESP32-S3 N16R8 FQBN (16 MB flash, octal8 MB PSRAM). Build metadata UTC `2026-10-16T10:14:54Z` =2026-10-16 17:14:54 UTC+7. It is build-start time, not device wall clock or uptime. Final application1,123,168 bytes; each emitted OTA slot6,291,456 bytes; partition/size/embedded identity gates PASS. SHA-256 `4760ed96f1215ebcf6d41256d5ffeb2d2e3f83b2b08798d57c38d6d1e9d3275c`. The earlier build started10:13:36Z was deliberately aborted for GCC helper formatting corrections; only the final10:14:54Z build was uploaded.

POST /stop acknowledged idle before web OTA. POST /api/v1/update returned200 ok/rebooting. After reboot at192.168.1.66, status reports v0.6.0 and matching build UTC, HuaweiJIN, ownernone/session0, motors0, laserfalse, servo_enabledfalse and ArduinoOTAfalse. Served HTML matches final source exactly. GET /eyes/catalog returns41 presets and11 category selectors (including all). Ignored evidence: build/ota-v0.6.0-result.json and build/build_manifest.json. No USB or partition upload was needed.

Actual-device API PASS: all41 expressions accepted and become active; four modes accepted; eight invalid settings return400 and preserve the complete previous settings; seven-field legacy form accepted; cosmetic operations leave all actuators idle without a session. White-left, white-right, checker-both and identity-both requests enter the selected diagnostic state, each returns to animate after5 seconds. This establishes software state/expiry only, not visible pixels. Restored Playful,all,energy2,enabled/blinktrue,swapfalse,brightness160,tempo1,expressionhappy for the current boot. Boot defaults remain Reactive/all/energy2. Eye settings still reset at reboot.

Actual scan at100 kHz completed all112 addresses8..119 at uptime27064 ms: found_addresses=[], left0x3D probe2 and right0x3C probe2 (address NACK). Both panels remain offline. Thus wrong address is not proven and no responding alternative address was found. The supplied8-bit0x78/0x7A conversions remain correct. Physical investigation required: SDA8/SCL9 and shared ground, one module at a time, actual board connection and module address jumpers. Do not claim a working OLED display or conclude the exact wiring/power cause from this scan alone.

Browser via in-app browser was reloaded from the older7-preset page to the served v0.6.0 interface; actual DOM shows41 presets, categories and selected Playful policy. Browser briefly marked offline during/around burst API testing, then recovered; this is not an endurance acceptance result or proof of its cause. Live preview screenshot saved locally as build/dashboard-v6-live.jpg. This screenshot shows the web preview, not physical OLED pixels. Native raster contact sheet build/eye-catalog-v6.png additionally reviews all41 programmed shapes.

Git remains feature/initial-robot-controller with modified tracked files and new eye source/tests/docs still local; no commit or GitHub push performed in this task. Existing remote remains origin at the robot repository. Preserve ignored preflash Ground Checker backup; never publish it. Mechanical direction, physical pixels, disconnected/stuck bus under real actuator load, failed OTA recovery and long-run concurrency/Wi-Fi remain unverified. Remote CI has not been run for these local changes.

### v0.7.0 family consistency and mounting rotation — 2026-10-16 local work

User clarified that Reactive automatic mode must keep the same rounded-square eye type; shapes that do not match that type must be separated. Current source v0.6.0 -> v0.7.0 (minor feature). isBoxEye classifies Solid/Happy/Focus/Sad as box (22 presets), all other shapes as special (19). The earlier commentary count24/17 was a counting mistake (category names matched the quick shape count); final renderer/metadata classification is22/19. All41 IDs retained, with two browsing families. Reactive/Playful idle and actuator/hand reactions stay box-family; Manual/Showcase permit special shapes. Low-angle hand reaction is now shy instead of hearts.

Repeated text icons replaced by actual two-eye rendered thumbnails with motion descriptions, sampled blink phases and hover/focus animation. Main preview animates locally between1 Hz status responses. More distinct temporal/pose behavior for laughter, giggle, skeptical/thinking, bored, panic and others. Shared character does not mean41 unique silhouettes; timing-only blink presets intentionally share a base. Preview is simulation, not physical OLED readback.

Independent rotate_left/rotate_right physical180-degree mounting flags added; both default true for the user's inverted installation. Separate from swap, applied to init and runtime orientation and therefore splash/diagnostic patterns too. Normal SSD1306 segment/COM A1/C8; rotated A0/C0. Settings still RAM-only; reboot restores installation defaults. Exact7/10/12-field API forms supported. Legacy forms preserve orientation; partial or nonboolean rotation rejected atomically. Python may supply both flags or read status to preserve an omitted side. No new NVS keys, partition/network/access/pin/motor/servo/control changes.

Rules read this task: workspace/repository AGENTS and RTK (rtk command unavailable, native PowerShell fallback); actual sibling vault Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md, Rules_Versioning.md and Rules_Change_Notifications.md; README/API/EYES_DESIGN and architecture/versioning skills. Shared lab STA HuaweiJIN, AP12345678, open LAN API/OTA remain. Existing boot migration updates differing lab STA/AP values, removes only obsolete apiToken/otaPassword, and preserves unrelated NVS; this task does not change it.

Local PASS: four native C++ suites (control, actual driver, animator/raster/family, actual EyeService/fake Wire),26 Chromium dashboard simulations,11 client tests,7 network-policy tests,2 identity tests. Native tests sample Reactive/Playful over80 seconds of simulated time and hand-low reaction, asserting box family; verify mixed panel initialization and runtime rotation changes only the intended physical address. Dashboard verifies family browsing, thumbnails differ for happy/laugh, reset to box when choosing Reactive, rotation preview independent of swap,12-field submit and no cosmetic motor session. GCC warning-as-error eye compilation passed. Actual Arduino copied sketch matches all23 source h/cpp files. Desktop1440/mobile390 snapshots inspected and kept ignored at build/eye-studio-v7-desktop.png and eye-studio-v7-mobile.png. Remote CI has not been run for these local changes.

Live browser before upload: robot192.168.1.66 serves v0.6.0, build10:14:54Z. Both0x3D/0x3C initially reported successful I2C transfers, later both reported offline with error count rising15->21 while idle. Thus the previous all-NACK scan is historical, and neither current bus stability nor physical pixels are established. Browser firmware section was accessed directly per user request. No motor/servo/laser actuation was performed.

### v0.7.0 OTA succeeded; final live test interrupted by connection loss

Final clean pinned build PASS: v0.7.0, build UTC2026-10-16T10:54:58Z (2026-10-16 17:54:58 UTC+7), application1,130,304 bytes, each OTA slot6,291,456 bytes. SHA-256 bc3f2c83e3285a511d7a404df627c933e41048cdf29baf28843295022936dcda. Size/emitted-partition/embedded-identity gates pass. No source h/cpp changed after the build snapshot.

OTA at192.168.1.66 returned200 ok/rebooting. Post-boot status verified v0.7.0/build UTC, HuaweiJIN, ownernone/session0, motors0, laserfalse, servo_enabledfalse. Served HTML matches current source and catalog has41 entries; actual family metadata22 box/19 special confirmed by the live test. Evidence saved ignored in build/ota-v0.7.0-result.json.

Actual-device checks completed before interruption: all four left/right rotation boolean combinations accepted with motor/laser/session state idle; seven/ten-field legacy forms preserved an asymmetric false/true mounting setting. Invalid rotation requests returned400; subsequent status read timed out before the complete rejection/family sampling sequence could finish. A later test's strict servo-disabled assertion also stopped: status was ownernone/session0, reasonreleased, motors0, arms90/90, servo_enabledtrue. This is a valid held-servo state after a separate control session, not proof that cosmetic settings enabled servo pulses. No actuator commands were sent by the eye test.

Most recent successful v0.7.0 status: uptime104582 ms, both OLEDs online, I2C errors0, physical rotationfalse/false temporarily selected by the interrupted second test. The requested final true/true rotation and Reactive profile were not yet restored/verified at this point. Firmware boot defaults are true/true, so reboot restores the intended mounting orientation. Do not present the partial test as complete or claim visible pixels.

Then repeated catalog/status TCP reads/connects timed out. No ARP entry/ping response for192.168.1.66; no USB ESP32 port present (only Bluetooth COM4/6/8/9). Laptop has Ethernet4 Realtek USB GbE192.168.1.123/index13/metric25 and HuaweiJIN Wi-Fi192.168.1.32/index6/metric45. Windows selected Ethernet4 for192.168.1.66. A diagnostic socket explicitly bound to Wi-Fi/index6 (Windows IP_UNICAST_IF31, verified in installed Windows SDK header) also timed out. No route, metric, adapter or router settings were changed. In-app browser restoration reported ERR_NETWORK_CHANGED. These observations do not establish whether robot power/IP, laptop connectivity or firmware caused the outage.

Asked user whether robot is still powered at the same IP or has a new IP/COM, to restore true/true and complete live UI/family sampling. Local source remains modified/uncommitted on feature/initial-robot-controller, origin unchanged; no push or merge. Preflash backup and all test binaries/records remain ignored in build/.

### v0.8.0 servo smoothing, steering and eye additions — local validation

User clarified reversed behavior is wheel steering: right button physically turns left and vice versa. Servo identity/mirroring is correct but motion is abrupt. Source inspection confirmed previous ControlLogic copied arm targets directly into output. New ArmMotion limits commanded speed/acceleration with braking and online retargeting; identical heartbeats do not restart. Default45°/s,120°/s²; Gentle25/60,Lively75/240. Stop/release/expiry freezes reference and cancels remaining travel; OTA disables pulses. No torque/jerk/position-feedback claim. New /arms/settings is strict finite/range validated and rejects an active lease/maintenance; RAM only and does not renew freshness. Read ARM_MOTION_DESIGN.md for research links, assumptions and limits.

Central swapMotorSides=true exchanges logical targets before the existing leftfalse/righttrue polarities. Equal-wheel forward/reverse unchanged; logical left/right wheel commands and eye gaze conventions preserved. Raw PWM fields remain GPIO groups12/13 and10/11. Physical direction still requires observation; unit tests prove mapping only. Servo mapping unchanged left180-angle/rightangle. No pin, Wi-Fi, partition or NVS migration changes.

User requested Sharingan, animal eyes and Rinnegan. Added six special presets (sharingan,rinnegan,cat,fox,snake,toad) under shinobi / Hewan & shinobi.47 total:22 box and25 special,12 category selectors including All. Reactive/Playful remain box-only. OLED monochrome graphics; static samples/thumbnail are not proof of physical pixels. Native raster contact sheet and local desktop/mobile Eye Studio screenshot inspected, saved ignored in build/eye-special-family-v7.png (filename reused by render helper), eye-studio-v8-shinobi.png and eye-studio-v8-mobile.png. No horizontal mobile overflow observed.

Local PASS: four C++ suites (ControlLogic/ArmMotion including speed/acceleration checks, target reversal, heartbeat continuity, timeout hold, settings lease/range; actual actuator driver mapping;47 eye rasters and box-family checks; actual EyeService fake Wire/RTOS). GCC -Wall -Wextra -Werror compiles control/eye tests.27 Chromium simulations and12 Python client tests PASS, plus7 network policy/2 build metadata tests. Initial two dashboard failures were ambiguous old .eye-diag summary test selectors after adding arm settings; fixed by scoping OLED checks to #eyes-panel and reran all27 PASS. No remote CI run for local changes.

Rules: read actual workspace/repo AGENTS, versioning and change notifications again this ongoing task; mandatory vault home/folder/firmware/Gitflow/credential/HuaweiJIN/access rules read in the preceding continuous implementation stage. STA HuaweiJIN, AP12345678 and open LAN access unchanged. Existing migration updates only differing lab STA/AP values and removes obsolete apiToken/otaPassword, preserving unrelated NVS. Build gate re-read credential/profile/version/notification files automatically. New versionv0.8.0 is a minor increment fromv0.7.0 due arm settings/new eye features. Build result follows separately.

Network continuation: user reported192.168.1.44, but HTTP through default Ethernet route and an explicit Wi-Fi socket timed out. Laptop Wi-Fi had disconnected; reconnected its existing HuaweiJIN profile using netsh without changing router/routes/passwords. Later neighbor cache showed robot MACe0:72:a1:fd:0d:88 at192.168.1.66. Live status from .66 confirmedv0.7.0/build10:54:58Z,both rotationtrue,both OLED online,errors0,manual Shy preset and motors0. This supersedes earlier un-restored false/false mounting observation: requested mounting is now true/true. Source of change not inferred. .44 still timed out. No new USB serial board (Bluetooth COMs only). OTA target will be the live verified .66, not a guessed device at .44.


### v0.8.0 clean build and OTA acceptance — 2026-10-16

Clean pinned Arduino-ESP32 3.3.10 build PASS, ESP32-S3 16 MB/OPI PSRAM configuration unchanged. Version v0.8.0; generated build UTC 2026-10-16T12:13:15Z (2026-10-16 19:13:15 UTC+7; build identity, not device RTC/uptime). Application 1,135,952 bytes, each emitted OTA slot 6,291,456 bytes; size, partition and embedded-identity gates PASS. SHA-256 096d95479cc9fa36640afc56239e7ba558684baee2635bdf7892690ca8970326.

OTA to http://192.168.1.66/api/v1/update returned200 ok/rebooting. After reboot API identity exactly matches manifest. Boot session0/owner none, wheels0, laser false, servo pulses disabled. Served dashboard matches source byte-for-byte. Device catalog47 presets/12 category selectors confirmed. Ignored evidence: build/ota-v0.8.0-result.json and build/v0.8.0-device-tests.json.

Live API PASS for all six new presets: Sharingan, Rinnegan, cat, fox, snake, toad. Both OLED addresses ACK throughout tests, errors0; this verifies I2C transactions and active expression telemetry, not visible physical pixels. Both physical rotation flags remain true (180 degrees). Restored Reactive/category All/box-family animation after tests.

Servo live test used wheel targets0 and laser false throughout. Gentle25/60 settings accepted, invalid numeric/range/bool requests rejected400, active-lease settings rejected409. Repeated a stricter expiry test at speed5 degrees/s, acceleration20 degrees/s2 so the target100/80 remained unfinished: first interpolated references 90.0640/89.9360; after heartbeat expiry held 94.9850/85.0150, unchanged on a subsequent sample. Thus timeout cancelled remaining travel rather than merely observing an already completed gesture. Restored Natural45/120 and target90/90 with fresh heartbeat, then stop. Final session0, motors0, laser false, arms90/90, servo pulses enabled to hold; no physical wheel test performed. Physical smoothness and steering direction require user observation; no mechanical/waveform/endurance acceptance claim.

Browser reload attempt was blocked by the browser security policy on its existing error-page URL; no bypass attempted. Live HTML identity/API checks and the27 local Chromium simulations passed, but the final rendered live page was not re-inspected. Local desktop/mobile previews are evidence of the mock browser UI only.

Final Git audit: feature/initial-robot-controller; origin https://github.com/TaufikAS0/2026-esp32s3-ai-robot-controller.git unchanged. Local changes modified/untracked (including previous v0.5-v0.7 work), uncommitted/unpushed; no merge. git diff --check PASS (line-ending normalization warnings only). No remote CI execution. Required rule audit and unchanged HuaweiJIN/AP12345678/NVS migration are recorded above; no pin/network/partition/NVS schema change in v0.8.0.

### Public GitHub continuation handoff — 2026-10-16

Repository visibility verified PUBLIC; no visibility change required. User explicitly requested pushing the tested baseline and a GitHub prompt for Codex on another laptop already powered24/7. New HANDOFF_CONNECTION_EYES.md records unresolved timeout/servo+laser correlation, Kurama out-of-silhouette highlight and more Sharingan designs, plus an automated24-hour soak protocol. AGENTS/README require reading this handoff. All wheel test commands must remain zero; servo/laser scenarios are included; no runner or automation has been launched on this publishing laptop. Creating the issue does not start a Codex worker. Network/power/software causes remain unproven.

Fresh pre-publication checks PASS: four MSVC native suites,12 Python client,7 network policy,2 build identity,27 Chromium dashboard tests. Verified all24 firmware h/cpp files match the actual pinned compiler build snapshot; application SHA/embedded UTC/version match the v0.8.0 manifest. No new firmware behavior since that successful build/OTA, so preserve its true build timestamp instead of relabeling/rebuilding just for documentation. No fresh flash in this handoff publication. No source claims that the three newly reported problems are fixed. Raw full-flash backup, binaries, generated metadata and device logs remain ignored.

Current diagnostic sample:15/16 HTTP status requests within2-second timeout; individual latencies recorded in handoff; one timed out. Example RSSI-72 dBm, both OLED ACK/errors0 and sleepfalse. No actuator request issued during this diagnostic. Shared lab credentials are the already-approved source exception, not a new secret publication. Rules read again: vault home, folder, firmware, Gitflow, credentials, HuaweiJIN, access, versioning and change notification; repository AGENTS and RTK. STA HuaweiJIN/AP12345678 and selective NVS migration remain unchanged.


## v0.9.0 continuation laptop — 2026-10-16

Rules read in full: repository AGENTS, README, API, complete VERIFICATION, EYES_DESIGN, ARM_MOTION_DESIGN and HANDOFF_CONNECTION_EYES plus issue #2 (no comments). Actual vault was absent locally; cloned2026-vault-firmware-rules and inspected branches. Its default branch was stale/missing mandatory notes; checked out exact remote feature/v1-firmware-ai-vault-bootstrap at5112d6e. Then read the required vault home, folder/workspace, firmware standard, Gitflow, credential rule, HuaweiJIN profile, lab access, versioning and change notification files in order. This does not change vault policy. Shared lab HuaweiJIN/AP12345678, open LAN API/OTA, pins, partitions and selective NVS migration remain unchanged.

Network preflight: testing laptop Ethernet192.168.1.122, interface15, direct192.168.1.0/24 route; Wi-Fi disconnected. HTTP at192.168.1.66 returnedv0.8.0/build2026-10-16T12:13:15Z with ESP32-S3/16 MiB flash/8 MiB PSRAM. Windows ARP matched robot MACe0:72:a1:fd:0d:88. Ownernone/session0, all wheel outputs/duties0 and laserfalse. Pulses were enabled with accepted reference0/70.8160934448242; this is not measured physical pose. Rechecked the same state immediately before OTA. Windows sleep idleAC/DC already0; no network, router, firewall or power policy modified.

Read-only baseline before code changes:60 sequential status samples at1 Hz/2-second timeout,1 timeout; p95~469 ms,p99~1766 ms,max~2016 ms. Timeouts are included in these statistics. Ignored raw artifactbuild/baseline-v080.jsonl. These observations fail the provisional target and do not establish power/RF/software causation.

Implementation: clipped/mirrored Fox highlight; separate six-design Sharingan category (52 total presets,22 box/30 special,13 category selectors); native bounded loops/precomputed tomoe geometry and unused-angle elimination; OLED overrun dropping; additive MAC, GOT_IP/disconnect/reset and OLED work telemetry. Browser defers catalog load during an actuator session and skips status launch while a command request is pending. Inspected actual installed Arduino-ESP32 core3.3.10 WebServer handleClient: regular parsed responses release the current client; SSE handling is separate. No speculative keep-alive timeout/core/network-profile changes made. Slow-header traffic can still block the synchronous server; this is measured by a bounded no-actuator scenario rather than claimed fixed.

First-pulse behavior is intentionally changed: a complete command after disabled pulses seeds the supplied requested arm reference; it avoids passing through assumed90 degrees. Native test verifies0/nearby reference seeding and subsequent rate-limited retargeting. No physical startup position is known. No NVS pose storage, new timer/output owner or autonomous gestures. Stop/session/sequence/500 ms expiry and both OLED rotations/swap retained.

Software PASS: five GCC C++17 -Wall -Wextra -Werror suites (control, actual actuator driver, animator/raster, real EyeService with fake Wire/RTOS, new eye regression). Overrun test verifies three120 ms iterations each yield a fresh60 ms delay (no catch-up burst).12 Python client,7 network-policy,2 build-metadata and10 soak safety/matrix tests.28 Chromium dashboard simulations passed; final Fox/Sharingan visual test additionally rerun after contact-sheet capture changes. Host-only soak matrix tests include all10 scenarios, laser expiry, final off, cancellation, conflicting owner, maintenance/reboot/nonzero output rejection, exact identity, duplicate-process lock and no network transmission after producer freshness loss/zero-wheel guard failure.

Native Fox: both eyes across100 phases have no pixels outside ellipse/lid. Six Sharingan variants are pairwise different by>50 pixels at each of4 phases. Native/browser bitmaps compared (<8 pixel tolerance per8192-pixel frame); actual native and browser contact sheets visually inspected for Fox and all six Sharingan designs. These are software pixels, not observed OLED pixels. Optimized host benchmark2000 three-tomoe frames:620.486 ms old /75.8876 ms new, same checksum58076. This is host timing, not ESP timing or Wi-Fi stability proof.

Pinned core3.3.10 ESP32-S3 N16R8 final build PASS. v0.9.0 buildUTC2026-10-16T13:10:42Z =2026-10-16 20:10:42 WIB. App1,138,640 bytes; program1,138,490 bytes; staticRAM53,224 bytes; both emitted6,291,456-byte OTA slots verified. App SHA-256314a75148b86bb16051a2a414a259cdb9b4e22e0b0cdbeb518f6bbc138c133c4. Initial clean build finished but was superseded because two source files changed while it ran; that image was never uploaded. Final build regenerated metadata and reused compiled library/core objects, rebuilt all firmware objects, rejected source mutation, and verified embedded version/UTC. Manifest records clean_build=false and all input hashes. All compiler-copied h/cpp files matched final source after normalizing CLI line markers. This is honestly an incremental final build, not a relabeled clean build.

Device OTA PASS: preflight required known MAC/baseline identity, no owner, idle wheels/laser and ArduinoOTAoff; POSTstop acknowledged before app-only multipart upload. Upload returnedok/rebooting. Post-boot status matchedv0.9.0/build UTC/MAC, Manual/ownernone/session0, wheel duties0, laserfalse, servo pulsesdisabled, reset_reason3, GOT_IP1/disconnect0. Served HTML byte-for-byte matched tested source. Both OLEDs online/errors0 at that sample; last work60344 us,max180019 us,overruns2 including boot. This establishes output state and I2C transfers, not physical pixels or supply stability. Evidence ignored inbuild/ota-v090-result.json andbuild/pre-ota-v090.json.

Live smoke/24-hour state is recorded below after execution; do not infer24-hour completion from source/build/OTA success.


### v0.9.0 live smoke and24-hour launch

Smoke completed144.188 seconds (2026-10-16T13:14:34Z to13:16:58Z): all10 scenarios reached,726 requests/2 failures,527 command attempts/0 command failures; all527 bodies left=right=0,132 laser-on attempts. Normal scenarios excluding slow_client:721 requests/0 failures. Commandp95=48 ms,p99=78 ms,worst ACK gap234 ms. Pooled requestp95=48 ms,p99=79 ms,worst2016 ms. No observed reboot or Wi-Fi disconnect; final scoped release verified ownernone/session0, all wheel duties0 and laserfalse. Servo references held1.00197196/69.81806183 with pulses enabled; settings restored45/120 and Manual. These are commanded/readback values, not physical observation.

The slow_client scenario intentionally sent a partial HTTP header with no actuator lease. It produced one timed-out status and one timed-out recovery request; scenario aborted and recovered at elapsed113.532 s,13.235 s after initial failure (8.235 s after recovery attempts began). This is a real synchronous HTTP limitation, not a passing request or established Wi-Fi radio disconnect. Firmware disconnect counter stayed0 and uptime continued. Producer-expiry scenario then turned laser on with zero wheels, stopped producing, and verified session invalidation/laser off after~0.7 s; no target resumed. Files remain ignored underbuild/smoke-v090/.

One durable24-hour runner launched on this testing laptop at2026-10-16T13:17:17.4144055Z =2026-10-16 20:17:17 WIB, PID1620. Outputbuild/soak-24h-v090, launch identitybuild/soak-24h-v090.launch.json; full command line/start time verified, stderr empty. Planned end around2026-10-17 20:17:18 WIB plus cleanup, if uninterrupted. CLI exact identity, seed20261016,300-second scenarios,86400-second overall monotonic deadline. Initial checkpoint advanced while process lived, correct MAC/build and wheel/laser idle. It is RUNNING, not24-hour completed/PASS.

Created and verified Codex heartbeat automation pantau-soak-robot-24-jam-tanpa-roda, ACTIVE every30 minutes on this continuation chat. It inspects durable process/checkpoints, stays quiet when unchanged, reports actionable failures/completion, never spawns replacement jobs or reflashes, and pauses itself on terminal completion/failure. Monitoring cannot make a sleeping/offline Codex app execute; no guarantee of host availability is claimed. Long-run evidence/final report will be appended by supervision. Pending:24-hour outcome, physical OLED pixel observation, actual mechanical response and supply measurements.


### Interrupted v0.9.0 soak — final partial evidence

The original PID1620 no longer exists. Checkpoint failed at2026-10-16T14:51:33.570337Z (21:51:33 WIB),5655.719 s elapsed, with Another controller owns the robot. No final_idle event exists; final_idle_unverified records the same conflict. The complete count-validated request/scenario/percentile/recovery report is [SOAK_RESULT_2026-10-16.md](SOAK_RESULT_2026-10-16.md). All8,202 command attempts had zero wheels.130/13,609 requests failed (21 induced slow-client,109 elsewhere);32 command failures. This was not24-hour completion or exhibition acceptance. Monitoring was paused when terminal state was recognized. No automatic restart or flash occurred.

On explicit user continuation, read-only probe found the same v0.9.0/buildUTC2026-10-16T13:10:42Z/MAC, active Manual owner, wheel outputs0 and lasertrue. The monitor did not interfere. User then clarified slider movements stop mid-travel, asked for limits/dynamic arms/faster eased motion and explicitly authorized source fixes, GitHub push and OTA. A subsequent read-only sample showed Manual/unowned/released, references99.0164/102.635 and speed5 degrees/s, acceleration20; this slow profile remained from the interrupted soak because cleanup could not override another owner. Decimal display precision is not the cause of speed. The500 ms safety freeze must remain. Subsequent implementation/build/device evidence follows separately.


## v0.10.0 servo follow-up — 2026-10-17 WIB

Read repository AGENTS, API, arm design and soak/handoff/latest verification; re-read all nine mandatory vault home/folder/firmware/Gitflow/credential/HuaweiJIN/access/versioning/notification notes before implementation/publication. User explicitly requested source fixes, push and app OTA after reporting slider42 stops mid-travel, repeated offline UI, slow movement, configurable min/max and optional moving hands with forward/reverse. Preserve zero physical wheel test commands. No pin/partition/NVS/lab STA/AP/open-access change. Old soak is failed/partial; monitor PAUSED.

Source v0.9.0 -> v0.10.0. Detailed behavior and strict API/transport contracts: [SERVO_CONTROL_V010.md](SERVO_CONTROL_V010.md). Natural now60/240, bounded accelerate/cruise/brake trajectory; per-arm RAM min/max and explicit dynamic mode (OFF at boot). Firmware rejects out-of-limit targets without lease renewal and clamps intermediate references. Dynamic hands derive only from fresh same-direction nonzero wheel commands, never write wheels; tested only in simulation. Slider/numeric drafts persist across release/poll; actual commanded reference shown separately at0.1 degree with pulse/stop reason. Healthy recent command replies survive a failed status poll; real command failure, blur/hidden and500 ms firmware expiry still stop.

New HTTP81 service uses bundled ESP-IDF http_server on core1 priority2 below actuator3, max4 client sockets,1-second socket timeouts,1024-byte exact JSON schema and same-host browser CORS. HTTP80 retains UI/eyes/OTA/legacy API. Shared strict JSON validators; AP/OTA flags are atomic for cross-task reads. Page uses advertised81 readiness, one pending command and300 ms browser command deadline; no replay queue, silent reconnect/resume or stale lease extension. This isolates HTTP80 stalls but cannot guarantee RF/power behavior or protect81 from all slow clients.

Local software PASS: six GCC C++17/Wall/Wextra/Werror suites (control, arm policy/trajectory, actual driver, eyes, actual EyeService/fakeRTOS, bitmap regression) and64 Python/browser tests:13 client,32 Chromium,7 network-policy,2 identity,10 soak. New coverage: bound rejection/atomic configuration, inertia/dynamic limits, forward/reverse phase/idle/turn filtering, expiry freeze,90-degree Natural simulation under2 s with acceleration/braking; slider42 persists after release/poll, numeric/reference separation, policy UI/range attributes, dedicated port/CORS simulation and failed status retaining fresh control. All physical wheel movement cases remain simulation only.

Final pinned core3.3.10 N16R8 build PASS: v0.10.0, generated UTC2026-10-16T17:04:24Z =2026-10-17 00:04:24 WIB; application1,179,616 bytes, program1,179,474, staticRAM53,272. Emitted two6,291,456-byte OTA slots verified. SHA2562b82afbeedee7058c10f0234d39eb9581a7af71b4408beea2ead6db88a056019. Final incremental build reused core/library objects, regenerated metadata and rebuilt changed firmware; all copied h/cpp inputs matched source and source hashes stayed unchanged during final build. The earlier build was rejected by the source mutation gate and was not published/uploaded.

### v0.10.0 actual OTA and bounded live verification

App-only OTA PASS at192.168.1.66 after matching MACe0:72:a1:fd:0d:88, baseline identity/slot and acknowledged stop. Upload returned ok/rebooting. Post-boot matched v0.10.0/buildUTC2026-10-16T17:04:24Z, ownernone/session0, all wheel duties0, laserfalse and servo pulsesdisabled. Natural60/240, bounds0..180, dynamicOFF and control_channel ready/port81 verified. Served dashboard HTML matched source byte-for-byte. Evidence: ignoredbuild/ota-v010-result.json. Pins, partitions, NVS schema, lab network and open-access policy unchanged.

Bounded live test PASS from2026-10-16T17:09:48.203463Z to17:09:53.443058Z:34 command requests, zero unexpected request failures, commandp95 approximately94 ms andp99/worst109 ms. Every physical command and checked PWM output had zero wheels; laser remained off. Re-established the accepted pre-OTA reference99.0164/102.635, moved each requested reference by+2 degrees then returned, verifying reference arrival within0.25 degree. An incomplete HTTP80 header overlapped fresh HTTP81 control without interrupting this short test. HTTP81 same-origin CORS/foreign-origin rejection, OPTIONS, bad schema/range and temporary narrow limits verified. Dynamic enabled with zero wheels remained inactive, then resetOFF. Final scoped release verified ownernone/session0, wheel duties0, laserfalse, references99.0164/102.635 held with pulses enabled, full bounds and Natural60/240. These are commanded reference/PWM evidence, not measured mechanical position. Evidence: ignoredbuild/live-v010-result.json; initial harness guard errors occurred before actuator commands and were corrected, raw harness error retained.

Actual robot-page Chromium inspection PASS at desktop1100 and mobile390 widths: v0.10.0, real HTTP81 status responses, zero JavaScript/CORS errors and no POST requests. Degree/reference, limits and Natural60 controls rendered; mobile servo card visually inspected without horizontal overflow. Evidence: ignoredbuild/browser-v010-result.json andservo-v010-live-{desktop,mobile}.png. No browser actuator input was sent during inspection.

Limits: this short v0.10.0 test is not24-hour stability proof. Dynamic forward/reverse behavior was tested in native simulation only; physical wheels were never driven. No full servo sweep, mechanical end-stop calibration, supply measurement or physical OLED inspection. The failed v0.9.0 soak remains partial and its automation PAUSED; no replacement runner launched while the user is controlling the robot.

## v0.11.0 TCP engine integration — 2026-10-19 WIB

User explicitly approved TCP9000/server plan, source publication and app OTA.
Read repository AGENTS/README/API, servo/arm design, SOAK, complete handoff/latest
verification and actual sibling vault home, workspace foldering, firmware,
Gitflow, Rules_Kredensial_WiFi, WiFi_HuaweiJIN, Rules_Akses_Robot_Lab,
Rules_Versioning and Rules_Change_Notifications. Preserve lab STA/AP, open access,
pins, partitions and NVS policy. Branch feature/initial-robot-controller; PR#1
remains open to develop, never merge. No replacement soak/automation launched.

Preflight HTTP81 matched v0.10.0/buildUTC2026-10-16T17:04:24Z and MAC
e0:72:a1:fd:0d:88; Manual/unowned, boot reason, all wheel duties0, laserfalse,
servo pulsesdisabled, disconnect0. Ethernet192.168.1.122/direct route verified.
User confirmed physical initial pose unknown and selected NO actuator testing:
do not treat the boot90 references as measured pose. Device verification must
send no command request and never enable servo/laser. This overrides the earlier
plan's small physical movement/laser test; those remain simulation evidence only.

Implementation: bounded nonblocking socket server in its own priority2/core1
task below actuator3, JSON lines/UTF-8/depth16/1024 bytes, first-byte framing and
blocked-send250 ms deadlines, single client, no queued command replay. Shared
parser/protocol/peer loop are compiled in native socket tests. All paths use the
existing control service; bound transport IDs prevent HTTP/other socket command
or release of TCP sessions. TCP mode refuses active ownership; HTTP explicit
Manual/global stop retain authority. Disconnect releases only its own lease;
500 ms expiry unchanged. Status advertises port/protocol/readiness and transport.
Python client requires fresh producer timestamps (TTL<=300 ms), bounded I/O,
correlated ACKs, no automatic command/retry/reconnect. Read-only example and full
backend contract: TCP_CONTROL_V011.md.

Software PASS:75 Python/browser tests (64 existing +11 new real-local-socket SDK
tests), six existing GCC C++17/Wall/Wextra/Werror suites, plus new native TCP suite
with the actual framing/dispatcher/peer loop and pure control logic. TCP suite
also PASS under AddressSanitizer/UndefinedBehaviorSanitizer. Coverage includes
fragmented/coalesced frames,1024-byte boundary/oversize, invalid UTF-8/NUL/depth,
duplicate fields, range/limits/schema, sequences, wrong transport/socket/owner,
mode conflict, expiry without valid commands, disconnect/partial-frame deadline,
nonreading client, command pipeline rejection, stale disconnect preserving newer
HTTP ownership and maintenance invalidation. Native nonzero wheel/laser commands
are simulation only. Host cJSON1.7.19 extracted to ignored build cache without
admin/system install; CI installs its host development package. Target uses the
core-bundled cJSON. Initial compile failed for missing cJSON forward declaration;
corrected before the final build, no failed image uploaded.

Final pinned Arduino core3.3.10/N16R8 incremental build PASS. v0.11.0 generated
buildUTC2026-10-19T03:04:54Z =2026-10-19 10:04:54 WIB. App1,185,408 bytes;
program1,185,254, staticRAM53,296; both6,291,456-byte OTA slots verified.
SHA256 dbf3be9f28bb66712cad4b881fc2b53c16e7c5c108a3475899b4d68adafbeed4.
Manifest records clean_build=false; source hash gate passed and every input was
checked against the manifest again immediately before upload. No source changes
during the final build; first failed compile was not uploaded.

App-only OTA PASS. Rechecked MAC/baseline identity/slot, unowned state, wheel
duties0, laserfalse and pulsesdisabled before acknowledged stop/upload. Upload
returned ok/rebooting. Postboot matched exact v0.11.0/build/MAC, Manual, no owner,
zero wheels, laserfalse, pulsesdisabled, HTTP81ready and TCP9000ready/protocol1.
Raw evidence ignored:build/ota-v011-result.json. No pin/partition/NVS/profile
change; network/API/OTA remain open under the approved lab policy.

Actual no-actuator TCP suite PASS:50 normal status requests/0 failures,
statusp95=46 ms,p99/worst=47 ms. Command count **0**; these are status/transport
latencies, not command ACK/motion measurements. Verified real split/coalesced
frames, second-client rejection, extra/duplicate fields/version rejection,
HTTP Manual owner protected from TCP mode/acquire, HTTP release of TCP lease
rejected, no-command lease expires despite status polling, partial-frame deadline
closes/releases, old TCP closure preserves replacement HTTP owner, oversized/NUL/
invalid UTF-8 closes, and nonreading client closes under receive-buffer pressure.
Read-only Python discovery example also ran successfully on the actual robot.

The first slow-reader experiment FAILED to observe closure: it resized the
receive buffer after connecting and began draining after1 second, so it did not
establish sustained send backpressure. Preserve it as an unsuccessful experiment,
not an erased retry:build/live-v011-initial-slow-reader-failure.json. A separate
probe confirmed an accepted hello,1024-byte receive buffer configured BEFORE
connect,100 status frames and4 seconds with no reads; connection then observed
closed. The final suite used that corrected setup and passed. This confirms
closure under that setup, not a directly measured250 ms send deadline; native
socket tests verify the deadline logic. All induced disconnect/schema cases are
separate from the50 normal requests. Raw final/probe evidence:
build/live-v011-no-actuators.json andbuild/slow-reader-v011-probe.json.

Final HTTP/TCP readback: Manual, ownernone/session0/control_transportnone,
all four wheel PWM duties0, laserfalse, servo pulsesdisabled. Device uptime
increased continuously and disconnect counter stayed0 during the final suite;
no observed reboot/Wi-Fi disconnect. No physical motion/laser-on test, command
latency/gap measurement, real ML-engine integration or24-hour stability claim.
Active-command OTA/expiry/laser shutdown and wheel dynamics are simulation
evidence only. Old soak monitor status re-read as PAUSED; no runner restarted.

## v0.12.0 saved Wi-Fi profiles — 2026-10-19 WIB

User explicitly requested detected-SSID selection, persistent backup profiles and
automatic switching like a laptop. This supersedes the old fixed-HuaweiJIN-only
runtime restriction; HuaweiJIN remains initial fallback. Read repository AGENTS,
handoff/latest verification and actual vault home/folder/firmware/Gitflow,
Rules_Kredensial_WiFi, WiFi_HuaweiJIN, Rules_Akses_Robot_Lab, Rules_Versioning and
Rules_Change_Notifications. Updated local policy gate/AGENTS/docs for this explicit
exception; no other repository/vault policy changed. AP12345678 and open access
stay, pins/partitions stay. New NVS robot/wifiProfilesV1 imports legacy once then
preserves user profiles, including empty list. No unrelated key erase or returned
profile passwords. Wi-Fi SDK persistence is disabled before init to avoid flash
rewrites on every attempt; Preferences still stores profiles explicitly.

Behavior/API/migration contract: WIFI_PROFILES_V012.md.5 profiles, ordered fallback,
15 s attempts/30 s cycle cooldown, no healthy roaming, async manual scan and
always-active recovery AP. Main-task Wi-Fi mutations atomically reserve idle
control, stop/invalidate at link loss, gate acquire/command/OTA, preserve servo
hold and never resume stale targets. Manual operations refuse a foreign owner.
Same TCPv1 command contract, additive network_busy; HTTP80 profile routes and
dashboard scan/save/connect/delete/up/down. Shared radio can stall AP access.

Software PASS:82 Python/browser tests (37 Chromium,13 HTTP client,11 TCP client,
9 policy,2 identity,10 soak) and8 GCC C++17/Wall/Wextra/Werror native suites.
Actual network_service.cpp compiled with fake Wi-Fi/NVS/control adapter verifies
default/legacy/profile persistence, save without reconnect, secret redaction,
owner rejection/acquire gating, manual preferred profile then remaining priorities,
15/30 s timing, no healthy roaming, link-loss cancellation/no target resume,
OTA retry suppression, scan completion/deadline, empty-list AP-only boot, invalid
NVS preservation, failed-write atomicity, max5/type/range/UTF-8, escaped credentials
roundtrip, reorder/delete and millis wrap. Network suite also PASS ASan/UBSan.
Existing TCP/control/driver/eye suites passed; dynamic/wheel motion stays simulation.
Mock desktop/mobile Wi-Fi panel visually inspected, no horizontal overflow.

Final pinned core3.3.10 ESP32-S3 N16R8 incremental build PASS: v0.12.0,
buildUTC2026-10-19T12:46:30Z =2026-10-19 19:46:30 WIB. App1,199,472 bytes;
program1,199,326, staticRAM54,952; emitted two6,291,456-byte OTA slots verified.
SHA25615a337a671bd974ef1c29c7f1471e6f3e1a20c383f76bcea7e3977afdc4447fb.
Manifest clean_build=false; source hashes passed and were rechecked before OTA.
Earlier build was rejected by source-mutation gate after the SDK persistence fix;
that superseded artifact was not uploaded.

Device initially unreachable at last STA IP192.168.1.22. User then confirmed robot
hotspot/access192.168.4.1. Pre-OTA matched v0.11.0/buildUTC2026-10-19T03:04:54Z,
MACe0:72:a1:fd:0d:88, no owner/session, wheels0/laserOFF/pulsesdisabled, no OTA
busy. App-only upload acknowledged ok/rebooting. Initial30-attempt postboot check
FAILED: Windows showed association but ARP unresolved/no HTTP response. Disconnect
and reconnect the same saved robot AP profile restored access; no router/route/
firewall change, no second flash. Recovery readback matched exact v0.12.0/build/MAC,
Manual/unowned, zero wheel duties, laserOFF/pulsesdisabled, HTTP81/TCPready,
AP192.168.4.1 and storage_ready with migrated HuaweiJIN profile. Preserve this
initial verification failure in ignored build/ota-v012-result.json rather than
calling the first postboot loop a pass.

Actual AP-only Wi-Fi API test PARTIAL with failures retained. GET profiles and
async scan worked (7 deduplicated SSIDs returned); invalid password/order400 and
network-busy acquire409 verified. Temporary absent SSID profile was saved/reordered/
connect queued, but a fallback poll timed out after2 s and initial cleanup poll
also timed out. API later recovered; temporary profile explicitly removed and
original sole HuaweiJIN profile restored. No new actual user credentials supplied,
no healthy STA connection forced down. Evidence:build/live-wifi-v012-result.json.

One follow-up trace retained all8 failed requests and observed target/state
transitions: temporary profile connecting at elapsed3.313 s, HuaweiJIN connecting
at15.594 s, waiting at30.625 s. This establishes actual profile attempt failover
and cooldown entry, not successful association to a second working router or
uninterrupted AP service. Cleanup succeeded; original sole HuaweiJIN profile
restored. Final snapshot uptime333572 ms, Manual/owner none, network_busyfalse,
wheels0/laserOFF/pulsesdisabled; no commands sent. All observed reconnect counter
changes include intentional attempts, not evidence of independent RF outages.
Evidence:build/trace-wifi-v012-result.json. No failures discarded as retries.

Actual dashboard read-only Chromium desktop/mobile inspection PASS: versionv0.12.0,
profile list/scan results shown, blank password input, editableSSID, no JS errors,
no horizontal overflow, no POST requests. Evidence:build/browser-wifi-v012-live.json
andwifi-v012-live-{desktop,mobile}.png. Final output evidence is software/PWM,
not physical position. User's unknown-pose/no-actuator constraint maintained;
no wheel/servo/laser activation, shared-network/router disruption or new soak.

Limitations: successful automatic roaming between two credentialed routers and
reboot persistence of new user profiles remain host-simulation evidence only.
AP requests can fail during association/scan; AP active does not mean zero radio
interruption. This feature does not fix or certify TCP200 ms telemetry timeout,
power stability, mechanical behavior, real backend integration or24-hour soak.


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
