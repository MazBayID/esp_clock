# NEO CLOCK Firmware Stability Audit

Audit snapshot: 2026-11-20

## Audit boundaries

- Production baseline: `main` at `edbdda1de332347ef5774711e035ae63deee0780`.
- Development candidate: open PR #13, head `9ab5ba289be2a38588c0212be3c77a5d8a573dcc`.
- This is a static source and CI review. No ESP32-C3 hardware was connected or tested during this audit.
- No firmware source, build workflow, GPIO map, or production branch was changed.

## Executive summary

The three targets have distinct strengths and maturity profiles. `via_wifi` has the most developed network provisioning path; `via_chronos` production `main` is still a small BLE clock milestone; PR #13 adds substantial Chronos UI/data coverage but remains unmerged; `mochi_chronos` has the broadest feature surface but also the most complex state, audio, and UI interactions. Feature count must not be confused with stability.

The first stabilization work should focus on input behavior, navigation state, persistence, connection/time state, and safe handling of empty/stale data. UI icon work should wait until screen/navigation behavior settles.

## Findings by target

### 1. via_wifi

**Observed implementation**
- PREV/NEXT/MODE touch inputs on GPIO4/6/7.
- MODE has 35 ms debounce and a 700 ms long-press threshold.
- PREV/NEXT use rising-edge checks against prior raw readings; no equivalent debounce is applied to those two buttons.
- Clock style is persisted in Preferences namespace `neo-ui`.
- Wi-Fi stores up to five network profiles in Preferences namespace `neo-wifi`, scans for visible saved networks, tries newer profiles first, and starts a setup AP if no saved/bootstrap network connects.
- Time synchronization uses regional NTP selection, an IP-based timezone lookup, and global NTP fallback.
- OLED initialization failure enters an infinite retry-free delay loop.

**Stability and security risks**
1. **Input asymmetry — medium/high:** PREV/NEXT are not debounced while MODE is. Touch bounce or noisy transitions can cause repeated or missed navigation. Validate actual TTP223 output polarity and pulse behavior before changing the implementation.
2. **Blocking startup/network work — medium:** saved-network scanning, sequential connection attempts, IP geolocation, and NTP waits are synchronous. A poor network can delay startup and make the device appear unresponsive.
3. **Open provisioning AP — high security concern:** the `NEO_CLOCK` setup hotspot is created without a password and submits Wi-Fi credentials over HTTP. Anyone able to join the AP may be able to observe submitted credentials. Review an authenticated/protected provisioning design before treating the portal as production-secure.
4. **Credential storage — medium security concern:** saved Wi-Fi passwords are held in Preferences/NVS. No NVS encryption configuration was found in the inspected firmware/workflow. Document the threat model and consider encryption where supported by the device provisioning model.
5. **OLED failure handling — medium:** unlike Mochi, the target halts forever if OLED initialization fails. Decide whether headless operation, a retry strategy, or an explicit fatal state is desired.
6. **Audio driver checks — medium:** I2S install/pin configuration return values are not checked in `setupI2S()`. A failed peripheral setup is not surfaced to the rest of the firmware.

**Recommended first tests**
- Repeated rapid PREV/NEXT/MODE presses, slow presses, held-at-boot, and noisy/repeated touches.
- Saved network success, unavailable SSID, incorrect password, open network, full profile store, and persistence across power loss.
- Portal connection failure, successful connection, failed NVS write, client disconnect, and restart.
- Wi-Fi loss/reconnect and NTP failure/recovery; confirm the clock's displayed time and timezone remain valid.
- OLED absent and I2S setup failure behavior.

### 2. via_chronos production main

**Observed implementation**
- Chronos BLE time and connection callbacks.
- Six digit styles, date, and BLE status indicator.
- MODE changes 12/24-hour setting in the older input code, but setup initializes Chronos to 24-hour mode.
- Input uses rising-edge detection with a 180 ms global lockout; no independent debounce state or long-press/menu behavior exists in production `main`.
- Clock style and time format are not persisted in production `main`.
- Notification callback only logs notification metadata; no notification screen/data browser exists in production `main`.
- OLED initialization failure halts in an infinite loop.

**Stability and alignment risks**
1. **Input behavior is less robust than the other targets — high priority:** the global 180 ms lockout is not equivalent to independent per-button debounce and does not support the long-press menu interaction used by the other targets.
2. **No persisted settings — medium:** clock style resets to its default; time format is initialized at boot rather than restored from NVS.
3. **Time-sync state semantics — medium:** `chronosTimeSynced` is set after a successful sync but is not cleared on disconnect in the inspected code. Decide whether this flag means “ever synced” or “currently synchronized”; the UI should not imply a live connection if it only records historical success.
4. **Unused state — low:** `chronosTimeChanged` is set on sync but no consumer was found in the inspected target.
5. **OLED failure handling — medium:** startup halts if the display is not detected.

### 3. via_chronos PR #13 candidate

**Observed implementation**
- Adds an eight-item menu: Clock Style, Time Format, BLE Status, Notifications, Weather, Music Control, Navigation, and Phone Information.
- Adds independent 35 ms debounce handling for PREV/NEXT/MODE and a 700 ms MODE long-press.
- Persists clock style and 12/24-hour mode in Preferences namespace `neo-ble-ui`.
- Adds browsing indices and guards for empty notification/weather lists.
- Adds music previous/next/play-pause controls.
- Adds text/data screens for navigation and phone information.

**Validation status**
- PR #13 remains open and is not included in production `main`.
- CI for the PR head completed successfully for the Chronos build and the Wi-Fi regression build.
- No physical-device validation has been performed.

**Review points before merge**
1. Verify button polarity, debounce feel, short/long MODE boundaries, and menu selection on the actual TTP223 hardware.
2. Confirm 12/24-hour preference restoration and Chronos app-side format updates survive reboot and reconnect.
3. Confirm notification/weather index behavior when the underlying list changes or becomes empty while the screen is open.
4. Verify music controls against supported phone/player combinations.
5. Verify navigation text, phone metadata, and OLED clipping with real Chronos payloads, including long and non-ASCII strings.
6. Decide whether time-sync state should remain latched after BLE disconnect or be represented as “last sync” separately from “connected”.
7. Test return-to-menu and long-press-to-home behavior from every screen.

### 4. mochi_chronos

**Observed implementation**
- Chronos BLE companion with animated face states, clock, weather, notifications, navigation, music metadata, phone information, contacts/SOS, QR/link screen, alarms, and incoming-call handling.
- Settings menu persists sound mode, volume, OLED rotation, time format, idle timeout, and mood frequency; includes BLE reset and device info.
- Three touch inputs use per-button debounce and a 700 ms long-press threshold.
- Default audio path initializes I2S speaker TX for beeps. Full microphone/speaker path is behind `ENABLE_AUDIO=0` by default.
- OLED failure is handled more gracefully than the other targets: BLE/audio can continue without the display.

**Stability and alignment risks**
1. **More state and interaction paths — high test priority:** automatic notification/navigation/connection screen overrides can interact with manual screen navigation and menu state. Test priority and return behavior rather than assuming the richer UI is stable.
2. **Finding screen path appears incomplete — medium:** `findingPhoneUntil` is read by the display override logic but no assignment was found in `main.cpp`. The `CF_FIND` callback plays a sound and triggers a mood, but does not set that timer. The Finding Phone screen therefore appears unreachable through the inspected path.
3. **Unused navigation timer — low:** `navigationUntil` is assigned/reset but no read was found. Navigation visibility currently appears to be driven by `watch.getNavigation().active`.
4. **README/control drift — medium:** the Mochi README describes older long-press actions (including Find Phone and MINUS-long behavior), while current `main.cpp` routes MODE/OK long press to the menu and PREV/NEXT short press to screen navigation. Documentation should be corrected after controls are physically reconfirmed.
5. **Audio initialization checks — medium:** `AudioEngine::begin()` sets speaker-ready state after attempting I2S driver installation without checking the return codes. Failure should be recorded and surfaced rather than treated as success.
6. **Optional audio risk — high when enabled:** the full-duplex mic/speaker path is experimental and disabled by default. Do not enable it in production without microphone capture, speaker output, simultaneous operation, and reset/boot testing on the actual wiring.
7. **QR scope — product decision:** QR/link generation and the `ricmoo/QRCode` dependency remain in the code. The user has said this feature is not useful; removal is a separate scoped change and has not been made.
8. **Music feature parity — medium:** Mochi displays music metadata/state, but current button routing does not provide the same explicit previous/next/play-pause controls implemented in via_chronos PR #13.
9. **Data presentation differs — medium:** Mochi currently displays weather item 0 and notification item 0, while PR #13 supports browsing lists. Decide whether this is intentional for a glanceable companion UI or a parity gap.
10. **Reset-pairing behavior — test required:** menu invokes Chronos stop/restart. Confirm reconnection, advertising, and phone-side stale pairing recovery on device.

**Recommended first tests**
- Rapid/held/overlapping button presses, including long press on every screen and while a notification/navigation override is active.
- BLE connect/disconnect/reconnect, time format restore, notification bursts, calls, alarms, weather, music metadata, navigation updates, contact list changes, and QR reception.
- Screen override priority and return behavior after transient events.
- Speaker beeps at all volume levels, stop/preemption behavior, and repeated event overlap.
- OLED absent and audio peripheral initialization failure.
- Separate experimental full-duplex test only after the standard TX-only firmware is stable.

## Cross-variant issues to resolve

1. **Button semantics:** align PREV/NEXT/MODE meaning and menu entry/exit behavior. Keep hardware pinout fixed.
2. **Debounce strategy:** prefer independent per-button debounce with explicit press/release/long-press state. Do not copy thresholds blindly; validate with actual TTP223 modules.
3. **Screen navigation:** define consistent back/home behavior and event-driven screen priority. Avoid an automatic screen override trapping the user in a screen.
4. **Time status:** distinguish BLE/Wi-Fi connection from valid current time and from last successful synchronization.
5. **NVS behavior:** validate defaults, invalid stored values, write failures, and restoration after power loss.
6. **Empty/stale data:** all screens should handle no data, changing counts, long text, disconnect, and stale payloads without out-of-range access or misleading values.
7. **Hardware initialization:** surface OLED/I2S failures consistently and avoid falsely reporting a subsystem as ready.
8. **Documentation:** synchronize controls, feature claims, pin maps, build steps, and release status with actual code.

## Risk order

| Priority | Area | Reason |
|---|---|---|
| P0 | Physical input/polarity and shared hardware map | Wrong assumptions can make every UI test misleading |
| P1 | Input debounce and navigation state | Directly affects all user interactions |
| P1 | Wi-Fi provisioning security | Credentials are entered through an open HTTP setup AP |
| P1 | BLE/time state and reconnect behavior | Core clock correctness |
| P1 | OLED/I2S initialization failure handling | Avoid false success or dead startup |
| P2 | Settings persistence and invalid NVS values | Prevent settings loss/corruption |
| P2 | Chronos data screens and empty/stale data | Avoid confusing or invalid UI |
| P3 | UI icons, animations, QR cleanup, cosmetic polish | Defer until screen structure and behavior settle |

## Explicit non-goals of this audit

- No firmware feature was removed or added.
- No PR was merged.
- No pin mapping was changed.
- No PlatformIO migration was started.
- No claim of physical stability or production readiness is made.
