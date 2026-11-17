#ifndef NEO_FIRMWARE_WIFI_SYSTEM_H
#define NEO_FIRMWARE_WIFI_SYSTEM_H

#include "../config/config.h"

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

  uint8_t count = wifiPreferences.getUChar("count", 0);
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

  return WiFi.status() == WL_CONNECTED;
}

bool saveWiFiNetwork(const String& networkSsid,
                     const String& networkPassword) {
  if (!wifiPreferencesReady || networkSsid.length() == 0 ||
      networkSsid.length() > 32 || networkPassword.length() > 64) {
    return false;
  }

  String ssids[MAX_SAVED_WIFI_NETWORKS];
  String passwords[MAX_SAVED_WIFI_NETWORKS];
  uint8_t count = savedWiFiNetworkCount();

  // Keep existing entries in order, except a duplicate that is being refreshed.
  uint8_t kept = 0;
  for (uint8_t i = 0; i < count; ++i) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", i);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", i);

    String existingSsid = wifiPreferences.getString(ssidKey, "");
    if (existingSsid == networkSsid) continue;

    ssids[kept] = existingSsid;
    passwords[kept] = wifiPreferences.getString(passwordKey, "");
    ++kept;
  }
  count = kept;

  // FIFO: make room by dropping the oldest saved entry.
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

  for (uint8_t i = 0; i < MAX_SAVED_WIFI_NETWORKS; ++i) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", i);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", i);

    if (i < count) {
      wifiPreferences.putString(ssidKey, ssids[i]);
      wifiPreferences.putString(passwordKey, passwords[i]);
    } else {
      wifiPreferences.remove(ssidKey);
      wifiPreferences.remove(passwordKey);
    }
  }

  wifiPreferences.putUChar("count", count);
  return true;
}

void startTimeSynchronization() {
  configTime(gmtOffset_sec, daylightOffset_sec, "ntp.bmkg.go.id");
  Serial.println(F("Wi-Fi connected; NTP synchronization requested."));
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
    <input id="password" name="password" type="password" maxlength="64" autocomplete="current-password">
    <p class="note">Biarkan password kosong jika jaringan tidak menggunakan password.</p>
    <button type="submit">Simpan &amp; Sambungkan</button>
  </form>
  <div id="message" class="message" hidden></div>
  <div class="footer note">Terhubung ke hotspot perangkat <strong>NEO_CLOCK</strong>. Halaman ini hanya mengatur jaringan Wi-Fi.</div>
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
    document.getElementById('message').hidden=false;
    document.getElementById('message').textContent='Pemindaian gagal. Masukkan SSID secara manual.';
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

  page.replace("<strong id=\"count\">0</strong>", String("<strong id=\"count\">") +
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
  wifiDnsServer.stop();
  wifiWebServer.stop();
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_STA);
  wifiPortalActive = false;
}

void startWiFiPortal() {
  if (wifiPortalActive) return;

  Serial.println(F("Starting open Wi-Fi setup hotspot: NEO_CLOCK"));
  WiFi.disconnect(false, true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  if (!WiFi.softAP(WIFI_PORTAL_SSID)) {
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
        networkPassword.length() > 64) {
      sendWiFiPortalPage("SSID atau password tidak valid. Periksa kembali.");
      return;
    }

    sendWiFiPortalPage("Sedang mencoba menyambungkan ke jaringan...");
    Serial.printf("Trying Wi-Fi network: %s\n", networkSsid.c_str());

    if (!connectToWiFi(networkSsid, networkPassword, 15000)) {
      sendWiFiPortalPage("Gagal tersambung. Periksa SSID dan password, lalu coba lagi.");
      Serial.println(F("New Wi-Fi network connection failed; saved list unchanged."));
      return;
    }

    if (!saveWiFiNetwork(networkSsid, networkPassword)) {
      sendWiFiPortalPage("Terhubung, tetapi data tidak dapat disimpan. Periksa penyimpanan perangkat.");
      Serial.println(F("Connected, but could not save Wi-Fi credentials."));
      return;
    }

    WiFi.setAutoReconnect(true);
    startTimeSynchronization();
    sendWiFiPortalPage("Berhasil tersambung dan jaringan telah disimpan. Hotspot konfigurasi akan ditutup.");
    delay(1000);
    stopWiFiPortal();
  });

  wifiWebServer.onNotFound([]() {
    wifiWebServer.sendHeader("Location",
      String("http://") + WiFi.softAPIP().toString() + "/", true);
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

  wifiPreferencesReady = wifiPreferences.begin(WIFI_PREFERENCES_NAMESPACE, false);
  if (!wifiPreferencesReady) {
    Serial.println(F("Could not open Wi-Fi credential storage."));
  }

  const uint8_t count = savedWiFiNetworkCount();
  for (uint8_t i = 0; i < count; ++i) {
    char ssidKey[8];
    char passwordKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%u", i);
    snprintf(passwordKey, sizeof(passwordKey), "pass%u", i);

    const String savedSsid = wifiPreferences.getString(ssidKey, "");
    const String savedPassword = wifiPreferences.getString(passwordKey, "");
    if (savedSsid.length() == 0) continue;

    Serial.printf("Trying saved Wi-Fi %u/%u: %s\n",
                  static_cast<unsigned>(i + 1),
                  static_cast<unsigned>(count),
                  savedSsid.c_str());

    if (connectToWiFi(savedSsid, savedPassword, WIFI_CONNECT_TIMEOUT_MS)) {
      WiFi.setAutoReconnect(true);
      startTimeSynchronization();
      return;
    }
  }

  // Keep support for a local secrets.h as a one-time bootstrap fallback.
  if (ssid != nullptr && ssid[0] != '\0') {
    Serial.printf("Trying local fallback Wi-Fi: %s\n", ssid);
    const String fallbackSsid = ssid;
    const String fallbackPassword = password == nullptr ? "" : password;

    if (connectToWiFi(fallbackSsid, fallbackPassword, WIFI_CONNECT_TIMEOUT_MS)) {
      saveWiFiNetwork(fallbackSsid, fallbackPassword);
      WiFi.setAutoReconnect(true);
      startTimeSynchronization();
      return;
    }
  }

  Serial.println(F("No configured Wi-Fi network connected. Starting setup portal."));
  startWiFiPortal();
}

void handleWiFiPortal() {
  if (!wifiPortalActive) return;

  wifiDnsServer.processNextRequest();
  wifiWebServer.handleClient();
}

#endif
