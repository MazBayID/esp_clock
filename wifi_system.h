#ifndef JAM_WIFI_SYSTEM_H
#define JAM_WIFI_SYSTEM_H

#include "config.h"

// Inisialisasi Wi-Fi dan sinkronisasi NTP.
// Alur koneksi dipertahankan sesuai implementasi sebelumnya.
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(ssid, password);

  unsigned long wifiStart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTime(gmtOffset_sec, daylightOffset_sec,
               "ntp.bmkg.go.id");
  } else {
    Serial.println("WiFi gagal terhubung!");
  }
}

#endif
