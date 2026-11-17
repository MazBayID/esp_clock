# Neo Firmware

Neo Firmware is the modular firmware foundation for the ESP32-C3 SuperMini.
It currently has two separate firmware tracks:

- **via_wifi** — the active Neo Clock firmware, developed and built first.
- **via_chronos_app** — the future firmware track for the Chronos Android app. Work on it is postponed until \`via_wifi\` is complete and stable.

## Repository layout

\`\`\`text
assets_master/                         # Incoming original UI assets + manifest
firmware/
├── via_wifi/                          # Current active firmware
│   ├── via_wifi.ino
│   └── src/
│       ├── config/
│       ├── systems/
│       └── assets/default/
└── via_chronos_app/                   # Future Chronos BLE firmware
\`\`\`

## Hardware target

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C
- MAX98357A I2S audio amplifier
- TTP223 touch inputs

Pin assignments are defined in \`firmware/via_wifi/src/config/config.h\`.

## GitHub Actions builds

There are two separate workflow files:

- \`.github/workflows/build-firmware-via-wifi.yml\` builds only \`firmware/via_wifi\` on pushes to \`main\`, pull requests targeting \`main\`, or manual dispatch.
- \`.github/workflows/build-firmware-via-chronos-app.yml\` is a disabled placeholder. It contains no Chronos build steps and will remain inactive until the Wi-Fi firmware is complete and stable.

After a successful Wi-Fi workflow run, download the \`NeoFirmware-via-wifi-ESP32-C3-...\` ZIP from the run's **Artifacts** section. It contains the ESP32 binary files and flashing-offset instructions.

## Local Wi-Fi credentials

The firmware stores up to **five Wi-Fi networks** in ESP32 NVS Preferences. When a sixth distinct network is successfully connected through the setup page, the oldest saved entry is removed (FIFO). Failed connection attempts do not alter the saved list.

If no saved network connects, the device starts an open setup hotspot named \`NEO_CLOCK\`. Connect to it and open the captive portal; if automatic redirection does not appear, browse to \`http://192.168.4.1\`. The page can scan nearby networks or accept an SSID manually. Passwords are never displayed back on the page.

A local \`secrets.h\` can still be used as an optional bootstrap fallback. Copy \`firmware/via_wifi/src/config/secrets.example.h\` to \`secrets.h\` and fill in the values. The real file is ignored by Git. If the file is absent, the firmware still builds and starts the setup hotspot when no saved network connects.

**Security note:** the \`NEO_CLOCK\` hotspot is open. Anyone nearby can access the setup page while it is active. Use it only for initial setup or trusted environments.

## Asset workflow

The current clock digit styles are built-in defaults under
\`firmware/via_wifi/src/assets/default/digits/\`.

Upload original UI assets and their manifest to \`assets_master/\`. Review the manifest before moving or converting assets into their final runtime locations under \`firmware/via_wifi/src/assets/\`.
