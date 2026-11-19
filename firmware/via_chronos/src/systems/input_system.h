#ifndef NEO_CHRONOS_INPUT_SYSTEM_H
#define NEO_CHRONOS_INPUT_SYSTEM_H

#include "../config/config.h"

static bool previousPrevState = false;
static bool previousNextState = false;
static bool previousModeState = false;
static unsigned long lastInputChange = 0;

void setupInput() {
  pinMode(PIN_PREV, INPUT);
  pinMode(PIN_NEXT, INPUT);
  pinMode(PIN_MODE, INPUT);
}

void updateInput() {
  const bool prevState = digitalRead(PIN_PREV) == HIGH;
  const bool nextState = digitalRead(PIN_NEXT) == HIGH;
  const bool modeState = digitalRead(PIN_MODE) == HIGH;
  const unsigned long now = millis();

  if (now - lastInputChange >= 180) {
    if (prevState && !previousPrevState) {
      clockStyle = (clockStyle + 5) % 6;
      lastInputChange = now;
    } else if (nextState && !previousNextState) {
      clockStyle = (clockStyle + 1) % 6;
      lastInputChange = now;
    } else if (modeState && !previousModeState) {
      watch.set24Hour(!watch.is24Hour());
      lastInputChange = now;
    }
  }

  previousPrevState = prevState;
  previousNextState = nextState;
  previousModeState = modeState;
}

#endif
