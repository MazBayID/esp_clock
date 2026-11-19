#ifndef NEO_INPUT_SYSTEM_H
#define NEO_INPUT_SYSTEM_H

#include "../config/config.h"

namespace {
Preferences menuPreferences;
bool menuPreferencesReady = false;
bool modePressActive = false;
bool modeLongHandled = false;
unsigned long modePressStarted = 0;
bool rawModeState = LOW;
bool stableModeState = LOW;
bool wifiStatusFromMenu = false;
unsigned long modeDebounceStarted = 0;
constexpr unsigned long INPUT_DEBOUNCE_MS = 35;
constexpr unsigned long MENU_LONG_PRESS_MS = 700;
constexpr unsigned long MENU_INTRO_MS = 900;

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

void setupMenuSettings() {
  menuPreferencesReady = menuPreferences.begin("neo-ui", false);
  if (menuPreferencesReady) {
    const uint8_t savedStyle = menuPreferences.getUChar("clockStyle", clockStyle);
    if (savedStyle < 6) clockStyle = savedStyle;
  }
}

void updateInput() {
  const bool signalState = digitalRead(PIN_PREV);
  const bool nextState = digitalRead(PIN_NEXT);
  const bool modeReading = digitalRead(PIN_MODE);

  if (modeReading != rawModeState) {
    rawModeState = modeReading;
    modeDebounceStarted = millis();
  }
  if (millis() - modeDebounceStarted >= INPUT_DEBOUNCE_MS &&
      stableModeState != rawModeState) {
    stableModeState = rawModeState;
    if (stableModeState == HIGH) {
      modePressActive = true;
      modeLongHandled = false;
      modePressStarted = millis();
    } else if (modePressActive) {
      if (!modeLongHandled) {
        beep(1);
        if (currentScreen == 0) {
          currentScreen = 1; // Brief NEO intro before the menu.
          signalTimer = millis();
        } else if (currentScreen == 3) {
          openSelectedMenuItem();
        } else if (currentScreen == 5) {
          currentScreen = 3;
        } else if (currentScreen == 2 || currentScreen == 4) {
          currentScreen = 3;

      }
      modePressActive = false;
    }
  }

  if (modePressActive && !modeLongHandled &&
      millis() - modePressStarted >= MENU_LONG_PRESS_MS) {
    modeLongHandled = true;
    beep(1);
    if (currentScreen == 1 || currentScreen == 3 ||
        currentScreen == 5 || currentScreen == 2 ||
        currentScreen == 4) {
      currentScreen = 0;
    }
  }

  if (currentScreen == 1 && millis() - signalTimer >= MENU_INTRO_MS) {
    currentScreen = 3;
    menuSelection = 0;
  }

  if (currentScreen == 3) {
    if (signalState == HIGH && lastTalkState == LOW) {
      beep(1);
      menuSelection = (menuSelection + 2) % 3;
    }
    if (nextState == HIGH && lastNextState == LOW) {
      beep(1);
      menuSelection = (menuSelection + 1) % 3;
    }
  } else if (currentScreen == 5) {
    if (signalState == HIGH && lastTalkState == LOW) {
      beep(1);
      clockStyle = (clockStyle + 5) % 6;
      saveClockStyle();
    }
    if (nextState == HIGH && lastNextState == LOW) {
      beep(1);
      clockStyle = (clockStyle + 1) % 6;
      saveClockStyle();
    }
  } else if (currentScreen == 0) {
    if (signalState == HIGH && lastTalkState == LOW) {
      beep(1);
      currentScreen = 2;
      wifiStatusFromMenu = false;
      signalTimer = millis();
    }
    if (nextState == HIGH && lastNextState == LOW) {
      beep(1);
      clockStyle = (clockStyle + 1) % 6;
      saveClockStyle();
      Serial.print(F("Active clock style: Style 0"));
      Serial.println(clockStyle + 1);
    }
  }

  lastTalkState = signalState;
  lastNextState = nextState;
  lastModeState = stableModeState;

  if (currentScreen == 2 && millis() - signalTimer > 8000) {
    currentScreen = wifiStatusFromMenu ? 3 : 0;
  }
}

#endif
