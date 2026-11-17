#ifndef JAM_QUOTE_SYSTEM_H
#define JAM_QUOTE_SYSTEM_H

#include "../config/config.h"
#include "../assets/default/logos/neo_logo.h"

// --- MENGAMBIL QUOTES DARI WEB ---
void fetchQuote() {
  display.clearDisplay();
  display.drawBitmap(32, 0, epd_bitmap_neo, 64, 64, SSD1306_WHITE);
  display.display();

  if (WiFi.status() != WL_CONNECTED) {
    currentQuote = "Tidak ada WiFi.";
    return;
  }

  HTTPClient http;
  http.setTimeout(5000);

  if (!http.begin("https://zenquotes.io/api/random")) {
    currentQuote = "Gagal membuka koneksi.";
    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc.is<JsonArray>() &&
        doc[0]["q"].is<const char*>()) {
      const char* quote = doc[0]["q"].as<const char*>();
      currentQuote = (quote != nullptr && quote[0] != '\0')
                       ? String(quote)
                       : "Quote kosong.";
    } else {
      currentQuote = "Format quote tidak valid.";
    }
  } else {
    currentQuote = "HTTP error: " + String(httpCode);
  }

  http.end();
}

#endif
