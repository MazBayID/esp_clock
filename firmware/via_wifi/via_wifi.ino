#include "src/config/config.h"

// Optional one-time bootstrap credentials from a local secrets.h.
// After a successful connection, they are saved in ESP32 NVS.
const char* ssid = NEO_WIFI_SSID;
const char* password = NEO_WIFI_PASSWORD;

const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int currentScreen = 0;
int clockFace = 0;
uint8_t clockStyle = 2; // Default asset: Style 03.
uint8_t menuSelection = 0;
unsigned long signalTimer = 0;
int lastHour = -1;

#include "src/systems/display_system.h"
#include "src/systems/audio_system.h"
#include "src/systems/wifi_system.h"
#include "src/systems/clock_system.h"
#include "src/systems/input_system.h"

void setup() {
  Serial.begin(115200);

  setupInput();
  setupMenuSettings();
  setupI2S();
  setupDisplay();
  setupWiFi();
}

void loop() {
  handleWiFiPortal();

  struct tm timeinfo = {};
  const bool isTimeAvail =
      WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo);

  if (isTimeAvail) {
    if (lastHour == -1) {
      lastHour = timeinfo.tm_hour;
    } else if (timeinfo.tm_hour != lastHour) {
      lastHour = timeinfo.tm_hour;
      beep(2);
    }
  }

  updateInput();
  renderDisplay(timeinfo, isTimeAvail);
}
