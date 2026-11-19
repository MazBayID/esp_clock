import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "tools" / "configure_gpio.py"

MACROS = {
    "via_wifi": (
        "firmware/via_wifi/src/config/config.h",
        ["PIN_SDA", "PIN_SCL", "I2S_BCLK", "I2S_LRC", "I2S_DIN", "PIN_PREV", "PIN_NEXT", "PIN_MODE"],
    ),
    "via_chronos": (
        "firmware/via_chronos/src/config/config.h",
        ["PIN_SDA", "PIN_SCL", "I2S_BCLK", "I2S_LRC", "I2S_DIN", "PIN_PREV", "PIN_NEXT", "PIN_MODE"],
    ),
    "mochi_chronos": (
        "firmware/mochi_chronos/include/dirgamochi_config.h",
        ["OLED_SDA_PIN", "OLED_SCL_PIN", "SPK_BCLK_PIN", "SPK_LRC_PIN", "SPK_DIN_PIN", "BTN_MINUS_PIN", "BTN_PLUS_PIN", "BTN_OK_PIN"],
    ),
}


class ConfigureGpioTests(unittest.TestCase):
    def make_repo(self, root: Path) -> None:
        for relative, macros in MACROS.values():
            header = root / relative
            header.parent.mkdir(parents=True, exist_ok=True)
            header.write_text(
                "\n".join(f"#define {macro} {index + 1}" for index, macro in enumerate(macros)) + "\n",
                encoding="utf-8",
            )

    def run_config(self, root: Path, variant: str, config: dict) -> subprocess.CompletedProcess:
        return subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--variant",
                variant,
                "--config-json",
                json.dumps(config),
                "--repo-root",
                str(root),
            ],
            text=True,
            capture_output=True,
            check=False,
        )

    def test_default_map_is_valid_for_all_variants(self) -> None:
        for variant in MACROS:
            with self.subTest(variant=variant), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                self.make_repo(root)
                result = self.run_config(root, variant, {})
                self.assertEqual(result.returncode, 0, result.stderr)
                payload = json.loads(result.stdout)
                self.assertEqual(payload["gpio"]["oled_sda"], 21)
                self.assertEqual(payload["gpio"]["speaker_lrc"], 2)

    def test_custom_map_updates_variant_macros(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            result = self.run_config(root, "mochi_chronos", {"oled_sda": 10, "button_mode": 3})
            self.assertEqual(result.returncode, 0, result.stderr)
            header = (root / MACROS["mochi_chronos"][0]).read_text(encoding="utf-8")
            self.assertIn("#define OLED_SDA_PIN 10", header)
            self.assertIn("#define BTN_OK_PIN 3", header)

    def test_duplicate_gpio_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            result = self.run_config(root, "via_wifi", {"oled_sda": 20})
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("GPIO conflict", result.stderr)

    def test_strapping_pin_is_allowed_with_warning(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            result = self.run_config(root, "via_wifi", {"button_prev": 2, "speaker_lrc": 3})
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("strapping pin", result.stdout)

    def test_reserved_or_unsupported_gpio_is_rejected(self) -> None:
        for pin in (8, 9, 18, 19, 11, 17):
            with self.subTest(pin=pin), tempfile.TemporaryDirectory() as temp:
                root = Path(temp)
                self.make_repo(root)
                result = self.run_config(root, "via_wifi", {"button_prev": pin})
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("not allowed", result.stderr)

    def test_unknown_signal_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            result = self.run_config(root, "via_wifi", {"microphone_data": 10})
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Unsupported GPIO signal", result.stderr)

    def test_boolean_is_not_accepted_as_gpio_number(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            self.make_repo(root)
            result = self.run_config(root, "via_wifi", {"button_prev": True})
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("must be an integer", result.stderr)


if __name__ == "__main__":
    unittest.main()
