#ifndef NEO_CHRONOS_SYSTEM_H
#define NEO_CHRONOS_SYSTEM_H

#include "../config/config.h"

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
    chronos24Hour = watch.is24Hour();
  }
}

void onChronosNotification(Notification notification) {
  Serial.print(F("Notification from "));
  Serial.print(notification.app);
  Serial.print(F(": "));
  Serial.println(notification.title);
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
