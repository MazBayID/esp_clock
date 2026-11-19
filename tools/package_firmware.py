#!/usr/bin/env python3
"""Create consistent NEO_CLOCK firmware download packages.

This script only packages already-built firmware. It does not compile firmware
or accept arbitrary user-supplied code/configuration.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import zipfile
from pathlib import Path

BOARD = "ESP32-C3 SuperMini"
CHIP = "esp32c3"
FLASH_MODE = "dio"
FLASH_FREQ = "80m"
FLASH_SIZE = "4MB"
APP_OFFSET = 0x10000
PARTITION_OFFSET = 0x8000
BOOTLOADER_OFFSET = 0x0000
BOOT_APP0_OFFSET = 0xE000
BOOT_APP0_SIZE = 0x2000


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def copy_required(source: Path, destination: Path) -> None:
    if not source.is_file():
        raise SystemExit(f"Required build output not found: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)


def validate_segment(merged: bytes, source: Path, offset: int, label: str) -> None:
    data = source.read_bytes()
    if merged[offset:offset + len(data)] != data:
        raise SystemExit(f"Merged image does not contain {label} at 0x{offset:04X}")


def write_instructions(path: Path, variant: str, has_boot_app0: bool) -> None:
    lines = [
        "NEO_CLOCK — Advanced firmware package",
        f"Firmware variant: {variant}",
        f"Target: {BOARD} ({CHIP})",
        "",
        "CLEAN INSTALL / FULL FIRMWARE FLASH",
        "Use all files below with the exact offsets shown:",
        f"  bootloader.bin       0x{BOOTLOADER_OFFSET:04X}",
        f"  partitions.bin       0x{PARTITION_OFFSET:04X}",
    ]
    if has_boot_app0:
        lines.append(f"  boot_app0.bin        0x{BOOT_APP0_OFFSET:04X}")
    lines.extend([
        f"  firmware.bin         0x{APP_OFFSET:04X}",
        "",
        f"Flash settings: mode={FLASH_MODE}, frequency={FLASH_FREQ}, size={FLASH_SIZE}",
        "Chip selection: ESP32-C3",
        "",
        "IMPORTANT:",
        "- Use files from this ZIP together; never mix files from different builds.",
        "- A full install overwrites the corresponding flash regions and resets",
        "  existing firmware data/configuration in those regions.",
        "- Disconnect/reconnect the board if the serial port disappears after reset.",
        "- If flashing fails, hold BOOT, tap RESET, release BOOT, then retry.",
        "",
        "For a simpler installation, use the separate merged.bin at flash offset 0x0000.",
        "",
        "Example PC command (replace PORT with the serial port):",
        "  esptool --chip esp32c3 --port PORT --baud 460800 write-flash --flash-mode dio --flash-freq 80m --flash-size 4MB",
        "  0x0000 bootloader.bin 0x8000 partitions.bin",
        *(["  0xE000 boot_app0.bin"] if has_boot_app0 else []),
        "  0x10000 firmware.bin",
    ])
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def package_arduino(build_dir: Path, out_dir: Path) -> dict:
    merged_matches = sorted(build_dir.glob("*.merged.bin"))
    boot_matches = sorted(build_dir.glob("*.bootloader.bin"))
    partition_matches = sorted(build_dir.glob("*.partitions.bin"))
    app_matches = [
        p for p in sorted(build_dir.glob("*.bin"))
        if not any(token in p.name for token in (".merged.", ".bootloader.", ".partitions."))
    ]
    if not (len(merged_matches) == len(boot_matches) == len(partition_matches) == len(app_matches) == 1):
        raise SystemExit(
            "Expected exactly one Arduino merged, bootloader, partition-table, and app BIN; "
            f"found merged={len(merged_matches)}, bootloader={len(boot_matches)}, "
            f"partitions={len(partition_matches)}, app={len(app_matches)} in {build_dir}"
        )

    merged = merged_matches[0].read_bytes()
    if len(merged) < BOOT_APP0_OFFSET + BOOT_APP0_SIZE:
        raise SystemExit("Arduino merged image is too small to contain boot_app0 data")
    boot_app0 = merged[BOOT_APP0_OFFSET:BOOT_APP0_OFFSET + BOOT_APP0_SIZE]
    if all(byte == 0xFF for byte in boot_app0):
        raise SystemExit("Expected boot_app0 data at 0xE000 was not found in merged image")

    validate_segment(merged, boot_matches[0], BOOTLOADER_OFFSET, "bootloader")
    validate_segment(merged, partition_matches[0], PARTITION_OFFSET, "partition table")
    validate_segment(merged, app_matches[0], APP_OFFSET, "application")
    copy_required(merged_matches[0], out_dir / "merged.bin")
    advanced_dir = out_dir / "_advanced"
    copy_required(boot_matches[0], advanced_dir / "bootloader.bin")
    copy_required(partition_matches[0], advanced_dir / "partitions.bin")
    copy_required(app_matches[0], advanced_dir / "firmware.bin")
    (advanced_dir / "boot_app0.bin").write_bytes(boot_app0)
    return {"has_boot_app0": True, "app_file": app_matches[0]}


def package_platformio(build_dir: Path, out_dir: Path) -> dict:
    merged_source = build_dir / "merged-firmware.bin"
    if not merged_source.is_file():
        raise SystemExit(f"Required merged image not found: {merged_source}")
    merged = merged_source.read_bytes()
    bootloader = build_dir / "bootloader.bin"
    partitions = build_dir / "partitions.bin"
    app_file = build_dir / "firmware.bin"
    validate_segment(merged, bootloader, BOOTLOADER_OFFSET, "bootloader")
    validate_segment(merged, partitions, PARTITION_OFFSET, "partition table")
    validate_segment(merged, app_file, APP_OFFSET, "application")
    copy_required(merged_source, out_dir / "merged.bin")
    advanced_dir = out_dir / "_advanced"
    copy_required(bootloader, advanced_dir / "bootloader.bin")
    copy_required(partitions, advanced_dir / "partitions.bin")
    copy_required(app_file, advanced_dir / "firmware.bin")
    return {"has_boot_app0": False, "app_file": app_file}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--variant", required=True, choices=("via_wifi", "via_chronos", "mochi_chronos"))
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--git-sha", required=True)
    args = parser.parse_args()

    if args.output_dir.exists():
        shutil.rmtree(args.output_dir)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    if args.variant == "mochi_chronos":
        result = package_platformio(args.build_dir, args.output_dir)
    else:
        result = package_arduino(args.build_dir, args.output_dir)

    merged_path = args.output_dir / "merged.bin"
    app_path = result["app_file"]
    info = {
        "schema_version": 1,
        "product": "NEO_CLOCK",
        "variant": args.variant,
        "board": BOARD,
        "chip": CHIP,
        "flash": {"mode": FLASH_MODE, "frequency": FLASH_FREQ, "size": FLASH_SIZE},
        "images": {
            "merged": {
                "file": "merged.bin",
                "address": "0x0000",
                "size_bytes": merged_path.stat().st_size,
                "sha256": sha256(merged_path),
            },
            "application": {
                "file": "firmware.bin",
                "address": f"0x{APP_OFFSET:04X}",
                "size_bytes": app_path.stat().st_size,
                "sha256": sha256(app_path),
            },
        },
        "advanced_offsets": {
            "bootloader.bin": f"0x{BOOTLOADER_OFFSET:04X}",
            "partitions.bin": f"0x{PARTITION_OFFSET:04X}",
            **({"boot_app0.bin": f"0x{BOOT_APP0_OFFSET:04X}"} if result["has_boot_app0"] else {}),
            "firmware.bin": f"0x{APP_OFFSET:04X}",
        },
        "git_sha": args.git_sha,
        "clean_install": True,
    }
    (args.output_dir / "build-info.json").write_text(json.dumps(info, indent=2) + "\n", encoding="utf-8")

    advanced_dir = args.output_dir / "_advanced"
    write_instructions(advanced_dir / "FLASH-INSTRUCTIONS.txt", args.variant, result["has_boot_app0"])
    shutil.copy2(args.output_dir / "build-info.json", advanced_dir / "build-info.json")
    with zipfile.ZipFile(args.output_dir / "advanced-bin.zip", "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for item in sorted(advanced_dir.iterdir()):
            archive.write(item, item.name)
    shutil.rmtree(advanced_dir)
    print(f"Packaged {args.variant}: {merged_path.stat().st_size} byte merged image; advanced-bin.zip created")


if __name__ == "__main__":
    main()
