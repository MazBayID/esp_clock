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

bool connectToWiFi(const String& networkSsid,
                   const String& networkPassword,
                   uint32_t timeoutMs) {
  WiFi.disconnect(false, false);
  delay(150);
  WiFi.begin(networkSsid.c_str(), networkPassword.c_str());

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < timeoutMs) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.disconnect(false, false);
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

    Serial.printf("Detected location zone: %s / %s, UTC offset: %ld seconds.\\n",
                  countryCode.c_str(), continentCode.c_str(),
                  utcOffsetSeconds);
  } else {
    Serial.printf("Location lookup failed; using saved UTC offset: %ld seconds.\\n",
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
      escaped += F("\n");
    } else if (character == '\r') {
      escaped += F("\\r");
    } else if (static_cast<uint8_t>(character) >= 0x20) {
      escaped += character;
    }
  }

  return escaped;
}

void sendWiFiPortalPage(const String& message = "") {
  String page = R"NEOPAGE(
<!doctype html>
<html lang="id">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="theme-color" content="#080d18">
  <title>NEO CLOCK · Wi-Fi Setup</title>
  <style>
    :root{color-scheme:dark;font-family:system-ui,-apple-system,sans-serif;background:#080d18;color:#e8f0ff}
    *{box-sizing:border-box}
    body{margin:0;min-height:100vh;display:grid;place-items:center;padding:22px}
    main{width:min(100%,440px);background:#111b2b;border:1px solid #263750;border-radius:22px;padding:25px;box-shadow:0 18px 55px #0008}
    .mark{color:#55d6ff;font-size:12px;font-weight:800;letter-spacing:.22em}
    h1{margin:8px 0 4px;font-size:29px}
    p{color:#9eb0c9;line-height:1.5}
    label{display:block;margin:17px 0 7px;font-size:13px;color:#c6d5e9}
    input,select{width:100%;padding:13px;border:1px solid #344762;border-radius:11px;background:#091321;color:#fff;font-size:15px}
    button{width:100%;border:0;border-radius:11px;padding:13px;margin-top:13px;background:#42c8ed;color:#06111b;font-weight:800;font-size:15px}
    button.secondary{background:#263b55;color:#e8f0ff}
    .note{font-size:12px;color:#8da2bd}
    .message{padding:11px;border-radius:10px;background:#1b3043;color:#b9eaff;margin-top:15px}
    .footer{margin-top:18px;border-top:1px solid #263750;padding-top:14px}
  </style>
</head>
<body>
<main>
  <div class="mark">NEO FIRMWARE</div>
  <h1>NEO CLOCK</h1>
  <p>Hubungkan perangkat ke jaringan Wi-Fi untuk mengaktifkan koneksi internet dan sinkronisasi waktu.</p>
  <p class="note">Jaringan tersimpan: <strong id="count">0</strong> / 5. Jika penuh, jaringan paling lama akan digantikan.</p>
  <button class="secondary" type="button" onclick="scanNetworks()">Cari jaringan Wi-Fi</button>
  <label for="networks">Jaringan yang ditemukan</label>
  <select id="networks" onchange="selectNetwork()">
    <option value="">Pilih jaringan (opsional)</option>
  </select>
  <form method="POST" action="/save">
    <label for="ssid">Nama Wi-Fi (SSID)</label>
    <input id="ssid" name="ssid" maxlength="32" autocomplete="username" required>
    <label for="password">Password Wi-Fi</label>
    <input id="password" name="password" type="password" maxlength="63" autocomplete="current-password">
    <p class="note">Biarkan password kosong jika jaringan tidak menggunakan password.</p>
    <button type="submit">Simpan &amp; Sambungkan</button>
  </form>
  <div id="message" class="message" hidden></div>
  <div class="footer note">Hotspot konfigurasi: <strong>NEO_CLOCK</strong> · Alamat: 192.168.4.1</div>
</main>
<script>
async function scanNetworks(){
  const button=document.querySelector('button.secondary');
  const select=document.getElementById('networks');
  button.textContent='Memindai...';
  button.disabled=true;
  try{
    const response=await fetch('/scan');
    const data=await response.json();
    select.innerHTML='<option value="">Pilih jaringan (opsional)</option>';
    data.networks.forEach(network=>{
      const option=document.createElement('option');
      option.value=network.ssid;
      option.textContent=network.ssid+' ('+network.rssi+' dBm)'+(network.secure?' · terkunci':' · terbuka');
      select.appendChild(option);
    });
  }catch(error){
    const message=document.getElementById('message');
    message.hidden=false;
    message.textContent='Pemindaian gagal. Masukkan SSID secara manual.';
  }
  button.textContent='Cari jaringan Wi-Fi';
  button.disabled=false;
}
function selectNetwork(){
  document.getElementById('ssid').value=document.getElementById('networks').value;
}
fetch('/status').then(response=>response.json()).then(data=>{
  document.getElementById('count').textContent=data.saved;
}).catch(()=>{});
</script>
</body>
</html>
)NEOPAGE";

  page.replace("<strong id=\"count\">0</strong>",
               String("<strong id=\"count\">") +
               String(savedWiFiNetworkCount()) + "</strong>");

  if (message.length() > 0) {
    page.replace("<div id=\"message\" class=\"message\" hidden></div>",
                 String("<div id=\"message\" class=\"message\">") +
                 message + "</div>");
  }

  wifiWebServer.sendHeader("Cache-Control", "no-store");
  wifiWebServer.send(200, "text/html; charset=utf-8", page);
}

void handleWiFiScan() {
  const int found = WiFi.scanNetworks(false, true);
  String json = "{\"networks\":[";

  if (found > 0) {
    for (int i = 0; i < found; ++i) {
      if (i > 0) json += ',';

      const String networkSsid = WiFi.SSID(i);
      const bool secured = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

      json += "{\"ssid\":\"";
      json += jsonEscape(networkSsid);
      json += "\",\"rssi\":";
      json += String(WiFi.RSSI(i));
      json += ",\"secure\":";
      json += secured ? "true" : "false";
      json += '}';
    }
  }

  json += "]}";
  WiFi.scanDelete();
  wifiWebServer.sendHeader("Cache-Control", "no-store");
  wifiWebServer.send(200, "application/json", json);
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

  wifiWebServer.on("/status", HTTP_GET, []() {
    const String json = String("{\"saved\":") +
                        String(savedWiFiNetworkCount()) + "}";
    wifiWebServer.send(200, "application/json", json);
  });

  wifiWebServer.on("/scan", HTTP_GET, []() {
    handleWiFiScan();
  });

  wifiWebServer.on("/save", HTTP_POST, []() {
    const String networkSsid = wifiWebServer.arg("ssid");
    const String networkPassword = wifiWebServer.arg("password");

    if (networkSsid.length() == 0 || networkSsid.length() > 32 ||
        networkPassword.length() > 63) {
      sendWiFiPortalPage("SSID atau password tidak valid. Periksa kembali.");
      return;
    }

    Serial.printf("Trying Wi-Fi network: %s\n", networkSsid.c_str());

    // Wait for one connection attempt, then send exactly one HTTP response.
    if (!connectToWiFi(networkSsid, networkPassword, 15000)) {
      sendWiFiPortalPage(
          "Gagal tersambung. Periksa SSID dan password, lalu coba lagi. Profil yang gagal tidak disimpan.");
      Serial.println(F("Connection failed; saved Wi-Fi list unchanged."));
      return;
    }

    if (!saveWiFiNetwork(networkSsid, networkPassword)) {
      sendWiFiPortalPage(
          "Terhubung, tetapi profil tidak dapat disimpan. Hotspot tetap aktif agar kamu bisa mencoba lagi.");
      Serial.println(F("Connected, but could not save Wi-Fi credentials."));
      return;
    }

    WiFi.setAutoReconnect(true);
    startTimeSynchronization();
    sendWiFiPortalPage(
        "Berhasil tersambung dan jaringan telah disimpan. Hotspot konfigurasi akan ditutup.");
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
