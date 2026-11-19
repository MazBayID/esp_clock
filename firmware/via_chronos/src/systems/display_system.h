#ifndef NEO_CHRONOS_DISPLAY_SYSTEM_H
#define NEO_CHRONOS_DISPLAY_SYSTEM_H

#include "../config/config.h"

void renderDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("NEO CLOCK"));
  display.setCursor(91, 0);
  display.print(chronosConnected ? F("BLE ON") : F("BLE OFF"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  if (chronosTimeSynced) {
    display.setTextSize(2);
    display.setCursor(13, 22);
    display.print(watch.getTime("%H:%M"));

    display.setTextSize(1);
    display.setCursor(0, 51);
    display.print(watch.getTime("%d/%m/%Y"));
    display.setCursor(91, 51);
    display.print(watch.getTime("%S"));
  } else {
    display.setTextSize(1);
    display.setCursor(8, 25);
    display.print(chronosConnected ? F("SYNCING TIME...") : F("PAIR WITH CHRONOS"));
    display.setCursor(8, 39);
    display.print(chronosConnected ? F("Waiting for time") : F("Chronos > Watches"));
  }

  display.display();
}

#endif
