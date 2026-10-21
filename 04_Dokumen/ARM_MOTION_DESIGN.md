# Arm movement design — v0.8.0

v0.18.1 override: Manual follows accepted bounded references directly without
ArmMotion easing. Auto keeps the planner/settings below. UI targets are1°;
fractional API targets still valid. See [MANUAL_RESPONSE_V0181.md](MANUAL_RESPONSE_V0181.md).

Current v0.10.0 overrides: [SERVO_CONTROL_V010.md](SERVO_CONTROL_V010.md).
Natural60/240; Lembut25/60; Lincah90/360, RAM min/max and optional bounded
forward/reverse dynamic offsets in the existing control task. No dynamic gestures
at idle; stop/expiry still freezes immediately. Earlier paragraphs are historical.

v0.9.0 clarification: with pulses disabled, the first complete accepted command
seeds the requested arm reference before PWM enable, rather than sweeping from
boot's assumed90 degrees. It must be an accepted setup pose; software still cannot
know physical position. All following targets use the existing limits. The soak
uses the pre-OTA accepted reference and at most three degrees inward variation.
This adds no NVS reference storage, homing or idle gesture.

The previous ControlLogic assigned arm targets directly to the output at each10 ms tick. Large target changes therefore produced immediate PWM setpoint jumps. The new ArmMotion interpolator limits setpoint speed and acceleration, slows before arrival, and replans from its current commanded position/velocity when a target changes. Identical100 ms command heartbeats do not restart interpolation.

## Profiles and contract

| Profile | Speed limit | Acceleration limit |
|---|---:|---:|
| Lembut |25°/s|60°/s²|
| Natural (boot default)|45°/s|120°/s²|
| Lincah|75°/s|240°/s²|

These are starting values for bench tuning, not manufacturer ratings or measured arm speed. Custom values: speed5..120°/s, acceleration20..600°/s². POST /api/v1/arms/settings requires exactly speed_dps and acceleration_dps2;400 rejects types/ranges/nonfinite values,409 rejects while a lease is active or maintenance is enabled. Configuration is RAM-only, requires no lease and never refreshes a lease. Endpoints remain open LAN per lab policy.

Interpolation runs only in the existing10 ms actuator logic; no delays or additional servo library/timer ownership. Duty still uses50 Hz LEDC, logical left arm mirrored180-angle, right unchanged. A settings change does not move an arm. Command targets remain0..180; commanded_output shows the evolving reference, arm_motion.target_left/right show the latest requested destination. Neither is a measured angle. The moving flag describes an unfinished accepted reference trajectory, not physical motor motion.

Stop/release/500 ms expiry freezes both arm reference angles immediately and resets trajectory velocities; holds pulses if previously enabled. No continued travel to stale targets. OTA disables pulses as before. New acquisition starts from the last commanded hold point. A delayed control tick is capped to one10 ms interpolation step, preventing catch-up angle jumps at the cost of a longer trajectory. Repeated commands must keep the lease fresh during a long gesture.

This implementation limits velocity and acceleration toward a braking envelope. Endpoint and position-bound clamps can truncate final acceleration; it is not a guaranteed jerk-limited or fully synchronized multi-axis planner. Both arms use the same limits but need not arrive simultaneously. Retargeting can require short continued motion in the previous direction to decelerate; output references remain0..180.

## What software cannot establish

- At boot no encoder/servo feedback exists. Internal90° is a reference assumption, not known physical position. First pulse can still cause movement from an unknown physical pose; interpolation cannot remove that uncertainty. No automatic homing movement is added.
- Slowing setpoints does not change a hobby servo's internal torque, holding stiffness or PID. Persistent buzzing/straining after arrival can involve a stop, load, calibration or supply problem. Do not claim trajectory smoothing fixes these.
- Full0..180° mapping and500..2500 µs remain the existing configuration; actual usable travel requires module-specific calibration. No unsolicited endpoint restriction is imposed.
- No random tremor, bounce, overshoot, autonomous idle arm gestures or automatic power release is added. Those would change movement intent or holding behavior.

## Research and implementation choice

Read2026-10-16:

- [Pololu Maestro manual](https://www.pololu.com/docs/0j40/all): separate speed/acceleration limits shape servo output transitions; physical response also depends on servo construction, voltage and load. This supports the first-stage limit approach, not a requirement to buy a Maestro.
- [ServoEasing upstream](https://github.com/ArminJo/ServoEasing): supports eased, nonblocking and synchronized moves; requires an additional servo/timer stack on ESP32. The current implementation reuses the already-owned LEDC path instead of adding another output owner.
- [Ruckig official documentation](https://docs.ruckig.com/index.html) and [tutorial](https://docs.ruckig.com/tutorial.html): arbitrary-state trajectory generation with velocity, acceleration and jerk constraints. A possible later benchmark for continuous retargeting/synchronization; Ruckig is not bundled or claimed by this firmware.

Reasoned choice: speed/acceleration limits are8/10 for this two-servo first improvement: small, testable and compatible with stale-command holds. It cannot score higher without physical tuning, known startup position and tighter jerk/axis synchronization. Random idle arm wobble is3/10 here: it can look nervous and would introduce motion not requested by the engine.

## Steering correction

User observed that Right turns left and Left turns right while forward/reverse and servo side mapping are correct. Central swapMotorSides=true exchanges logical wheel targets before applying the existing physical channel polarities. This reverses differential steering while leaving equal-wheel forward/backward unchanged. Web/API/Python share the correction; labels and logical left/right signs remain intact. Pin wiring and polarity constants do not change. PWM raw left/right duty fields still refer to GPIO groups12/13 and10/11; wheel_channels_swapped reports the mapping. Physical turning direction remains a bench observation, not a unit-test measurement.
