# AI Robot Controller Rules

- Read README and 04_Dokumen/API.md before changing control behavior.
- Arduino-ESP32 core 3.3.10; generic ESP32-S3 N16R8. Build with scripts/build.py.
- Maintain thin sketch, separate modules, and a single firmware_version.h version source.
- Only the 10 ms control task initializes/writes actuator GPIO and LEDC.
- Never hold the control spinlock during GPIO, network, flash, or logging work.
- Invalid commands must not refresh the lease. Expired sessions cannot restart motion.
- Keep producer freshness separate from client HTTP heartbeat.
- No physical feedback exists. Never describe commanded output as measured position/speed.
- OTA must acknowledge stopped outputs before flash writes. No automatic rollback promise.
- No credentials, absolute machine paths, binaries, or local profiles in tracked source.
- Gitflow: feature/* from develop; PR to develop; main reserved for reviewed releases.
- Run native control tests, Python client tests, dashboard tests, and pinned firmware build.
- Distinguish compile, simulation, and actual hardware verification in reports.
