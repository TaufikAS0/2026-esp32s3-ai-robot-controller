# Operations hand ranges and VR sliders — v0.18.2

## Previous

v0.18.1 already enforced device-side hand limits and synchronized joystick slider
min/max. Their editor was hidden on the dashboard; compact landscape tracks were
short for the operator's VR pointer.

## Changes

Open **Operasi → Batas tangan**, enter each hand's minimum/maximum, then press
**Terapkan batas**. Whole degrees0..180 and minimum<maximum are required. The
existing `/arms/policy` contract applies the limits; its dynamic/amplitude/period
settings are preserved from current device status, rather than overwritten by
stale dashboard drafts. Each slider uses its full travel across the selected
range: for60..120°, bottom is60° and top120°. Endpoint labels show device limits.
Reload reads the active device policy; there is no browser-only range bypass.

Opening settings releases only this page's session and clears its wheel/laser
intent. Apply never acquires control or sends movement. Other owners are not
stopped; firmware rejects changes while occupied or when an enabled reference
lies outside the proposed limits. Move to an accepted pose manually first in
that case; there is no automatic centering. Failed apply retains the draft and
keeps actual slider limits at device-confirmed values. Cancel/close never resumes.
The dialog includes explicit global STOP. Limits are the same shared device
policy for HTTP/TCP/demo, not a joystick-only security boundary.

Normal tracks and thumb targets are larger. Check **Panjang ekstra untuk VR**
inside the dialog, then close it: header/power are hidden to allocate more height
to the sliders. **Batas tangan**, **Keluar VR**, and STOP remain reachable. VR is
an expanded browser layout, not a WebXR session or a hardware VR-controller
integration. Exiting VR releases own control before changing layout; no replay.
VR preference is page-local and does not write firmware configuration.

## Unchanged

Existing policy is RAM-only: reboot restores0..180°, dynamicOFF. No NVS key/schema
or persistence added. No new API, actuator/controller logic, pins/partitions,
network/access change. Manual remains direct with1° UI steps; Auto keeps easing.
One fresh100ms producer and500ms firmware expiry, wheel ramp and laser Stop
behavior remain. Old soak stays partial and its monitor paused.

## Tests

Chromium checks60..120° application and slider Home/End endpoints, policy-field
preservation/no movement from settings, invalid draft and backend rejection,
draft retention during polling, own release/laser clear, and extra VR length with
reachable STOP/settings/exit at desktop, portrait and landscape sizes. Existing
native policy guards and all regression/build checks retained. Final outcomes,
build identity and OTA status are in VERIFICATION.md.

## Limitations

UI range/1° reference is not measured physical position or a calibrated mechanical
stop. Browser geometry and pointer tests do not prove operation on the user's
physical VR headset; headset comfort remains an operator observation. Tiny or
high-zoom viewports can scroll to preserve access. No real actuator commands or
example limits are applied during idle validation.

## Upgrade

v0.18.1 → v0.18.2 app-only OTA after pinned-core build and fresh device identity/
idle checks. Ctrl+F5, Operasi → Batas tangan. Configure actual accepted limits
after boot as needed; no settings or movement resume automatically.
