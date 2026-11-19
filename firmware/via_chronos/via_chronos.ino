#include "src/config/config.h"
#include "src/systems/chronos_system.h"
#include "src/systems/display_system.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ChronosESP32 watch("NEO_CLOCK", CS_0x0_000_CFF);

volatile bool chronosConnected = false;
volatile bool chronosTimeSynced = false;
volatile bool chronosTimeChanged = false;

unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(150);

  pinMode(PIN_PREV, INPUT);
  pinMode(PIN_NEXT, INPUT);
  pinMode(PIN_MODE, INPUT);

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 initialization failed"));
    while (true) delay(100);
  }

  display.setRotation(2);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("NEO CLOCK"));
  display.println(F("Starting BLE..."));
  display.display();

  setupChronos();
}

void loop() {
  updateChronos();

  const unsigned long now = millis();
  if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
    lastDisplayUpdate = now;
    renderDisplay();
  }
}
