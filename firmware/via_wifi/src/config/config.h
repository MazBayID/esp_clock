#ifndef NEO_FIRMWARE_CONFIG_H
#define NEO_FIRMWARE_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <driver/i2s.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>

// Optional local bootstrap credentials. secrets.h is intentionally not committed.
#if defined(__has_include)
  #if __has_include("secrets.h")
    #include "secrets.h"
  #endif
#endif

#ifndef NEO_WIFI_SSID
  #define NEO_WIFI_SSID ""
#endif

#ifndef NEO_WIFI_PASSWORD
  #define NEO_WIFI_PASSWORD ""
#endif

// Board: ESP32-C3 SuperMini
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PIN_SCL 20
#define PIN_SDA 21

// MAX98357A (I2S)
#define I2S_BCLK 1
#define I2S_LRC  2
#define I2S_DIN  5

// TTP223 touch sensors
#define PIN_PREV 4
#define PIN_NEXT 6
#define PIN_MODE 7

extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

extern Adafruit_SSD1306 display;
extern const char* ssid;
extern const char* password;

extern int currentScreen;
extern int clockFace;
extern unsigned long signalTimer;
extern int lastHour;
extern uint8_t clockStyle;
extern uint8_t menuSelection;

extern bool lastTalkState;
extern bool lastNextState;
extern bool lastModeState;

void beep(int times);
void setupI2S();
void setupDisplay();
void setupWiFi();
void handleWiFiPortal();
String formatDuaDigit(int angka);
void drawDigit(int x, int y, int number);
void drawCustomClock(const struct tm& timeinfo, int digitY, int colonY,
                     int secondsX, int secondsY);
void setupMenuSettings();
void updateInput();
void renderDisplay(const struct tm& timeinfo, bool isTimeAvail);

#endif
