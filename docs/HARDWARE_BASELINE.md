# NEO_CLOCK Hardware Baseline

This document records the shared hardware baseline and the GPIO contract used by the Firmware Builder. It is a reference for the three independent firmware targets; it does not imply that every optional peripheral is enabled in every target.

## Board and display

- Board: ESP32-C3 SuperMini
- Display: SSD1306 OLED, 128×64, I²C address 0x3C

## Official default wiring

| Component | Signal | ESP32-C3 GPIO | Notes |
|---|---|---:|---|
| SSD1306 OLED | SDA | 21 | I²C data |
| SSD1306 OLED | SCL | 20 | I²C clock |
| MAX98357A | BCLK | 1 | I²S bit clock |
| MAX98357A | LRC / WS | 2 | I²S word select |
| MAX98357A | DIN | 5 | I²S data to amplifier |
| TTP223 | PREV / MINUS | 4 | Previous navigation |
| TTP223 | NEXT / PLUS | 6 | Next navigation |
| TTP223 | MODE / OK | 7 | Select / confirm |
| INMP441 | SCK | 1 | Shares I²S clock with amplifier |
| INMP441 | WS | 2 | Shares I²S word-select with amplifier |
| INMP441 | SD | 10 | Microphone data; not exposed by Builder v1 |
| Optional passive piezo (Mochi only) | Signal | 3 | Disabled by default; separate from MAX98357A |

All modules must share a common ground. Check each breakout board's supply-voltage and logic-level requirements before connecting power.

## Firmware variants

| Variant | Build system | Notes |
|---|---|---|
| `via_wifi` | Arduino CLI | Wi-Fi clock firmware. |
| `via_chronos` | Arduino CLI | Chronos BLE clock firmware. |
| `mochi_chronos` | PlatformIO | Companion-style Chronos firmware. Default optional audio/microphone pipeline is disabled; its speaker beep can use the existing MAX98357A path. |

The three variants are separate firmware targets. A successful build of one variant does not prove that another variant works on hardware.

## Firmware Builder GPIO contract

Builder v1 exposes these eight independent signals:

```json
{
  "oled_sda": 21,
  "oled_scl": 20,
  "speaker_bclk": 1,
  "speaker_lrc": 2,
  "speaker_din": 5,
  "button_prev": 4,
  "button_next": 6,
  "button_mode": 7
}
```

- Allowed GPIO list: 0, 1, 2, 3, 4, 5, 6, 7, 10, 20, 21.
- Each of the eight exposed signals must use a different GPIO.
- GPIO8 and GPIO9 are excluded because they are ESP32-C3 strapping pins.
- GPIO11–19 are excluded from this Builder profile; GPIO18/19 are reserved for native USB.
- GPIO2 is allowed because the official default map uses it for I²S LRC/WS, but it is a strapping pin. External circuitry on GPIO2 can affect boot. Treat rewiring this pin with care.
- The INMP441 data pin and optional piezo are not configurable in Builder v1.

The browser validates the selection and the backend and GitHub Actions configuration scripts validate it again. The server-side validation is authoritative.

## Physical validation record

The project owner has reported a successful end-to-end test: the website deployed, a firmware build completed, flashing succeeded, and the firmware ran on the physical board.

The exact firmware variant and the detailed peripheral-by-peripheral test results were not recorded in that report. Therefore this is evidence of one successful physical end-to-end test, not proof that all three variants or every optional peripheral have passed hardware validation.

Record future tests below before changing the baseline:

| Date | Variant | GPIO map | Build ID / source SHA | Flash method | Hardware result | Tester notes |
|---|---|---|---|---|---|---|
| To be recorded | To be confirmed | Official defaults unless noted | To be recorded | Browser / PC / Android | Owner-reported E2E success; exact variant not yet identified | Do not infer all variants tested |

## Hardware safety notes

- The full merged image is a clean install and erases the ESP32-C3's 4 MB flash, including saved settings.
- Do not enable simultaneous INMP441 capture and MAX98357A playback until the shared I²S clocking and driver behavior have been tested on the actual board.
- Keep optional Mochi piezo disabled unless a separate passive piezo is physically connected to GPIO3.
- If the board fails to boot after rewiring, remove external circuitry from strapping pins and retry with the known-good default map.
