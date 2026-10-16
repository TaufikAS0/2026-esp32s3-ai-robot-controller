# Zero-wheel lab soak (v0.9.0)

The soak is a local device experiment, not a certification. Keep the robot in its
accepted bench arrangement. All command bodies pass a mandatory zero-wheel guard
at the final HTTP boundary. Never run `python/example.py` for this experiment.
The API reports references and LEDC readback, not measured movement or position.

## Before launch

Read AGENTS and HANDOFF_CONNECTION_EYES. Verify route/interface, robot MAC and
status identity. Do not guess IPs or run a second job. Check no human/controller
owns a session and no maintenance is active. Smoke-test first and inspect logs.
The runner requires exact MAC, version, UTC build, ESP32-S3, 16 MiB flash and
8 MiB PSRAM. A process-held device-MAC lock prevents duplicate local runners.
Do not use different controllers or other laptops during the experiment.

Use Python 3.11+, standard library only:

```text
python scripts/soak.py --url http://<verified-ip> --mac <verified-mac> --version v0.9.0 --build <manifest-build-utc> --hours 0.05 --scenario-seconds 12 --output build/smoke
python scripts/soak.py --url http://<verified-ip> --mac <verified-mac> --version v0.9.0 --build <manifest-build-utc> --hours 24 --scenario-seconds 300 --seed 20261016 --output build/soak-24h
```

After reboot the pulses are disabled. The runner then refuses to start unless an
explicit `--accepted-pose <logical-left> <logical-right>` is supplied from a
previously accepted pose, preferably with short operator supervision. It must
not be invented from boot's internal 90-degree reference. v0.9.0 seeds the first
complete command's reference directly before enabling pulses; it does not pass
through a fictitious 90-degree pose. Subsequent commands retain speed/acceleration
limits. This cannot establish the physical startup angle or prevent a jump when
the supplied reference differs from the physical position. OTA disables pulses;
no automatic servo reference is stored in NVS.

## Matrix and failure contract

Each cycle includes idle/current character, OLED-off, Reactive, heavy three-tomoe
Sharingan, bounded browser-like concurrent status/catalog requests, servo-only,
laser-only, servo+laser, a 250 ms partial-header slow client, and deliberate
producer expiry. First cycle order is fixed; later cycles use the recorded seed.
Normal scenarios last 300 seconds, bounded by the overall monotonic 24-hour
deadline. The expiry case deliberately ends shortly after checking lease loss.
Scenario durations, interruptions and recovery time are logged separately;
24 hours elapsed does not mean every scenario had uninterrupted control.

Servo speed5 degrees/s, acceleration20 degrees/s2. Gestures stay within three
degrees of the initial accepted pose, moving inward from configured endpoints;
laser-only holds the accepted arm reference. Laser alternates on/off in bounded
two-second windows. Every scenario releases its session (laser off). No aiming,
wheel travel, full servo sweep, router/network changes or unattended OTA occurs.
The partial-header test runs without an actuator lease.

There is one synchronous actuator producer, never a background command heartbeat.
Every fresh target is sent once, at least100 ms apart. Producer gaps/command
acknowledgements reaching300 ms abandon the target. Firmware500 ms expiry is
unchanged. Request errors are logged, never deleted by retries. A failed scenario
releases its own lease, waits5 seconds, then tries at most six read-only recovery
samples with5-second gaps. Only a later distinct scenario may explicitly acquire
a new lease. A reboot, identity mismatch, conflicting owner, nonzero wheel output
or maintenance is fatal. The runner never global-stops a newer human session.
No background heartbeat can keep stale servo/laser targets alive after a crash.

## Durable execution and monitoring

Launch one independent process on the testing laptop, with stdout/stderr redirected
into ignored build artifacts. Windows launches should use `Start-Process` with
`-WindowStyle Hidden`. Keep the computer awake and connected; the runner does not
change Windows power policies. Monitoring is a Codex heartbeat on this continuation
chat every30 minutes, reading checkpoint/process identity and tail events. It must
never spawn a replacement soak automatically. Notify only actionable failure or
completion, then pause/disable the monitor on terminal completion/failure.

Cancellation: create `stop.flag` inside the run output folder or send Ctrl+C.
Do not terminate unrelated Python processes. Cancellation stops production,
attempts scoped release, restores cosmetic/mode settings only if unowned, and
verifies final zero-wheel/laser-off state. If connectivity is gone, report final
state unverified; firmware expiry is the independent fallback. No automatic
restart after reboot/sleep is claimed.

`events.jsonl` records configuration, PID, attempts (including full zero-wheel
bodies), scenario identities, status telemetry and failures. `requests.csv`
includes every request, including timeouts, with monotonic start/latency/error.
`checkpoint.json` is atomically replaced at least every5 seconds during normal
execution; it contains elapsed/planned duration, scenario durations, counts,
latency p95/p99 (1 ms histogram resolution, failures included), separate command
percentiles, worst request/ack gaps and latest full telemetry. Events/CSV can total
hundreds of MB during24 hours; keep them ignored. Status includes RSSI, heap minima,
reset reason, disconnect reason/count, uptime and OLED work budget/overruns.
The runner attempts periodic status reads while commanding; it cannot sample
physical current, supply droop, visible pixels or measured servo position.

Completion means execution reached the monotonic deadline and final idle state
was verified. It is not automatically a PASS. Assess normal loaded-command
p95<150 ms/p99<300 ms, worst gaps vs500 ms, all failed requests/scenarios,
unexpected reboots, disconnect counts and recovery evidence. Separate intentional
producer-expiry/slow-client failures from normal traffic. Inspect CSV by scenario
and endpoint; pooled percentiles alone can hide severe failures. Lab RF results
do not guarantee exhibition RF performance.
