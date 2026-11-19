# NEO_CLOCK Regression Test Checklist

Use this checklist before merging changes to firmware, the Firmware Builder, Supabase Edge Functions, or the flashing flow. A CI build proves compilation and packaging; it does not replace a physical flash test.

## 1. Firmware source and GPIO configuration

- [ ] Run `python -m unittest discover -s tests -v`.
- [ ] Verify the default GPIO map is accepted for all three variants.
- [ ] Verify a valid custom GPIO map is applied to the correct macros in each variant.
- [ ] Verify duplicate GPIO assignments are rejected.
- [ ] Verify GPIO8, GPIO9, GPIO11–19 and any other non-allow-listed pin are rejected.
- [ ] Verify unknown signal names and non-integer values (including booleans) are rejected.
- [ ] Verify GPIO2 remains allowed with a boot-strapping warning when a custom map includes it.
- [ ] Confirm a generated custom configuration does not leak into a later build using defaults.

## 2. GitHub Actions build and packaging

Run the matching workflow for every variant after changes to shared tooling:

- [ ] `via_wifi` workflow completes successfully.
- [ ] `via_chronos` workflow completes successfully.
- [ ] `mochi_chronos` workflow completes successfully.
- [ ] `merged.bin` is exactly 4,194,304 bytes.
- [ ] `build-info.json` identifies the correct variant, board, source SHA, and image hash.
- [ ] `advanced-bin.zip` contains the expected files and exact offsets from its own `FLASH-INSTRUCTIONS.txt`.
- [ ] SHA-256 values match the generated files.
- [ ] Mochi's optional audio artifact remains clearly marked experimental and is not substituted for the default image.
- [ ] GitHub Actions artifact retention remains intentional and is not confused with Supabase Storage retention.

## 3. Firmware Builder frontend

- [ ] Page loads at `/firmware-builder/` on mobile and desktop.
- [ ] All three variants can be selected.
- [ ] Default GPIO values match the hardware baseline.
- [ ] Duplicate and unsupported GPIO selections block build submission.
- [ ] Changing variant or GPIO invalidates the previous build and disables stale download/flash actions.
- [ ] Build button prevents duplicate submissions while a request is active.
- [ ] Queued, building, succeeded, failed, and expired states are displayed correctly.
- [ ] Both download actions request fresh signed URLs.
- [ ] A browser without Web Serial can still download `merged.bin` and the Advanced ZIP.
- [ ] No GitHub token, Supabase service-role key, or callback secret appears in frontend source or network responses.

## 4. Supabase backend and artifact lifecycle

- [ ] Build creation rejects unapproved origins, variants, GPIO keys, invalid GPIO values, and duplicate GPIOs.
- [ ] Rate limit and maximum active queue capacity behave as documented.
- [ ] A queue-full rejection does not consume an hourly quota slot.
- [ ] Status access requires the random status token; invalid tokens cannot read build state.
- [ ] Signed URLs are short-lived (currently 300 seconds) and point to objects in the private bucket.
- [ ] Callback rejects an invalid shared secret and mismatched build/run/artifact identities.
- [ ] Callback handles failed and cancelled builds without marking them successful.
- [ ] Callback rejects missing required files, incorrect merged-image size, invalid metadata, and SHA-256 mismatch.
- [ ] Partial Storage uploads are rolled back when a later upload or metadata write fails.
- [ ] Duplicate callbacks are tested for races before relying on the callback-processing lease.
- [ ] Storage object counts and total bytes are periodically checked.
- [ ] Build-record expiry and actual Storage-object deletion are tested separately; an expired status token does not itself prove that the Storage objects were deleted.

## 5. Download integrity and flashing

- [ ] Downloaded `merged.bin` is exactly 4 MiB.
- [ ] Browser-computed SHA-256 matches the artifact metadata.
- [ ] The user sees the full-flash / saved-settings erase warning before confirmation.
- [ ] Web Serial device picker appears in a supported browser.
- [ ] Detected chip is ESP32-C3; a different chip is rejected before writing.
- [ ] Flash uses the verified settings: address `0x0000`, DIO, 80 MHz, 4 MB.
- [ ] Progress is shown and USB disconnects/errors are handled without claiming success.
- [ ] Board resets after a successful write, or the UI gives a manual-reset instruction.
- [ ] PC download-and-flash path is tested independently of browser flashing.
- [ ] Android WebSerial and Android merged-BIN app paths are tested on the actual phone, OTG adapter, cable, and board before claiming broad Android support.

## 6. Physical hardware verification

For each firmware variant, record a separate result. Do not extrapolate one variant's success to the others.

- [ ] Board boots repeatedly from a cold power cycle.
- [ ] OLED initializes at 128×64 / I²C 0x3C and remains stable.
- [ ] PREV, NEXT, and MODE/OK inputs work and do not trigger unintended repeats.
- [ ] Variant-specific time sync / BLE pairing / Wi-Fi behavior works.
- [ ] MAX98357A output is tested only when its power and wiring are verified.
- [ ] INMP441 capture remains disabled unless separately validated with the shared I²S clocks.
- [ ] GPIO2 boot behavior is checked with the connected hardware.
- [ ] No unexpected resets, boot loops, or USB connection failures occur during a reasonable soak test.
- [ ] Record variant, GPIO map, build ID, source SHA, flash method, and observed results in [HARDWARE_BASELINE.md](HARDWARE_BASELINE.md).

## 7. Website deployment

- [ ] GitHub Pages deployment workflow succeeds.
- [ ] Homepage NEO_CLOCK card links to `/firmware-builder/`.
- [ ] Firmware Builder route loads from the deployed site.
- [ ] Existing homepage language selector, themes, navigation, and other project links still work.
- [ ] The production page is checked after deployment; a successful CI run alone is not a visual browser test.

## Baseline rule

Do not move the baseline tag after a failed regression. Fix the issue on a branch, rerun the affected checks, and create a new baseline tag only after the user confirms the new physical result.
