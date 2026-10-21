# Manual joystick and laser — v0.17.0

## Previous

v0.16.0 direction buttons called the full dashboard `stop()` on pointer release.
That released the entire actuator lease, cleared the dashboard laser selection,
and correctly switched the firmware laser off. Thus a normal end to driving
also ended the requested laser output. This source path reproduces the reported
symptom; it is not evidence of an electrical fault or a Wi-Fi dropout.

## Changes

In Manual choose **Tampilan kontrol Manual → Joystick + slider tangan vertikal**.
The old **Tombol arah** view remains selectable. Changing views releases this
dashboard's lease, clears wheel/laser targets, and never acquires/moves by itself.

- Drag up/down for forward/reverse, left/right for differential steering; diagonal
  input mixes both axes. Radial input is clamped, center12% is neutral, and wheel
  mixing is normalized to the existing Daya motor limit. Wheel signs/mapping
  remain the same as legacy buttons. This is a screen joystick, not a physical
  Bluetooth/USB gamepad driver.
- Slider on each side controls its corresponding logical arm angle. Higher
  angle is at the top; displayed target, numeric input, old slider and new slider
  stay synchronized. Current per-arm software min/max are shared. There is no
  inferred mechanical limit or new physical arm calibration.
- Multiple touchscreen pointers can operate wheel/arm controls together. One
  pointer owns the drive control at a time; another drive pointer cannot replace
  it. Focused joystick also accepts arrow keys; held-key repeat cannot restart
  after cancellation without a fresh keydown.
- Normal pointer/key release zeros BOTH wheel targets. If laser is selected,
  the SAME Manual session stays active and its100 ms full-command updates keep
  laser on, including while driving or changing arm targets. No lease release
  or automatic laser re-acquire/replay occurs. If laser is off, normal drive
  release keeps the legacy full-session stop behavior (arms freeze).
- **STOP SEMUA**, session loss, request error/late ACK, blur, hidden/pagehide,
  mode/view change, pointer cancel or unexpected lost capture clear laser and
  wheel targets and release only this dashboard lease. Normal lost capture AFTER
  a handled pointerup is ignored so it cannot undo the laser-preserving release.
  Auto/exhibition continues to require laserOFF; its behavior is unchanged.

The checkbox shows this dashboard's requested Manual laser state, not an optical
sensor. A continued laser selection requires the active browser/controller; no
laser state is saved across release/reboot/reconnect. Firmware500 ms timeout
remains the backstop if browser/session/network fails.

## Unchanged

No API body/endpoint/protocol changes, GPIO/polarity/servo mapping, partitions,
Wi-Fi/strongest-saved profile policy, LAN access/authentication, NVS keys/schema,
actuator task, servo trajectory, or500 ms timeout changes. A page load/poll, view
selection and slider synchronization do not move actuators. Buttons stay the
default view for compatibility; the joystick is an explicit UI option.

## Tests

Chromium regressions cover laser on → drive → normal release → laser still on /
wheelszero; joystick diagonal/reverse/spin/deadzone/power normalization; pointer
and keyboard cancellation; simultaneous wheel/arm targets with laser preserved;
vertical slider min/max/target sync; layout switch without acquisition; mobile;
Auto/blur and held-key non-replay; and global Stop clearing laser. Existing client,
native firmware, demo/exhibition, build/metadata and dashboard checks are retained.
See VERIFICATION for actual final test/build/device evidence and limitations.

## Limitations

Touchscreen operation and position targets are software tests, not measured
physical wheel/servo/laser proof. Do not assume the current references are physical
arm positions after reboot. Keep100 ms producer freshness distinct from status
polling; a lost connection must still stop outputs. The old failed/partial soak
stays partial and the monitor stays paused. No endurance claim follows from this
fix. No unattended wheel movement or laser activation is part of idle validation.

## Upgrade

v0.16.0 → v0.17.0 app-only OTA after pinned-core build and identity/idle checks.
Reload dashboard (Ctrl+F5), choose Manual then Joystick view. Stored profiles and
demoV1 remain; boot Manual/no owner/pulses disabled and no automatic resumption.
