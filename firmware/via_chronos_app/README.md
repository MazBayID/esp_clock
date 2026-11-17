# via_chronos_app

This folder is reserved for the future Neo Firmware variant that connects to
the Chronos Android application over Bluetooth Low Energy (BLE).

## Planned reference template

- Firmware template: [MazBayID/Dirgamochi-C3_v0.7.0--BiG-Update](https://github.com/MazBayID/Dirgamochi-C3_v0.7.0--BiG-Update)
- Chronos app: [Google Play Store](https://play.google.com/store/apps/details?id=com.fbiego.chronos)
- Chronos ESP32 library: [fbiego/chronos-esp32](https://github.com/fbiego/chronos-esp32)

## Current status

**Not started — intentionally postponed.**

The current `via_wifi` firmware is the active development and build target.
Do not copy or merge the Dirgamochi implementation into `via_wifi`.

When `via_wifi` is complete and stable, this folder can receive its own
firmware structure, configuration, dependencies, and build workflow after the
template has been reviewed against the actual board wiring and required features.

This folder is not compiled by the current GitHub Actions workflow.
