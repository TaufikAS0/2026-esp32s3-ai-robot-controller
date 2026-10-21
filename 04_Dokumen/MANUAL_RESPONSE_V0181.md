# Manual response and larger controls — v0.18.1

## Previous

v0.18.0 applied the same speed/acceleration reference planner to Manual and Auto.
Natural60°/s with240°/s² eased into/out of slider targets. That software delay
was contrary to the operator's request for direct, responsive Manual control.

## Changes

Manual-owned control now applies the latest accepted, bounded hand reference at
the next10 ms control tick, without software speed/acceleration easing. Both
internal ArmMotion states are held at that reference with zero velocity, so
switching to Auto cannot inherit velocity from an earlier trajectory. Auto,
program/TCP engines in Auto and onboard demo/exhibition retain the planner.
The existing servo driver remains the only GPIO/LEDC writer at50Hz; radio,
request scheduling, PWM timing, servo mechanics and load still affect response.

Dashboard inputs remain integer1° targets. Repeated slider events mapping to the
same integer do not issue extra immediate commands during an active session;
the existing fresh100ms full-command heartbeat continues. No added smoothing,
movement queue or target retry. New explicit input can reacquire after Stop,
but heartbeat alone cannot restore an expired session. Fractional API targets
remain accepted for compatibility; UI quantization is not a new API restriction.
Optional dynamic offsets still respect the existing explicit policy/limits.

Operations has a larger joystick and knob, wider/longer hand sliders with bigger
thumbs, and a circular **LASER** toggle button. Text, color and `aria-pressed`
show requested ON/OFF; Enter/Space work with the native button. It uses the same
laser state as the legacy checkbox and is disabled outside available Manual
control. OFF is the initial state, with no remembered/replayed laser selection.
Controls adapt to phone portrait/landscape; STOP remains reachable. Advanced
motion settings are clearly labeled **Kehalusan gerak tangan Auto**.

## Unchanged

Command bodies/protocols, finite/range/ownership/sequence validation, configured
hand limits and mirrored left-hand mapping, wheel ramp/reversal,100ms producer /
500ms firmware expiry, Stop/blur/network/OTA behavior, pins/partitions/NVS and
saved Wi-Fi/open-LAN policy. Old soak remains partial, monitor paused.

## Tests

Native checks cover a large Manual target applied in one tick, a1° retarget,
Stop/expiry before a queued target can apply, Auto smoothing after Manual, and
unchanged policy/dynamic/ramp/owner guards. Chromium uses actual embedded HTML
with mocked HTTP: round button size/semantics/keyboard toggling/state, integer
targets and duplicate-input filtering, responsive controls, STOP/navigation,
laser retention and shared limits. See VERIFICATION for final regression/build/
deployment results; screenshots and positive mocked targets are not hardware proof.

## Limitations

Commanding an integer1° target does not establish physical1° positioning. No
encoder or servo feedback exists; direct control can produce faster physical
movement and does not establish safe endpoint travel or torque. This change
does not claim a measured maximum hand speed or RF latency improvement.

## Upgrade

v0.18.0 → v0.18.1 app-only OTA after pinned-core build/source identity gates and
fresh device MAC/build/idle verification. Ctrl+F5 then open **Operasi**. Boot
Manual/no owner/pulsesdisabled; no automatic movement or laser resumption.
Validation keeps real wheels/laser off and sends no hand movement command.
