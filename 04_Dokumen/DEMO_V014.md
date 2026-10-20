# Demo meja: simulasi dan jelajah mandiri — v0.14.0

v0.14.1 supersedes the25% PWM cap and raw-PWM model below. Read
[DEMO_V0141_PWM.md](DEMO_V0141_PWM.md) for optional minimum/deadband fields,
80% upper limit, calibration invalidation, speed guards and storage compatibility.

## Previous / Changes

Previous v0.13.0 only provides manual/external-engine actuator control. v0.14.0
adds an onboard demo: choose a random goal, turn, drive forward, brake, gesture,
repeat until Stop. Closing the browser does not stop this explicitly started
onboard producer. No automatic demo starts at boot, reconnect, OTA or mode switch.

The map is an **ideal open-loop estimate**, never measured position. A physical
fence is required for the agreed tabletop use. Wheel slip, unequal motors, battery
voltage, surface and calibration errors can move the real robot outside the map.
This is not edge detection, homing or guaranteed containment.

## Setup in the dashboard

1. Open **Demo meja**. Enter table length/width in cm and maximum robot/hand reach
   measured from the wheel-axle midpoint. Include all hand positions used by the
   gestures. Additional edge clearance defaults to10 cm. The map's green rectangle
   is the allowed region for the estimated robot center, not the full tabletop.
2. Save. Simulation can run without taking an actuator lease, selecting Auto,
   enabling servo pulses or writing physical outputs. Its initial gains are
   illustrative until physically calibrated. Stop simulation before real setup.
3. Open **Kalibrasi gerak & tangan**, choose Manual and use the existing controls
   to establish an accepted physical hand pose. Enter base angles and safe min/max
   for each hand, then explicitly confirm physical checking. Do not trust boot90
   or any status angle as a physical measurement. Limits/base must also satisfy
   the current global arm policy. Before real Start/calibration, enabled references
   must be within1 degree of the supplied base pose; place them there manually.
4. Save, select Auto explicitly, place the axle midpoint at table center, face
   forward on the map (+Y), then **Set Zero**. Confirm center placement and fence.
   Zero assigns `(0,0,0)`; it does not move/find the center. X is right, Y forward,
   heading degrees increase clockwise.
5. Start each one-second calibration explicitly: forward, clockwise, anticlockwise.
   PWM defaults15%, maximum25%. Watch the robot and measure actual cm/angle after
   each test, including the stated coast-distance input. A result must be positive
   and finite; zero/no movement is invalid. Ensure the actual turn direction
   matches the requested direction; an opposite turn is not a positive result.
   Reposition and Set Zero before each test. Results expire after120 s and belong
   only to the latest sample ID. Enter all3 results; changing PWM invalidates them.
6. After calibration, reposition, Set Zero again and press **Mulai robot**. It
   remains active until Stop or a fault. **Stop demo** is scoped to its run ID;
   **STOP SEMUA** remains the unconditional stop for any controller. Stop freezes
   arm travel rather than automatically moving hands to a parking angle.

All dimensions, envelope and clearances are editable in the web dashboard and
persist after Save. The operator may enter estimates for simulation and revise
them later; estimates do not establish physical containment. This delivery leaves
dimensions blank for the operator rather than inventing the actual table size.
Physical testing requires an operator next to the fenced table. Default
base90/min0/max180 values are editable placeholders, not accepted physical limits.
After reboot, hand confirmation is cleared even though saved limits/gains survive.

## Model and implementation

- Pure `DemoPlanner` is shared by simulation and real mode. Targets are sampled
  inside a rectangle inset by `radius + margin + coast + speed*0.5`; minimum goal
  distance5 cm, bounded64 selection attempts. Invalid/tiny areas are rejected.
- Integrates applied logical wheel output, including the existing motor ramp and
  reversal dwell, into a clockwise differential-drive model. Forward scale is
  cm per normalized-PWM-second; clockwise/anticlockwise have independent scales
  in degrees per normalized-PWM-second. No encoder/IMU exists.
- Planner updates20 ms; commands every100 ms. Initial speed cap5 cm/s, turn cap
  30 deg/s, PWM cap25%. Slowdown uses remaining distance/angle. Model boundary
  crossing, nonfinite state, >300 ms planner gap, or120 s phase timeout stop demo.
- Drive only forward. The brake stage lasts at least500 ms and gestures require
  applied wheel outputs to be zero. Hand motion never drives the wheels. Gesture outputs
  remain within approved limits, max10 degrees from base: one-hand two waves,
  both-hand small raise, or eyes-only. Existing arm trajectory limits still apply.
- Demo suppresses dynamic hands only for its own lease. Eye cues use Happy,
  WinkLeft and Curious, all existing box-family expressions. Overlay changes local
  render settings, not the user's stored/current eye configuration. Laser alwaysOFF.
- `DemoService` runs independently of synchronous web handling. Only the existing
  control task writes actuator GPIO. A dedicated internal lease flag separates
  HTTP, TCP and demo even if an external client knows the public session ID.
  Real start checks the Zero epoch atomically with acquire. External mode/global
  Stop, network loss, OTA, session loss and PWM maintenance cancel the producer.
  Scoped release cannot stop a replacement owner. Simulation uses private virtual
  ControlLogic and never renews/changes the real actuator lease.
- Trace is a256-point ring, sampled at most every200 ms. HTTP GET/JSON snapshot
  serialization happens outside the demo mutex. Runtime remains bounded when
  browser is closed; no unbounded trajectory queue or event history.

## API

HTTP80 supports GET `/api/v1/demo` and POST routes below. HTTP81 additionally
supports GET/demo and POST/demo/stop with the existing host-limited CORS policy.
TCPv1 commands remain unchanged; HTTP/TCP status may return control_transport=demo.
Requests require JSON Content-Type, strict fields and <=1024 bytes.

| POST suffix | Body / behavior |
|---|---|
| `/demo/config` | All numeric fields: `width_cm,length_cm,radius_cm,margin_cm,coast_cm,speed_cm_s,turn_deg_s,pwm,base_left,base_right,min_left,max_left,min_right,max_right`, plus boolean `arms_confirmed`. PWM is0..0.25, not percentage. Idle only. Does not move actuators. |
| `/demo/zero` | `{"confirmed_center":true,"confirmed_fence":true}`; idle/valid geometry required, does not move. |
| `/demo/start` | `{"mode":"simulation"}` or `{"mode":"robot"}`. Real requires explicit Auto, ready motion/hand calibration, current Zero, no other owner/OTA/network change. |
| `/demo/stop` | `{"run_id":N}`; cannot stop another/newer run. |
| `/demo/calibration/start` | `{"kind":"forward"}` / `"cw"` / `"ccw"`. Same real guards except full motion calibration is not yet required. One-second operation; forward predicted travel must fit geometry. |
| `/demo/calibration/result` | `{"sample_id":N,"measured":positive_number,"coast_cm":nonnegative_number}`. cm for forward, degrees for turns. Normalized PWM-time integral determines gain. Does not move actuators. |

Mutations return200 or202;400 schema/range/geometry error,409 unavailable lease/
mode/Zero/calibration/run ID,503 unavailable task/storage. No valid command lease
is extended by configuration/status requests. Explicit calibration is real movement.

GET/demo reports config, task/storage readiness, running/simulation/calibrating,
run/sample ID, sample integral, Zero/calibration/real-start readiness, phase/reason,
estimated pose/goal and ordered bounded trace. Config exposes stored gains and a
3-bit motion-calibration mask (forward1, CW2, CCW4). These fields cannot be injected
through POST/config. Fresh hand confirmation is separately required after reboot.

## Unchanged / Upgrade

No pin, partition, Wi-Fi profile schema/default, open-LAN policy or TCPv1 schema
change. Firmware command expiry remains500 ms, boot remains Manual. New versioned
NVS key `robot/demoV1` contains config/calibration only; no pose, Zero or running
state. Writes use the existing exclusive idle gate (network_busy briefly true),
blocking HTTP/TCP acquire during flash; no Wi-Fi changes occur. Invalid stored
data is preserved and real demo remains unavailable; never clear unrelated NVS.
App-only OTA preserves Wi-Fi and other settings; no automatic resume.

## Tests / Limitations

Native tests cover model containment, many deterministic random paths, ramp,
turns, gestures/bounds, timer wrap/stall, internal lease separation, calibration,
global stop/replacement owner/OTA and reboot/storage behavior. Browser tests cover
simulation without physical commands, configuration/Zero/readiness, explicit real
confirmation, sample IDs, scoped Stop, reopen monitoring and mobile layout. Eye
service tests verify box-family cues and unchanged user settings. Full regression,
sanitizers, pinned build and actual upload evidence appear in VERIFICATION.md.

Host/model success is not physical position proof. No safety sensor detects a
fence collision, lifting or dragging the robot, edge position, or accumulating
pose drift. Reposition and re-zero after any stop or manual intervention. This
feature is neither a 24-hour soak nor a restart of the previous partial soak.
