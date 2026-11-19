# ESP32-C3 SuperMini — Hardware Pinout

Dokumentasi koneksi perangkat keras untuk firmware **Neo Firmware — via Wi-Fi**.

## Board

- **Microcontroller:** ESP32-C3 SuperMini

## Pin Mapping

### INMP441 — I2S Microphone

| INMP441 | ESP32-C3 SuperMini | Keterangan |
|---|---:|---|
| SCK | GPIO 1 | I2S Bit Clock (BCLK) |
| WS | GPIO 2 | I2S Word Select (WS/LRCLK) |
| SD | GPIO 10 | Serial audio data output |

### MAX98357A — I2S Audio Amplifier

| MAX98357A | ESP32-C3 SuperMini | Keterangan |
|---|---:|---|
| BCLK | GPIO 1 | I2S Bit Clock |
| LRC | GPIO 2 | I2S Left/Right Clock |
| DIN | GPIO 5 | Serial audio data input |

### TTP223 — Touch Inputs

| Fungsi | ESP32-C3 SuperMini | Keterangan |
|---|---:|---|
| PREV | GPIO 4 | Navigasi sebelumnya |
| NEXT | GPIO 6 | Navigasi berikutnya |
| OK / PAUSE / PLAY | GPIO 7 | Tombol konfirmasi dan kontrol |

### OLED — I2C Display

| OLED | ESP32-C3 SuperMini | Keterangan |
|---|---:|---|
| SCL | GPIO 20 | I2C Clock |
| SDA | GPIO 21 | I2C Data |

## Pin Summary

| GPIO | Perangkat | Sinyal |
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

- **Shared I2S clock:** INMP441 dan MAX98357A menggunakan jalur clock yang sama: GPIO 1 (BCLK/SCK) dan GPIO 2 (WS/LRC). Hubungkan kedua pin clock perangkat ke GPIO yang sama sesuai tabel.
- **Separate audio data:** INMP441 mengirim data audio melalui GPIO 10, sedangkan MAX98357A menerima data audio melalui GPIO 5.
- Pastikan konfigurasi driver I2S firmware mendukung penggunaan mikrofon dan amplifier pada clock bersama sebelum mengaktifkan keduanya secara bersamaan.
- Pastikan semua perangkat menggunakan **GND bersama**.
- Periksa kebutuhan tegangan dan level logika masing-masing modul sebelum menyalakan rangkaian.

> **Catatan:** Dokumen ini mencatat pemetaan pin yang direncanakan. Selalu cocokkan wiring fisik dengan label pin pada modul dan konfigurasi firmware.
