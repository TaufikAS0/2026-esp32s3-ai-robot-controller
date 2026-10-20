# Pameran lantai dan Reset ke Tengah — v0.16.0

## Previous

v0.15.0 supports measured table calibration and a guided Auto setup. Subsequent
operator-observed trials used a laptop HTTP producer, not saved demo calibration:
80% PWM for one second moved3–4 cm with4–5° left drift; right and left turns were
about45°, with no visible coasting. A bounded out-and-back trial was reported by
the operator as returning to center; no numeric final position error was measured.
Left90/130/150/170/180 were reported safe;180 points up, and the approved wave was
then lowered to raised140, swing130–150. Right remains90. The user now explicitly
requests supervised random FLOOR exhibition within an estimated30x30 cm axle area,
accepts drift and will manually reposition at the center. This supersedes the
measured-table prerequisite ONLY for the new, clearly separate exhibition mode.

## Changes

Open **Demo meja → Jelajah pameran · pagar virtual**. The robot itself runs the
planner; a laptop command producer is no longer required for this mode. Browser
close/reload only affects observation; it never sends/replays motion commands.

1. Use a clear, flat floor with an operator present. Mark the axle center and
   initial front direction. Width/length describe the desired estimated AXLE
   center area, not the table, body/hand clearance or a measured physical fence.
2. With the robot stopped, put it at that mark facing the initial direction.
   Confirm the physical arm poses/limits: left90–150 and right90 are safe.
3. Check the placement checkbox, then **Reset ke Tengah · siapkan Auto**.
   This explicitly selects Auto, installs the virtual area, and sets estimated
   position(0,0)/heading0 with an empty trace. It does NOT physically drive home,
   move a servo, acquire an actuator lease, or start motion.
4. Press **Mulai jelajah pameran** separately. Robot chooses random goals, turns,
   advances, brakes, then smiles and waves the LEFT arm at raised140,130–150
   twice, then returns to90. Right stays90; laser always OFF. Gesture starts only
   after wheel outputs are zero. A maximum256 trace points is retained.
5. If the physical robot drifts: **STOP → manually replace at center/front →
   Reset ke Tengah → Mulai**. Reset is refused during motion, another controller,
   network changes or OTA. Stop never automatically resets/restarts/returns home.

**Simulasi pameran tanpa gerak** uses the same model with a virtual controller,
without actual acquisition, GPIO or eye effects. It clears prior center validity.

## Estimate model

Defaults use the midpoint3.5 cm and4.5° left drift, plus45° turns, from the above
rough observations. They are estimates, NOT newly measured/calibrated gains.
Gains normalize these one-second observations through the pure MotorRamp applied
effort integral at80% and a50% modeled deadband. The existing planner integrates
current ramped outputs, not raw target durations, and includes estimated left yaw
while advancing. Demo limits remain80% PWM,3.5 cm/s target cap and30°/s turn cap,
60% minimum PWM. Accuracy away from that single test point is not established.

The30x30 cm default bounds the model axle center to±15 cm. Random destinations
are further inset by the existing stopping reserve. A0.1 cm internal coordinate
offset adapts the existing planner geometry; it is NOT an invented robot radius
or table measurement. Geometry and center are RAM-only, with editable15–200 cm
sides. Reboot restores30x30 defaults, Manual, no owner, invalid center, no run.
The real robot has no position/heading/edge sensing and may leave the virtual
box. Neither the model, the successful out-and-back observation, nor manual
repositioning establishes physical containment or repeatable return accuracy.

## API

Existing `GET /api/v1/demo` adds `exhibition`:

- `selected`, `center_valid`, `can_start`, `width_cm`, `length_cm`;
- `source: operator_estimates_not_calibration`, `persistent:false`;
- observation midpoint fields, trial duration and approved left wave angles.

New POST endpoints on port80 use strict JSON schemas (no duplicate/extra fields):

| Path under `/api/v1/demo/exhibition` | Body | Effect |
|---|---|---|
| `/config` | `{"width_cm":30,"length_cm":30}` | Idle-only RAM config, invalidate center/pending sample; no GPIO or NVS write. |
| `/center` | `{"confirmed_center":true,"confirmed_floor":true,"confirmed_hands":true}` | Idle Auto, no other owner, allowed arm range; resets estimate/trace only. |
| `/start` | `{"mode":"robot","accept_estimates":true}` | Requires fresh center and explicit Auto, takes scoped internal demo lease. |
| `/start` | `{"mode":"simulation","accept_estimates":true}` | Starts isolated model without real control. |

202 means started;200 config/center accepted;400 invalid/unsupported;409 current
state/owner/pose/center conflict. Global Stop remains `/api/v1/stop {}`.
Scoped Stop remains `/api/v1/demo/stop {"run_id":...}` on80/81. Read status on81.
Reset during a run is409; the browser requires Stop and physical repositioning
before Reset. Reset/GET/config never masquerade as movement back to center.

## Unchanged

Existing table mode keeps its measured calibration/geometry/pose/Zero requirements;
the old `robot/demoV1` schema and calibration bits are never overwritten by floor
estimates. No new NVS keys. Pins, partitions, Wi-Fi profiles/strongest-saved policy,
open-LAN HTTP/TCP protocol, actuator500 ms expiry and single10 ms GPIO writer stay.
Both modes share the existing scoped internal `demo` transport. External HTTP/TCP
cannot command/release it. Stop/mode/other control/network/OTA/stalled planner
invalidate the center and terminate motion; no resume or replay after recovery.
Temporary Happy eye effects use the existing internal demo eye override and
release on stop; user eye settings are never overwritten.

## Tests

Native tests cover long random model paths, box bounds, rollover, arm/eye/stopped
gesture constraints, profile validation, no fake calibration/NVS write, no GPIO
from center/config/simulation, all stop phases, foreign owner/scoped release,
stale centers, network/OTA/planner failure and reboot. Chromium covers explicit
reset/start/stop, reload without replay, simulation, ownership and mobile layout.
Actual build/upload and live evidence are in VERIFICATION.md; software PASS is
not physical motion, containment, endurance or24-hour PASS.

## Limitations / Upgrade

v0.15.0 → v0.16.0 app-only OTA after tests and identity/idle checks. Saved profiles
and table calibration survive unchanged; runtime arm policy still resets at boot.
After reboot inspect hands/placement and use the explicit Reset/Start sequence.
No automatic physical start after upload. Old soak remains partial, monitor paused.
