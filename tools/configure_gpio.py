#!/usr/bin/env python3
"""Validate and apply the supported NEO_CLOCK ESP32-C3 GPIO map."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ALLOWED_GPIO = {0, 1, 2, 3, 4, 5, 6, 7, 10, 20, 21}
SIGNALS = (
    "oled_sda",
    "oled_scl",
    "speaker_bclk",
    "speaker_lrc",
    "speaker_din",
    "button_prev",
    "button_next",
    "button_mode",
)

DEFAULT_GPIO = {
    "oled_sda": 21,
    "oled_scl": 20,
    "speaker_bclk": 1,
    "speaker_lrc": 2,
    "speaker_din": 5,
    "button_prev": 4,
    "button_next": 6,
    "button_mode": 7,
}

VARIANTS = {
    "via_wifi": {
        "path": Path("firmware/via_wifi/src/config/config.h"),
        "macros": {
            "oled_sda": "PIN_SDA",
            "oled_scl": "PIN_SCL",
            "speaker_bclk": "I2S_BCLK",
            "speaker_lrc": "I2S_LRC",
            "speaker_din": "I2S_DIN",
            "button_prev": "PIN_PREV",
            "button_next": "PIN_NEXT",
            "button_mode": "PIN_MODE",
        },
    },
    "via_chronos": {
        "path": Path("firmware/via_chronos/src/config/config.h"),
        "macros": {
            "oled_sda": "PIN_SDA",
            "oled_scl": "PIN_SCL",
            "speaker_bclk": "I2S_BCLK",
            "speaker_lrc": "I2S_LRC",
            "speaker_din": "I2S_DIN",
            "button_prev": "PIN_PREV",
            "button_next": "PIN_NEXT",
            "button_mode": "PIN_MODE",
        },
    },
    "mochi_chronos": {
        "path": Path("firmware/mochi_chronos/include/dirgamochi_config.h"),
        "macros": {
            "oled_sda": "OLED_SDA_PIN",
            "oled_scl": "OLED_SCL_PIN",
            "speaker_bclk": "SPK_BCLK_PIN",
            "speaker_lrc": "SPK_LRC_PIN",
            "speaker_din": "SPK_DIN_PIN",
            "button_prev": "BTN_MINUS_PIN",
            "button_next": "BTN_PLUS_PIN",
            "button_mode": "BTN_OK_PIN",
        },
    },
}


def parse_config(raw: str) -> dict[str, int]:
    try:
        parsed: Any = json.loads(raw or "{}")
    except json.JSONDecodeError as exc:
        raise SystemExit(f"GPIO_CONFIG_JSON is not valid JSON: {exc}") from exc

    if not isinstance(parsed, dict):
        raise SystemExit("GPIO configuration must be a JSON object")

    unknown = sorted(set(parsed) - set(SIGNALS))
    if unknown:
        raise SystemExit("Unsupported GPIO signal(s): " + ", ".join(unknown))

    config = dict(DEFAULT_GPIO)
    for signal, pin in parsed.items():
        if isinstance(pin, bool) or not isinstance(pin, int):
            raise SystemExit(f"{signal} must be an integer GPIO number")
        if pin not in ALLOWED_GPIO:
            allowed = ", ".join(str(value) for value in sorted(ALLOWED_GPIO))
            raise SystemExit(f"{signal}: GPIO{pin} is not allowed. Allowed GPIOs: {allowed}")
        config[signal] = pin

    reverse: dict[int, str] = {}
    for signal, pin in config.items():
        if pin in reverse:
            raise SystemExit(
                f"GPIO conflict: {reverse[pin]} and {signal} both use GPIO{pin}"
            )
        reverse[pin] = signal

    return config


def replace_macro(header: Path, macro: str, value: int) -> None:
    source = header.read_text(encoding="utf-8")
    pattern = re.compile(
        rf"^(?P<prefix>\s*#\s*define\s+{re.escape(macro)}\s+)"
        rf"(?P<value>-?\d+)(?P<suffix>\s*(?://.*)?)$",
        re.MULTILINE,
    )
    updated, count = pattern.subn(
        lambda match: f"{match.group('prefix')}{value}{match.group('suffix')}",
        source,
    )
    if count != 1:
        raise SystemExit(
            f"Expected exactly one numeric #define for {macro} in {header}, found {count}"
        )
    if updated != source:
        header.write_text(updated, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--variant",
        required=True,
        choices=tuple(VARIANTS),
    )
    parser.add_argument(
        "--config-json",
        default=None,
        help="JSON object mapping supported signal names to GPIO numbers",
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path.cwd(),
        help=argparse.SUPPRESS,
    )
    args = parser.parse_args()

    raw = args.config_json
    if raw is None:
        import os
        raw = os.environ.get("GPIO_CONFIG_JSON", "{}")

    config = parse_config(raw)
    header = args.repo_root / VARIANTS[args.variant]["path"]
    if not header.is_file():
        raise SystemExit(f"Firmware configuration header not found: {header}")

    for signal, macro in VARIANTS[args.variant]["macros"].items():
        replace_macro(header, macro, config[signal])

    warnings = []
    if raw.strip() not in ("", "{}") and 2 in config.values():
        warnings.append(
            "GPIO2 is an ESP32-C3 strapping pin; attached circuitry may affect boot."
        )

    print(json.dumps({
        "variant": args.variant,
        "gpio": config,
        "warnings": warnings,
    }, sort_keys=True))


if __name__ == "__main__":
    main()
