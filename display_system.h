#ifndef JAM_DISPLAY_SYSTEM_H
#define JAM_DISPLAY_SYSTEM_H

#include "config.h"
#include "logo.h"
#include "style01.h"
#include "style02.h"
#include "style03.h"

// Mengambil bitmap digit sesuai gaya jam yang sedang aktif.
const unsigned char* getDigitBitmap(int number) {
  if (number < 0 || number > 9) return nullptr;

  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[number];
    case 1: return ClockStyle02::epd_bitmap_allArray[number];
    case 2: return ClockStyle03::epd_bitmap_allArray[number];
    default: return ClockStyle03::epd_bitmap_allArray[number];
  }
}

// Mengambil bitmap titik dua dari gaya jam yang aktif.
const unsigned char* getClockColonBitmap() {
  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[10];
    case 1: return ClockStyle02::epd_bitmap_allArray[10];
    case 2: return ClockStyle03::epd_bitmap_allArray[10];
    default: return ClockStyle03::epd_bitmap_allArray[10];
  }
}

// Menggambar satu digit bitmap.
void drawDigit(int x, int y, int number) {
  const unsigned char* bitmap = getDigitBitmap(number);
  if (bitmap == nullptr) return;

  display.drawBitmap(x, y, bitmap, 24, 24, SSD1306_WHITE);
}

// Inisialisasi OLED dan logo startup.
void setupDisplay() {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 Gagal"));
    for (;;) {}
  }

  display.setRotation(2);

  display.clearDisplay();
  display.drawBitmap(32, 0, epd_bitmap_neo_clock, 64, 64, SSD1306_WHITE);
  display.display();
}

// Menggambar layar sesuai state aplikasi saat ini.
void renderDisplay(const struct tm& timeinfo, bool isTimeAvail) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (currentScreen == 0) {
    if (isTimeAvail) {
      if (clockFace == 0) {
        // MODE JAM BITMAP
        drawCustomClock(timeinfo, 20, 20, 110, 10);
      } else {
        // MODE JAM BITMAP + TANGGAL (disimpan untuk menu berikutnya)
        drawCustomClock(timeinfo, 20, 20, 110, 10);

        display.setTextSize(1);
        display.setCursor(2, 55);

        String tgl = formatDuaDigit(timeinfo.tm_mday) + "/" +
                     formatDuaDigit(timeinfo.tm_mon + 1) + "/" +
                     String(timeinfo.tm_year + 1900);

        display.print("Tanggal: " + tgl);
      }
    } else {
      display.setTextSize(1);
      display.setCursor(10, 24);
      display.print(WiFi.status() == WL_CONNECTED
                      ? "Sinkronisasi NTP..."
                      : "Menunggu WiFi...");
    }
  }
  else if (currentScreen == 2) {
    int rssi = WiFi.RSSI();
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("Status: Connected");
    display.setCursor(10, 30);
    display.print("SSID: ");
    display.print(ssid);
    display.setCursor(10, 50);
    display.print("Sinyal RSSI: ");
    display.print(rssi);
    display.print(" dBm");
  }
  else if (currentScreen == 3) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Quote of the moment:");
    display.println();
    display.println(currentQuote);
  }

  display.display();
  delay(50);
}

#endif
