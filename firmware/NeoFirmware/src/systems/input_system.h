#ifndef JAM_INPUT_SYSTEM_H
#define JAM_INPUT_SYSTEM_H

#include "config.h"

// Membaca input sentuh dan mengelola perpindahan layar.
void updateInput() {
  bool talkState = digitalRead(PIN_PREV);
  bool nextState = digitalRead(PIN_NEXT);
  bool modeState = digitalRead(PIN_MODE);

  // Tombol TALK: Kekuatan Sinyal (Auto balik 5 detik)
  if (talkState == HIGH && lastTalkState == LOW) {
    beep(1);
    currentScreen = 2;
    signalTimer = millis();
  }

  // Tombol NEXT (GPIO6): Siklus Style 01 -> 02 -> 03 -> 01
  if (nextState == HIGH && lastNextState == LOW) {
    beep(1);
    clockStyle = (clockStyle + 1) % 3;
    currentScreen = 0;

    Serial.print(F("Clock style aktif: Style 0"));
    Serial.println(clockStyle + 1);
  }

  // Tombol MODE: Quote Web
  if (modeState == HIGH && lastModeState == LOW) {
    beep(1);
    currentScreen = 3;
    fetchQuote();
    quoteTimer = millis();
  }

  lastTalkState = talkState;
  lastNextState = nextState;
  lastModeState = modeState;

  // AUTO KEMBALI KE JAM
  if (currentScreen == 2 && millis() - signalTimer > 5000) {
    currentScreen = 0;
  }
  if (currentScreen == 3 && millis() - quoteTimer > 10000) {
    currentScreen = 0;
  }
}

#endif
