# NEO CLOCK Firmware Feature Matrix

> Audit baseline: `main` at `f9d8570e7e89a5a7b9a6bf2c15861c2a1190c0f4`
>
> Scope: source-level comparison of `firmware/via_wifi`, `firmware/via_chronos`, and `firmware/mochi_chronos`. This document records what is implemented in source; it does not claim that every feature has passed physical-device testing.

## 1. Executive summary

The three targets have different primary jobs and should remain separate firmware builds:

- **via_wifi** is the standalone network clock: Wi-Fi provisioning, saved networks, NTP time, clock UI, and basic speaker feedback.
- **via_chronos** is currently a minimal Chronos BLE clock: phone time sync, six bitmap clock styles, connection status, and three touch inputs. It is the least feature-complete target.
- **mochi_chronos** is the broadest Chronos companion: animated face, Chronos phone data screens, notifications, weather, navigation, music, contacts, QR, alarms, menu settings, and speaker effects.

The recommended direction is **shared product behavior where it makes sense, variant-specific implementations where the connectivity or hardware requires it**. Do not force all three targets to have identical screens or connectivity.

## 2. Feature status matrix

Status definitions:

- **Implemented**: source code for the feature exists in the target.
- **Partial**: some plumbing exists, but the user-facing behavior is missing or incomplete.
- **Not implemented**: no working implementation was found in the audited target source.
- **Variant-only**: deliberately belongs to one connectivity/experience model.
- **Needs device test**: source implementation exists but still needs real-device validation.

| Feature / behavior | via_wifi | via_chronos | mochi_chronos | Notes |
|---|---|---|---|---|
| 128×64 SSD1306 OLED | Implemented | Implemented | Implemented | Shared display hardware. |
| Six NEO bitmap clock styles | Implemented | Implemented | Not the main face | Mochi has its own animated companion face and a text clock screen. |
| Animated companion face / moods | Not implemented | Not implemented | Implemented | Keep as a Mochi identity feature unless a separate optional face mode is deliberately designed. |
| Startup NEO logo | Implemented | Implemented | Boot/status text + face | Different startup presentation. |
| Clock date/time display | Implemented | Implemented | Implemented | Time source differs by variant. |
| 12/24-hour selection | Partial / not clearly exposed in current UI | Partial | Implemented + persisted | Chronos can set the format; via_chronos toggles it but does not persist it. via_wifi currently has no clear user-facing format selector. |
| Wi-Fi station connection | Implemented | Not applicable | Not applicable | via_wifi-only connectivity. |
| Wi-Fi setup portal | Implemented | Not applicable | Not applicable | AP + captive DNS + web form. |
| Save multiple Wi-Fi networks | Implemented | Not applicable | Not applicable | via_wifi stores up to five profiles in NVS. |
| Wi-Fi auto reconnect / NTP | Implemented | Not applicable | Not applicable | Keep out of BLE-only targets unless an explicit dual-source clock design is approved. |
| Chronos BLE advertising/pairing | Not applicable | Implemented | Implemented | BLE library version management needs attention (see conflicts). |
| Chronos phone time sync | Not applicable | Implemented | Implemented | via_chronos handles CF_TIME; Mochi requests sync on connect. |
| BLE connection status UI | Not applicable | Implemented | Implemented | via_chronos shows BT/BLE state; Mochi shows a status dot and Phone screen. |
| Notification callback | Not applicable | Partial | Implemented | via_chronos currently prints notification title/app to Serial only; no OLED history/screen. |
| Notification OLED screen | Not applicable | Not implemented | Implemented | Mochi reads Chronos notification buffer. |
| Weather | Not applicable | Not implemented | Implemented | Mochi displays current city/temp/high/low. |
| Navigation screen | Not applicable | Not implemented | Implemented | Mochi renders icon, direction, distance, duration. |
| Navigation sound cues | Not applicable | Not implemented | Implemented | Mochi classifies turn instructions and plays distinct tones. |
| Music metadata screen | Not applicable | Not implemented | Implemented | Mochi reads title, artist, play/pause state. |
| Music remote controls | Not applicable | Not implemented | Not confirmed in OLED UI | Chronos library exposes musicControl(); do not mark as implemented until UI/actions are wired and tested. |
| Phone battery / charging | Not applicable | Not implemented | Implemented | Mochi requests battery notifications and displays state. |
| Phone model / device info | Not applicable | Not implemented | Implemented | Shown in Mochi Device Info panel. |
| Incoming call/ringer alert | Not applicable | Not implemented | Implemented | Mochi uses the Chronos ringer callback and buzzer. |
| Find Phone | Not applicable | Not implemented | Implemented | Mochi reacts to CF_FIND and plays alert; verify phone-side behavior on device. |
| Contacts / SOS screen | Not applicable | Not implemented | Implemented | Mochi cycles Chronos contacts and marks SOS contact. |
| QR / link relay | Not applicable | Not implemented | Implemented | Mochi renders received links as QR. |
| Alarm alert | Not applicable | Not implemented | Implemented | Mochi polls Chronos alarm state and plays a repeating alert. |
| Quiet / sleep awareness | Not applicable | Not implemented | Implemented | Mochi uses Chronos quiet/sleep state to switch to calm clock behavior. |
| Remote touch reaction | Not applicable | Not implemented | Implemented | Mochi reads Chronos remote-touch state and triggers face reaction. |
| On-device menu | Implemented | Not implemented | Implemented | via_wifi menu is small; Mochi menu is scrollable and broader. |
| Persistent UI settings | Partial | Not implemented | Implemented | via_wifi persists clock style; Mochi persists sound, volume, rotation, time format, idle timeout, mood frequency. |
| Persistent BLE pairing reset action | Not applicable | Not implemented | Implemented | Mochi has Reset BLE menu action. |
| OLED rotation setting | Fixed in source | Fixed in source | Implemented + persisted | Hardware layout is 128×64; only 0°/180° are currently supported in Mochi. |
| Speaker output via MAX98357A | Implemented | Not initialized | Implemented | Different I²S driver implementations; don't copy the driver blindly. |
| Hourly chime | Implemented | Not implemented | Not confirmed | via_wifi beeps on hour change. |
| Configurable volume / sound mode | Not implemented | Not implemented | Implemented | Mochi offers OFF/ALRT/ALL and Low/Med/High. |
| Recorded voice clips | Not implemented | Not implemented | Implemented for speaker playback | Mic capture / voice interaction is still experimental and not completed. |
| INMP441 microphone capture | Not implemented | Not implemented | Experimental / disabled by default | Full-duplex path is behind ENABLE_AUDIO; startListening/stopListening are TODO. |
| Separate passive piezo | Not implemented | Not implemented | Optional, disabled by default | GPIO3; only enable if physically installed. |
| Camera / health metrics | Not implemented | Not implemented | Deliberately not implemented | No camera or health sensors are present on this board. |

### Evidence and interpretation

- `via_wifi`: `src/systems/wifi_system.h`, `input_system.h`, `display_system.h`, `clock_system.h`, and `audio_system.h`.
- `via_chronos`: `via_chronos.ino`, `src/systems/chronos_system.h`, `display_system.h`, and `input_system.h`. Its README explicitly says notification rendering is Serial-only and audio/microphone are not enabled.
- `mochi_chronos`: `src/main.cpp`, `src/chronos_ui.cpp`, `src/menu_engine.cpp`, `src/audio_engine.cpp`, `src/buzzer_engine.cpp`, and `include/dirgamochi_config.h`.

"Not applicable" means the feature is not part of that target's intended connectivity model; it is not a missing implementation to port.

## 3. Conflicts and recommended decisions

### 3.1 Time source

**Conflict:** via_wifi obtains time through Wi-Fi/NTP; the two Chronos targets receive phone time over BLE.

**Decision:** keep time providers variant-specific. Share only display formatting helpers or UI behavior if doing so does not couple the network stack to Chronos. Do not add Wi-Fi/NTP to Chronos targets merely for parity.

### 3.2 Display identity

**Conflict:** via_chronos constructs ChronosESP32 with `CS_0x0_000_CFF` (no display profile) because the physical display is 128×64. Mochi calls `watch.setScreen(CF_ESP32_240x240)`, while its physical OLED is also 128×64 and its remote-touch comments assume a 240×240 coordinate space.

**Decision:** do not silently copy the 240×240 setting into via_chronos. Keep the current via_chronos no-profile behavior until a controlled Chronos-app compatibility test determines whether the Mochi profile is required for any relayed data. Record remote-touch coordinate mapping separately from the physical OLED resolution.

### 3.3 ChronosESP32 library version

**Conflict:** Mochi vendors ChronosESP32 v1.9.1 under `lib/ChronosESP32`; via_chronos installs the Arduino Library Manager package named ChronosESP32 without an explicit version pin in its workflow.

**Decision:** pin via_chronos to a known tested ChronosESP32 version and align it with the vendored version where compatible. Avoid copying code that relies on an API absent from the pinned library. Add a build check for the expected library version/API before merging shared BLE work.

### 3.4 Button semantics

Current mappings are not fully consistent:

- via_chronos: PREV/NEXT change clock style; MODE toggles 12/24-hour format.
- via_wifi: PREV opens Wi-Fi status from the clock and moves menu selection backward; NEXT changes style or moves menu selection forward; MODE short press selects/backs; MODE long press enters/exits the menu.
- mochi_chronos: PREV/NEXT cycle screens; MODE short press opens Phone or cycles Contacts/QR; MODE long press opens the menu; inside the menu PREV/NEXT navigate and MODE short selects, MODE long returns to face.

**Decision:** standardize the physical roles, not every context-specific action:

- PREV = move backward in the current navigable context.
- NEXT = move forward in the current navigable context.
- MODE short = context action / select / confirm.
- MODE long = menu entry or return to the primary face, depending on context.

Preserve essential shortcuts (for example, Mochi Contacts/QR cycling) but make their behavior visible on-screen. Do not let one button silently change clock style in one context while navigating screens in another without a clear UI cue. Define a per-screen control table before implementing changes.

### 3.5 Clock UI versus animated face

**Conflict:** via_wifi and via_chronos are digital clock-first firmware with six NEO bitmap styles; Mochi is an animated companion-first firmware with face moods and a separate text clock screen.

**Decision:** preserve these as distinct product personalities. Common clock data/time formatting can be shared conceptually, but do not replace the six NEO styles with Mochi sprites or force the animated face into the Wi-Fi target.

### 3.6 Audio and microphone

**Conflict:** via_wifi uses a blocking I²S TX beep implementation; Mochi has a non-blocking speaker engine with tone and IMA ADPCM playback, while its optional mic path configures I²S RX+TX. via_chronos currently does not initialize I²S. The ESP32-C3 has one I²S peripheral and the microphone/amplifier share GPIO1/GPIO2 clocks.

**Decision:** use Mochi's event-driven audio behavior as the design reference for future audio work, but port it through a separately reviewed, tested audio module. Do not run the blocking via_wifi beep in the Chronos main loop. Keep microphone capture disabled until full-duplex behavior is verified on the actual board. Keep the optional piezo on GPIO3 separate from the MAX98357A speaker path.

### 3.7 OLED rotation and persistent settings

**Conflict:** via_wifi/via_chronos hardcode 180° rotation; Mochi supports 0°/180° and persists the selection. Preferences namespaces differ (`neo-ui`, `neo-wifi`, `dirga`).

**Decision:** make rotation configurable only where there is a demonstrated hardware need. Keep each firmware's NVS namespace independent; do not reuse keys or migrate values between targets without a versioned migration plan.

### 3.8 Hardware pin constraints

All targets share the same intended default map: OLED SDA21/SCL20; MAX98357A BCLK1/LRC2/DIN5; TTP223 PREV4/NEXT6/MODE7; optional INMP441 SD10; optional Mochi piezo GPIO3.

**Decision:** treat the hardware baseline and GPIO Builder contract as authoritative. GPIO2 is an ESP32-C3 strapping pin despite being used by the official I²S LRC/WS wiring. Never enable or move optional peripherals by changing a single target's pin map without reviewing all targets, Builder validation, boot behavior, and the physical wiring together.

## 4. Recommended product architecture

Do not merge the three firmware folders into one binary. Keep three independent build targets and introduce shared conventions gradually.

| Layer | Shared rule | Variant-specific implementation |
|---|---|---|
| Hardware contract | One reviewed default pin map and GPIO validation contract | Optional devices remain target-gated |
| Input behavior | Same PREV/NEXT/MODE roles and debounce/long-press expectations | Per-screen action map and shortcuts |
| Clock settings | Same user-facing naming and expected behavior | Wi-Fi time source vs Chronos phone time source |
| Display helpers | Consistent text clipping, date/time formatting, status indicators | Six-style bitmap clock vs animated Mochi face |
| BLE data model | Same Chronos callback/API contract and tested library version | OLED screen layout and event presentation |
| Audio behavior | Non-blocking event priority, mute/volume semantics | Hardware output may be MAX98357A, optional piezo, or disabled |
| Persistence | Validate values and keep settings versioned | Separate NVS namespace per target |
| Build/package | Preserve each workflow, package names, metadata, and flash contract | Arduino CLI for via_wifi/via_chronos; PlatformIO for Mochi |

## 5. Proposed implementation roadmap

Priorities are ordered to reduce risk and unlock later work.

### Phase 0 — Stabilize shared contracts

1. Pin and verify the ChronosESP32 library version used by via_chronos.
2. Document the button action table for every screen in all targets.
3. Verify the BLE screen-profile behavior with the Chronos Android app before changing the 128×64 identification.
4. Add explicit build/test coverage for all three targets to the regression process.

### Phase 1 — Settings and navigation parity

1. Add a real screen/menu state model to via_chronos.
2. Add debounced short/long button handling consistent with the agreed physical roles.
3. Add NVS persistence for clock style and 12/24-hour format.
4. Add a compact BLE/device information screen and a user-facing connection/sync state.
5. Improve via_wifi's settings so its existing style/format behavior is discoverable and consistent without disrupting Wi-Fi setup.

### Phase 2 — Chronos information features

Implement in both Chronos targets, with layouts appropriate to each:

1. Notification history screen and readable title/message rendering.
2. Weather summary and forecast navigation.
3. Navigation screen with icon/direction/distance/ETA and robust expiry behavior.
4. Music metadata screen; remote playback controls only after the UI and command behavior are specified.
5. Phone battery/charging and phone information.

### Phase 3 — Chronos interaction features

1. Contacts and SOS contact view.
2. QR/link relay screen.
3. Find Phone action and feedback.
4. Incoming call/ringer handling.
5. Alarm state and user-visible alarm behavior.
6. Quiet/sleep awareness and optional remote-touch reactions.

### Phase 4 — Audio and companion extras

1. Port a non-blocking speaker engine to via_chronos only after a minimal speaker test.
2. Add configurable sound mode and volume where audio hardware is present.
3. Evaluate optional recorded voice playback.
4. Keep mic capture/voice interaction experimental until validated.
5. Preserve Mochi mood animations as an exclusive personality feature unless a deliberate optional mode is approved.

### via_wifi-specific improvements

- Keep Wi-Fi provisioning, multiple saved networks, reconnect, and NTP as Wi-Fi-only features.
- Consider a discoverable settings screen for 12/24-hour format and display rotation.
- Consider reworking the current blocking beep into a non-blocking audio pattern engine; do this separately from the BLE feature port.
- Do not add Chronos phone-data screens to via_wifi unless a dual-connectivity product requirement is explicitly approved.

## 6. Definition of done for each feature

A feature is not marked **Implemented + verified** merely because it compiles.

For each target, record:

1. Source implementation and screen/control entry point.
2. Build result for that target.
3. Chronos app / Wi-Fi behavior used to supply the data.
4. Physical test result on ESP32-C3 + 128×64 OLED.
5. Button interaction, timeout/empty state, and reconnect behavior.
6. Memory/flash impact if the feature adds assets or large buffers.
7. Regression result for the other two firmware targets.

A change to a shared header, asset pipeline, GPIO macro, library version, or package script must trigger the affected workflows for all three targets.

## 7. Audit limitations

- This was a source review of the repository baseline above, not a complete dynamic test of every screen.
- Chronos features depend on the Android app version, permissions, and data relay configuration.
- via_chronos README still lists pairing, time sync, touch polarity/usability, notification display, and audio as requiring physical validation.
- Mochi's optional microphone path is not a finished voice feature: `startListening()` and `stopListening()` remain TODO placeholders.
- CI compilation and artifact packaging do not prove BLE pairing, notification relay, navigation, sound, or physical button behavior.
- The user's reported physical test of the firmware builder was not variant-specific; it is not treated as evidence that all three targets have been tested.

## 8. Source files reviewed

- `firmware/via_wifi/via_wifi.ino`
- `firmware/via_wifi/src/config/config.h`
- `firmware/via_wifi/src/systems/audio_system.h`
- `firmware/via_wifi/src/systems/clock_system.h`
- `firmware/via_wifi/src/systems/display_system.h`
- `firmware/via_wifi/src/systems/input_system.h`
- `firmware/via_wifi/src/systems/wifi_system.h`
- `firmware/via_wifi/PINOUT.md`
- `firmware/via_chronos/via_chronos.ino`
- `firmware/via_chronos/README.md`
- `firmware/via_chronos/src/config/config.h`
- `firmware/via_chronos/src/systems/chronos_system.h`
- `firmware/via_chronos/src/systems/display_system.h`
- `firmware/via_chronos/src/systems/input_system.h`
- `firmware/mochi_chronos/README.md`
- `firmware/mochi_chronos/docs/reference-hardware.md`
- `firmware/mochi_chronos/include/dirgamochi_config.h`
- `firmware/mochi_chronos/include/chronos_ui.h`
- `firmware/mochi_chronos/include/menu_engine.h`
- `firmware/mochi_chronos/src/main.cpp`
- `firmware/mochi_chronos/src/chronos_ui.cpp`
- `firmware/mochi_chronos/src/menu_engine.cpp`
- `firmware/mochi_chronos/src/button_engine.cpp`
- `firmware/mochi_chronos/src/audio_engine.cpp`
- `firmware/mochi_chronos/src/buzzer_engine.cpp`
- `firmware/mochi_chronos/lib/ChronosESP32/library.properties`
- `firmware/mochi_chronos/lib/ChronosESP32/src/ChronosESP32.h`
- The three GitHub Actions firmware build workflows.
- `docs/HARDWARE_BASELINE.md` and `docs/REGRESSION_TEST_CHECKLIST.md`.
