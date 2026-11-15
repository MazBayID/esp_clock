#include "config.h"

// --- KONFIGURASI WIFI & NTP ---
const char* ssid = "ipong 50pro";
const char* password = "00000000";
const long gmtOffset_sec = 7 * 3600; // GMT+7
const int daylightOffset_sec = 0;

// --- OBJEK OLED ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- VARIABEL GLOBAL ---
int currentScreen = 0; // 0=Clock1, 1=Clock2, 2=Signal, 3=Quote
int clockFace = 0;     // Tampilan tanggal disimpan untuk menu berikutnya
uint8_t clockStyle = 2; // Default: Style 03
unsigned long signalTimer = 0;
unsigned long quoteTimer = 0;
String currentQuote = "";
int lastHour = -1;

// State tombol (untuk debounce)
bool lastTalkState = LOW;
bool lastNextState = LOW;
bool lastModeState = LOW;

// --- MODUL FIRMWARE ---
#include "display_system.h"
#include "audio_system.h"
#include "wifi_system.h"
#include "quote_system.h"
#include "clock_system.h"
#include "input_system.h"

void setup() {
  Serial.begin(115200);

  // Inisialisasi Pin Touch
  pinMode(PIN_PREV, INPUT);
  pinMode(PIN_NEXT, INPUT);
  pinMode(PIN_MODE, INPUT);

  // Inisialisasi I2S Audio
  setupI2S();

  // Inisialisasi OLED dan logo startup
  setupDisplay();

  // Inisialisasi Wi-Fi dan sinkronisasi waktu
  setupWiFi();
}

void loop() {
  struct tm timeinfo;
  bool isTimeAvail = getLocalTime(&timeinfo);

  // LOGIKA BUNYI SAAT JAM BERGANTI
  if (isTimeAvail) {
    if (lastHour == -1) {
      lastHour = timeinfo.tm_hour;
    } else if (timeinfo.tm_hour != lastHour) {
      lastHour = timeinfo.tm_hour;
      beep(2);
    }
  }

  // Membaca input dan memperbarui state layar
  updateInput();

  // Render layar aktif
  renderDisplay(timeinfo, isTimeAvail);
}
