#ifndef NEO_CHRONOS_INPUT_SYSTEM_H
#define NEO_CHRONOS_INPUT_SYSTEM_H

#include "../config/config.h"

namespace {
Preferences chronosUiPreferences;
bool chronosUiPreferencesReady = false;

bool rawPrevState = LOW;
bool stablePrevState = LOW;
bool rawNextState = LOW;
bool stableNextState = LOW;
bool rawModeState = LOW;
bool stableModeState = LOW;

unsigned long prevDebounceStarted = 0;
unsigned long nextDebounceStarted = 0;
unsigned long modeDebounceStarted = 0;
unsigned long modePressStarted = 0;
bool modeLongHandled = false;

bool updateDebouncedButton(bool reading, bool& rawState, bool& stableState,
                           unsigned long& debounceStarted, unsigned long now) {
  if (reading != rawState) {
    rawState = reading;
    debounceStarted = now;
  }

  if (now - debounceStarted >= INPUT_DEBOUNCE_MS && stableState != rawState) {
    stableState = rawState;
    return stableState == HIGH;
  }
  return false;
}

void saveChronosUiSettings() {
  if (!chronosUiPreferencesReady) return;
  chronosUiPreferences.putUChar("clockStyle", clockStyle);
  chronosUiPreferences.putBool("hour24", chronos24Hour);
}

void changeClockStyle(int direction) {
  clockStyle = (clockStyle + (direction > 0 ? 1 : 5)) % 6;
  saveChronosUiSettings();
}

void changeTimeFormat() {
  chronos24Hour = !chronos24Hour;
  watch.set24Hour(chronos24Hour);
  saveChronosUiSettings();
}

void activateChronosMenuItem() {
  switch (chronosMenuSelection) {
    case 0:
      chronosScreen = CHRONOS_SCREEN_STYLE;
      break;
    case 1:
      chronosScreen = CHRONOS_SCREEN_TIME_FORMAT;
      break;
    case 2:
      chronosScreen = CHRONOS_SCREEN_BLE_INFO;
      break;
    case 3:
      chronosNotificationIndex = 0;
      chronosScreen = CHRONOS_SCREEN_NOTIFICATIONS;
      break;
    case 4:
      chronosWeatherIndex = 0;
      chronosScreen = CHRONOS_SCREEN_WEATHER;
      break;
    case 5:
      chronosScreen = CHRONOS_SCREEN_MUSIC;
      break;
    case 6:
      chronosScreen = CHRONOS_SCREEN_NAVIGATION;
      break;
    case 7:
      chronosScreen = CHRONOS_SCREEN_PHONE_INFO;
      break;
    default:
      chronosScreen = CHRONOS_SCREEN_CLOCK;
      break;
  }
}
}  // namespace

void setupInput() {
  pinMode(PIN_PREV, INPUT);
  pinMode(PIN_NEXT, INPUT);
  pinMode(PIN_MODE, INPUT);

  // Ignore any buttons already held during boot. Each input starts from
  // its sampled hardware state, so a held button must be released and
  // pressed again before it can trigger an action.
  const unsigned long now = millis();
  rawPrevState = stablePrevState = digitalRead(PIN_PREV) == HIGH;
  rawNextState = stableNextState = digitalRead(PIN_NEXT) == HIGH;
  rawModeState = stableModeState = digitalRead(PIN_MODE) == HIGH;
  prevDebounceStarted = now;
  nextDebounceStarted = now;
  modeDebounceStarted = now;
  modePressStarted = now;
  modeLongHandled = stableModeState;

  chronosUiPreferencesReady = chronosUiPreferences.begin("neo-ble-ui", false);
  if (chronosUiPreferencesReady) {
    const uint8_t savedStyle = chronosUiPreferences.getUChar("clockStyle", clockStyle);
    if (savedStyle < 6) clockStyle = savedStyle;
    chronos24Hour = chronosUiPreferences.getBool("hour24", true);
  }

  // Apply the saved time format before Chronos starts its BLE service.
  watch.set24Hour(chronos24Hour);
}

void persistChronosTimeFormat() {
  chronos24Hour = watch.is24Hour();
  saveChronosUiSettings();
}

void updateInput() {
  const unsigned long now = millis();
  const bool prevPressed = updateDebouncedButton(
      digitalRead(PIN_PREV) == HIGH, rawPrevState, stablePrevState,
      prevDebounceStarted, now);
  const bool nextPressed = updateDebouncedButton(
      digitalRead(PIN_NEXT) == HIGH, rawNextState, stableNextState,
      nextDebounceStarted, now);
  const bool modePressed = updateDebouncedButton(
      digitalRead(PIN_MODE) == HIGH, rawModeState, stableModeState,
      modeDebounceStarted, now);

  if (prevPressed) {
    if (chronosScreen == CHRONOS_SCREEN_MENU) {
      chronosMenuSelection = (chronosMenuSelection + 7) % 8;
    } else if (chronosScreen == CHRONOS_SCREEN_CLOCK ||
               chronosScreen == CHRONOS_SCREEN_STYLE) {
      changeClockStyle(-1);
    } else if (chronosScreen == CHRONOS_SCREEN_TIME_FORMAT) {
      changeTimeFormat();
    } else if (chronosScreen == CHRONOS_SCREEN_BLE_INFO) {
      chronosScreen = CHRONOS_SCREEN_MENU;
    } else if (chronosScreen == CHRONOS_SCREEN_WEATHER) {
      const int count = watch.getWeatherCount();
      if (count > 0) chronosWeatherIndex = (chronosWeatherIndex + count - 1) % count;
    } else if (chronosScreen == CHRONOS_SCREEN_MUSIC) {
      watch.musicControl(MUSIC_PREVIOUS);
    } else if (chronosScreen == CHRONOS_SCREEN_NOTIFICATIONS) {
      const int count = watch.getNotificationCount();
      if (count > 0) chronosNotificationIndex = (chronosNotificationIndex + count - 1) % count;
    }
  }

  if (nextPressed) {
    if (chronosScreen == CHRONOS_SCREEN_MENU) {
      chronosMenuSelection = (chronosMenuSelection + 1) % 8;
    } else if (chronosScreen == CHRONOS_SCREEN_CLOCK ||
               chronosScreen == CHRONOS_SCREEN_STYLE) {
      changeClockStyle(1);
    } else if (chronosScreen == CHRONOS_SCREEN_TIME_FORMAT) {
      changeTimeFormat();
    } else if (chronosScreen == CHRONOS_SCREEN_BLE_INFO) {
      chronosScreen = CHRONOS_SCREEN_MENU;
    } else if (chronosScreen == CHRONOS_SCREEN_WEATHER) {
      const int count = watch.getWeatherCount();
      if (count > 0) chronosWeatherIndex = (chronosWeatherIndex + 1) % count;
    } else if (chronosScreen == CHRONOS_SCREEN_MUSIC) {
      watch.musicControl(MUSIC_NEXT);
    } else if (chronosScreen == CHRONOS_SCREEN_NOTIFICATIONS) {
      const int count = watch.getNotificationCount();
      if (count > 0) chronosNotificationIndex = (chronosNotificationIndex + 1) % count;
    }
  }

  if (modePressed) {
    modePressStarted = now;
    modeLongHandled = false;
  }

  if (stableModeState == HIGH && !modeLongHandled &&
      now - modePressStarted >= MODE_LONG_PRESS_MS) {
    modeLongHandled = true;
    chronosScreen = (chronosScreen == CHRONOS_SCREEN_CLOCK)
        ? CHRONOS_SCREEN_MENU
        : CHRONOS_SCREEN_CLOCK;
  }

  // A short MODE action is processed on release to distinguish it from a hold.
  static bool modeWasPressed = false;
  if (!stableModeState && modeWasPressed && !modeLongHandled) {
    if (chronosScreen == CHRONOS_SCREEN_CLOCK) {
      // Preserve the original quick action on the clock face.
      changeTimeFormat();
    } else if (chronosScreen == CHRONOS_SCREEN_MENU) {
      activateChronosMenuItem();
    } else if (chronosScreen == CHRONOS_SCREEN_MUSIC) {
      watch.musicControl(MUSIC_TOGGLE);
    } else {
      chronosScreen = CHRONOS_SCREEN_MENU;
    }
  }
  modeWasPressed = stableModeState;
}

#endif
