#ifndef NEO_FIRMWARE_WIFI_SYSTEM_H
#define NEO_FIRMWARE_WIFI_SYSTEM_H

#include "../config/config.h"
#include <esp_sntp.h>

namespace {
constexpr uint8_t MAX_SAVED_WIFI_NETWORKS = 5;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 10000;
constexpr char WIFI_PREFERENCES_NAMESPACE[] = "neo-wifi";
constexpr char WIFI_PORTAL_SSID[] = "NEO_CLOCK";

Preferences wifiPreferences;
WebServer wifiWebServer(80);
DNSServer wifiDnsServer;
bool wifiPreferencesReady = false;
bool wifiPortalActive = false;

uint8_t savedWiFiNetworkCount() {
  if (!wifiPreferencesReady) return 0;

  const uint8_t count = wifiPreferences.getUChar("count", 0);
  return count > MAX_SAVED_WIFI_NETWORKS ? MAX_SAVED_WIFI_NETWORKS : count;
}

void showWiFiConnectionMessage(const String& title,
                               const String& network = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print(title);
  if (network.length() > 0) {
    display.setCursor(0, 34);
    display.print(network.substring(0, 20));
  }
  display.display();
}

bool connectToWiFi(const String& networkSsid,
                   const String& networkPassword,
                   uint32_t timeoutMs) {
  showWiFiConnectionMessage("CONNECTING TO WIFI", networkSsid);
  WiFi.disconnect(false, false);
  delay(150);
  WiFi.begin(networkSsid.c_str(), networkPassword.c_str());

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < timeoutMs) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    showWiFiConnectionMessage("CONNECTED", networkSsid);
    return true;
  }

  showWiFiConnectionMessage("WIFI FAILED", networkSsid);
  WiFi.disconnect(false, false);
  delay(700);
  return false;
}

bool saveWiFiNetwork(const String& networkSsid,
                     const String& networkPassword) {
  if (!wifiPreferencesReady || networkSsid.length() == 0 ||
      networkSsid.length() > 32 || networkPassword.length() > 63) {
    return false;
  }

  String ssids[MAX_SAVED_WIFI_NETWORKS];
  String passwords[MAX_SAVED_WIFI_NETWORKS];
  uint8_t count = savedWiFiNetworkCount();

  // Preserve the existing order, but move a refreshed SSID to newest.
  uint8_t kept = 0;
  for (uint8_t i = 0; i < count; ++i) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", i);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", i);

    const String existingSsid = wifiPreferences.getString(ssidKey, "");
    if (existingSsid.length() == 0 || existingSsid == networkSsid) {
      continue;
    }

    ssids[kept] = existingSsid;
    passwords[kept] = wifiPreferences.getString(passwordKey, "");
    ++kept;
  }
  count = kept;

  // FIFO: remove the oldest entry only when a new successful profile is added.
  if (count >= MAX_SAVED_WIFI_NETWORKS) {
    for (uint8_t i = 1; i < count; ++i) {
      ssids[i - 1] = ssids[i];
      passwords[i - 1] = passwords[i];
    }
    --count;
  }

  ssids[count] = networkSsid;
  passwords[count] = networkPassword;
  ++count;

  bool saved = true;

  // Write the new profile list first; update the count last.
  for (uint8_t i = 0; i < MAX_SAVED_WIFI_NETWORKS; ++i) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", i);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", i);

    if (i < count) {
      wifiPreferences.putString(ssidKey, ssids[i]);
      wifiPreferences.putString(passwordKey, passwords[i]);

      // putString() returns strlen(value), so a valid empty password returns 0.
      // isKey() distinguishes that valid open-network password from a failed write.
      if (!wifiPreferences.isKey(ssidKey) ||
          !wifiPreferences.isKey(passwordKey)) {
        saved = false;
      }
    } else {
      wifiPreferences.remove(ssidKey);
      wifiPreferences.remove(passwordKey);
    }
  }

  if (saved && wifiPreferences.putUChar("count", count) == 0) {
    saved = false;
  }

  return saved;
}

// IP-based geolocation supplies the current UTC offset and country/continent.
// The free ip-api endpoint is HTTP-only; it receives the network's public IP.
volatile bool ntpSyncReceived = false;
String ntpServer1;
String ntpServer2;
String ntpServer3;

void onNtpTimeSync(struct timeval* timeValue) {
  ntpSyncReceived = true;
}

String getRegionalNtpServer(const String& continentCode) {
  if (continentCode == "AS") return "asia.pool.ntp.org";
  if (continentCode == "EU") return "europe.pool.ntp.org";
  if (continentCode == "NA") return "north-america.pool.ntp.org";
  if (continentCode == "OC") return "oceania.pool.ntp.org";
  if (continentCode == "SA") return "south-america.pool.ntp.org";
  return "pool.ntp.org";
}

bool detectNetworkTimeZone(long& utcOffsetSeconds,
                           String& countryCode,
                           String& continentCode) {
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(5000);

  if (!http.begin(client,
      "http://ip-api.com/json/?fields=status,countryCode,continentCode,offset")) {
    return false;
  }

  const int responseCode = http.GET();
  if (responseCode != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  JsonDocument document;
  const DeserializationError error =
      deserializeJson(document, http.getStream());
  http.end();

  if (error || document["status"] != "success") return false;

  const long detectedOffset = document["offset"] | 0L;
  const String detectedCountry = document["countryCode"] | "";
  const String detectedContinent = document["continentCode"] | "";

  if (detectedOffset < -43200L || detectedOffset > 50400L ||
      detectedCountry.length() != 2 || detectedContinent.length() != 2) {
    return false;
  }

  utcOffsetSeconds = detectedOffset;
  countryCode = detectedCountry;
  continentCode = detectedContinent;
  return true;
}

bool waitForNtpSynchronization(uint32_t timeoutMs) {
  const unsigned long startedAt = millis();

  while (!ntpSyncReceived && millis() - startedAt < timeoutMs) {
    delay(100);
  }

  return ntpSyncReceived;
}

void startTimeSynchronization() {
  // Restore the last known timezone before attempting a fresh lookup.
  long utcOffsetSeconds = wifiPreferencesReady
      ? wifiPreferences.getLong("tzOffset", gmtOffset_sec)
      : gmtOffset_sec;
  String countryCode = wifiPreferencesReady
      ? wifiPreferences.getString("tzCountry", "ID")
      : "ID";
  String continentCode = wifiPreferencesReady
      ? wifiPreferences.getString("tzCont", "AS")
      : "AS";

  if (detectNetworkTimeZone(utcOffsetSeconds, countryCode, continentCode)) {
    if (wifiPreferencesReady) {
      wifiPreferences.putLong("tzOffset", utcOffsetSeconds);
      wifiPreferences.putString("tzCountry", countryCode);
      wifiPreferences.putString("tzCont", continentCode);
    }

    Serial.printf("Detected location zone: %s / %s, UTC offset: %ld seconds.\n",
                  countryCode.c_str(), continentCode.c_str(),
                  utcOffsetSeconds);
  } else {
    Serial.printf("Location lookup failed; using saved UTC offset: %ld seconds.\n",
                  utcOffsetSeconds);
  }

  // Keep timezone independent from the NTP server selected below.
  ntpSyncReceived = false;
  sntp_set_time_sync_notification_cb(onNtpTimeSync);

  if (countryCode == "ID") {
    ntpServer1 = "ntp.bmkg.go.id";
    ntpServer2 = "id.pool.ntp.org";
  } else {
    ntpServer1 = countryCode;
    ntpServer1.toLowerCase();
    ntpServer1 += ".pool.ntp.org";
    ntpServer2 = getRegionalNtpServer(continentCode);
  }
  ntpServer3 = getRegionalNtpServer(continentCode);

  Serial.printf("Trying regional NTP: %s, %s, %s\n",
                ntpServer1.c_str(), ntpServer2.c_str(),
                ntpServer3.c_str());
  configTime(utcOffsetSeconds, 0, ntpServer1.c_str(),
             ntpServer2.c_str(), ntpServer3.c_str());

  if (waitForNtpSynchronization(15000)) {
    Serial.println(F("Regional NTP synchronization successful."));
    return;
  }

  Serial.println(F("Regional NTP failed; switching to global NTP."));
  ntpSyncReceived = false;
  ntpServer1 = "pool.ntp.org";
  ntpServer2 = "time.google.com";
  ntpServer3 = "time.nist.gov";

  configTime(utcOffsetSeconds, 0, ntpServer1.c_str(),
             ntpServer2.c_str(), ntpServer3.c_str());

  if (waitForNtpSynchronization(15000)) {
    Serial.println(F("Global NTP synchronization successful."));
  } else {
    Serial.println(F("NTP synchronization failed; will retry on next Wi-Fi connection."));
  }
}

String jsonEscape(const String& value) {
  String escaped;
  escaped.reserve(value.length() + 8);

  for (size_t i = 0; i < value.length(); ++i) {
    const char character = value[i];

    if (character == '"' || character == '\\') {
      escaped += '\\';
      escaped += character;
    } else if (character == '\n') {
      escaped += F("\\n");
    } else if (character == '\r') {
      escaped += F("\\r");
    } else if (static_cast<uint8_t>(character) >= 0x20) {
      escaped += character;
    }
  }

  return escaped;
}

void sendWiFiPortalPage() {
  const char page[] = R"NEOPAGE(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>NEO CLOCK</title>
  <style>
    body{font-family:Arial,sans-serif;max-width:360px;margin:48px auto;padding:0 18px;color:#111}
    h1{font-size:26px}
    label{display:block;margin:18px 0 6px}
    input,button{box-sizing:border-box;width:100%;padding:12px;font-size:16px}
    button{margin-top:20px;cursor:pointer}
    #popup{display:none;position:fixed;inset:0;background:#0008;align-items:center;justify-content:center}
    #popup div{background:#fff;padding:24px 32px;border-radius:8px}
  </style>
</head>
<body>
  <h1>NEO CLOCK</h1>
  <form id="wifi-form">
    <label for="ssid">Wi-Fi Name (SSID)</label>
    <input id="ssid" name="ssid" maxlength="32" autocomplete="username" required>
    <label for="password">Password</label>
    <input id="password" name="password" type="password" maxlength="63" autocomplete="current-password">
    <button id="connect" type="submit">Connect</button>
  </form>
  <div id="popup" role="status" aria-live="polite"><div id="popup-text">Connecting...</div></div>
  <script>
    const form=document.getElementById('wifi-form');
    const popup=document.getElementById('popup');
    const popupText=document.getElementById('popup-text');
    const button=document.getElementById('connect');
    form.addEventListener('submit',async event=>{
      event.preventDefault();
      popupText.textContent='Connecting...';
      popup.style.display='flex';
      button.disabled=true;
      try{
        const response=await fetch('/connect',{method:'POST',body:new URLSearchParams(new FormData(form))});
        const result=await response.json();
        popupText.textContent=result.success?'Done':(result.message||'Connection failed. Please try again.');
        if(result.success){
          form.reset();
        }else{
          button.disabled=false;
          setTimeout(()=>{popup.style.display='none';},1800);
        }
      }catch(error){
        popupText.textContent='Connection failed. Please try again.';
        button.disabled=false;
        setTimeout(()=>{popup.style.display='none';},1800);
      }
    });
  </script>
</body>
</html>
)NEOPAGE";
  wifiWebServer.sendHeader("Cache-Control", "no-store");
  wifiWebServer.send(200, "text/html; charset=utf-8", page);
}

void stopWiFiPortal() {
  if (!wifiPortalActive) return;

  wifiDnsServer.stop();
  wifiWebServer.stop();
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_STA);
  wifiPortalActive = false;
}

void startWiFiPortal() {
  if (wifiPortalActive) return;

  Serial.println(F("Starting open Wi-Fi setup hotspot: NEO_CLOCK"));
  WiFi.disconnect(false, false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  if (!WiFi.softAP(WIFI_PORTAL_SSID, nullptr, 1, false, 4)) {
    Serial.println(F("Could not start NEO_CLOCK hotspot."));
    return;
  }

  wifiDnsServer.start(53, "*", WiFi.softAPIP());

  wifiWebServer.on("/", HTTP_GET, []() {
    sendWiFiPortalPage();
  });

  wifiWebServer.on("/connect", HTTP_POST, []() {
    const String networkSsid = wifiWebServer.arg("ssid");
    const String networkPassword = wifiWebServer.arg("password");

    if (networkSsid.length() == 0 || networkSsid.length() > 32 ||
        networkPassword.length() > 63) {
      wifiWebServer.send(400, "application/json",
                        "{\"success\":false,\"message\":\"Invalid Wi-Fi details.\"}");
      return;
    }

    Serial.printf("Trying Wi-Fi network: %s\n", networkSsid.c_str());
    if (!connectToWiFi(networkSsid, networkPassword, 15000)) {
      wifiWebServer.send(400, "application/json",
                        "{\"success\":false,\"message\":\"Connection failed. Check the SSID and password.\"}");
      Serial.println(F("Connection failed; saved Wi-Fi list unchanged."));
      return;
    }

    if (!saveWiFiNetwork(networkSsid, networkPassword)) {
      wifiWebServer.send(500, "application/json",
                        "{\"success\":false,\"message\":\"Connected, but could not save the network.\"}");
      Serial.println(F("Connected, but could not save Wi-Fi credentials."));
      return;
    }

    WiFi.setAutoReconnect(true);
    startTimeSynchronization();
    wifiWebServer.send(200, "application/json", "{\"success\":true}");
    delay(1200);
    stopWiFiPortal();
  });

  wifiWebServer.onNotFound([]() {
    wifiWebServer.sendHeader(
        "Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    wifiWebServer.send(302, "text/plain", "");
  });

  wifiWebServer.begin();
  wifiPortalActive = true;

  Serial.print(F("Wi-Fi setup page: http://"));
  Serial.println(WiFi.softAPIP());
}
}  // namespace

void setupWiFi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  wifiPreferencesReady =
      wifiPreferences.begin(WIFI_PREFERENCES_NAMESPACE, false);

  if (!wifiPreferencesReady) {
    Serial.println(F("Could not open Wi-Fi credential storage."));
  }

  const uint8_t count = savedWiFiNetworkCount();
  bool scanSucceeded = false;
  int visibleNetworkCount = 0;

  if (count > 0) {
    // Scan once so unavailable SSIDs are skipped instead of waiting 10 seconds
    // for each missing profile. Hidden networks are included in the scan.
    visibleNetworkCount = WiFi.scanNetworks(false, true);
    scanSucceeded = visibleNetworkCount >= 0;
  }

  // Newer profiles are tried first; storage order remains oldest-to-newest.
  for (int profile = static_cast<int>(count) - 1; profile >= 0; --profile) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", profile);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", profile);

    const String savedSsid = wifiPreferencesReady
                                 ? wifiPreferences.getString(ssidKey, "")
                                 : "";
    const String savedPassword = wifiPreferencesReady
                                     ? wifiPreferences.getString(passwordKey, "")
                                     : "";

    if (savedSsid.length() == 0) continue;

    bool visible = !scanSucceeded;
    if (scanSucceeded) {
      for (int found = 0; found < visibleNetworkCount; ++found) {
        if (WiFi.SSID(found) == savedSsid) {
          visible = true;
          break;
        }
      }
    }

    if (!visible) {
      Serial.printf("Saved Wi-Fi not found nearby: %s\n", savedSsid.c_str());
      continue;
    }

    Serial.printf("Trying saved Wi-Fi profile: %s\n", savedSsid.c_str());
    if (connectToWiFi(savedSsid, savedPassword, WIFI_CONNECT_TIMEOUT_MS)) {
      WiFi.scanDelete();
      WiFi.setAutoReconnect(true);
      startTimeSynchronization();
      return;
    }
  }

  if (count > 0) {
    WiFi.scanDelete();
  }

  // Backward-compatible one-time bootstrap from an existing local secrets.h.
  if (ssid != nullptr && ssid[0] != '\0') {
    Serial.printf("Trying local bootstrap Wi-Fi: %s\n", ssid);
    const String bootstrapSsid = ssid;
    const String bootstrapPassword = password == nullptr ? "" : password;

    if (connectToWiFi(bootstrapSsid, bootstrapPassword,
                      WIFI_CONNECT_TIMEOUT_MS)) {
      saveWiFiNetwork(bootstrapSsid, bootstrapPassword);
      WiFi.setAutoReconnect(true);
      startTimeSynchronization();
      return;
    }
  }

  Serial.println(F("No saved Wi-Fi network connected. Starting setup portal."));
  startWiFiPortal();
}

void handleWiFiPortal() {
  if (!wifiPortalActive) return;

  wifiDnsServer.processNextRequest();
  wifiWebServer.handleClient();
}

#endif
