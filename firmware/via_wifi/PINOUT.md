# ESP32-C3 SuperMini — Hardware Pinout

Hardware connection reference for **Neo Firmware — via Wi-Fi**.

## Board

- **Microcontroller:** ESP32-C3 SuperMini

## Pin Mapping

### INMP441 — I2S Microphone

| INMP441 | ESP32-C3 SuperMini | Description |
|---|---:|---|
| SCK | GPIO 1 | I2S Bit Clock (BCLK) |
| WS | GPIO 2 | I2S Word Select (WS/LRCLK) |
| SD | GPIO 10 | Serial audio data output |

### MAX98357A — I2S Audio Amplifier

| MAX98357A | ESP32-C3 SuperMini | Description |
|---|---:|---|
| BCLK | GPIO 1 | I2S Bit Clock |
| LRC | GPIO 2 | I2S Left/Right Clock |
| DIN | GPIO 5 | Serial audio data input |

### TTP223 — Touch Inputs

| Function | ESP32-C3 SuperMini | Description |
|---|---:|---|
| PREV | GPIO 4 | Previous navigation |
| NEXT | GPIO 6 | Next navigation |
| OK / PAUSE / PLAY | GPIO 7 | Confirmation and playback control |

### OLED — I2C Display

| OLED | ESP32-C3 SuperMini | Description |
|---|---:|---|
| SCL | GPIO 20 | I2C Clock |
| SDA | GPIO 21 | I2C Data |

## Pin Summary

| GPIO | Device | Signal |
|---:|---|---|
| GPIO 1 | INMP441 + MAX98357A | I2S BCLK / SCK |
| GPIO 2 | INMP441 + MAX98357A | I2S WS / LRC |
| GPIO 4 | TTP223 | PREV |
| GPIO 5 | MAX98357A | I2S DIN |
| GPIO 6 | TTP223 | NEXT |
| GPIO 7 | TTP223 | OK / PAUSE / PLAY |
| GPIO 10 | INMP441 | I2S SD |
| GPIO 20 | OLED | I2C SCL |
| GPIO 21 | OLED | I2C SDA |

## Wiring Notes

- **Shared I2S clock:** The INMP441 and MAX98357A share the same clock lines: GPIO 1 (BCLK/SCK) and GPIO 2 (WS/LRC). Connect both devices' clock pins to the corresponding shared GPIO lines.
- **Separate audio data:** The INMP441 sends audio data through GPIO 10, while the MAX98357A receives audio data through GPIO 5.
- Verify that the firmware's I2S driver configuration supports using the microphone and amplifier with shared clock lines before enabling both devices simultaneously.
- Ensure that all devices share a **common GND**.
- Check each module's voltage and logic-level requirements before powering the circuit.

> **Note:** This document records the planned pin mapping. Always verify the physical wiring against the labels on each module and the firmware configuration.
