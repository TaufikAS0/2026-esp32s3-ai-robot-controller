# Layar Operasi — v0.18.0

## Previous

v0.17.0 placed the screen joystick and flanking arm sliders inside the
connection/control card, requiring scrolling between drive, hands and laser.

## Changes

Choose **Operasi · layar kendali** in the navigation, or open `/#operation-panel`.
The dedicated viewport replaces the dashboard while open. Joystick is on the
left; both vertical hand sliders and **Laser / LED kanan** switch are on the
right. This switch operates the existing right-hand laser output; there is no
additional independent LED output or GPIO mapping. Higher logical angle is at
the top. Targets/limits are shared with existing Manual controls.

Manual mode, motor-power limit, target angles, requested laser state, connection
state and **STOP SEMUA** fit without scrolling on tested desktop, portrait phone
and landscape phone sizes. The header also shows connection/build identity;
compact landscape hides that long identity line, available on dashboard.
**Dashboard** returns to settings.
This is a browser viewport layout, without requiring browser Fullscreen API,
landscape lock, installation or an external gamepad.

Opening, reloading, changing power, or navigating to this screen does not acquire
control, select Manual, or move the robot. Auto requires an explicit **Pilih
Manual** action. Actual joystick, arm or laser input uses the existing Manual
controller and ownership checks. No actuator state is restored from storage.
The old Manual view selector opens this same screen; controls are not duplicated
into a second producer. Direction buttons and advanced settings stay on dashboard.

Entering or leaving the Operations screen releases this browser's session and
clears wheel/laser targets. Navigation during an asynchronous release cannot
reopen an obsolete screen or accept control during the transition. STOP SEMUA
also sends the existing global Stop. Normal joystick release retains a selected
laser within the same fresh Manual session, as in v0.17.0. Pointer cancellation,
unexpected capture loss, blur, hidden/pagehide, errors, session loss and 500 ms
firmware expiry retain their stopping behavior. No automatic reacquire/replay.

## Unchanged

HTTP/TCP/API schemas, actuator/control tasks, GPIO/polarity/servo mapping,
partitions, NVS, Wi-Fi saved-profile selection, open LAN policy, and Auto/demo/
exhibition behavior. Power default remains30%; no silent increase to80%.
Old soak remains partial and its monitor paused.

## Tests

Actual embedded HTML runs in Chromium against mocked robot HTTP. Regression
coverage includes direct navigation/reload without acquisition, joystick left /
both arms right, no-scroll desktop/mobile/landscape layouts, shared power/limits,
explicit Manual selection, busy-network guards, laser synchronization, global
Stop and leaving while driving. Existing pointer/key/lease/timeout cases remain.
Native/Python/build outcomes and device upload status are in VERIFICATION.md.

## Limitations

Angles are target/reference values, not measured physical hand positions. The
laser switch indicates this browser's request, not optical feedback. CSS/browser
tests are not physical wheel/servo/laser validation. Very small displays, browser
text zoom and accessibility scaling may require scrolling so controls remain
reachable; no forced zoom or orientation restriction is imposed.

## Upgrade

v0.17.0 → v0.18.0 requires a pinned-core build and later app-only OTA after fresh
device identity/idle verification. This task publishes source/build to GitHub;
it does not upload or physically move the robot. After deployment reload with
Ctrl+F5 and choose **Operasi · layar kendali**. Boot remains Manual/unowned,
servo pulses disabled, no automatic movement or laser resumption.
