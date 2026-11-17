#ifndef JAM_CLOCK_SYSTEM_H
#define JAM_CLOCK_SYSTEM_H

#include "config.h"

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

  // Menggambar empat digit HH:MM
  drawDigit(10, digitY, digit1);
  drawDigit(34, digitY, digit2);
  drawDigit(70, digitY, digit3);
  drawDigit(94, digitY, digit4);

  // Titik dua berkedip setiap detik
  if (timeinfo.tm_sec % 2 == 0) {
    display.drawBitmap(
      60, colonY,
      getClockColonBitmap(),
      8, 24,
      SSD1306_WHITE
    );
  }

  // Informasi detik
  display.setTextSize(1);
  display.setCursor(secondsX, secondsY);
  display.print(":");
  display.print(formatDuaDigit(timeinfo.tm_sec));
}

#endif
