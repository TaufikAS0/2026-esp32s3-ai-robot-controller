# Dual OLED eyes — v0.9.0

v0.9.0: Fox/animal highlights are mirrored and intersected with the intended
ellipse/lid silhouette, never painted outside it. Native and browser regressions
compare both Fox eyes across100 phases and six Sharingan variants across4 phases.
52 presets,22 box/30 special;13 selectors including All. Separate Sharingan:
1/2/3 tomoe and Mangekyo pinwheel/flower/star, with original `sharingan` retained.
Native raster precomputes tomoe geometry, bounds work and avoids unused angle
evaluation. EyeService reports last/max work microseconds and60 ms overruns;
overdue work is dropped instead of catching up. I2C still has one owner off the
actuator task. Earlier version sections below are historical design context.

Confirmed by user 2026-10-16: SSD1306, SDA GPIO8, SCL GPIO9, two 128x64 modules. Physical right: 7-bit0x3C (8-bit write0x78); physical left:7-bit0x3D (write0x7A). Both displays share the same bus. The module address must be selected in hardware; the software swap only exchanges images.

## Reactions

| Situation from commanded output | Eyes |
|---|---|
| Forward +/+ | Focused upper lids, centered gaze |
| Reverse -/- | Wide surprised eyes, slightly up |
| Turn left/right | Gaze toward the commanded turn; asymmetric curious lids |
| Hands at low logical angles | Shy rounded-box eyes |
| Hands at high logical angles | Happy crescents and cheek marks |
| Unequal hand angles | Curious gaze biased toward the hand pose |
| Laser on, wheels idle | Focus |
| Idle | Gentle breathing, glances, changing happy/curious expressions, periodic wink |
| Idle over time | Rotate selected category; no permanent passive sleep mode |
| OTA | Pause bus animation; keep actuator maintenance independent |

Reactive/Playful are restricted to the box character. Priority: OTA, selected manual expression (with optional blink), or automatic motor reaction > laser > hand pose > idle. These are expressive rules, not actual emotion, sensed motion or autonomous navigation. Arms are commanded logical angles, not measured movement. Mechanical forward direction must be checked after flashing.

## Eye Studio

Bundled dark/cyan card follows the existing dashboard. Two 128x64 pixel previews show eye identity and physical I2C transfer health. The mode selector switches only expression policy, independently of motor Manual/Auto. 41 categorized preset buttons select a single expression, with brightness1..255, tempo0.5..2x, natural blink, screen on/off and swap. Draft edits survive status polls; Apply sends one complete validated setting object; rejected requests leave drafts available. Offline and unsupported firmware disable Apply. Local preview remains usable offline, clearly labeled.

Settings are RAM-only and return to defaults at reboot, including swap. This version adds no NVS keys or flash writes for eyes. Python set_eyes and POST /api/v1/eyes are cosmetic; neither updates motor lease or producer freshness. Full control sessions/timeout remain unchanged.

Preview uses device pose snapshots at1 Hz; unsaved edits animate locally; physical animation targets a60 ms period (~16.7 updates/s maximum, dependent on transfer time). It is intentionally not frame-synchronized and cannot prove OLED pixels. ACK status means last successful command/frame transfer; unplugged panels are detected on the next attempted transaction, with5 s retry. A dark/incorrectly wired display may ACK without visible pixels.

## Ownership and failure behavior

- EyeAnimator and EyeRaster are portable shape/bitmap logic; native tests cover reactions, blink, setting ranges, raster clipping and rollover.
- EyeService owns Wire on its dedicated priority1/core0 task. Actuators remain priority3/core1/10 ms and never wait for OLED rendering.
- 400 kHz shared bus, 10 ms transaction timeout,32-byte payload chunks with early abort. Missing OLEDs are nonfatal and separately reported.
- No new firmware dependency beyond core Wire. Version/build UTC is shown on both OLEDs for the first4 seconds from boot, then animations start. A late reconnect after4 s enters animation directly.
- Hardware validation pending: address scan/ACK, clear version/time splash, physical left/right swap, actual pixels/brightness, disconnected/stuck bus during motor timeout, combined Wi-Fi/OTA load and direction observation.

## Rule audit

Read before implementation: Firmware_AI_Vault_Home.md, Rules_Nama_dan_Folder_Workspace.md, Rules_Firmware_Standar.md, Rules_GitHub_Gitflow.md, Rules_Kredensial_WiFi.md, WiFi_HuaweiJIN.md, Rules_Akses_Robot_Lab.md, Rules_Versioning.md and Rules_Change_Notifications.md. Lab STA stays HuaweiJIN; recovery AP password12345678. Existing boot migration updates differing lab STA/AP keys and removes only obsolete apiToken/otaPassword, preserving unrelated NVS. This change does not alter that migration.

Native driver-boundary tests execute the actual EyeService against fake Wire/RTOS: complete framebuffer transfer, swapping images, 5-second retry (not retry every frame), immediate abort on data failure, bus initialization failure and no I2C activity in maintenance. They prove branch/protocol behavior, not on-device scheduling or electrical bus behavior.

## Animation policies and selection

- Reactive: wheel/laser/arm reactions take priority; idle rotates the selected category every1.8 s at tempo1.
- Playful: same motion priority, idle changes every0.9 s in a deterministic shuffled order.
- Showcase: cycles the chosen category every2.2 s even during motion. Cosmetic only; never drives the robot.
- Manual: holds the selected preset; its own animation and optional blink continue.

Energy1/2/3 is Tenang/Aktif/Liar, controlling idle gaze amplitude, excited/panic bounce and chaos/glitch displacement. Tempo0.5..2 affects animation rate. Category filters both the chooser and automatic idle/showcase rotation; motion-priority reactions may use another category. Selecting a preset switches to Manual. Apply sends the complete draft; category changes alone do not affect the device until Apply. Swap exchanges logical images, not hardware addresses. Blink off also disables wink routines.

Digital Loading and Error are decorative, not device fault/progress telemetry. Googly means comic moving pupils, not Google service integration. All physical output is monochrome.

## Catalog

- Ceria: Senang (`happy`), Tertawa (`laugh`), Antusias (`excited`), Cekikikan (`giggle`).
- Sayang: Sayang (`love`), Kedip sayang (`love_wink`), Malu-malu (`shy`), Berbinar (`sparkle`).
- Penasaran: Netral (`neutral`), Penasaran (`curious`), Ragu (`skeptical`), Melirik diam-diam (`side_eye`), Berpikir (`thinking`).
- Fokus: Fokus (`angry`), Bertekad (`determined`), Usil (`sneaky`).
- Terkejut: Terkejut (`surprised`), Kaget besar (`shocked`), Panik lucu (`panic`).
- Santai: Mengantuk (`sleepy`), Terlelap (`dozing`), Tenang (`zen`), Bosan (`bored`).
- Sedih: Sedih (`sad`), Menangis (`cry`), Memohon (`pleading`).
- Kedip: Kedip kiri (`wink_left`), Kedip kanan (`wink_right`), Kedip ganda (`double_blink`), Kedip bergantian (`alternating`).
- Liar: Googly (`googly`), Pusing lucu (`dizzy`), Konyol (`derp`), Kacau ceria (`chaos`), Hipnotis (`spiral`), Pesta (`party`).
- Digital: Mata bintang (`stars`), Mata dolar (`dollars`), Gaya loading (`loading`), Glitch (`glitch`), Gaya error (`robot_error`).

## OLED check mode

POST /eyes/check queues scan/reinit/white/checker/identity/animate for both/left/right. Latest request replaces the active one. Scan probes all nonreserved7-bit addresses8..119, one per60 ms eye iteration, at100 kHz (~6.7 s); normal400 kHz is restored afterward. Scan always covers the bus, regardless of target. ACK identifies a responding address, not a controller type or physical display count. Two displays at one address cannot be distinguished; address jumper selection must be done on the modules. Unknown addresses are reported and never automatically treated as SSD1306 devices.

White/checker/identity force only selected panels enabled at full contrast for5 seconds, then restore settings. Identity labels physical address L3D/R3C and includes version/build UTC, independent of swap. Reinit/animate restore settings immediately. During maintenance the task performs no I2C; a pending diagnostic starts after maintenance ends. Failed transfers retain5 s retry. Probe code0=ACK,2=address NACK,5=timeout,255=not scanned; other raw codes may also occur. A queued202 response does not confirm visible pixels.

## Family coherence and mounting v0.7.0

Authoritative shape classification: Solid/Happy/Focus/Sad are box-family; all other shapes are special. This yields22 box presets and19 special presets. User explicitly requires the square-ish character to stay consistent in automatic reactive mode. Reactive/Playful idle selection filters by family; forward/reverse/turn/laser/arm reactions are also box-family. Category filters still apply, with Happy fallback when a category has no eligible box entry. Showcase intentionally includes both families; Manual can select any preset. UI family tabs are browsing filters and never change actuator state. Original catalog IDs are retained.

Continuous local main preview and real pair thumbnails replace the earlier repeated text icons. Thumbnails show a sampled frame; hover/focus animates that preset. Motion notes distinguish timing-only presets such as left/right/double/alternating blink. Laughter pulses, giggle is asymmetric, skeptical raises one lid, thinking narrows one eye, and bored lowers unequal lids. These remain expressions of commanded behavior, not sensed emotions. Static screenshots cannot show temporal differences.

Physical left0x3D and right0x3C each have a180-degree rotation flag, independent of swap. Both boot defaults are enabled because both OLEDs are mounted upside down. SSD1306 segment/COM orientation changes at initialization or upon settings update; splash, animation and diagnostics all share it. Normal commands A1/C8; rotated A0/C0. Preview rotates panel data; the physically inverted module then appears upright. Settings remain volatile; a reboot restores both mounting defaults. Existing seven/ten-field clients preserve orientation. No new NVS keys or flash writes.

## v0.8.0 Hewan & shinobi

Six additional special-family presets: Sharingan (central pupil, three rotating tomoe), Rinnegan (concentric rings/pulse), Cat (breathing vertical slit), Fox/Kurama (slit under angled lids), Snake (thin slit and lower lid), Toad (horizontal pupil). Monochrome stylized rendering; no color claim. Category shinobi separates these from the main character. All47 IDs remain selectable in Manual/Showcase; automatic Reactive/Playful never picks special-family eyes. Totals22 box,25 special,12 category selectors including All.
