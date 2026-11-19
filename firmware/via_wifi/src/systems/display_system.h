#ifndef NEO_FIRMWARE_DISPLAY_SYSTEM_H
#define NEO_FIRMWARE_DISPLAY_SYSTEM_H

#include "../config/config.h"
#include "../assets/default/logos/neo_logo.h"
#include "../assets/default/digits/style01.h"
#include "../assets/default/digits/style02.h"
#include "../assets/default/digits/style03.h"
#include "../assets/default/digits/style04.h"
#include "../assets/default/digits/style05.h"
#include "../assets/default/digits/style06.h"
#if __has_include("../assets/generated/ui_assets.h")
#include "../assets/generated/ui_assets.h"
#define NEO_HAS_GENERATED_UI_ASSETS 1
#endif

const unsigned char* getDigitBitmap(int number) {
  if (number < 0 || number > 9) return nullptr;

  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[number];
    case 1: return ClockStyle02::epd_bitmap_allArray[number];
    case 2: return ClockStyle03::epd_bitmap_allArray[number];
    case 3: return ClockStyle04::ClockStyle04_allArray[number];
    case 4: return ClockStyle05::ClockStyle05_allArray[number];
    case 5: return ClockStyle06::ClockStyle06_allArray[number];
    default: return ClockStyle03::epd_bitmap_allArray[number];
  }
}

int getClockDigitWidth() {
  return clockStyle >= 3 ? 25 : 24;
}

int getClockDigitHeight() {
  if (clockStyle == 3 || clockStyle == 5) return 48;
  if (clockStyle == 4) return 25;
  return 24;
}

const unsigned char* getClockColonBitmap() {
  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[10];
    case 1: return ClockStyle02::epd_bitmap_allArray[10];
    case 2: return ClockStyle03::epd_bitmap_allArray[10];
    case 3: return ClockStyle04::ClockStyle04_allArray[10];
    case 4: return ClockStyle05::ClockStyle05_allArray[10];
    case 5: return ClockStyle06::ClockStyle06_allArray[10];
    default: return ClockStyle03::epd_bitmap_allArray[10];
  }
}

void drawDigit(int x, int y, int number) {
  const unsigned char* bitmap = getDigitBitmap(number);
  if (bitmap == nullptr) return;

  display.drawBitmap(x, y, bitmap, getClockDigitWidth(),
                    getClockDigitHeight(), SSD1306_WHITE);
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
  delay(1800);
}

void drawWiFiIcon(int x, int y, uint8_t bars) {
#ifdef NEO_HAS_GENERATED_UI_ASSETS
  const unsigned char* icon = nullptr;
  switch (bars) {
    case 0: icon = NeoUiAssets::wifi_0; break;
    case 1: icon = NeoUiAssets::wifi_1; break;
    case 2: icon = NeoUiAssets::wifi_2; break;
    default: icon = NeoUiAssets::wifi_3; break;
  }
  display.drawBitmap(x, y, icon, 18, 16, SSD1306_WHITE);
#else
  display.drawPixel(x + 8, y + 13, SSD1306_WHITE);
  if (bars >= 1) display.drawLine(x + 5, y + 10, x + 11, y + 10, SSD1306_WHITE);
  if (bars >= 2) display.drawLine(x + 2, y + 7, x + 14, y + 7, SSD1306_WHITE);
  if (bars >= 3) display.drawLine(x, y + 4, x + 16, y + 4, SSD1306_WHITE);
#endif
}

uint8_t getSignalBars(int rssi) {
  if (rssi == 0 || WiFi.status() != WL_CONNECTED) return 0;
  if (rssi >= -55) return 3;
  if (rssi >= -67) return 2;
  if (rssi >= -78) return 1;
  return 0;
}

void renderDisplay(const struct tm& timeinfo, bool isTimeAvail) {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (currentScreen == 0) {
    if (isTimeAvail) {
      drawCustomClock(timeinfo, 20, 20, 110, 10);

      if (clockFace != 0 && clockStyle != 3 && clockStyle != 5) {
        display.setTextSize(1);
        display.setCursor(2, 55);
        const String dateText =
          formatDuaDigit(timeinfo.tm_mday) + "/" +
          formatDuaDigit(timeinfo.tm_mon + 1) + "/" +
          String(timeinfo.tm_year + 1900);
        display.print(dateText);
      }
    } else {
      display.setTextSize(1);
      if (WiFi.status() == WL_CONNECTED) {
        display.setCursor(10, 24);
        display.print("SYNCING TIME...");
      } else if (WiFi.getMode() == WIFI_AP_STA) {
        display.setCursor(0, 0);
        display.print("CONNECT TO WIFI");
        display.setCursor(0, 16);
        display.print("SSID: NEO_CLOCK");

        display.setCursor(0, 38);
        display.print("OPEN YOUR BROWSER");
        display.setCursor(0, 50);
        display.print(WiFi.softAPIP().toString());
      } else {
        display.setCursor(10, 24);
        display.print("CONNECTING TO WIFI");
      }
    }
  } else if (currentScreen == 1) {
    display.drawBitmap(32, 0, epd_bitmap_neo, 64, 64, SSD1306_WHITE);
  } else if (currentScreen == 3) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("MENU");
    display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
    const char* items[] = {"Clock Style", "Wi-Fi Status", "System Info"};
    for (uint8_t i = 0; i < 3; ++i) {
      const int y = 17 + i * 14;
      if (menuSelection == i) {
        display.fillRect(0, y - 2, 128, 12, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(4, y);
        display.print(">");
        display.setCursor(16, y);
        display.print(items[i]);
        display.setTextColor(SSD1306_WHITE);
      } else {
        display.setCursor(16, y);
        display.print(items[i]);
      }
    }
    display.setCursor(0, 56);
    display.print("UP/DN  OK:SELECT");
  } else if (currentScreen == 4) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("SYSTEM INFORMATION");
    display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
    display.setCursor(0, 17);
    display.print("DEVICE: NEO CLOCK");
    display.setCursor(0, 30);
    display.print("BOARD: ESP32-C3");
    display.setCursor(0, 43);
    display.print("WIFI: ");
    display.print(WiFi.status() == WL_CONNECTED ? "ONLINE" : "OFFLINE");
    display.setCursor(0, 56);
    display.print("TIME: ");
    display.print(isTimeAvail ? "SYNCED" : "NOT SYNCED");
  } else if (currentScreen == 5) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("CLOCK STYLE");
    display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(25, 22);
    display.print("STYLE 0");
    display.print(clockStyle + 1);
    display.setTextSize(1);
    display.setCursor(0, 52);
    display.print("UP/DOWN TO CHANGE");
    display.setCursor(0, 56);
    display.print("OK:BACK  HOLD:EXIT");
  } else if (currentScreen == 2) {
    static unsigned long lastRssiSample = 0;
    static int sampledRssi = 0;
    if (millis() - lastRssiSample >= 1000 || lastRssiSample == 0) {
      lastRssiSample = millis();
      sampledRssi = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
    }

    const bool connected = WiFi.status() == WL_CONNECTED;
    const uint8_t bars = connected ? getSignalBars(sampledRssi) : 0;

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WI-FI STATUS");
    drawWiFiIcon(105, 0, bars);

    display.setCursor(0, 19);
    display.print(connected ? "CONNECTED" : "OFFLINE");

    display.setCursor(0, 32);
    display.print("NETWORK");
    display.setCursor(48, 32);
    display.print(connected ? WiFi.SSID().substring(0, 12) : "--");

    display.setCursor(0, 45);
    display.print("SIGNAL");
    display.setCursor(48, 45);
    if (connected) {
      display.print(sampledRssi);
      display.print(" dBm");
    } else {
      display.print("-- dBm");
    }

    display.drawRect(0, 58, 128, 5, SSD1306_WHITE);
    const int barWidth = connected ? (bars * 42) : 0;
    if (barWidth > 0) display.fillRect(1, 59, barWidth, 3, SSD1306_WHITE);
  }

  display.display();
  delay(50);
}

#endif
