# NEO CLOCK Development Roadmap

Roadmap created from the static audit dated 2026-11-20.

## Guiding rules

- Stabilize existing behavior before adding more features.
- Keep `main` production-safe.
- Develop in focused feature branches and review through pull requests.
- Preserve the three firmware identities; parity does not require identical code.
- Do not treat CI compilation as physical-device validation.
- Keep the ESP32-C3 SuperMini pinout fixed unless a separately reviewed hardware change is approved.
- Keep each task small enough to test and revert independently.

## Stage 0 — Establish a trustworthy baseline

**Goal:** know exactly which source and firmware package are being tested.

Tasks:
- Record the current production `main` SHA and all open firmware PRs.
- Keep PR #13 open for review; do not merge based on CI alone.
- Record board, OLED, touch, speaker, and microphone wiring for each physical test.
- Confirm which device is available for testing and its exact TTP223 active level.
- Capture baseline behavior and build artifacts for all three targets.

Exit criteria:
- The source revision and firmware image used in every test are traceable.
- The physical pinout and button polarity are confirmed.
- No unreviewed source is flashed as production firmware.

## Stage 1 — Input and navigation stability

**Priority: highest**

Tasks:
- Compare PREV/NEXT/MODE behavior across all targets.
- Define a shared interaction contract: short press, long press, menu navigation, select, back, and home.
- Use independent debouncing for each input; verify behavior with the real touch modules.
- Test simultaneous/overlapping presses and long press while the screen is changing.
- Review Mochi automatic screen overrides for navigation, notifications, and connection events.
- Review PR #13 input state transitions and test all screen return paths.

Suggested interaction contract:
- PREV: previous item/screen.
- NEXT: next item/screen.
- MODE short: select/confirm or the screen's documented primary action.
- MODE long: open menu from normal screens; return home from menu/detail states.
- Every screen has an unambiguous way to return to its parent or home.

Exit criteria:
- No repeated/missed actions during the agreed button test sequence.
- Menu entry, selection, back, and home work from every relevant screen.
- Automatic events do not permanently trap the user or unexpectedly erase navigation context.

## Stage 2 — Time, connection, persistence, and startup reliability

Tasks:
- Separate “connected”, “time valid”, and “last successful sync” state.
- Test Wi-Fi loss/reconnect, NTP failure/recovery, BLE disconnect/reconnect, and Chronos time updates.
- Verify NVS defaults, invalid stored values, write failure handling, and power-cycle restore.
- Review OLED and I2S initialization return values and failure behavior.
- Review blocking startup paths and determine which should become bounded or asynchronous.
- Review Wi-Fi setup portal security and credential-storage threat model before production distribution.

Exit criteria:
- The clock never presents stale time as a fresh successful synchronization without a defined policy.
- Settings survive reboot and invalid NVS data safely falls back to defaults.
- Peripheral failures are detected and represented honestly.
- Wi-Fi provisioning does not expose credentials to unintended local clients.

## Stage 3 — Chronos data feature stability

Tasks:
- Test notifications, weather, music, navigation, and phone information using real Chronos app payloads.
- Validate empty lists, list shrink/growth, stale data after disconnect, long strings, and non-ASCII text.
- Compare browsing behavior in via_chronos PR #13 with Mochi's glanceable single-item screens.
- Decide which features should be common and which should remain Mochi-specific.
- Add explicit music controls to Mochi only if user experience and hardware interaction justify it.
- Resolve the Mochi Finding Phone screen/timer path if the screen is intended to be a real feature.

Exit criteria:
- No out-of-range list access or misleading screen state in tested payload conditions.
- Each feature has a defined empty/stale/disconnected presentation.
- Parity decisions are documented before implementation.

## Stage 4 — Feature parity and product boundaries

Tasks:
- Convert the feature matrix into approved common/variant-exclusive requirements.
- Preserve Wi-Fi provisioning and NTP as via_wifi-specific.
- Preserve animated moods, sound personality, contacts/SOS, and other deliberate companion behaviors as Mochi-specific where appropriate.
- Decide which Chronos data screens belong in the simpler via_chronos experience.
- Review QR/link relay with the user; the current feature has low user value and should not be retained merely because it exists.

Exit criteria:
- Every feature is classified as shared, variant-exclusive, deferred, or removed.
- Each implementation has a test plan and no duplicated work is started before the classification.

## Stage 5 — UI and asset consistency

Tasks:
- Wait until screen structure and navigation are stable.
- Standardize typography, spacing, labels, status indicators, and menu affordances where the displays have similar needs.
- Add/reuse icons only where they improve readability on a 128x64 monochrome OLED.
- Keep Mochi animation and personality distinct.
- Verify memory/flash impact of generated assets and ensure each target's asset generation remains isolated.

Exit criteria:
- No clipping/overlap on the physical OLED.
- UI changes do not alter navigation semantics or regress clock rendering.
- Asset generation remains deterministic and target-specific.

## Stage 6 — Regression, release, and merge

For each PR:
1. Review the diff and confirm only the intended target/paths changed.
2. Run GPIO validation and repository tests.
3. Build the modified target.
4. Build all unaffected targets as regression checks.
5. Inspect packaging and artifact contents.
6. Flash and test the physical ESP32-C3.
7. Record firmware SHA, board, test steps, observed results, and remaining limitations.
8. Merge only after review and required device checks.

Release gates:
- CI passes for the relevant target and regression targets.
- No unresolved high-priority static findings remain in changed code.
- Physical input, display, connectivity/time, and target-specific features pass their test cases.
- README and feature matrix match the shipped firmware.
- `main` remains the production branch; no untested feature is merged.

## First execution sequence

1. Complete the physical input/navigation test plan for all three variants.
2. Review PR #13 in isolation and prepare a device test checklist; do not merge yet.
3. Stabilize via_wifi input debounce and provisioning/startup concerns in separate scoped PRs.
4. Stabilize Mochi screen override and data behavior without adding new features.
5. Re-run cross-variant comparison and approve parity requirements.
