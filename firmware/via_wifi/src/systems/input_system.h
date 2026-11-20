#ifndef NEO_INPUT_SYSTEM_H
#define NEO_INPUT_SYSTEM_H

#include "../config/config.h"

namespace {
Preferences menuPreferences;
bool menuPreferencesReady = false;
bool modePressActive = false;
bool modeLongHandled = false;
unsigned long modePressStarted = 0;
bool rawPrevState = LOW;
bool stablePrevState = LOW;
bool rawNextState = LOW;
bool stableNextState = LOW;
bool rawModeState = LOW;
bool stableModeState = LOW;
bool wifiStatusFromMenu = false;
unsigned long prevDebounceStarted = 0;
unsigned long nextDebounceStarted = 0;
unsigned long modeDebounceStarted = 0;
constexpr unsigned long INPUT_DEBOUNCE_MS = 35;
constexpr unsigned long MENU_LONG_PRESS_MS = 700;
constexpr unsigned long MENU_INTRO_MS = 900;

bool updateDebouncedButton(uint8_t pin, bool& rawState, bool& stableState,
                           unsigned long& debounceStarted,
                           unsigned long now) {
  const bool reading = digitalRead(pin) == HIGH;

  if (reading != rawState) {
    rawState = reading;
    debounceStarted = now;
  }

  if (now - debounceStarted >= INPUT_DEBOUNCE_MS &&
      stableState != rawState) {
    stableState = rawState;
    return stableState;
  }

  return false;
}

void saveClockStyle() {
  if (menuPreferencesReady) menuPreferences.putUChar("clockStyle", clockStyle);
}

void openSelectedMenuItem() {
  switch (menuSelection) {
    case 0:
      currentScreen = 5; // Clock style selector.
      break;
    case 1:
      currentScreen = 2; // Wi-Fi status.
      wifiStatusFromMenu = true;
      signalTimer = millis();
      break;
    case 2:
      currentScreen = 4; // System information.
      break;
  }
}
}

void setupInput() {
  pinMode(PIN_PREV, INPUT);
  pinMode(PIN_NEXT, INPUT);
  pinMode(PIN_MODE, INPUT);

  // Start from the physical levels so a button held during boot is not
  // interpreted as a fresh press after the debounce interval.
  const unsigned long now = millis();
  rawPrevState = stablePrevState = digitalRead(PIN_PREV) == HIGH;
  rawNextState = stableNextState = digitalRead(PIN_NEXT) == HIGH;
  rawModeState = stableModeState = digitalRead(PIN_MODE) == HIGH;
  prevDebounceStarted = now;
  nextDebounceStarted = now;
  modeDebounceStarted = now;
  modePressActive = false;
  modeLongHandled = false;
}

void setupMenuSettings() {
  menuPreferencesReady = menuPreferences.begin("neo-ui", false);
  if (menuPreferencesReady) {
    const uint8_t savedStyle = menuPreferences.getUChar("clockStyle", clockStyle);
    if (savedStyle < 6) clockStyle = savedStyle;
  }
}

void updateInput() {
  const unsigned long now = millis();
  const bool prevPressed = updateDebouncedButton(
      PIN_PREV, rawPrevState, stablePrevState, prevDebounceStarted, now);
  const bool nextPressed = updateDebouncedButton(
      PIN_NEXT, rawNextState, stableNextState, nextDebounceStarted, now);

  const bool modeReading = digitalRead(PIN_MODE) == HIGH;
  if (modeReading != rawModeState) {
    rawModeState = modeReading;
    modeDebounceStarted = now;
  }
  if (now - modeDebounceStarted >= INPUT_DEBOUNCE_MS &&
      stableModeState != rawModeState) {
    stableModeState = rawModeState;
    if (stableModeState == HIGH) {
      modePressActive = true;
      modeLongHandled = false;
      modePressStarted = now;
    } else if (modePressActive) {
      if (!modeLongHandled) {
        beep(1);
        if (currentScreen == 3) {
          openSelectedMenuItem();
        } else if (currentScreen == 5) {
          currentScreen = 0; // Return directly to the clock.
        } else if (currentScreen == 2 || currentScreen == 4) {
          currentScreen = 3;
        }
      }
      modePressActive = false;
    }
  }

  if (modePressActive && !modeLongHandled &&
      now - modePressStarted >= MENU_LONG_PRESS_MS) {
    modeLongHandled = true;
    beep(1);
    if (currentScreen == 0) {
      currentScreen = 1; // Hold to enter the menu through the NEO intro.
      signalTimer = now;
    } else if (currentScreen == 1 || currentScreen == 3 ||
               currentScreen == 5 || currentScreen == 2 ||
               currentScreen == 4) {
      currentScreen = 0;
    }
  }

  if (currentScreen == 1 && now - signalTimer >= MENU_INTRO_MS) {
    currentScreen = 3;
    menuSelection = 0;
  }

  if (currentScreen == 3) {
    if (prevPressed) {
      beep(1);
      menuSelection = (menuSelection + 2) % 3;
    }
    if (nextPressed) {
      beep(1);
      menuSelection = (menuSelection + 1) % 3;
    }
  } else if (currentScreen == 5) {
    if (prevPressed) {
      beep(1);
      clockStyle = (clockStyle + 5) % 6;
      saveClockStyle();
    }
    if (nextPressed) {
      beep(1);
      clockStyle = (clockStyle + 1) % 6;
      saveClockStyle();
    }
  } else if (currentScreen == 0) {
    if (prevPressed) {
      beep(1);
      currentScreen = 2;
      wifiStatusFromMenu = false;
      signalTimer = now;
    }
    if (nextPressed) {
      beep(1);
      clockStyle = (clockStyle + 1) % 6;
      saveClockStyle();
      Serial.print(F("Active clock style: Style 0"));
      Serial.println(clockStyle + 1);
    }
  }

  if (currentScreen == 2 && now - signalTimer > 8000) {
    currentScreen = wifiStatusFromMenu ? 3 : 0;
  }
}

#endif
