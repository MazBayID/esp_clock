#ifndef NEO_FIRMWARE_WIFI_SYSTEM_H
#define NEO_FIRMWARE_WIFI_SYSTEM_H

#include "../config/config.h"

void setupWiFi() {
  if (ssid == nullptr || password == nullptr ||
      ssid[0] == '\0' || password[0] == '\0') {
    Serial.println(F("Wi-Fi credentials not configured; skipping Wi-Fi."));
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(ssid, password);

  const unsigned long wifiStart = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - wifiStart < 15000) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTime(gmtOffset_sec, daylightOffset_sec, "ntp.bmkg.go.id");
    Serial.println(F("Wi-Fi connected; NTP synchronization requested."));
  } else {
    Serial.println(F("Wi-Fi connection failed."));
  }
}

#endif
