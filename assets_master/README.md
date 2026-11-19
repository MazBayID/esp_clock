# Assets Master

This folder is the immutable source area for original UI PNG assets and their manifest.

Do not resize, recolor, rename, or replace the source PNGs here. Firmware-ready files are generated during the GitHub Actions build by `tools/process_assets.py`.

## Build pipeline

1. Original PNGs remain in `assets_master/`.
2. GitHub Actions installs Pillow and runs the asset processor before compiling firmware.
3. The processor resizes each asset to its own target dimensions, centers it on a black canvas, converts it to 1-bit monochrome, and emits Adafruit GFX-compatible PROGMEM arrays.
4. The generated header is written to `firmware/via_wifi/src/assets/generated/ui_assets.h` in the Actions workspace. It is not committed and is recreated on every build.
5. Firmware includes the generated header when available. A simple vector-style Wi-Fi icon fallback remains available for local builds that have not run the asset processor.

## Per-asset output sizes

| Asset group | Generated dimensions |
| --- | ---: |
| Wi-Fi meter frames | 18 × 16 px |
| Signal frames | 18 × 16 px |
| Battery frames | 24 × 14 px |
| Progress arc frames | 32 × 32 px |
| Progress bar frames | 48 × 8 px |
| Dynamic heart | 16 × 16 px |
| Bluetooth icon | 16 × 18 px |
| Static Wi-Fi icon | 16 × 14 px |
| Sun, moon, alarm, notification, heart icons | 16 × 16 px |
| ECG divider | 48 × 12 px |

The target sizes are selected per asset use, not by one global resize rule. Original clock digit styles remain separately managed under `firmware/via_wifi/src/assets/default/digits/`.
