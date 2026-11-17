# Neo Firmware

Neo Firmware is the modular firmware foundation for the ESP32-C3 SuperMini.
It currently has two separate firmware tracks:

- **via_wifi** — the current Neo Clock firmware, built and maintained first.
- **via_chronos_app** — a future firmware track for connecting the device to the Chronos Android app. Its implementation is intentionally postponed until `via_wifi` is complete and stable.

## Repository layout

```text
assets_master/                         # Incoming original UI assets + manifest
firmware/
├── via_wifi/                          # Current firmware; active build target
│   ├── via_wifi.ino                   # Arduino sketch entry point
│   └── src/
│       ├── config/
│       │   ├── config.h
│       │   └── secrets.example.h
│       ├── systems/
│       │   ├── display_system.h
│       │   ├── clock_system.h
│       │   ├── input_system.h
│       │   ├── wifi_system.h
│       │   ├── audio_system.h
│       │   └── quote_system.h
│       └── assets/
│           └── default/
│               ├── digits/            # Built-in clock digit styles
│               └── logos/             # Built-in Neo logos
└── via_chronos_app/
    └── README.md                      # Future Chronos integration notes
```

## Hardware target

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C address 0x3C
- MAX98357A I2S audio amplifier
- TTP223 touch inputs

Pin assignments for the current Wi-Fi firmware are defined in
`firmware/via_wifi/src/config/config.h`.

## Build with GitHub Actions

The workflow `.github/workflows/build-firmware.yml` currently builds **only**
`firmware/via_wifi` for ESP32-C3 SuperMini. It runs on pushes to `main`,
pull requests targeting `main`, and manual dispatches.

After a successful run, open the workflow run's **Artifacts** section and
download the `NeoFirmware-via-wifi-ESP32-C3-...` ZIP. It contains the generated
ESP32 binary files and flashing-offset instructions. The build does not require
Wi-Fi credentials.

The `via_chronos_app` folder is not included in this build. A separate build
target will be added only when the Wi-Fi firmware is finished and the Chronos
integration work begins.

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
  firmware/via_wifi
```

## Local Wi-Fi credentials

Copy `firmware/via_wifi/src/config/secrets.example.h` to
`firmware/via_wifi/src/config/secrets.h` and enter your Wi-Fi SSID/password.
The real `secrets.h` is ignored by Git. If it is absent, the firmware still
builds and skips Wi-Fi initialization at runtime.

Do not commit real credentials or paste them into source files.

## Asset workflow

The three current clock digit styles are preserved as built-in firmware defaults
under `firmware/via_wifi/src/assets/default/digits/`.

Upload new original UI files and their manifest to `assets_master/`. They will
be inspected first; only then will each asset be converted or moved to its final
runtime folder under `firmware/via_wifi/src/assets/`. The intake folder is not
automatically compiled into firmware.
