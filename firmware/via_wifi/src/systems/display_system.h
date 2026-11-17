#ifndef NEO_FIRMWARE_DISPLAY_SYSTEM_H
#define NEO_FIRMWARE_DISPLAY_SYSTEM_H

#include "../config/config.h"
#include "../assets/default/logos/neo_logo.h"
#include "../assets/default/digits/style01.h"
#include "../assets/default/digits/style02.h"
#include "../assets/default/digits/style03.h"

const unsigned char* getDigitBitmap(int number) {
  if (number < 0 || number > 9) return nullptr;

  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[number];
    case 1: return ClockStyle02::epd_bitmap_allArray[number];
    case 2: return ClockStyle03::epd_bitmap_allArray[number];
    default: return ClockStyle03::epd_bitmap_allArray[number];
  }
}

const unsigned char* getClockColonBitmap() {
  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[10];
    case 1: return ClockStyle02::epd_bitmap_allArray[10];
    case 2: return ClockStyle03::epd_bitmap_allArray[10];
    default: return ClockStyle03::epd_bitmap_allArray[10];
  }
}

void drawDigit(int x, int y, int number) {
  const unsigned char* bitmap = getDigitBitmap(number);
  if (bitmap == nullptr) return;

  display.drawBitmap(x, y, bitmap, 24, 24, SSD1306_WHITE);
}

void setupDisplay() {
  Wire.begin(PIN_SDA, PIN_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 initialization failed"));
    for (;;) delay(100);
  }

  display.setRotation(2);
  display.clearDisplay();
  display.drawBitmap(32, 0, epd_bitmap_neo_clock, 64, 64, SSD1306_WHITE);
  display.display();
}

void renderDisplay(const struct tm& timeinfo, bool isTimeAvail) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (currentScreen == 0) {
    if (isTimeAvail) {
      drawCustomClock(timeinfo, 20, 20, 110, 10);

      if (clockFace != 0) {
        display.setTextSize(1);
        display.setCursor(2, 55);

        const String dateText =
          formatDuaDigit(timeinfo.tm_mday) + "/" +
          formatDuaDigit(timeinfo.tm_mon + 1) + "/" +
          String(timeinfo.tm_year + 1900);

        display.print("Tanggal: " + dateText);
      }
    } else {
      display.setTextSize(1);
      display.setCursor(10, 24);
      display.print(WiFi.status() == WL_CONNECTED
                      ? "Sinkronisasi NTP..."
                      : "Menunggu WiFi...");
    }
  } else if (currentScreen == 2) {
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print(WiFi.status() == WL_CONNECTED
                    ? "Status: Connected"
                    : "Status: Offline");
    display.setCursor(10, 30);
    display.print("SSID: ");
    display.print(ssid);
    display.setCursor(10, 50);
    display.print("Sinyal RSSI: ");
    display.print(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
    display.print(" dBm");
  } else if (currentScreen == 3) {
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
