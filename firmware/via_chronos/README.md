# NEO CLOCK — via_chronos (BLE)

Separate BLE firmware for the existing NEO CLOCK hardware. This build does not modify or replace `via_wifi`.

## Project references

- Chronos Android app: [Google Play Store](https://play.google.com/store/apps/details?id=com.fbiego.chronos)
- Chronos website: [chronos.ke](https://chronos.ke/)
- Official ESP32 library: [fbiego/chronos-esp32](https://github.com/fbiego/chronos-esp32)
- Additional hardware/reference project: [Dirgamochi-C3 v0.7.0](https://github.com/MazBayID/Dirgamochi-C3_v0.7.0--BiG-Update)

The Chronos app and the official ChronosESP32 library are the BLE integration references. Dirgamochi-C3 is an additional reference for reviewing compatible hardware/firmware ideas; its implementation must not be copied blindly or allowed to change the fixed NEO CLOCK pinout.

## Fixed hardware / pinout

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C address 0x3C
- OLED: SCL GPIO20, SDA GPIO21
- MAX98357A: BCLK GPIO1, LRC GPIO2, DIN GPIO5
- INMP441: SCK GPIO1, WS GPIO2, SD GPIO10
- TTP223: PREV GPIO4, NEXT GPIO6, MODE GPIO7

The microphone is not enabled in the first milestone. Its clock pins are shared with MAX98357A; full-duplex I2S behavior must be validated before microphone capture is enabled.

## BLE implementation

Uses fbiego's official [ChronosESP32](https://github.com/fbiego/chronos-esp32) Arduino library. The screen identifier is `CS_0x0_000_CFF` (no display profile), because the physical OLED is 128x64 and must not be identified as a 240x240 or 466x466 watch.

Pair through the Chronos app:

1. Open Chronos.
2. Go to Watches > Watches > Pair New Devices.
3. Search and select **NEO_CLOCK**.
4. Do not pair it first through Android Bluetooth settings.

## Display and assets

- Reuses the six original NEO CLOCK digit styles and both logo bitmaps from `via_wifi`.
- Uses `assets_master/` as the source for additional UI assets when required.
- The Chronos workflow runs `tools/process_assets.py` with a separate output path, so generated assets are placed under `via_chronos` and do not overwrite the Wi-Fi firmware assets.
- `assets_master/` source PNGs remain unchanged.

## Initial controls

- PREV (GPIO4): previous clock style.
- NEXT (GPIO6): next clock style.
- MODE (GPIO7): toggle 12/24-hour display.

## Milestone status

Implemented in firmware:

- SSD1306 initialization and NEO startup logo.
- Chronos BLE advertising and connection callback.
- Chronos time synchronization callback.
- Six clock styles and generated Bluetooth UI asset.
- Debounced PREV/NEXT/MODE input handling with short/long MODE actions.
- On-device settings menu for clock style, time format, BLE status, notifications, and weather.
- Persistent clock style and 12/24-hour settings.

Still requires physical-device validation:

- Pairing with Chronos app.
- Time synchronization and display behavior.
- Touch sensor polarity and usability.
- Notification rendering (currently logged to Serial only).
- Audio output and microphone support (not yet enabled).
