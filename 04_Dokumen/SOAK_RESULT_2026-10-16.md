# Interrupted robot soak — 2026-10-16

**PARTIAL / FAILED. The 24-hour acceptance was not completed.**

Firmware stayed v0.9.0, build UTC2026-10-16T13:10:42Z, MACe0:72:a1:fd:0d:88. No new build or flash was performed for this report.

The only runner (original PID1620) executed2026-10-16T13:17:17Z through14:51:33.570337Z (20:17:17–21:51:33 WIB),5655.719 seconds: **1 h34 min15.719 s**,6.546% of the intended86400 seconds. Its process is no longer alive. Checkpoint state is failed: `Another controller owns the robot`. The runner correctly refused takeover. Last accepted samples had already switched to Manual; the identity/person of that other controller is unknown.

## Counts and timing

-13,609 requests;130 failures, including21 in intentional slow_client scenarios and109 elsewhere. Those109 include release conflicts after expiry; see endpoint classification below. No retries were discarded.
-8,202 command attempts;32 command request failures. All8,202 transmitted candidate bodies had left=right=0. All recorded status samples also had zero commanded wheel outputs.
-86 scenario failures with86 bounded recoveries. Recovery duration p95=8.219 s,p99=14.282 s,max=14.282 s (timed from recovery phase start, excludes prior5-second cooldown).
-Pooled requestp95=64 ms,p99=1016 ms,max3375 ms. Commandp95=48 ms,p99=125 ms. Worst host ACK gap437 ms; this is not a measured firmware-side command acceptance gap or certification of500 ms expiry timing.
-Observed Wi-Fi disconnect count remained0/GOT_IP1;0 uptime decreases in accepted status samples. This does not prove radio/supply health between samples. Last recorded minimum heap162844 bytes.
-Intentional producer-expiry checks succeeded7 times. Slow-client timeouts are induced faults; normal scenario timeouts and command failures remain separate real failures.

## Per-scenario requests (failures included in percentiles)

Durations are accumulated scenario execution time, not guaranteed uninterrupted control time. Setup, recovery and cleanup explain the difference from total elapsed time.

| Scenario | Executed s | Requests | Failures | p95 ms | p99 ms | Worst ms |
|---|---:|---:|---:|---:|---:|---:|
| setup | 0.000 | 2 | 0 | 47 | 47 | 47 |
| idle | 1314.641 | 1297 | 8 | 78 | 1172 | 3234 |
| oled_off | 1387.953 | 1369 | 8 | 110 | 1625 | 2344 |
| reactive | 330.283 | 331 | 12 | 1922 | 2187 | 3375 |
| sharingan | 853.560 | 845 | 9 | 93 | 2015 | 3172 |
| concurrency | 24.561 | 834 | 21 | 109 | 453 | 2297 |
| servo | 513.907 | 4305 | 13 | 47 | 109 | 1016 |
| laser | 98.734 | 882 | 17 | 78 | 406 | 2141 |
| servo_laser | 251.765 | 3626 | 17 | 47 | 125 | 2016 |
| slow_client | 20.297 | 51 | 21 | 2031 | 2032 | 2032 |
| producer_expiry | 7.766 | 67 | 4 | 1000 | 2047 | 2047 |

## Failed endpoint requests

| Endpoint | Error | Count |
|---|---|---:|
| `/status` | timed out | 56 |
| `/command` | timed out | 19 |
| `/control/acquire` | timed out | 2 |
| `/control/release` | HTTP Error 409: Conflict | 11 |
| `/status` | <urlopen error timed out> | 13 |
| `/command` | <urlopen error timed out> | 13 |
| `/control/release` | timed out | 5 |
| `/eyes/catalog` | timed out | 2 |
| `/arms/settings` | timed out | 2 |
| `/control/release` | <urlopen error timed out> | 6 |
| `/eyes` | timed out | 1 |

## Final state and limits

Final idle verification explicitly failed with `Another controller owns the robot`; no final_idle event exists. The last accepted snapshot was unowned/Manual with wheels0/laserfalse, but it predates the conflicting response and cannot certify final outputs. A later read-only probe during the user continuation found the same firmware/MAC, an active Manual session, wheels0 and lasertrue. The monitor issued no stop, mode change or actuator command against that owner. That later laser state must not be attributed to the terminated runner.

The original monitor has been paused after recognizing the terminal state. The user has now explicitly asked to continue, but a new soak must wait until the active Manual controller releases control. A new run must use a new output directory and a fresh identity/owner/accepted-pose preflight; do not resume the old session or overwrite this evidence. A repeated run is not a demonstrated fix for the latency failures.

Facts: normal timeouts occurred even with OLEDs off; the original HTTP responsiveness problem is unresolved. Reasonable inference: changing raster work alone was insufficient. Unknowns: the cause of tail latency, radio packet loss without association loss, supply quality, human-controller interference duration, actual physical wheel/servo response and OLED pixels. No power/RF diagnosis or exhibition readiness is claimed.

Raw local ignored evidence: `build/soak-24h-v090/{checkpoint.json,events.jsonl,requests.csv}`, launch identity and redirected stderr/stdout. This report was generated from those files, with request/command counts and zero-wheel assertions verified. Firmware v0.9.0 and its build UTC are unchanged; no compile/flash or new device soak is claimed by this documentation publication. Read Rules_Versioning and Rules_Change_Notifications before publication.
