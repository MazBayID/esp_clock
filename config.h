#ifndef JAM_CONFIG_H
#define JAM_CONFIG_H

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

// OLED SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PIN_SCL 20
#define PIN_SDA 21

// MAX98357A (I2S)
#define I2S_BCLK 1
#define I2S_LRC  2
#define I2S_DIN  5

// TTP223 TOUCH SENSORS
#define PIN_PREV 4
#define PIN_NEXT 6
#define PIN_MODE 7

// Konfigurasi waktu
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Objek dan konfigurasi perangkat
extern Adafruit_SSD1306 display;
extern const char* ssid;
extern const char* password;

// State aplikasi
extern int currentScreen;
extern int clockFace;
extern unsigned long signalTimer;
extern unsigned long quoteTimer;
extern String currentQuote;
extern int lastHour;

// State input
extern bool lastTalkState;
extern bool lastNextState;
extern bool lastModeState;

// Bitmap digit
extern uint8_t clockStyle;

// Deklarasi fungsi lintas-modul
void beep(int times);
void setupI2S();
void setupDisplay();
void setupWiFi();
void fetchQuote();
String formatDuaDigit(int angka);
void drawDigit(int x, int y, int number);
void drawCustomClock(const struct tm& timeinfo, int digitY, int colonY,
                     int secondsX, int secondsY);
void updateInput();
void renderDisplay(const struct tm& timeinfo, bool isTimeAvail);

#endif
