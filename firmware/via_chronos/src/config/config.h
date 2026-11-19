#ifndef NEO_CHRONOS_CONFIG_H
#define NEO_CHRONOS_CONFIG_H

#include <Arduino.h>
#include <Wire.h>
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

extern Adafruit_SSD1306 display;
extern ChronosESP32 watch;
extern uint8_t clockStyle;

extern volatile bool chronosConnected;
extern volatile bool chronosTimeSynced;
extern volatile bool chronosTimeChanged;

void setupChronos();
void updateChronos();
void setupInput();
void updateInput();
void renderDisplay();
void setupDisplay();

#endif
