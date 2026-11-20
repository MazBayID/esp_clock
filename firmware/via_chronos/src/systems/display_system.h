#ifndef NEO_CHRONOS_DISPLAY_SYSTEM_H
#define NEO_CHRONOS_DISPLAY_SYSTEM_H

#include "../config/config.h"
#include "../assets/default/logos/neo_logo.h"
#include "../assets/default/digits/style01.h"
#include "../assets/default/digits/style02.h"
#include "../assets/default/digits/style03.h"
#include "../assets/default/digits/style04.h"
#include "../assets/default/digits/style05.h"
#include "../assets/default/digits/style06.h"
#if __has_include("../assets/generated/ui_assets.h")
#include "../assets/generated/ui_assets.h"
#define NEO_CHRONOS_HAS_UI_ASSETS 1
#endif

const unsigned char* getDigitBitmap(int number) {
  if (number < 0 || number > 9) return nullptr;

  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[number];
    case 1: return ClockStyle02::epd_bitmap_allArray[number];
    case 2: return ClockStyle03::epd_bitmap_allArray[number];
    case 3: return ClockStyle04::ClockStyle04_allArray[number];
    case 4: return ClockStyle05::ClockStyle05_allArray[number];
    case 5: return ClockStyle06::ClockStyle06_allArray[number];
    default: return ClockStyle01::epd_bitmap_allArray[number];
  }
}

const unsigned char* getColonBitmap() {
  switch (clockStyle) {
    case 0: return ClockStyle01::epd_bitmap_allArray[10];
    case 1: return ClockStyle02::epd_bitmap_allArray[10];
    case 2: return ClockStyle03::epd_bitmap_allArray[10];
    case 3: return ClockStyle04::ClockStyle04_allArray[10];
    case 4: return ClockStyle05::ClockStyle05_allArray[10];
    case 5: return ClockStyle06::ClockStyle06_allArray[10];
    default: return ClockStyle01::epd_bitmap_allArray[10];
  }
}

int getDigitWidth() {
  return clockStyle >= 3 ? 25 : 24;
}

int getDigitHeight() {
  if (clockStyle == 3 || clockStyle == 5) return 48;
  if (clockStyle == 4) return 25;
  return 24;
}

void drawClockDigit(int x, int y, int number) {
  const unsigned char* bitmap = getDigitBitmap(number);
  if (bitmap) {
    display.drawBitmap(x, y, bitmap, getDigitWidth(), getDigitHeight(), SSD1306_WHITE);
  }
}

void setupDisplay() {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 initialization failed"));
    while (true) delay(100);
  }
  display.setRotation(2);
  display.clearDisplay();
  display.display();
}

void renderChronosMenu() {
  static const char* menuItems[] = {
    "Clock Style", "Time Format", "BLE Status", "Notifications",
    "Weather", "Music Control", "Navigation", "Phone Information"
  };

  const uint8_t firstVisible = chronosMenuSelection >= 5
      ? chronosMenuSelection - 4 : 0;
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("NEO CLOCK MENU"));
  display.setCursor(105, 0);
  display.print(chronosMenuSelection + 1);
  display.print(F("/8"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  for (uint8_t row = 0; row < 5; ++row) {
    const uint8_t i = firstVisible + row;
    if (i >= 8) break;
    const int y = 14 + row * 9;
    if (chronosMenuSelection == i) {
      display.fillRect(0, y - 1, SCREEN_WIDTH, 11, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(5, y);
      display.print('>');
      display.setCursor(17, y);
      display.print(menuItems[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(17, y);
      display.print(menuItems[i]);
    }
  }
}

void renderClockStyleScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("CLOCK STYLE"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(24, 23);
  display.print(F("STYLE "));
  display.print(clockStyle + 1);

  display.setTextSize(1);
  display.setCursor(0, 51);
  display.print(F("PREV/NEXT: CHANGE"));
  display.setCursor(0, 57);
  display.print(F("OK: MENU  HOLD: HOME"));
}

void renderTimeFormatScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("TIME FORMAT"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(27, 24);
  display.print(chronos24Hour ? F("24 HOUR") : F("12 HOUR"));

  display.setTextSize(1);
  display.setCursor(0, 51);
  display.print(F("PREV/NEXT: CHANGE"));
  display.setCursor(0, 57);
  display.print(F("OK: MENU  HOLD: HOME"));
}

void renderBleInfoScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("CHRONOS BLE STATUS"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  display.setCursor(0, 19);
  display.print(F("LINK: "));
  display.print(chronosConnected ? F("CONNECTED") : F("WAITING"));

  display.setCursor(0, 32);
  display.print(F("TIME: "));
  display.print(chronosTimeSynced ? F("SYNCED") : F("NOT SYNCED"));

  display.setCursor(0, 45);
  display.print(F("FORMAT: "));
  display.print(chronos24Hour ? F("24-HOUR") : F("12-HOUR"));

  display.setCursor(0, 57);
  display.print(F("OK: MENU  HOLD: HOME"));
}

void renderNotifications() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("NOTIFICATIONS"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  const int count = watch.getNotificationCount();
  if (count <= 0) {
    display.setCursor(0, 27);
    display.print(F("No notifications"));
    display.setCursor(0, 43);
    display.print(F("Check Chronos app"));
    return;
  }

  if (chronosNotificationIndex >= count) chronosNotificationIndex = count - 1;
  Notification note = watch.getNotificationAt(chronosNotificationIndex);
  display.setCursor(0, 14);
  display.print(note.app.substring(0, 15));
  display.setCursor(96, 14);
  display.print(chronosNotificationIndex + 1);
  display.print('/');
  display.print(count);

  String content = note.message.length() ? note.message : note.title;
  content.trim();
  int pos = 0;
  for (int row = 0; row < 4 && pos < content.length(); ++row) {
    int end = min(pos + 21, (int)content.length());
    if (end < content.length()) {
      int space = content.lastIndexOf(' ', end - 1);
      if (space > pos + 7) end = space;
    }
    String line = content.substring(pos, end);
    line.trim();
    if (row == 3 && end < content.length()) {
      if (line.length() > 18) line = line.substring(0, 18);
      line += "...";
    }
    display.setCursor(0, 25 + row * 9);
    display.print(line);
    pos = end;
    while (pos < content.length() && content[pos] == ' ') ++pos;
  }
}

void renderWeather() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("WEATHER"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);

  const int count = watch.getWeatherCount();
  if (count <= 0) {
    display.setCursor(0, 27);
    display.print(F("No weather data"));
    display.setCursor(0, 43);
    display.print(F("Check Chronos app"));
    return;
  }

  if (chronosWeatherIndex >= count) chronosWeatherIndex = count - 1;
  Weather weather = watch.getWeatherAt(chronosWeatherIndex);
  display.setCursor(0, 14);
  display.print(watch.getWeatherCity().substring(0, 20));
  display.setTextSize(2);
  display.setCursor(0, 27);
  display.print(weather.temp);
  display.print(F("C"));
  display.setTextSize(1);
  display.setCursor(0, 51);
  display.print(F("H:"));
  display.print(weather.high);
  display.print(F(" L:"));
  display.print(weather.low);
  display.setCursor(84, 51);
  display.print(chronosWeatherIndex + 1);
  display.print('/');
  display.print(count);
}

void renderMusicScreen() {
  const MusicInfo music = watch.getMusicInfo();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("MUSIC CONTROL"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(0, 16);
  display.print(music.state ? F("PLAYING") : F("PAUSED"));
  display.setCursor(0, 29);
  display.print(music.title.substring(0, 21));
  display.setCursor(0, 41);
  display.print(music.artist.substring(0, 21));
  display.setCursor(0, 48);
  display.print(F("PREV/NEXT: TRACK"));
  display.setCursor(0, 56);
  display.print(F("OK: PLAY/PAUSE"));
}

void renderNavigationScreen() {
  const Navigation nav = watch.getNavigation();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("NAVIGATION"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);
  if (!nav.active) {
    display.setCursor(0, 28);
    display.print(F("No active navigation"));
    display.setCursor(0, 43);
    display.print(F("Open navigation app"));
    return;
  }
  String direction = nav.directions.length() ? nav.directions : nav.title;
  direction.trim();
  display.setCursor(0, 15);
  display.print(nav.title.substring(0, 21));
  display.setCursor(0, 27);
  display.print(direction.substring(0, 21));
  display.setCursor(0, 41);
  display.print(nav.distance.substring(0, 21));
  display.setCursor(0, 53);
  display.print(nav.duration.substring(0, 21));
}

void renderPhoneInfoScreen() {
  const PhoneInfo phone = watch.getPhoneInfo();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("PHONE INFORMATION"));
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(0, 16);
  display.print(F("BLE: "));
  display.print(chronosConnected ? F("CONNECTED") : F("WAITING"));
  display.setCursor(0, 28);
  display.print(F("BATTERY: "));
  display.print(phone.batteryLevel);
  display.print('%');
  display.setCursor(0, 40);
  display.print(phone.isCharging ? F("CHARGING") : F("ON BATTERY"));
  display.setCursor(0, 52);
  String model = phone.manufacturer + " " + phone.model;
  display.print(model.substring(0, 21));
}
void renderClockFace() {
  if (!chronosTimeSynced) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("NEO CLOCK"));
#ifdef NEO_CHRONOS_HAS_UI_ASSETS
    display.drawBitmap(0, 22, NeoUiAssets::bluetooth, 16, 18, SSD1306_WHITE);
#endif
    display.setCursor(22, 25);
    display.print(chronosConnected ? F("WAITING FOR TIME") : F("PAIR WITH CHRONOS"));
    display.setCursor(22, 39);
    display.print(chronosConnected ? F("SYNC IN PROGRESS") : F("WATCHES > PAIR NEW"));
    return;
  }

  const int hour = watch.getHourC();
  const int minute = watch.getMinute();
  const int digits[4] = {hour / 10, hour % 10, minute / 10, minute % 10};

  if (clockStyle <= 2) {
    const int y = 20;
    drawClockDigit(10, y, digits[0]);
    drawClockDigit(34, y, digits[1]);
    drawClockDigit(70, y, digits[2]);
    drawClockDigit(94, y, digits[3]);
    if (watch.getSecond() % 2 == 0) {
      display.drawBitmap(60, y, getColonBitmap(), 8, 24, SSD1306_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print(watch.getTime("%d/%m/%Y"));
    display.setCursor(75, 55);
    display.print(chronos24Hour ? F("24H") : F("12H"));
    display.setCursor(105, 55);
    display.print(chronosConnected ? F("BT") : F("--"));
  } else {
    const int y = (SCREEN_HEIGHT - getDigitHeight()) / 2;
    const int slotX[5] = {1, 26, 51, 76, 101};
    drawClockDigit(slotX[0], y, digits[0]);
    drawClockDigit(slotX[1], y, digits[1]);
    drawClockDigit(slotX[3], y, digits[2]);
    drawClockDigit(slotX[4], y, digits[3]);
    if (watch.getSecond() % 2 == 0) {
      display.drawBitmap(slotX[2], y, getColonBitmap(),
                         getDigitWidth(), getDigitHeight(), SSD1306_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(chronosConnected ? F("BLE") : F("OFF"));
    display.setCursor(101, 0);
    display.print(chronos24Hour ? F("24H") : F("12H"));
  }
}

void renderDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  switch (chronosScreen) {
    case CHRONOS_SCREEN_MENU:
      renderChronosMenu();
      break;
    case CHRONOS_SCREEN_STYLE:
      renderClockStyleScreen();
      break;
    case CHRONOS_SCREEN_TIME_FORMAT:
      renderTimeFormatScreen();
      break;
    case CHRONOS_SCREEN_BLE_INFO:
      renderBleInfoScreen();
      break;
    case CHRONOS_SCREEN_NOTIFICATIONS:
      renderNotifications();
      break;
    case CHRONOS_SCREEN_WEATHER:
      renderWeather();
      break;
    case CHRONOS_SCREEN_MUSIC:
      renderMusicScreen();
      break;
    case CHRONOS_SCREEN_NAVIGATION:
      renderNavigationScreen();
      break;
    case CHRONOS_SCREEN_PHONE_INFO:
      renderPhoneInfoScreen();
      break;
    case CHRONOS_SCREEN_CLOCK:
    default:
      renderClockFace();
      break;
  }

  display.display();
}

#endif
