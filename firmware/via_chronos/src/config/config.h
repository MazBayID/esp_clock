#ifndef NEO_CHRONOS_CONFIG_H
#define NEO_CHRONOS_CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>

// Hardware: ESP32-C3 SuperMini. Keep this pinout identical to via_wifi.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

#define PIN_SCL 20
#define PIN_SDA 21

// MAX98357A speaker output
#define I2S_BCLK 1
#define I2S_LRC  2
#define I2S_DIN  5

// INMP441 microphone input (not enabled in the first BLE milestone)
#define MIC_SCK 1
#define MIC_WS  2
#define MIC_SD  10

// TTP223 touch inputs
#define PIN_PREV 4
#define PIN_NEXT 6
#define PIN_MODE 7

#define DISPLAY_REFRESH_MS 100
#define INPUT_DEBOUNCE_MS 35
#define MODE_LONG_PRESS_MS 700

// Screen IDs used by the compact settings/navigation UI.
enum ChronosScreen : uint8_t {
  CHRONOS_SCREEN_CLOCK = 0,
  CHRONOS_SCREEN_MENU,
  CHRONOS_SCREEN_STYLE,
  CHRONOS_SCREEN_TIME_FORMAT,
  CHRONOS_SCREEN_BLE_INFO
};

extern Adafruit_SSD1306 display;
extern ChronosESP32 watch;
extern uint8_t clockStyle;
extern uint8_t chronosScreen;
extern uint8_t chronosMenuSelection;
extern bool chronos24Hour;

extern volatile bool chronosConnected;
extern volatile bool chronosTimeSynced;
extern volatile bool chronosTimeChanged;

void setupChronos();
void updateChronos();
void setupInput();
void updateInput();
void persistChronosTimeFormat();
void renderDisplay();
void setupDisplay();

#endif
