# NEO CLOCK Firmware Stability Audit

Audit baseline: production `main` at `1bcf8f2212ee4e0a5d1aa8e597ca70d44137aa1b` (2026-11-20).

## Audit boundaries

This is a source and CI review of the three firmware variants and the recent merged PRs. CI success is not physical-device validation. The user has confirmed hardware testing for earlier via_chronos/Mochi behavior, including the merged navigation and Find Phone changes; the two held-at-boot fixes remain pending dedicated tests.

## Executive summary

The three targets have distinct roles and should remain separate firmware builds. The priority remains stability, not feature-count parity: input reliability, screen navigation, connection/time state, persistence, empty/stale data handling, and hardware initialization. UI/icon work should wait until screen structure and navigation settle.

## via_wifi

**Strengths**
- Wi-Fi provisioning, multiple saved network profiles, reconnect behavior, and NTP time source.
- Clock style persistence.
- PREV/NEXT debounce was added in merged PR #15.

**Risks to address**
1. The setup AP is open and submits Wi-Fi credentials over HTTP. Do not treat provisioning as production-secure without a protected design.
2. Saved Wi-Fi credentials are stored in Preferences/NVS; review the device threat model and encryption options.
3. Startup/network discovery, connection attempts, geolocation, and time synchronization include blocking paths that can delay responsiveness.
4. OLED initialization failure and I2S setup errors need clearer failure handling.
5. Test Wi-Fi loss/reconnect, NTP failure/recovery, incorrect credentials, unavailable SSIDs, profile capacity, and persistence after power loss.

## via_chronos

**Current production features**
- Chronos BLE time and connection state, six clock styles, date, and BLE status.
- The merged PR #13 adds an eight-item menu, 35 ms per-button debounce, 700 ms MODE long-press, persisted clock style/time format, notification/weather browsing, music controls, navigation data, and phone information.
- ChronosESP32 is pinned to v1.9.1 by merged PR #12.
- PR #19 adds held-at-boot input initialization but is not merged pending physical validation.

**Risks to address**
1. Confirm the distinction between currently connected, valid time, and last successful time sync; avoid presenting stale sync state as live status.
2. Test menu entry/exit, all screens, short/long MODE boundaries, music controls, empty/shrinking data lists, long/non-ASCII text, disconnect/reconnect, and settings restore on device.
3. OLED initialization failure currently needs a deliberate recovery/failure policy.
4. Physical validation of PR #19 must boot the device while PREV, NEXT, and MODE are each held, then release and press again; no action should occur until a fresh press.

## mochi_chronos

**Current production features**
- Chronos companion with animated face, notifications, weather, navigation, music metadata, phone/device information, contacts/SOS, QR/link relay, alarms, and incoming-call handling.
- Settings include sound mode/volume, rotation, time format, idle timeout, and mood frequency.
- Merged PR #20 allows manual screen browsing during an active navigation session while incoming navigation events can still surface the navigation screen.
- Merged PR #21 adds a Find Phone menu action with explicit offline feedback; the user confirmed phone ringing and offline feedback on hardware.
- Merged PR #17 protects contact selection when the contact list shrinks or becomes empty.
- Merged PR #18 validates QR link indices/counts against the fixed-size Chronos QR array.
- PR #16 adds held-at-boot button initialization but remains unmerged pending physical validation.

**Risks to address**
1. Mochi has the broadest state and interaction surface. Test screen override priority, notification bursts, navigation updates, manual browsing, return behavior, reconnect, and overlapping events.
2. Verify I2S driver installation/pin configuration return values are handled honestly; do not report audio ready after failed initialization.
3. Keep microphone/full-duplex audio experimental and disabled until capture, playback, simultaneous operation, and reset/boot behavior are tested on the actual wiring.
4. Test contact and QR list changes, empty data, malformed payloads, and disconnect behavior.
5. Physical validation of PR #16 must boot while each touch input is held, release it, and confirm only a fresh press triggers an action.

## Cross-variant priorities

| Priority | Area | Why |
|---|---|---|
| P0 | Physical input polarity and hardware pin map | Incorrect assumptions invalidate interaction tests |
| P1 | Debounce, long-press, and navigation | Core user interaction across all targets |
| P1 | Wi-Fi provisioning security | Protect user credentials |
| P1 | Time validity and reconnect state | Core clock correctness |
| P1 | OLED/I2S initialization failures | Avoid dead startup or false-ready state |
| P2 | NVS defaults, invalid values, and persistence | Prevent lost or corrupted settings |
| P2 | Empty/stale Chronos data and list bounds | Avoid invalid reads and misleading screens |
| P3 | Cosmetic UI, icons, and optional feature expansion | Defer until behavior is stable |

## Validation rule

For each firmware change, record the exact source SHA and firmware artifact, build the changed target, run unaffected-target regressions, then test the actual ESP32-C3 + OLED + TTP223 + relevant peripherals. Compilation is necessary but not sufficient. Do not merge hardware-sensitive behavior solely because CI passes.
