# NEO CLOCK

**NEO CLOCK** is an ESP32-C3-based smart clock project with a 128×64 OLED display. The repository now contains multiple independent firmware implementations exploring different connectivity, interface, and companion-device features.

Each firmware is maintained as a separate target. They share the intended hardware platform, but their code, features, build process, and maturity are different. Building or changing one target should not affect the others.

## Firmware targets

| Firmware | Connectivity | Overview | Status |
|---|---|---|---|
| [via_wifi](firmware/via_wifi/) | Wi-Fi | Standalone clock, network time synchronization, saved Wi-Fi profiles, setup portal, OLED UI, and touch/audio feedback | Main Wi-Fi firmware |
| [via_chronos](firmware/via_chronos/) | Bluetooth LE / Chronos | Phone-synchronized clock, clock styles, BLE status, startup logo, and touch controls | Early BLE implementation; hardware validation pending |
| [mochi_chronos](firmware/mochi_chronos/) | Bluetooth LE / Chronos | Animated companion interface, mood animations, phone notifications/data, navigation, music metadata, settings, and optional audio | Independent feature-rich firmware; see its README for build and feature details |

These firmware targets are **not interchangeable feature sets**. Choose the firmware that matches the experience you want to run.

### Firmware documentation

- **[via_wifi README](firmware/via_wifi/)** — Wi-Fi clock firmware and setup.
- **[via_chronos README](firmware/via_chronos/README.md)** — BLE clock implementation, pairing, controls, and milestone status.
- **[mochi_chronos README](firmware/mochi_chronos/README.md)** — companion firmware, supported features, controls, audio options, and build instructions.

## Hardware platform

The firmware targets the ESP32-C3 SuperMini with a 128×64 SSD1306 OLED.

The intended shared hardware configuration is:

| Component | Connection |
|---|---|
| ESP32-C3 SuperMini | Main MCU |
| SSD1306 OLED, 128×64, I²C address 0x3C | SDA GPIO21, SCL GPIO20 |
| MAX98357A I²S amplifier | BCLK GPIO1, LRC GPIO2, DIN GPIO5 |
| INMP441 I²S microphone | SCK GPIO1, WS GPIO2, SD GPIO10 |
| TTP223 touch buttons | GPIO4, GPIO6, GPIO7 |
| Optional passive piezo buzzer | GPIO3 (Mochi target; disabled by default) |

The amplifier and microphone share I²S clock pins. Microphone/audio operation is not enabled by default in the Mochi firmware; full-duplex operation must be validated before enabling simultaneous capture and playback.

For the detailed Wi-Fi firmware wiring, see [via_wifi PINOUT](firmware/via_wifi/PINOUT.md). Firmware-specific control mappings and hardware notes are documented in each target's README.

## Repository structure

```text
.
├── assets_master/                  # Original/source UI assets and asset manifest
├── firmware/
│   ├── via_wifi/                  # Wi-Fi clock firmware
│   ├── via_chronos/               # BLE Chronos clock firmware
│   └── mochi_chronos/              # Mochi-inspired Chronos companion firmware
├── tools/
│   └── process_assets.py           # Asset processing for the Arduino targets
└── .github/workflows/
    ├── build-firmware-via-wifi.yml
    ├── build-firmware-via-chronos.yml
    └── build-firmware-mochi-chronos.yml
```

Each firmware folder contains its own source code and target-specific documentation. The Mochi target also has its own asset generator and PlatformIO configuration.

## Building firmware

Build requirements depend on the selected target. Refer to that target's README for exact dependencies and commands.

### via_wifi

Uses Arduino CLI and the ESP32 Arduino core. Its GitHub Actions workflow compiles the firmware and packages binaries with flash instructions.

### via_chronos

Uses Arduino CLI and the ESP32 Arduino core, with Chronos BLE-related libraries and generated UI assets. Its workflow builds and packages the BLE firmware separately from via_wifi.

### mochi_chronos

Uses PlatformIO. The default build disables optional audio; an additional audio-enabled environment is available. Its workflow generates assets and builds both configurations.

## GitHub Actions

The repository provides independent build workflows:

- [Build NEO CLOCK - ESP32-C3](.github/workflows/build-firmware-via-wifi.yml)
- [Build NEO CLOCK - via Chronos BLE](.github/workflows/build-firmware-via-chronos.yml)
- [Build Mochi Chronos Firmware](.github/workflows/build-firmware-mochi-chronos.yml)

Workflows can be triggered by their configured push/PR events or manually through GitHub Actions where enabled. Build artifacts are available from successful workflow runs. **A successful CI build confirms compilation and packaging only; it does not replace testing on the physical device.**

## Asset workflow

Original and incoming UI assets are collected in [assets_master](assets_master/). This directory is the source/reference library—not automatically the runtime asset directory for every firmware.

Assets are reviewed, converted, or generated for the target firmware as needed. Firmware-specific generated files should remain in their respective target folders so that asset processing for one firmware does not overwrite another firmware's assets.

See [assets_master README](assets_master/README.md) for the asset organization and handling guidelines.

## Development status

NEO CLOCK is an evolving project. Firmware targets are developed and validated independently, and feature availability varies by target.

- **via_wifi:** Wi-Fi clock implementation with network time and Wi-Fi configuration support.
- **via_chronos:** BLE/Chronos integration milestone; physical pairing, synchronization, display, and control testing remain important validation steps.
- **mochi_chronos:** separate companion-style firmware with a wider feature set. Optional audio and hardware-specific behavior require testing on the actual board.

Check the relevant firmware README and workflow run before flashing. Use the firmware image intended for your target and follow its flash instructions.

---

*Project firmware and hardware notes are maintained in the individual target folders. This root README provides an overview of the repository as a whole.*
