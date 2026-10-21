# Siapkan Auto — v0.15.0

## Previous / Changes

v0.14.1 exposes separate mode, Zero, sample and result controls and expires a
result after120 seconds. v0.15.0 adds a guided Siapkan Auto flow and holds one
completed result in RAM until accepted or invalidated during the same boot.
No automatic motion starts on page load, polling, reload, config Save or reboot.

1. Open Demo meja → **Siapkan Auto**. Check connection/controller, geometry and
   saved setup. Geometry is labeled as a model estimate, not sensor measurement.
2. Check the displayed base pose and limits against the actual hands. Confirm
   physical checking or open Manual to adjust them. Confirmation saves settings
   without sending an actuator command. Do not infer pose from boot90/reference.
3. Each missing calibration is offered in order: forward, right, left. Place the
   axle center at the fenced table center facing map front. The explicit button
   states that it selects Auto and moves wheels for one second at the shown PWM.
   It checks no active owner, selects Auto, sets confirmed Zero, starts ONE test.
4. After stopping, enter actual cm/degrees and observed coast distance (0 only if
   observed). Turns require direction confirmation. **Tidak bergerak/Arah salah**
   discard the pending result, not create valid gains. No automatic retry or next
   movement. Save once, then reposition before the next explicitly started test.
5. Once all three valid results are saved, reposition/confirm center and heading,
   select Auto and Set Zero through the guided button. **Mulai jelajah** is a
   separate explicit action; never starts as a consequence of saving calibration.

Peta, stage/status and STOP SEMUA remain available in the demo panel. Motor PWM,
minimum/deadband, limits and manual calibration controls remain under Pengaturan
lanjutan. Existing direct controls/API stay compatible. A conflict refuses without
stopping another controller. Real Start retains firmware checks and confirmation.

## Public API / result lifetime

Existing endpoints/bodies are unchanged. GET `/api/v1/demo` additionally returns:

- `sample_pending`: a successfully completed calibration awaits measurement.
- `sample_reason`: `no_sample`, `awaiting_measurement`, `result_saved`,
  `configuration_changed`, `new_test`, `new_run`, `demo_stop`,
  `control_or_network_changed`, or a run failure reason.
- Existing `sample_id,sample_kind,sample_integral` identify the actual test;
  kind0=forward,1=clockwise/right,2=anticlockwise/left.
- `config_valid`, `calibration_forward_fits`, `minimum_speed_allowed` explain
  configuration/space/speed guards in the guided UI.

One pending result survives time passing (including>120 s), browser close/reload
and operator hand-repositioning WITHOUT control commands. It holds no actuator
lease and cannot refresh500 ms command expiry. It is not persisted across reboot.
Global Stop/mode change, another controller acquisition (including a whole quick
acquire/release between demo ticks), network loss/change, OTA, a new run/test,
config Save or scoped demo Stop invalidate it. Confirmed Zero alone does not.
The internal release after a successful calibration is excluded; its expected
control epoch is checked so a concurrent controller/Stop cannot create a valid
pending result. Result Save rechecks control epoch under the existing exclusive
idle flash gate. Duplicate/old sample results are rejected; no automatic replay.

The guide does not submit old45-degree or estimated3–5 cm observations as fresh
results. Results require positive finite measured input; no movement is invalid.

## Unchanged / Upgrade

NVS robot/demoV1 format and saved gains remain unchanged fromv0.14.1. No schema
migration or unrelated key erase. PWM max80%, raw external HTTP/TCP commands,
pin/partition/network/access policy, single GPIO writer and500 ms expiry remain.
Boot Manual, hands confirmation/Zero/run/pending result clear. Valid stored wheel
calibration remains, provided motor PWM parameters have not changed. A normal
reboot then needs fresh physical hand checking and Zero, not all3 wheel tests.
No claim that old firmware gains were actually calibrated if mask remains0.

## Tests / Limitations

Native tests cover>120-second result acceptance, foreign/quick sessions, Stop,
config, new samples, network/OTA/reboot invalidation and storage preservation.
Chromium tests cover full sequence, units/direction, discarded invalid movement,
reload recovery, no command from preparation/config, mobile layout/space guard,
and no automatic start after result Save. See VERIFICATION for actual results.

Current temporary geometry75x75/reach15/margin10/coast0/speed5 gives10 cm half
goal extent. Uncalibrated forward prediction40*(.8-.5)=12 cm exceeds that extent,
so one-second forward calibration is correctly refused. The guide explains this
instead of silently inflating the table, inventing calibration or reducing a
physical clearance. Establish actual available space and appropriate settings
with the operator before a real trial. A floor test needs adequate physical
clearance and explicit operator setup; do not falsely confirm a fenced table.

No sensor measures hands, wheel speed, heading or edge position. Operator remains
responsible for physical observations and fenced-table setup. No physical random
exploration/containment/endurance claim follows from model/host tests. Old soak
remains failed/partial and monitor paused; no runner restarted.
