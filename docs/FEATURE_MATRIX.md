# NEO CLOCK Firmware Feature Matrix

Audit snapshot: 2026-11-20

## Scope and interpretation

This matrix separates the production `main` branch from the open `via_chronos` PR #13. PR #13 is not part of production `main` and must not be treated as released firmware.

Status meanings:
- **Present**: implementation is visible in the inspected source.
- **Partial**: some of the feature exists, but behavior or coverage is limited.
- **Pending PR #13**: present only in the open development branch.
- **Not present**: no implementation found in the inspected target.
- **Hardware validation pending**: code/build evidence is not proof of physical behavior.

## Feature comparison

| Capability | via_wifi (main) | via_chronos (main) | via_chronos (PR #13) | mochi_chronos (main) |
|---|---|---|---|---|
| Primary connection | Wi-Fi | Chronos BLE | Chronos BLE | Chronos BLE |
| Clock/time source | NTP over Wi-Fi | Chronos phone sync | Chronos phone sync | Chronos phone sync |
| Six clock digit styles | Present | Present | Present | Animated face + text clock; six-style digit UI not used |
| 12/24-hour setting | Not exposed; uses local time | Fixed to 24-hour at startup | Present and persisted | Present and persisted |
| Clock-style persistence | Present (Preferences) | Not present | Present (Preferences) | Not applicable to current animated face |
| Main menu | 3-item menu | Not present | 8-item menu | 8-item settings menu |
| Button debounce | MODE only; PREV/NEXT use raw edge transitions | 180 ms global edge lockout, no per-button debounce | 35 ms per-button debounce | 35 ms per-button debounce |
| MODE short/long actions | Present | Not present | Present | Present |
| Wi-Fi setup portal | Present | Not applicable | Not applicable | Not applicable |
| Saved Wi-Fi profiles | Present, up to five | Not applicable | Not applicable | Not applicable |
| BLE status | Not applicable | Connection state on clock | Dedicated BLE status screen | Phone/status screen |
| Notifications | Not present | Callback logs notification | Browsable notification screen | Auto-displayed notification screen; currently renders item 0 |
| Weather | Not present | Not present | Browsable weather screen | Weather screen; currently renders item 0 |
| Music metadata | Not present | Not present | Present | Present |
| Music controls | Not present | Not present | Previous/next/play-pause | No equivalent track controls found in current button routing |
| Navigation | Not present | Not present | Text/data screen | Icon + text + audio turn cues |
| Phone battery/charging/model | Not present | Not present | Phone information screen | Phone screen and device information |
| Contacts/SOS | Not present | Not present | Not present | Present |
| QR/link relay | Not present | Not present | Not present | Present; user has indicated this is not useful |
| Alarm/call alerts | Hourly clock beep | Not present | Not present | Alarm and incoming-call sound handling |
| Sound settings | Basic beep | Not present | Not present | OFF/ALRT/ALL, volume, speaker effects |
| Display rotation setting | Fixed | Fixed | Fixed | Present and persisted |
| Idle/ambient behavior | Not present | Not present | Not present | Present; idle/quiet-hours clock and animated moods |
| Optional microphone | Not implemented | Not enabled | Not enabled | Experimental full-duplex path; disabled by default |
| Speaker output | I2S beep | No active audio UI | No active audio UI | I2S speaker beep enabled; full voice/mic path disabled by default |
| Settings persistence | Clock style | None in production main | Clock style + time format | Multiple settings in NVS |

## Intended product identities

- **via_wifi** remains the standalone Wi-Fi/NTP clock, including network provisioning and saved network profiles.
- **via_chronos** remains the simpler Chronos BLE clock, with the shared clock UI and a curated subset of Chronos data screens.
- **mochi_chronos** remains the companion-style firmware, with animated moods, audio personality, contacts/SOS and its richer settings.

Feature parity does not mean identical source code or identical screen count. A feature should be shared only where its data source, hardware needs, and user experience make sense in all relevant variants.

## Immediate alignment opportunities

1. Standardize physical button semantics and input reliability across all three variants.
2. Standardize return/back behavior, menu entry, and screen navigation conventions.
3. Define a common meaning for connection state, last successful time sync, and stale data.
4. Align empty-data and invalid-data presentation for notifications, weather, music, navigation, and phone information.
5. Decide which Chronos data features belong in the standard `via_chronos` experience after PR #13 is reviewed and device-tested.
6. Keep Mochi-exclusive animation/audio/personality features separate where they are part of its identity.
7. Revisit QR/link relay. It is currently implemented in Mochi, but the user has stated it has little practical value; no removal is made by this audit.

## Evidence and limits

The comparison is based on source inspection of repository `main` at `edbdda1de332347ef5774711e035ae63deee0780`, plus the open PR #13 head at `9ab5ba289be2a38588c0212be3c77a5d8a573dcc`.

The PR #13 CI runs for `via_chronos` and the `via_wifi` regression workflow completed successfully. No physical-device validation is implied. Mochi feature descriptions should be considered implementation presence, not proof of stable runtime behavior.
