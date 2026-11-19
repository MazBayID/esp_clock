#ifndef NEO_CHRONOS_DISPLAY_SYSTEM_H
#define NEO_CHRONOS_DISPLAY_SYSTEM_H

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
#define NEO_CHRONOS_HAS_UI_ASSETS 1
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
    default: return ClockStyle01::epd_bitmap_allArray[number];
  }
}

const unsigned char* getColonBitmap() {
  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[10];
    case 1: return ClockStyle02::epd_bitmap_allArray[10];
    case 2: return ClockStyle03::epd_bitmap_allArray[10];
    case 3: return ClockStyle04::ClockStyle04_allArray[10];
    case 4: return ClockStyle05::ClockStyle05_allArray[10];
    case 5: return ClockStyle06::ClockStyle06_allArray[10];
    default: return ClockStyle01::epd_bitmap_allArray[10];
  }
}

int getDigitWidth() {
  return clockStyle >= 3 ? 25 : 24;
}

int getDigitHeight() {
  if (clockStyle == 3 || clockStyle == 5) return 48;
  if (clockStyle == 4) return 25;
  return 24;
}

void drawClockDigit(int x, int y, int number) {
  const unsigned char* bitmap = getDigitBitmap(number);
  if (bitmap) {
    display.drawBitmap(x, y, bitmap, getDigitWidth(), getDigitHeight(), SSD1306_WHITE);
  }
}

void setupDisplay() {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 initialization failed"));
    while (true) delay(100);
  }
  display.setRotation(2);
  display.clearDisplay();
  display.display();
}

void renderDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (!chronosTimeSynced) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("NEO CLOCK"));
#ifdef NEO_CHRONOS_HAS_UI_ASSETS
    display.drawBitmap(0, 22, NeoUiAssets::bluetooth, 16, 18, SSD1306_WHITE);
#endif
    display.setCursor(22, 25);
    display.print(chronosConnected ? F("WAITING FOR TIME") : F("PAIR WITH CHRONOS"));
    display.setCursor(22, 39);
    display.print(chronosConnected ? F("SYNC IN PROGRESS") : F("WATCHES > PAIR NEW"));
    display.display();
    return;
  }

  const int hour = watch.getHourC();
  const int minute = watch.getMinute();
  const int digits[4] = {hour / 10, hour % 10, minute / 10, minute % 10};

  if (clockStyle <= 2) {
    const int y = 20;
    drawClockDigit(10, y, digits[0]);
    drawClockDigit(34, y, digits[1]);
    drawClockDigit(70, y, digits[2]);
    drawClockDigit(94, y, digits[3]);
    if (watch.getSecond() % 2 == 0) {
      display.drawBitmap(60, y, getColonBitmap(), 8, 24, SSD1306_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print(watch.getTime("%d/%m/%Y"));
    display.setCursor(105, 55);
    display.print(chronosConnected ? F("BT") : F("--"));
  } else {
    const int y = (SCREEN_HEIGHT - getDigitHeight()) / 2;
    const int slotX[5] = {1, 26, 51, 76, 101};
    drawClockDigit(slotX[0], y, digits[0]);
    drawClockDigit(slotX[1], y, digits[1]);
    drawClockDigit(slotX[3], y, digits[2]);
    drawClockDigit(slotX[4], y, digits[3]);
    if (watch.getSecond() % 2 == 0) {
      display.drawBitmap(slotX[2], y, getColonBitmap(),
                         getDigitWidth(), getDigitHeight(), SSD1306_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(chronosConnected ? F("BLE") : F("OFF"));
  }

  display.display();
}

#endif
