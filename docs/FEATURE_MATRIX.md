# NEO CLOCK Firmware Feature Matrix

Audit baseline: production `main` at `1bcf8f2212ee4e0a5d1aa8e597ca70d44137aa1b` (2026-11-20).

This is a source-level comparison. “Implemented” means present in source; it does not by itself prove reliable behavior on physical hardware.

## Feature comparison

| Capability | via_wifi | via_chronos | mochi_chronos |
|---|---|---|---|
| Primary connection/time source | Wi-Fi + NTP | Chronos BLE + phone time | Chronos BLE + phone time |
| Main presentation | Six-style digital clock | Six-style digital clock | Animated Mochi face + clock/data screens |
| Wi-Fi provisioning and saved networks | Implemented | Not applicable | Not applicable |
| BLE connection/status | Not applicable | Implemented | Implemented |
| PREV/NEXT/MODE navigation | Debounced input; Wi-Fi/menu context actions | Debounced input; menu and screen navigation | Debounced input; screen/menu actions |
| Clock style / time format persistence | Clock style | Clock style and 12/24-hour format | Multiple UI settings including time format |
| Menu/settings | Compact menu | Eight-item menu | Settings menu |
| Notifications | Not applicable | Notification browser | Notification screen |
| Weather | Not applicable | Weather browser | Weather screen |
| Music | Not applicable | Metadata and previous/next/play-pause controls | Metadata/status; no equivalent full track-control set confirmed |
| Navigation | Not applicable | Navigation data screen | Navigation screen and audio cues |
| Phone information | Not applicable | Phone information screen | Phone/device information |
| Contacts / SOS | Not applicable | Not implemented | Implemented |
| QR/link relay | Not applicable | Not implemented | Implemented; usefulness remains a separate product decision |
| Find Phone | Not applicable | Not implemented | Menu action implemented; device test confirmed |
| Alarm / incoming call handling | Hourly chime | Not implemented | Implemented |
| Speaker output | I2S beep | No active audio feature | I2S speaker effects; experimental microphone/full-duplex path disabled by default |
| OLED rotation setting | Fixed | Fixed | Configurable and persisted |

## Product boundaries

- **via_wifi** remains the standalone Wi-Fi/NTP clock. Wi-Fi provisioning and network recovery are not missing BLE parity features.
- **via_chronos** remains the simpler Chronos BLE clock with a curated set of phone-data screens and music controls.
- **mochi_chronos** remains the companion-style firmware with animated moods, audio personality, contacts/SOS, and richer settings.

Parity does not require identical screens or source code. Share behavior only when connectivity, hardware, and user experience genuinely align.

## Recent merged changes represented in this baseline

- PR #13: via_chronos settings, notification/weather/music/navigation/phone screens.
- PR #15: via_wifi PREV/NEXT debounce.
- PR #20: Mochi navigation no longer forcibly locks the display to its screen; new navigation events can still bring it forward.
- PR #21: Mochi Find Phone menu action.
- PR #17: clamp Mochi contact index when the synchronized list shrinks or becomes empty.
- PR #18: validate incoming QR link indices and clamp QR count/index to the fixed array bounds.

## Remaining validation

- PR #16 (Mochi) and PR #19 (via_chronos): held-at-boot touch input behavior still requires physical testing on the actual TTP223 hardware.
- CI builds do not prove BLE pairing, data relay, input feel, audio, or OLED behavior.
- QR feature retention/removal is a product decision; PR #18 only hardens its bounds handling.
