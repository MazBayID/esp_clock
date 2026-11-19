#ifndef NEO_CHRONOS_SYSTEM_H
#define NEO_CHRONOS_SYSTEM_H

#include "../config/config.h"

void onChronosConnection(bool connected) {
  chronosConnected = connected;
  Serial.println(connected ? F("Chronos: connected") : F("Chronos: disconnected"));
}

void onChronosConfiguration(Config config, uint32_t a, uint32_t b) {
  (void)b;

  if (config == CF_TIME) {
    // In ChronosESP32, a == 0 indicates that the time has been applied.
    if (a == 0) {
      chronosTimeSynced = true;
      chronosTimeChanged = true;
      Serial.print(F("Chronos time synced: "));
      Serial.println(watch.getTimeDate());
    }
  }
}

void onChronosNotification(Notification notification) {
  Serial.print(F("Notification from "));
  Serial.print(notification.app);
  Serial.print(F(": "));
  Serial.println(notification.title);
}

void setupChronos() {
  // CS_0x0_000_CFF deliberately advertises no display profile.
  // NEO CLOCK is 128x64; do not report it as a 240x240/466x466 watch.
  watch.setConnectionCallback(onChronosConnection);
  watch.setNotificationCallback(onChronosNotification);
  watch.setConfigurationCallback(onChronosConfiguration);

  watch.set24Hour(true);
  watch.begin();

  Serial.print(F("NEO CLOCK BLE address: "));
  Serial.println(watch.getAddress());
}

void updateChronos() {
  watch.loop();
}

#endif
