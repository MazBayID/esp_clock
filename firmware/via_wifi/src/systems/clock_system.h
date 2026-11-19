#ifndef JAM_CLOCK_SYSTEM_H
#define JAM_CLOCK_SYSTEM_H

#include "../config/config.h"

// --- FORMAT JAM TANPA STRFTIME ---
String formatDuaDigit(int angka) {
  if (angka < 10) return "0" + String(angka);
  return String(angka);
}

// --- JAM DIGITAL BITMAP CUSTOM ---
void drawCustomClock(
  const struct tm& timeinfo,
  int digitY,
  int colonY,
  int secondsX,
  int secondsY
) {
  int hour = timeinfo.tm_hour;
  int minute = timeinfo.tm_min;

  int digit1 = hour / 10;
  int digit2 = hour % 10;
  int digit3 = minute / 10;
  int digit4 = minute % 10;

  if (clockStyle <= 2) {
    // Layout legacy untuk bitmap 24x24.
    drawDigit(10, digitY, digit1);
    drawDigit(34, digitY, digit2);
    drawDigit(70, digitY, digit3);
    drawDigit(94, digitY, digit4);

    if (timeinfo.tm_sec % 2 == 0) {
      display.drawBitmap(60, colonY, getClockColonBitmap(),
                         8, 24, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setCursor(secondsX, secondsY);
    display.print(":");
    display.print(formatDuaDigit(timeinfo.tm_sec));
    return;
  }

  // Layout styles 04–06: lima slot bitmap berjajar.
  const int slotX[5] = {1, 26, 51, 76, 101};
  const int bitmapY = (SCREEN_HEIGHT - getClockDigitHeight()) / 2;

  drawDigit(slotX[0], bitmapY, digit1);
  drawDigit(slotX[1], bitmapY, digit2);
  drawDigit(slotX[3], bitmapY, digit3);
  drawDigit(slotX[4], bitmapY, digit4);

  if (timeinfo.tm_sec % 2 == 0) {
    display.drawBitmap(slotX[2], bitmapY, getClockColonBitmap(),
                       getClockDigitWidth(), getClockDigitHeight(),
                       SSD1306_WHITE);
  }
}

#endif
