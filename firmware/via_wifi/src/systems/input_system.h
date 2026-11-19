#ifndef NEO_INPUT_SYSTEM_H
#define NEO_INPUT_SYSTEM_H

#include "../config/config.h"

void updateInput() {
  const bool signalState = digitalRead(PIN_PREV);
  const bool nextState = digitalRead(PIN_NEXT);
  const bool menuState = digitalRead(PIN_MODE);

  if (signalState == HIGH && lastTalkState == LOW) {
    beep(1);
    currentScreen = 2;
    signalTimer = millis();
  }

  if (nextState == HIGH && lastNextState == LOW) {
    beep(1);
    clockStyle = (clockStyle + 1) % 6;
    currentScreen = 0;
    Serial.print(F("Active clock style: Style 0"));
    Serial.println(clockStyle + 1);
  }

  if (menuState == HIGH && lastModeState == LOW) {
    beep(1);
    currentScreen = currentScreen == 1 ? 0 : 1;
  }

  lastTalkState = signalState;
  lastNextState = nextState;
  lastModeState = menuState;

  if (currentScreen == 2 && millis() - signalTimer > 8000) {
    currentScreen = 0;
  }
}

#endif
