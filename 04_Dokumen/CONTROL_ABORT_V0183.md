# HTTP abort diagnosis and laser anger — v0.18.3

## Previous

v0.18.2 dashboard uses HTTP POST commands every100ms, normally to dedicated
HTTP81, not WebSocket. The300ms per-command browser AbortController and400ms
producer guard precede the unchanged500ms firmware expiry. Status uses1500ms.
Timer cancellation, rejected commands, browser blur/hidden, pointer cancellation
and stale commands all stop control; Wi-Fi need not disconnect for this to happen.

## Evidence

Read-only observation of expectedMACe0:72:a1:fd:0d:88 at192.168.1.14,
v0.18.2/build2026-10-21T04:55:20Z, while the operator controlled the robot:
20 status GETs,0 failures,p95~47ms. Uptime continuously increased and Wi-Fi
disconnect counter stayed0. Session1553119055 ran, a new session1247450466 was
acquired, then owner/session cleared with reason `timeout`. No read-only probe
commanded, acquired, stopped or moved hardware. Some sampled wheel outputs were
nonzero because the operator owned and drove the robot; probes were observation.

Fact: expiry shows a500ms gap without an accepted command. Unknown: the triggering
browser request timeline, focus/pointer event, client packet loss or scheduling
delay. One healthy RSSI reading(-53dBm) and no STA disconnect do not prove all
packets or the browser producer healthy. No reboot occurred in this short sample;
this does not rule out unrelated electrical problems at another time. Do not
label the report a Wi-Fi failure, power fault or an HTTP root cause without logs.

HTTP81's single server task serves status/demo and commands with4 client sockets,
1s receive/send deadlines. A slow request can still delay a command. The page
also polled demo/Wi-Fi and rendered hidden eye canvases during Operations. Those
are avoidable load/contention paths, not proven to have caused this exact event.

## Changes

- Suspend demo/Wi-Fi background polls while this browser owns/acquires Manual.
  Resume on later scheduled polls when released; normal status monitoring stays.
  Stop rendering Eye Studio canvas previews while Operations is displayed or the
  document hidden. Firmware eye animation is unchanged by this UI optimization.
- Classify local ACK timeout with path, measured elapsed time and deadline;
  distinguish HTTP/network errors and backend rejection text. Do not claim a
  timeout proves Wi-Fi disconnected. A bounded20-entry UTC diagnostic history
  retains errors/focus/hidden/session-stop notifications after recovery until
  reload, under **Batas tangan → Diagnostik kendali · HTTP**. No command payload,
  SSID/password or persistent storage included. History is per browser page.
- Laser output now overrides movement/hands/selected Manual expression/Showcase
  with Angry box eyes (`laser_focus`). OTA remains higher priority. LaserOFF
  returns to existing settings/reactions without modifying saved eye settings;
  disabled displays/diagnostic pixel tests remain respected. New priority lives
  in portable EyeAnimator, so it applies equally to accepted HTTP/TCP targets.

## Unchanged

Transport remains HTTP; TCP9000 engine API stays available. No automatic command
retry/replay or lease takeover;100ms production,300ms command abort,400ms producer
guard and500ms firmware expiry unchanged. Pointer/focus/hidden fail-safe remains.
No actuator, GPIO, partition, NVS, Wi-Fi or open-LAN policy changes. Old soak
partial/monitor paused. This mitigation is not a claim of eliminated dropouts.

## Tests

Chromium reproduces a450ms delayed ACK while connected, confirms session/laser/
wheels clear and no automatic reacquisition, and verifies explicit timeout text.
Checks background polls and hidden preview suppression without stopping fresh
Manual production; blur still stops and records the reason. Native eye tests
cover laser priority during forward/reverse/turn/hands and every eye mode,
OTA precedence and restoration of selected expression after laserOFF. Existing
regression/build checks and actual deployment status are in VERIFICATION.

## Limitations

Mock delay proves a software path, not the cause of every physical incident.
Read-only LAN status cannot capture the remote browser's ACK/focus timeline.
WebSocket could remove HTTP request overhead but would still need fresh-command
deadlines, no replay and safe stop, and would not fix browser suspension, Wi-Fi
loss or power resets. A transport migration requires separate implementation and
comparison; it is not silently bundled as an untested fix here.

## Upgrade

v0.18.2 → v0.18.3 app-only OTA only after build and current identity/idle checks;
never interrupt the operator's active Manual session. Ctrl+F5 after deployment.
On another failure, open Diagnostik and copy the history before reloading. Real
combined wheel/laser trials require the operator's physical setup and separate
observation; idle checks do not activate outputs or prove long-term stability.
