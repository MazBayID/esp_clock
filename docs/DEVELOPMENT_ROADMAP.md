# NEO CLOCK Development Roadmap

Baseline: production `main` at `1bcf8f2212ee4e0a5d1aa8e597ca70d44137aa1b` (2026-11-20).

## Guiding rules

- Stabilize existing behavior before adding features.
- Keep `main` production-safe; use focused branches and PRs.
- Preserve the three firmware identities; parity does not require identical implementations.
- Never treat CI compilation as physical-device validation.
- Keep the ESP32-C3 SuperMini pinout fixed unless a separately reviewed hardware change is approved.
- Keep each task small, testable, and independently revertible.

## Stage 1 — Finish input and navigation validation

- Physically test merged input/navigation behavior on all variants.
- Complete PR #16 and PR #19 held-at-boot tests: boot while each TTP223 input is held, release it, and verify no action occurs until a new press.
- Test rapid, slow, simultaneous, short, and long presses.
- Verify menu entry, selection, back, and home from every relevant screen.
- Confirm active navigation does not trap Mochi on one screen.

**Exit criteria:** no repeated/missed actions in the agreed test sequence; each screen has a reliable return path.

## Stage 2 — Time, connection, persistence, and startup

- Separate “connected”, “time valid”, and “last successful sync” state.
- Test Wi-Fi loss/reconnect, NTP failure/recovery, BLE disconnect/reconnect, and Chronos time updates.
- Verify NVS defaults, invalid stored values, write failures, and power-cycle restoration.
- Review OLED and I2S initialization return values and failure behavior.
- Review blocking Wi-Fi startup paths and provisioning security.

**Exit criteria:** no stale time presented as a fresh sync; invalid settings safely fall back; peripheral failures are represented honestly.

## Stage 3 — Chronos data robustness

- Test notifications, weather, music, navigation, phone information, contacts, and QR payloads from the Chronos app.
- Validate empty lists, list shrink/growth, stale data after disconnect, malformed payloads, long strings, and non-ASCII text.
- Decide whether Mochi's glanceable item-0 presentation is intentional or whether browsing is required.
- Treat QR feature retention/removal as a separate product decision; bounds hardening does not imply that the feature must remain.

**Exit criteria:** no out-of-range access; every screen has defined empty, stale, and disconnected states.

## Stage 4 — Feature parity and product boundaries

- Keep Wi-Fi provisioning/NTP as via_wifi-specific.
- Keep Mochi animated moods, companion audio, contacts/SOS, and other deliberate identity features variant-specific where appropriate.
- Decide which additional Chronos data features belong in the simpler via_chronos experience.
- Convert the matrix into approved shared, variant-exclusive, deferred, and removed requirements before implementation.

## Stage 5 — UI and asset consistency

- Wait until screen structure and navigation are stable.
- Standardize typography, spacing, labels, status indicators, and menu affordances where useful.
- Add icons only where they improve readability on the 128x64 monochrome OLED.
- Verify memory/flash impact and keep asset generation deterministic and target-specific.

## Stage 6 — Regression and merge gates

For every PR:
1. Review the diff and confirm only intended paths changed.
2. Run repository checks and GPIO validation where relevant.
3. Build the modified target.
4. Build unaffected targets as regression checks.
5. Inspect packaging/artifact contents where applicable.
6. Flash and test the physical ESP32-C3 for hardware-sensitive behavior.
7. Record source SHA, firmware artifact, board, test steps, results, and limitations.
8. Merge only when CI, review, and required device checks pass.

## Immediate execution order

1. Complete the physical held-at-boot tests for PR #16 and PR #19.
2. Test the remaining navigation, data-list, persistence, and reconnect edge cases.
3. Address high-priority Wi-Fi provisioning and startup risks in separate PRs.
4. Re-run the feature/stability audit after each meaningful stabilization batch.
5. Only then expand feature parity or undertake UI/icon redesign.
