# Neo Firmware

Neo Firmware is the modular firmware foundation for the ESP32-C3 SuperMini.
**Neo Clock** is its first built-in feature. The architecture is intended to grow
with additional device, UI, networking, and IoT capabilities.

## Repository layout

```text
assets_master/                         # Incoming original UI assets + manifest
firmware/
└── NeoFirmware/
    ├── NeoFirmware.ino                # Arduino sketch entry point
    └── src/
        ├── config/
        │   ├── config.h               # Board config + shared declarations
        │   └── secrets.example.h      # Local Wi-Fi setup template
        ├── systems/
        │   ├── display_system.h
        │   ├── clock_system.h
        │   ├── input_system.h
        │   ├── wifi_system.h
        │   ├── audio_system.h
        │   └── quote_system.h
        └── assets/
            └── default/
                ├── digits/            # Built-in clock digit styles
                └── logos/             # Built-in Neo logos
```

## Hardware target

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C address 0x3C
- MAX98357A I2S audio amplifier
- TTP223 touch inputs

Pin assignments are defined in `firmware/NeoFirmware/src/config/config.h`.

## Build with GitHub Actions

The workflow `.github/workflows/build-firmware.yml` builds the ESP32-C3 firmware
on pushes to `main`, pull requests targeting `main`, and manual dispatches.

After a successful run, open the workflow run's **Artifacts** section and download
the `NeoFirmware-ESP32-C3-...` ZIP. It contains the generated ESP32 binary files
and flashing-offset instructions. The build does not require Wi-Fi credentials.

## Build locally with Arduino CLI

Install Arduino CLI, then run:

```sh
arduino-cli core update-index \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

arduino-cli core install esp32:esp32@3.3.12 \
  --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit SSD1306"
arduino-cli lib install "ArduinoJson"

arduino-cli compile \
  --fqbn esp32:esp32:esp32c3 \
  firmware/NeoFirmware
```

## Local Wi-Fi credentials

Copy `firmware/NeoFirmware/src/config/secrets.example.h` to
`firmware/NeoFirmware/src/config/secrets.h` and enter your Wi-Fi SSID/password.
The real `secrets.h` is ignored by Git. If it is absent, the firmware still builds
and skips Wi-Fi initialization at runtime.

Do not commit real credentials or paste them into source files.

## Asset workflow

The three current clock digit styles are preserved as built-in firmware defaults
under `firmware/NeoFirmware/src/assets/default/digits/`.

Upload new original UI files and their manifest to `assets_master/`. They will
be inspected first; only then will each asset be converted or moved to its final
runtime folder under `firmware/NeoFirmware/src/assets/`. The intake folder is
not automatically compiled into firmware.
