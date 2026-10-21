# Servo control v0.10.0

## Trigger and evidence

The user reported that a42-degree slider target stopped mid-travel and the page
frequently went offline. A read-only v0.9.0 sample showed `owner:none`,
`reason:released`, reference99.0164/102.635 and speed5 degrees/s, acceleration20.
The interrupted soak could not restore its slow profile while another controller
owned the robot. A90-degree move at5 degrees/s needs at least18 seconds, excluding
acceleration. Decimal display precision does not cause that speed limit.

The previous page overwrote slider targets from polled held references whenever
its session ended, and any status error released control even if command replies
were still fresh. Loss of focus intentionally releases control. The500 ms device
expiry must still freeze the reference and discard remaining travel; completing a
stale target after the operator loses control is not an acceptable fix.

## Motion and limits

Natural defaults60 degrees/s and240 degrees/s2. Lembut remains25/60; Lincah90/360.
The existing reference planner accelerates, cruises where distance permits, then
brakes toward arrival; repeated heartbeats do not restart it. It is not a measured
servo-speed or jerk-limited planner. Native simulation verifies a90-degree move
under2 seconds at Natural, bounded startup step and exact target arrival.

The page shows an integer target (slider or numeric input), separate reference
angle rounded to0.1 degree, pulse enabled/disabled, and stop/session reason. A
released/failed session preserves the operator's draft; polling cannot replace42
with the unfinished held reference. Sending a new target explicitly reacquires.
After network failure there is no automatic target resumption.

`POST /api/v1/arms/policy` requires exactly:

```json
{"min_left":20,"max_left":140,"min_right":30,"max_right":150,"dynamic":false,"amplitude_deg":8,"period_ms":2400}
```

These example numbers are NOT calibrated mechanical limits. Min/max are finite
logical angles within0..180 with min<max for each hand. Amplitude0..30 degrees;
integer period1000..10000 ms. `dynamic` must be boolean. Invalid schemas/types/
duplicates/ranges reject400. Occupied ownership, maintenance, or enabled reference
outside the proposed limits reject409. Settings themselves never move an arm.
The page's Stop & Apply releases only its own session before sending settings.
Apply discards old drafts and repopulates from held references.

Firmware rejects targets outside configured limits without refreshing freshness,
and bounds intermediate references even during deceleration or dynamic movement.
Limits are logical before left180-angle pulse mirroring. The initial0..180 range
is the electrical command range, not measured safe mechanical travel. Operator
calibration is still required. All settings are RAM-only; boot resets0..180,
dynamic OFF, Natural60/240. No NVS schema or pin/partition change.

## Optional dynamic hands

Dynamic mode is explicit and OFF at boot. Only same-direction, nonzero forward/
reverse wheel targets with fresh ownership activate an opposite-phase arm offset
around the complete command's hand targets. Magnitude scales with ramped wheel
command; configured amplitude and limits bound it. Reverse changes phase direction.
Stationary wheels and in-place turns do not generate gestures. Arms still use the
same speed/acceleration planner. Stop/expiry immediately freezes references and
cancels the oscillator. This never generates wheel commands or introduces another
GPIO/timer owner. Physical wheel tests remain unauthorized; forward/reverse arm
responses are simulated with native logic only.

## Separate command channel

The dashboard/eyes/OTA continue at HTTP80. A core-bundled ESP-IDF HTTP server on81
serves status, command, acquire/release/mode, stop, arm settings and policy. It runs
on core1/priority2, below the priority3 actuator task. The pinned core3.3.10 supports
this server; no new firmware library is installed. Max4 client sockets,1-second
socket receive/send timeout, bounded1024-byte JSON bodies and exact schemas apply.

Status advertises `control_channel:{ready:true,port:81}`. The page selects81 only
when ready; legacy clients/routes on80 remain available. Browser CORS accepts only
the dashboard origin on the same hostname/IP (port80); non-browser LAN clients
remain open per the existing policy. Responses include no-store and errors are
JSON. API token/OTA password are not introduced.

Command requests are single-flight with300 ms browser timeout; no queued old
commands or silent reconnect are added. A producer gap reaching400 ms stops local
production, while firmware still expires at500 ms. A failed status poll does not
release a session whose command replies are still fresh. Genuine command failure,
blur/hidden tab, rejection and stop still release control. Cosmetic/catalog loads
use80 and cannot occupy the81 server's request loop.

This isolates control from synchronous HTTP80 slow-client/dashboard stalls. It
does not prove RF quality, supply stability or universal latency; a slow client on
81 can still cause a stall and safe expiry. Long-run and physical servo behavior
need separate evidence. Read VERIFICATION for actual tested/uploaded identity.

Python helpers: `set_arm_motion(60,240)` and `set_arm_policy(...)`. They require
released ownership and never renew producer freshness. Python can explicitly use
`http://<verified-ip>:81` as its base URL for a future guarded experiment.
