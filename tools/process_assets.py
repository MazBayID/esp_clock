#!/usr/bin/env python3
"""Resize original UI PNGs and generate compact monochrome firmware bitmaps."""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets_master"
OUTPUT = ROOT / "firmware/via_wifi/src/assets/generated/ui_assets.h"

ASSETS = {}

def add(name, relative_path, size):
    ASSETS[name] = (relative_path, size)

for level in range(4):
    add(f"wifi_{level}", f"dynamic/wifi-meter/wifi-meter_{level}bars.png", (18, 16))
    add(f"signal_{level}", f"dynamic/signal/signal_{level}bars.png", (18, 16))
for percent in range(0, 101, 10):
    add(f"battery_{percent}", f"dynamic/battery/battery_{percent}pct.png", (24, 14))
for percent in range(0, 101, 5):
    add(f"progress_arc_{percent}", f"dynamic/progress-arc/progress-arc_{percent}pct.png", (32, 32))
    add(f"progress_bar_{percent}", f"dynamic/progress-bar/progress-bar_{percent}pct.png", (48, 8))
add("heart_dynamic", "dynamic/heart-dynamic/heart-dynamic_base.png", (16, 16))

for name, filename, size in [
    ("bluetooth", "bluetooth_24x28.png", (16, 18)),
    ("wifi_static", "wifi-static_28x24.png", (16, 14)),
    ("weather_sun", "weather-sun_28x28.png", (16, 16)),
    ("weather_moon", "weather-moon_28x28.png", (16, 16)),
    ("alarm", "alarm_28x28.png", (16, 16)),
    ("notification", "notification_28x28.png", (16, 16)),
    ("heart_static", "heart-static_28x28.png", (16, 16)),
    ("ecg_divider", "ecg-divider_64x20.png", (48, 12)),
]:
    add(name, f"static/png/{filename}", size)

def to_xbm(image: Image.Image) -> bytes:
    width, height = image.size
    data = bytearray()
    for y in range(height):
        for byte_x in range((width + 7) // 8):
            value = 0
            for bit in range(8):
                x = byte_x * 8 + bit
                if x < width and image.getpixel((x, y)) != 0:
                    value |= 1 << (7 - bit)
            data.append(value)
    return bytes(data)

def main() -> None:
    lines = [
        "#ifndef NEO_GENERATED_UI_ASSETS_H",
        "#define NEO_GENERATED_UI_ASSETS_H",
        "#include <Arduino.h>",
        "namespace NeoUiAssets {",
    ]
    for name, (relative_path, size) in ASSETS.items():
        path = SOURCE / relative_path
        if not path.is_file():
            raise FileNotFoundError(f"Required source asset missing: {path}")
        with Image.open(path) as source:
            image = source.convert("RGBA")
            image.thumbnail(size, Image.Resampling.LANCZOS)
            canvas = Image.new("RGBA", size, (0, 0, 0, 255))
            x = (size[0] - image.width) // 2
            y = (size[1] - image.height) // 2
            canvas.alpha_composite(image, (x, y))
            gray = canvas.convert("L")
            mono = gray.point(lambda pixel: 255 if pixel >= 128 else 0, mode="1")
            data = to_xbm(mono)
        values = ",".join(f"0x{byte:02X}" for byte in data)
        lines.append(f"static const unsigned char {name}[] PROGMEM = {{{values}}};")
    lines.extend(["}", "#endif", ""])
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text("\n".join(lines), encoding="utf-8")
    print(f"Generated {OUTPUT.relative_to(ROOT)} ({len(ASSETS)} bitmaps)")

if __name__ == "__main__":
    main()
