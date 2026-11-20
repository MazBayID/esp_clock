#include "src/config/config.h"
#include "src/systems/chronos_system.h"
#include "src/systems/display_system.h"
#include "src/systems/input_system.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ChronosESP32 watch("NEO_CLOCK", CS_0x0_000_CFF);

volatile bool chronosConnected = false;
volatile bool chronosTimeSynced = false;
volatile bool chronosTimeChanged = false;

uint8_t clockStyle = 0;
uint8_t chronosScreen = CHRONOS_SCREEN_CLOCK;
uint8_t chronosMenuSelection = 0;
int chronosNotificationIndex = 0;
int chronosWeatherIndex = 0;
bool chronos24Hour = true;
unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(150);

  setupInput();
  setupDisplay();

  display.clearDisplay();
  display.drawBitmap(32, 0, epd_bitmap_neo_clock, 64, 64, SSD1306_WHITE);
  display.display();
  delay(1200);

  setupChronos();
}

void loop() {
  updateChronos();
  updateInput();

  const unsigned long now = millis();
  if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
    lastDisplayUpdate = now;
    renderDisplay();
  }
}
