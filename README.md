# Neo Firmware

Neo Firmware is the firmware foundation for the ESP32-C3 SuperMini. It has two separate development tracks:

- **via_wifi** — the active Neo Clock firmware. This is the only firmware being built and developed now.
- **via_chronos_app** — a future firmware track for the Chronos Android app. Work on it is postponed until `via_wifi` is complete and stable.

## Repository layout

```text
assets_master/                         # Incoming original UI assets and manifest
firmware/
├── via_wifi/                          # Current active firmware
│   ├── via_wifi.ino
│   └── src/
│       ├── config/
│       ├── systems/
│       └── assets/default/
└── via_chronos_app/                   # Future Chronos BLE firmware
.github/workflows/
├── build-firmware-via-wifi.yml
└── build-firmware-via-chronos-app.yml
```

## Hardware target

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C
- MAX98357A I2S audio amplifier
- TTP223 touch inputs

Pin assignments are defined in `firmware/via_wifi/src/config/config.h`.

## Wi-Fi setup portal

The firmware stores up to **5 Wi-Fi profiles** in ESP32 NVS Preferences.

1. At startup, Neo Clock scans for saved networks and tries the networks it can currently see, starting with the most recently added profile.
2. If no profile is saved, or no saved SSID is found or can be connected to, Neo Clock creates an open hotspot named `NEO_CLOCK`.
3. Connect a phone to `NEO_CLOCK`. The captive portal should open automatically. If it does not, open `http://192.168.4.1` in a browser.
4. Select a nearby network from the scan list or enter its SSID manually, then enter the password. A profile is saved only after the connection succeeds.
5. When five profiles are already stored, adding a sixth distinct network removes the oldest profile (FIFO). Updating an existing SSID refreshes its password and moves it to the newest position. Failed connection attempts do not change the saved list.

The setup hotspot is open and intended for local configuration only. Open Wi-Fi networks (empty password) are supported. Anyone nearby can join while it is active, so configure Neo Clock in a trusted environment. Wi-Fi credentials are stored in ESP32 NVS; flash encryption is not enabled by this firmware.

## GitHub Actions builds

There are two separate workflow files:

- `.github/workflows/build-firmware-via-wifi.yml` builds only `firmware/via_wifi` for ESP32-C3 on pushes, pull requests targeting `main`, and manual dispatch.
- `.github/workflows/build-firmware-via-chronos-app.yml` is a disabled placeholder. It does not compile or package anything yet.

After a successful Wi-Fi workflow run, download the `NeoFirmware-via-wifi-ESP32-C3-...` ZIP from the run's **Artifacts** section.

Chronos reference template: [Dirgamochi C3](https://github.com/MazBayID/Dirgamochi-C3_v0.7.0--BiG-Update)

## Asset workflow

The current clock digit styles remain built-in defaults under
`firmware/via_wifi/src/assets/default/digits/`.

Upload original UI assets and their manifest to `assets_master/`. They will be
reviewed first, then placed or converted into the appropriate runtime folders.
