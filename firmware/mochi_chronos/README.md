# Mochi Chronos — ESP32-C3

A DasaiMochi-inspired BLE companion firmware for the **ESP32-C3 Super Mini** with a 128×64 SSD1306 OLED, animated bitmap moods, Chronos phone integration, and optional I2S audio.

This firmware lives in `firmware/mochi_chronos/` and is an independent build target. It does not share or modify the `via_wifi` or `via_chronos` firmware.

## Features

- Chronos BLE companion connection and phone-synced clock (12/24-hour setting).
- Animated bitmap face: idle moods, blinks, glances, and remote-touch reactions.
- Weather, notifications, navigation, music metadata, phone battery/charging, and phone model.
- Find Phone, alarms, contacts/SOS, QR/link relay, and quiet-hours/idle clock.
- On-device settings menu with NVS persistence.
- Sound effects through MAX98357A; optional I2S microphone/voice pipeline (disabled by default).
- Optional passive piezo buzzer on GPIO3.

Feature availability depends on the Chronos app, phone permissions, and the data it relays. Camera commands and health metrics are intentionally not implemented because this hardware has no camera or health sensors.

## Hardware / pin map

| Hardware | Connection |
|---|---|
| ESP32-C3 Super Mini | MCU |
| SSD1306 128×64 OLED | SDA GPIO21, SCL GPIO20, address 0x3C |
| INMP441 microphone | SCK GPIO1, WS GPIO2, SD GPIO10 |
| MAX98357A amplifier | BCLK GPIO1, LRC GPIO2, DIN GPIO5 |
| TTP223 touch buttons | MINUS GPIO4, PLUS GPIO6, OK GPIO7 |
| Optional passive piezo | GPIO3 (disabled by default) |

The microphone and amplifier share I2S clock pins. The ESP32-C3 has one I2S peripheral; do not attempt separate incompatible RX/TX clock configurations simultaneously. Full microphone/audio operation is experimental and disabled in the default build.

GPIO2, GPIO8, and GPIO9 are ESP32-C3 strapping pins. GPIO8/9 are unused in this pin map; GPIO2 is shared for I2S WS/LRC. Be mindful of the hardware state during reset.

## Controls

| Button | Short press | Long press |
|---|---|---|
| OK (GPIO7) | Open Phone/status screen; cycle item on Contacts/QR | Find Phone |
| PLUS (GPIO6) | Next screen | Open Settings |
| MINUS (GPIO4) | Previous screen | Return to clock |

In Settings, PLUS/MINUS navigate and OK changes/selects. MINUS long-press exits.

## Screen cycle

Face → Time → Weather → Notifications → Navigation → Music → Phone → Contacts → QR → Face.

## Repository layout

```
firmware/mochi_chronos/
├── assets/
│   ├── faces/              # mood animation PNG-frame archives
│   └── audio/              # voice clips archive (IMA ADPCM WAV)
├── docs/                   # hardware/design reference notes
├── include/                # headers, configuration, interfaces
├── lib/ChronosESP32/       # vendored Chronos BLE library
├── src/                    # firmware implementation
├── tools/generate_assets.py # archive → generated C++ assets
├── platformio.ini          # build environments
├── LICENSES.md
└── README.md
```

Generated asset files are build products, not source-of-truth files. The source archives are kept in `assets/`; generated C++ is written to `include/generated/` and `src/generated/` and should not be committed.

## Build locally

Install PlatformIO Core, Python 3, and Pillow (the generator can install Pillow into PlatformIO's Python environment when invoked by PlatformIO).

From the repository root:

```sh
cd firmware/mochi_chronos
pio run -e esp32-c3-supermini
```

Optional audio-enabled build:

```sh
pio run -e esp32-c3-supermini-audio
```

Upload and serial monitor:

```sh
pio run -e esp32-c3-supermini -t upload
pio device monitor
```

The default environment keeps `ENABLE_AUDIO=0` for safe initial bring-up. The audio environment sets it to 1; test only after the basic firmware boots and the speaker/mic wiring is verified.

## GitHub Actions

The **Build Mochi Chronos Firmware** workflow runs on pushes/PRs affecting this folder or its workflow, and can be started manually from Actions.

It generates assets, compiles the default and audio-enabled environments, and uploads build artifacts:

- `mochi-chronos-firmware` — default application binary and merged flash image.
- `mochi-chronos-audio-firmware` — optional audio-enabled application binary.

The default app binary is for updating an already configured board (app offset 0x10000). The merged image includes bootloader, partition table, and application and is intended for initial flashing at offset 0x0.

## Flashing

For a blank board / first flash, use the merged image at address `0x0`. For an app-only update, flash the application image at `0x10000`.

With esptool:

```sh
esptool.py --chip esp32c3 write_flash 0x0 mochi-chronos-merged.bin
```

## Pairing

1. Open the Chronos app.
2. Go to Watches → Watches → Pair New Devices → Search.
3. Select **Dirgamochi**.
4. Do not pair from Android's Bluetooth settings; use Chronos's in-app BLE pairing flow.

Grant Chronos the relevant Android permissions (notification access, background access, and navigation relay settings) for the corresponding features to work.

## Troubleshooting

- **No notifications:** enable Android Notification Access for Chronos.
- **No navigation data:** start navigation in a supported maps app and enable the relay in Chronos.
- **Stale BLE pairing:** use the firmware Settings → Reset BLE, then pair again in Chronos.
- **Boot loop:** hold BOOT, tap RESET, release BOOT, then flash; try a reliable USB cable; if necessary erase flash and reflash the merged image at 0x0.
- **No speaker audio:** check MAX98357A power, shutdown/gain configuration, and BCLK/LRC/DIN wiring. Keep optional audio disabled while troubleshooting the base firmware.
- **OLED orientation:** Settings → Rotate, or change the default `OLED_ROTATION` in `include/dirgamochi_config.h`.

## Asset replacement

- Face archives: `assets/faces/mochi_0.zip` through `mochi_9.zip`; PNG frames should be 98×64 and numbered for frame order.
- Voice archive: `assets/audio/mochi_voices.zip`; mood clips use `mochi_001.wav` … `mochi_010.wav` plus `mochi_blink.wav`, mono 16 kHz IMA ADPCM WAV.
- Run `python3 tools/generate_assets.py --force` to regenerate C++ asset files.

## License

See [LICENSES.md](LICENSES.md) for this project's license and third-party notices.
