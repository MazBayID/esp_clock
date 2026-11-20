#ifndef NEO_CHRONOS_SYSTEM_H
#define NEO_CHRONOS_SYSTEM_H

#include "../config/config.h"

namespace {
constexpr unsigned long CHRONOS_NOTIFICATION_POPUP_MS = 6000UL;
unsigned long chronosNotificationPopupUntil = 0;
bool chronosNotificationPopupActive = false;
}

void onChronosConnection(bool connected) {
  chronosConnected = connected;
  Serial.println(connected ? F("Chronos: connected") : F("Chronos: disconnected"));
}

void onChronosConfiguration(Config config, uint32_t value1, uint32_t value2) {
  (void)value2;

  if (config == CF_TIME && value1 == 1) {
    // ChronosESP32 calls CF_TIME twice: 0 before applying time, 1 after.
    chronosTimeSynced = true;
    chronosTimeChanged = true;
    Serial.print(F("Chronos time synced: "));
    Serial.println(watch.getTimeDate());
  } else if (config == CF_HR24) {
    persistChronosTimeFormat();
  }
}

void onChronosNotification(Notification notification) {
  Serial.print(F("Notification from "));
  Serial.print(notification.app);
  Serial.print(F(": "));
  Serial.println(notification.title);

  // ChronosESP32 stores the incoming notification before invoking this
  // callback; index 0 is therefore the newest notification.
  chronosNotificationIndex = 0;

  // Pop up only over the clock face. Do not interrupt settings or another
  // screen the user is actively viewing. A second notification refreshes
  // the timeout while the popup is still visible.
  if (chronosScreen == CHRONOS_SCREEN_CLOCK ||
      (chronosNotificationPopupActive &&
       chronosScreen == CHRONOS_SCREEN_NOTIFICATIONS)) {
    chronosScreen = CHRONOS_SCREEN_NOTIFICATIONS;
    chronosNotificationPopupActive = true;
    chronosNotificationPopupUntil = millis() + CHRONOS_NOTIFICATION_POPUP_MS;
  }
}

void updateChronosNotificationPopup() {
  if (!chronosNotificationPopupActive ||
      static_cast<int32_t>(millis() - chronosNotificationPopupUntil) < 0) {
    return;
  }

  // Return automatically only if the notification screen is still active.
  // If the user navigated elsewhere during the popup, preserve that choice.
  if (chronosScreen == CHRONOS_SCREEN_NOTIFICATIONS) {
    chronosScreen = CHRONOS_SCREEN_CLOCK;
  }
  chronosNotificationPopupActive = false;
}

void setupChronos() {
  // No display profile: the physical NEO CLOCK OLED is 128x64.
  watch.setConnectionCallback(onChronosConnection);
  watch.setNotificationCallback(onChronosNotification);
  watch.setConfigurationCallback(onChronosConfiguration);

  watch.set24Hour(chronos24Hour);
  watch.begin();

  Serial.print(F("NEO CLOCK BLE address: "));
  Serial.println(watch.getAddress());
}

void updateChronos() {
  watch.loop();
}

#endif
