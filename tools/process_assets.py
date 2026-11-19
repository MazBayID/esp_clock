#!/usr/bin/env python3
"""Generate compact monochrome firmware bitmaps from immutable assets_master PNGs."""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets_master"
OUTPUT = ROOT / "firmware/via_wifi/src/assets/generated/ui_assets.h"

ASSETS = {
    "wifi_0": ("dynamic/wifi-meter/wifi-meter_0bars.png", (18, 16)),
    "wifi_1": ("dynamic/wifi-meter/wifi-meter_1bars.png", (18, 16)),
    "wifi_2": ("dynamic/wifi-meter/wifi-meter_2bars.png", (18, 16)),
    "wifi_3": ("dynamic/wifi-meter/wifi-meter_3bars.png", (18, 16)),
}

def to_xbm(image: Image.Image) -> bytes:
    image = image.convert("1")
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
    print(f"Generated {OUTPUT.relative_to(ROOT)} ({len(ASSETS)} assets)")

if __name__ == "__main__":
    main()
