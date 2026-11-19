# NEO CLOCK — via_chronos (BLE)

This firmware is a separate build target for the existing NEO CLOCK hardware.

## Fixed hardware

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C address 0x3C
- MAX98357A: BCLK GPIO1, LRC GPIO2, DIN GPIO5
- INMP441: SCK GPIO1, WS GPIO2, SD GPIO10
- TTP223: PREV GPIO4, NEXT GPIO6, MODE GPIO7
- OLED: SCL GPIO20, SDA GPIO21

The pinout is intentionally identical to `via_wifi`. The microphone is not enabled in the first BLE milestone; the shared I2S clock arrangement needs a dedicated full-duplex validation before capture is added.

## BLE implementation

Uses fbiego's official [ChronosESP32](https://github.com/fbiego/chronos-esp32) library. The screen identifier is set to `CS_0x0_000_CFF` (no display profile), because the physical OLED is 128x64 and must not be misidentified as one of the app's 240x240 or 466x466 profiles.

Pair through the Chronos app:

1. Open Chronos.
2. Go to Watches > Watches > Pair New Devices.
3. Search and select **NEO_CLOCK**.
4. Do not pair it first through Android's Bluetooth settings.

## First milestone

- Initialize the SSD1306.
- Start Chronos BLE and expose connection state.
- Receive time synchronization from the phone.
- Display time and BLE state.

Notifications are currently logged to Serial. The custom clock faces, notification UI, audio output, and microphone capture will be integrated in later milestones after BLE pairing and time sync are verified on the actual hardware.
