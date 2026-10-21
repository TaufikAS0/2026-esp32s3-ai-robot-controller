# Demo motor deadband correction — v0.14.1

## Previous / Changes

v0.14.0 capped demo PWM at25% and used speed proportional to raw PWM. Operator
testing on20 October2026 found no floor displacement at15%, about1 cm straight
at60%, and an estimated3–5 cm at80% in one-second trials. With wheels lifted,
40% appeared stationary,60% slow and80% stronger. The descending test answer
"40%" was interpreted as the stopping point, not a precisely measured threshold.
These observations do not identify the electrical/mechanical cause or establish
a precise loaded speed curve. Motor driver remains MX1508, PWM20 kHz.

Demo configuration now supports an optional PAIR of normalized fields:

- `min_pwm`: minimum nonzero demo target,0..`pwm`.
- `deadband_pwm`: model's stationary threshold,0..<`min_pwm` when minimum is nonzero.
- `pwm`: calibration amplitude and upper demo limit, now>0..0.80. Default remains15%.

Both new fields default0, retaining the legacy linear model. Deadband>0 requires
minimum>deadband. The advanced web panel displays all3 as percentages. Raising
the allowed limit does not automatically raise a saved/default value or start
an actuator. No kick-start pulse or automatic retry was added.

The model uses signed effective effort `sign(p)*max(abs(p)-deadband_pwm,0)`.
Calibration integrates this effective APPLIED effort, including the existing ramp;
gains are distance/angle per effective-effort-second. Planner inversely maps the
desired effective effort into PWM, bounded by minimum and maximum. At zero target,
PWM remains exactly zero. Raw HTTP/TCP commands remain normalized raw PWM.

Turn control explicitly commands zero inside its heading tolerance before
advancing to Drive; it no longer relies on proportional PWM shrinking to zero,
which would fail with a nonzero minimum. If modeled speed/turn rate at minimum
PWM exceeds5 cm/s or the selected speed limit, or30 deg/s/the selected turn limit,
real readiness remains false. GET/demo adds `minimum_speed_allowed`; the dashboard
explains this guard. There is no claim of measured speed limiting without sensors.

Changing any PWM parameter invalidates all3 motion-calibration results and Zero.
Gains/defaults do not become accepted calibration from these observations. Positive
operator measurements from fresh forward/CW/CCW samples remain required. A zero
movement result is not calibration. Never populate readiness flags with guesses.

## Unchanged / Upgrade

HTTP config accepts either the exact old15-field schema or the exact new17-field
schema; both new fields must appear together. Duplicate/unknown fields still fail.
NVS `robot/demoV1` accepts the old19-field object and the new21-field object;
future saves use the extended object without erasing unrelated keys. Old gains
retain their meaning because missing deadband defaults0. Downgrading to v0.14.0
after an extended save causes its demo parser to reject the object and leave
demo storage unavailable; unrelated settings remain. Do not erase NVS to downgrade.

App-only OTA preserves settings but resets mode to Manual and clears runtime hand
confirmation/Zero/running state. Nothing resumes. The operator must establish the
accepted pose and Zero again. Pins, partition slots, Wi-Fi profiles/defaults,
open-LAN access, TCPv1 bodies,500 ms expiry and the single GPIO writer are unchanged.
Old soak remains partial/paused. No new soak/automation starts.

## Tests / Limitations

Native tests cover legacy/extended schema, duplicates/ranges, no modeled movement
below deadband, inverse PWM limits, minimum-speed refusal, random paths with a
nonzero minimum, service calibration units/invalidation and reboot preservation.
Browser tests verify percentage conversion, no actuator request during Save and
the minimum-speed explanation. Full build/OTA evidence is in VERIFICATION.md.

Observed floor distances are approximate, not a calibrated trajectory guarantee.
Table/reach configuration currently reflects a user-authorized temporary MODEL,
not measured table dimensions:75x75 cm/reach15 cm produced a20x20 cm goal region
with10 cm margin, zero coast and5 cm/s cap. Hand88–92 degree trial was reported
safe, base90 accepted. New firmware still requires fresh physical commissioning.
Measure coast, forward/CW/CCW, and physical pose drift before real exploration.
High PWM plus uncertain traction can produce sudden motion; use the agreed
physical fence/operator. No24-hour or physical containment claim.
