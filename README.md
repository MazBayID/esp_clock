# NEO CLOCK Firmware

**NEO CLOCK** is the first device firmware built on the Neo Firmware foundation for the ESP32-C3 SuperMini.

The active firmware implementation is:

- **NEO CLOCK (via_wifi)** — the current product firmware, using Wi-Fi for network connectivity and automatic time synchronization.
- **via_chronos_app** — a future firmware track for the Chronos Android app. Development is postponed until NEO CLOCK is complete and stable.

## Repository layout

~~~text
assets_master/                         # Incoming original UI assets and manifest
firmware/
├── via_wifi/                          # NEO CLOCK product firmware
│   ├── via_wifi.ino
│   └── src/
│       ├── config/
│       ├── systems/
│       └── assets/default/
└── via_chronos_app/                   # Future Chronos BLE firmware
.github/workflows/
├── build-firmware-via-wifi.yml
└── build-firmware-via-chronos-app.yml
~~~

The via_wifi directory and sketch name are intentionally retained as implementation/build identifiers. The firmware product name is **NEO CLOCK**.

## Hardware target

- ESP32-C3 SuperMini
- SSD1306 OLED, 128x64, I2C
- MAX98357A I2S audio amplifier
- TTP223 touch inputs
- INMP441 I2S microphone (documented pin mapping; firmware integration is not enabled yet)

Pin assignments are defined in firmware/via_wifi/src/config/config.h and documented in [PINOUT.md](firmware/via_wifi/PINOUT.md).

## Current firmware capabilities

- Digital clock with three built-in digit styles.
- Wi-Fi connection using saved network profiles.
- NTP time synchronization through ntp.bmkg.go.id.
- Local Wi-Fi setup portal hosted as NEO_CLOCK.
- OLED status screen and web quote screen.
- Touch input and audio beep feedback.

## Wi-Fi setup portal

The firmware stores up to **5 Wi-Fi profiles** in ESP32 NVS Preferences.

1. At startup, NEO CLOCK scans for saved networks and tries the networks it can currently see, starting with the most recently added profile.
2. If no profile is saved, or no saved SSID is found or can be connected to, NEO CLOCK creates an open hotspot named NEO_CLOCK.
3. Connect a phone to NEO_CLOCK. The captive portal should open automatically. If it does not, open http://192.168.4.1 in a browser.
4. Select a nearby network from the scan list or enter its SSID manually, then enter the password. A profile is saved only after the connection succeeds.
5. When five profiles are already stored, adding a sixth distinct network removes the oldest profile (FIFO). Updating an existing SSID refreshes its password and moves it to the newest position. Failed connection attempts do not change the saved list.

The setup hotspot is open and intended for local configuration only. Anyone nearby can join while it is active, so configure NEO CLOCK in a trusted environment. Wi-Fi credentials are stored in ESP32 NVS; flash encryption is not enabled by this firmware.

## GitHub Actions builds

- .github/workflows/build-firmware-via-wifi.yml builds the NEO CLOCK firmware for ESP32-C3 on pushes, pull requests targeting main, and manual dispatch.
- .github/workflows/build-firmware-via-chronos-app.yml is a disabled placeholder. It does not compile or package anything yet.

After a successful build, download the **NEO-CLOCK-ESP32-C3** ZIP from the workflow run's **Artifacts** section.

## Asset workflow

The current clock digit styles remain built-in defaults under
firmware/via_wifi/src/assets/default/digits/.

Upload original UI assets and their manifest to assets_master/. They will be
reviewed first, then placed or converted into the appropriate runtime folders.
