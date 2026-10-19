# TCP control v1 — firmware v0.11.0

The existing engine runs on a LAN PC and decides wheel/arm/laser targets. The
robot is a TCP **server on port9000**; it does not run ML or convert camera/world
coordinates into servo angles. HTTP80/81, dashboard, eyes and OTA remain available.
Same approved open-LAN/AP policy, no authentication or new NVS keys.

## Wire contract

One client connection at a time. Additional connections are immediately closed,
without disturbing the existing client. UTF-8 JSON objects separated by LF (`\n`),
maximum1024 bytes before LF (a CR in CRLF counts). No raw NUL, invalid UTF-8,
escaped U+0000, or nesting deeper than16. Frame fragments must finish within250 ms
from their first received byte; incremental progress does not reset the timer.
Oversized/invalid framing, incomplete-frame deadline, socket disconnect and a
reply blocked for250 ms close the connection and release only its own lease.
The service handles fragmented and combined frames with bounded work per tick.

Each request has exactly `v:1`, positive uint32 `id`, string `type`, plus only the
fields listed below. Requests/replies use decimal JSON numbers; booleans are not
integers. Unknown, duplicate, missing and wrongly typed fields reject. Replies:

```json
{"id":1,"ok":true,"data":{}}
{"id":2,"ok":false,"error":"mode_or_control_unavailable"}
```

Invalid envelopes may return `id:0` when no usable ID is available. A framing
error can close the connection without a reply. No unsolicited telemetry pushes.
Request `id` correlates replies; control `sequence` separately prevents replay.

| type | Extra request fields | Meaning / successful data |
|---|---|---|
| hello | none | protocol_version, firmware_version, build_time_utc, timeout_ms, max_frame_bytes, frame_timeout_ms |
| status | none | hello identity plus mode/owner/session/sequence/control_transport/reason, maintenance, commanded_output, arm_motion, PWM health and network telemetry |
| mode | `mode`: `auto` or `manual` | Change explicit mode only while no owner exists; empty data |
| acquire | none | Acquire **program** ownership in Auto; data.session and timeout_ms |
| command | `session`, `sequence`, `left`, `right`, `arm_left`, `arm_right`, `laser` | Atomically accept target; data.sequence; does not prove GPIO application or physical arrival |
| release | `session` | Release this socket's matching lease; empty data |
| stop | none | Explicit **global stop**, even of another owner, as with HTTP/stop; empty data |

Example exchange (each line ends in LF, sample session is replaced by acquire):

```json
{"v":1,"id":1,"type":"hello"}
{"v":1,"id":2,"type":"mode","mode":"auto"}
{"v":1,"id":3,"type":"acquire"}
{"v":1,"id":4,"type":"command","session":123,"sequence":1,"left":0,"right":0,"arm_left":42,"arm_right":79,"laser":false}
{"v":1,"id":5,"type":"status"}
{"v":1,"id":6,"type":"release","session":123}
```

**Example angles are not an approved physical starting pose.** First command
after disabled servo pulses directly seeds its supplied reference. Engine/setup
must know an accepted pose before enabling pulses. No encoder/homing exists.

## Control and freshness

Logical wheels are finite −1..1. Logical arm angles are finite0..180 and must also
satisfy configured per-arm bounds (left mirroring stays in the driver). Laser is
boolean. Session/sequence are positive uint32; sequence strictly increases and
must not wrap. All accepted complete commands enable both servo pulses.

Engine sends a fresh complete target every100 ms and waits for ACK. **One request
in flight is required for control**: a command with already-queued following
bytes rejects `command_pipeline_not_allowed` and closes after its error reply.
Combined discovery/status frames are supported, but actuator targets must not be
batched or buffered for later replay. No automatic retries/reconnect/resumption.

Only valid commands refresh the existing500 ms device lease. `hello`, `status`,
mode requests, socket liveness, bad targets and old sequences do not. Expiry,
release and disconnect stop wheels, turn laser off and freeze the last servo
reference, cancelling remaining travel. OTA invalidates sessions and disables
pulses. Reconnecting requires explicit acquire and a fresh intended target.

TCP leases are bound to one socket generation. Even knowledge of the public
session ID cannot command/release that lease through another socket or HTTP.
HTTP leases remain usable through HTTP. Dashboard explicit mode changes and
global stop retain their existing authority. Closing an old TCP socket must not
stop a newer dashboard/HTTP lease. This is arbitration, not authentication.

Network transit time is not measured target age; a sequence alone cannot prove
fresh inference. Raw clients must bound their producer target age, avoid an
outbound replay queue, and stop producing when the engine result is stale. The
Python SDK requires a PC `time.monotonic()` producer timestamp and rejects age
>=300 ms or a future timestamp; reconnect/reacquire is explicit after failure.
Do not stamp an old inference with a new timestamp to keep it alive. If an engine
chooses to maintain a target between inferences, that choice needs its own bounded
validity policy; a background socket heartbeat is insufficient.

Common errors: invalid_envelope, invalid_fields, invalid_json, unknown_type,
invalid_mode, release_control_before_mode, mode_or_control_unavailable,
invalid_command, arm_limits_or_range, session_or_sequence_rejected,
invalid_session, session_rejected, command_pipeline_not_allowed, task_unresponsive.

## Backend Python integration

No third-party runtime dependency. Read-only discovery (no acquire/actuator output):

```text
python 04_Dokumen/python/tcp_example.py <robot-ip>
```

Import `RobotTcpClient` from the same directory. Engine integration pattern:

```python
import time
from robot_tcp_client import RobotTcpClient

with RobotTcpClient(robot_ip) as robot:
    robot.set_mode("auto")  # Fails rather than displacing an active owner.
    robot.acquire()
    try:
        while engine_is_running():
            target, produced_at = engine_next_target()  # Fresh complete intended target.
            robot.command(left=target.left, right=target.right,
                          arm_left=target.arm_left, arm_right=target.arm_right,
                          laser=target.laser, produced_at=produced_at)
            # Engine schedules the next update at100 ms; never run a stale replay thread.
    finally:
        robot.release()
```

`produced_at` is captured when the engine produces/authorizes that target, using
the same process's monotonic clock, not when the socket sends it. SDK I/O deadline
defaults200 ms (max250 ms), target TTL300 ms (may be shortened). A command error
closes the socket; there is no default servo pose, nonzero wheel command, laser-on
example or background producer. Arm settings/policy/eyes remain on HTTP81/80.

HTTP status advertises `tcp_control:{ready,port:9000,protocol_version:1}` and
`control_transport:none/http/tcp`. Task TCP core1/priority2 remains below the
10 ms actuator task priority3. Only the actuator task writes GPIO/LEDC.

## Verification boundary

Host protocol/socket tests use the same parser, dispatcher, peer loop and pure
control logic as firmware; simulated nonzero wheels are never hardware proof.
Read latest VERIFICATION for actual build/OTA/device evidence. User confirmed
the physical servo starting pose is unknown, so this delivery's device test must
not enable servo/laser or send any command. Firmware timeout/laser/trajectory
behavior is therefore verified in simulation; live tests cover TCP/session/control
arbitration with no actuator targets. Old24-hour soak remains partial/paused.
