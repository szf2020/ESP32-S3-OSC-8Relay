#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

#include "config.h"
#include "pca9554.h"
#include "osc_router.h"
#include "web_ui.h"
#include "changelog_data.h"
#include "logger.h"
#include "watchdog.h"
#include "mutex.h"
#include "network_mgr.h"
#include "led_status.h"

// ========== HARDWARE PINOUT ==========
// Ethernet (W5500 SPI): SCK=15, MISO=14, MOSI=13, CS=16, INT=12, RST=-1
// I2C (PCA9554 + RTC): SDA=42, SCL=41
// RGB LED = GPIO38, Buzzer = GPIO46


static constexpr int PIN_RGB = 38;
static constexpr int PIN_BUZZER = 46;

// ========== GLOBALS ==========
static ConfigStore gStore;
static AppCfg gCfg;
static WebServer gWeb(80);
static PCA9554 gRelayExpander(0x20);
static OscRelayRouter gOsc;
static DNSServer gDns;

// 🔒 MUTEX PROTECTION : Protège l'accès concurrent aux relais
// Raison : Le thread Web ET le thread OSC accèdent à gRelayLogical simultanément
//          Sans mutex = race condition = corruption de données !
static Mutex gRelayMutex;

static bool gRelayLogical[8] = {false};
static bool gEthLinked = false;
static IPAddress gEthIp(0, 0, 0, 0);
static bool gWiFiApActive = false;
static unsigned long gLastOscLoop = 0;
static unsigned long gLastWebLoop = 0;
static unsigned long gLastStatusUpdate = 0;
static unsigned long gLastWatchdogFeed = 0;

// 📡 AP auto-management
// AP timeout calculé dynamiquement depuis gCfg.apTimeoutMin
static unsigned long gApLastClientSeen = 0;  // dernier moment avec client connecté
static bool gApForcedOff = false;            // désactivé via OSC
static bool gApReloadPending = false;        // hot-reload AP différé post-HTTP
static unsigned long gApReloadTime = 0;
static bool gRebootPending = false;          // reboot différé post-HTTP
static bool gFactoryResetPending = false;    // factory reset différé post-HTTP
static unsigned long gPendingActionTime = 0;

// 📨 OSC message log ring buffer
static constexpr int OSC_LOG_SIZE = 20;
struct OscLogEntry {
  unsigned long ts;       // millis() timestamp
  char address[48];
  char typeTag[4];
  char value[16];
};
static OscLogEntry gOscLog[OSC_LOG_SIZE];
static int gOscLogHead = 0;
static int gOscLogCount = 0;

// 💾 Deferred NVS save: avoid blocking flash writes in OSC hot path
static bool gRelayStatesDirty = false;
static unsigned long gRelayDirtyTime = 0;
static constexpr unsigned long NVS_SAVE_DEFER_MS = 500;  // save 500ms after last change

void recordOscMessage(const char* address, const char* typeTag, const char* valueStr) {
  OscLogEntry& e = gOscLog[gOscLogHead];
  e.ts = millis();
  strncpy(e.address, address, sizeof(e.address) - 1);
  e.address[sizeof(e.address) - 1] = 0;
  strncpy(e.typeTag, typeTag, sizeof(e.typeTag) - 1);
  e.typeTag[sizeof(e.typeTag) - 1] = 0;
  strncpy(e.value, valueStr, sizeof(e.value) - 1);
  e.value[sizeof(e.value) - 1] = 0;
  gOscLogHead = (gOscLogHead + 1) % OSC_LOG_SIZE;
  if (gOscLogCount < OSC_LOG_SIZE) gOscLogCount++;
}

// ========== FORWARD DECLARATIONS ==========
void setupWebServer();
void handleRelayCommand(uint8_t relayIdx, bool newState);
void handleSystemOsc(const char* address, bool value);
void updatePhysicalRelay(uint8_t idx);
void updateAllPhysicalRelays();
void setupRGB(uint8_t r, uint8_t g, uint8_t b);

static String getCaptivePortalRedirectUrl() {
  IPAddress apIp = NETMGR.getWiFiAPIP();
  if (apIp == IPAddress(0, 0, 0, 0)) {
    apIp = gCfg.apIp;
  }
  return String("http://") + apIp.toString() + "/";
}

// ========== Legacy stub (kept for compatibility, now uses LedStatus) ==========
void setupRGB(uint8_t r, uint8_t g, uint8_t b) {
  LedStatus::setRGB(r, g, b);
}

// ========== RELAY CONTROL WITH MUTEX PROTECTION ==========
// 🔒 Fonction critique : protégée par mutex
// Raison : appelée depuis le thread Web ET depuis le thread OSC
void handleRelayCommand(uint8_t relayIdx, bool newState) {
  if (relayIdx >= 8) return;

  // 🔒 Acquérir le verrou (attendre max 100ms)
  LockGuard lock(gRelayMutex, 100);
  if (!lock.isLocked()) {
    LOG_ERROR("RELAY", "Mutex timeout for relay %d", relayIdx);
    return;
  }

  const RelayCfg& cfg = gCfg.relays[relayIdx];

  // Gestion des modes : Toggle vs Latch
  if (cfg.mode == RelayMode::Toggle) {
    // 🔄 Mode TOGGLE : la valeur true = basculer l'état
    // Exemple : /relay/1 1 → bascule l'état
    if (newState) {
      gRelayLogical[relayIdx] = !gRelayLogical[relayIdx];
    }
  } else {
    // 📍 Mode LATCH : la valeur = l'état direct
    // Exemple : /relay/1 0 → OFF, /relay/1 1 → ON
    gRelayLogical[relayIdx] = newState;
  }

  // Appliquer l'état physique au PCA9554
  updatePhysicalRelay(relayIdx);

  // 💾 Marquer pour sauvegarde différée (évite écriture flash bloquante dans le hot path OSC)
  gRelayStatesDirty = true;
  gRelayDirtyTime = millis();

  // �📝 Log détaillé pour traçabilité
  LOG_RELAY("RELAY", 
    "Relay %d set to %s (mode=%s, invert=%s)", 
    relayIdx,
    gRelayLogical[relayIdx] ? "ON" : "OFF",
    cfg.mode == RelayMode::Toggle ? "TOGGLE" : "LATCH",
    cfg.invert ? "YES" : "NO"
  );
}

void updatePhysicalRelay(uint8_t idx) {
  if (idx >= 8) return;

  bool logical = gRelayLogical[idx];
  // 🔄 Inversion : logical XOR invert
  //    true  XOR false = true  (relais ON)
  //    true  XOR true  = false (relais OFF)
  //    false XOR false = false (relais OFF)
  //    false XOR true  = true  (relais ON)
  bool physical = logical ^ gCfg.relays[idx].invert;

  gRelayExpander.writeChannel(idx, physical);
}

void updateAllPhysicalRelays() {
  bool physical[8];
  for (int i = 0; i < 8; i++) {
    physical[i] = gRelayLogical[i] ^ gCfg.relays[i].invert;
  }
  gRelayExpander.writeAll(physical);
}

// ========== OSC SYSTEM COMMAND HANDLER ==========
void handleSystemOsc(const char* address, bool value) {
  if (strcmp(address, "/ap") == 0) {
    if (value) {
      // Allumer l'AP
      gApForcedOff = false;
      if (!NETMGR.isWiFiAPActive()) {
        NETMGR.startWiFiAP(&gCfg);
        gApLastClientSeen = millis();  // reset timer
        LOG_INFO("OSC", "📡 AP enabled via OSC");
      }
    } else {
      // Éteindre l'AP
      gApForcedOff = true;
      if (NETMGR.isWiFiAPActive()) {
        NETMGR.stopWiFiAP();
        LOG_INFO("OSC", "📡 AP disabled via OSC");
      }
    }
  } else if (strcmp(address, "/reboot") == 0 && value) {
    LOG_INFO("OSC", "🔄 Reboot requested via OSC");
    delay(200);
    ESP.restart();
  } else if (strcmp(address, "/factory-reset") == 0 && value) {
    LOG_INFO("OSC", "🏭 Factory reset requested via OSC");
    gStore.factoryReset();
    delay(200);
    ESP.restart();
  }
}

// ========== WEB SERVER SETUP ==========
void setupWebServer() {
  LOG_INFO("WEB", "Setting up Web Server on port 80...");
  
  // Debug: Print current network state
  LOG_INFO("WEB", "WiFi mode: %d (AP=2, STA=1, OFF=0)", WiFi.getMode());
  LOG_INFO("WEB", "WiFi AP IP: %s", WiFi.softAPIP().toString().c_str());
  LOG_INFO("WEB", "Station IP: %s", WiFi.localIP().toString().c_str());
  LOG_INFO("WEB", "Ethernet IP from NETMGR: %s", NETMGR.getEthernetIP().toString().c_str());

  // 📄 Route GET "/" : Servir la page HTML principale
  gWeb.on("/", []() {
    LOG_INFO("WEB", "📄 GET / from %s", gWeb.client().remoteIP().toString().c_str());
    gWeb.send(200, "text/html", INDEX_HTML);
  });

  // 🌐 Captive Portal detection routes
  // Apple iOS/macOS
  gWeb.on("/hotspot-detect.html", []() {
    LOG_INFO("WEB", "🌐 Captive portal (Apple) from %s", gWeb.client().remoteIP().toString().c_str());
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  // Android
  gWeb.on("/generate_204", []() {
    LOG_INFO("WEB", "🌐 Captive portal (Android) from %s", gWeb.client().remoteIP().toString().c_str());
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  gWeb.on("/gen_204", []() {
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  // Windows
  gWeb.on("/connecttest.txt", []() {
    LOG_INFO("WEB", "🌐 Captive portal (Windows) from %s", gWeb.client().remoteIP().toString().c_str());
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  gWeb.on("/ncsi.txt", []() {
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  gWeb.on("/redirect", []() {
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  // Firefox
  gWeb.on("/success.txt", []() {
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });
  // Microsoft NCSI
  gWeb.on("/fwlink", []() {
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });

  // Redirect all unknown non-API requests to main page (captive portal fallback)
  gWeb.onNotFound([]() {
    String uri = gWeb.uri();
    // Don't redirect API calls — return 404 for those
    if (uri.startsWith("/api/")) {
      LOG_WARN("WEB", "🔴 404 Not Found: %s from %s", 
        uri.c_str(), gWeb.client().remoteIP().toString().c_str());
      gWeb.send(404, "text/plain", "Not Found");
      return;
    }
    LOG_INFO("WEB", "🌐 Captive redirect: %s from %s", 
      uri.c_str(), gWeb.client().remoteIP().toString().c_str());
    gWeb.sendHeader("Location", getCaptivePortalRedirectUrl());
    gWeb.send(302, "text/plain", "");
  });

  // 🔧 API: GET /api/config - Retourner la configuration complète en JSON
  gWeb.on("/api/config", HTTP_GET, []() {
    LOG_DEBUG("WEB", "🔧 GET /api/config from %s", gWeb.client().remoteIP().toString().c_str());
    StaticJsonDocument<2048> doc;
    
    // Configuration OSC
    doc["oscListenPort"] = gCfg.oscListenPort;
    doc["hostname"] = gCfg.hostname;

    // Configuration Ethernet
    doc["eth"]["dhcp"] = gCfg.eth.dhcp;
    doc["eth"]["ip"] = gCfg.eth.ip.toString();
    doc["eth"]["gw"] = gCfg.eth.gw.toString();
    doc["eth"]["mask"] = gCfg.eth.mask.toString();
    doc["eth"]["dns1"] = gCfg.eth.dns1.toString();
    doc["eth"]["dns2"] = gCfg.eth.dns2.toString();

    // Configuration WiFi AP
    doc["wifiApAllowed"] = gCfg.wifiApAllowed;
    doc["apSsid"] = gCfg.apSsid;
    doc["apPass"] = gCfg.apPass;
    doc["apIp"] = gCfg.apIp.toString();
    doc["apGw"] = gCfg.apGw.toString();
    doc["apMask"] = gCfg.apMask.toString();
    doc["apTimeoutMin"] = gCfg.apTimeoutMin;

    // Configuration des relais
    JsonArray relays = doc.createNestedArray("relays");
    for (int i = 0; i < 8; i++) {
      JsonObject r = relays.createNestedObject();
      r["oscAddress"] = gCfg.relays[i].oscAddress;
      r["invert"] = gCfg.relays[i].invert;
      r["mode"] = (int)gCfg.relays[i].mode;
    }

    String response;
    serializeJson(doc, response);
    gWeb.send(200, "application/json", response);
    
    LOG_INFO("WEB", "Config requested");
  });

  // 📊 API: GET /api/relays/status - État actuel des 8 relais
  gWeb.on("/api/relays/status", HTTP_GET, []() {
    LOG_DEBUG("WEB", "📊 GET /api/relays/status from %s", gWeb.client().remoteIP().toString().c_str());
    // 🔒 Protéger la lecture de gRelayLogical
    LockGuard lock(gRelayMutex, 100);
    
    StaticJsonDocument<256> doc;
    for (int i = 0; i < 8; i++) {
      doc[i] = gRelayLogical[i];
    }
    String response;
    serializeJson(doc, response);
    gWeb.send(200, "application/json", response);
  });

  // ⚡ API: POST /api/relays/0 à /api/relays/7 - Contrôler un relais individuel
  for (int i = 0; i < 8; i++) {
    char route[32];
    snprintf(route, sizeof(route), "/api/relays/%d", i);
    gWeb.on(route, HTTP_POST, [i]() {
      LOG_DEBUG("WEB", "⚡ POST /api/relays/%d from %s", i, gWeb.client().remoteIP().toString().c_str());
      String body = gWeb.arg("plain");
      StaticJsonDocument<128> doc;
      DeserializationError err = deserializeJson(doc, body);
      if (err) {
        gWeb.send(400, "text/plain", "Invalid JSON");
        return;
      }
      
      if (doc.containsKey("state")) {
        handleRelayCommand(i, doc["state"].as<bool>());
        gWeb.send(200, "text/plain", "OK");
      } else {
        gWeb.send(400, "text/plain", "Invalid request");
      }
    });
  }

  // ⚡ API: POST /api/relays/all - ALL ON / ALL OFF
  gWeb.on("/api/relays/all", HTTP_POST, []() {
    LOG_DEBUG("WEB", "⚡ POST /api/relays/all from %s", gWeb.client().remoteIP().toString().c_str());
    String body = gWeb.arg("plain");
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      gWeb.send(400, "text/plain", "Invalid JSON");
      return;
    }

    if (doc.containsKey("state")) {
      bool state = doc["state"].as<bool>();
      for (int i = 0; i < 8; i++) {
        handleRelayCommand(i, state);
      }
      gWeb.send(200, "text/plain", "OK");
    } else {
      gWeb.send(400, "text/plain", "Invalid request");
    }
  });

  // 🎛️ API: POST /api/config/relays - Sauvegarder config des relais
  gWeb.on("/api/config/relays", HTTP_POST, []() {
    String body = gWeb.arg("plain");
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      gWeb.send(400, "text/plain", "Invalid JSON");
      return;
    }

    if (!doc["relays"].is<JsonArray>()) {
      gWeb.send(400, "text/plain", "Missing relays array");
      return;
    }

    JsonArray relays = doc["relays"];
    for (int i = 0; i < 8 && i < (int)relays.size(); i++) {
      strlcpy(gCfg.relays[i].oscAddress, relays[i]["oscAddress"], 
              sizeof(gCfg.relays[i].oscAddress));
      gCfg.relays[i].invert = relays[i]["invert"];
      gCfg.relays[i].mode = (RelayMode)(int)relays[i]["mode"];
    }

    gStore.save(gCfg);
    gOsc.setConfig(&gCfg);
    gWeb.send(200, "text/plain", "OK");
    
    LOG_INFO("WEB", "Relay config updated");
  });

  // 🌐 API: POST /api/config/network - Sauvegarder config réseau
  gWeb.on("/api/config/network", HTTP_POST, []() {
    String body = gWeb.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      gWeb.send(400, "text/plain", "Invalid JSON");
      return;
    }

    gCfg.hostname[0] = 0;
    if (doc.containsKey("hostname")) {
      strlcpy(gCfg.hostname, doc["hostname"], sizeof(gCfg.hostname));
    }

    if (doc.containsKey("eth")) {
      gCfg.eth.dhcp = doc["eth"]["dhcp"];
      gCfg.eth.ip.fromString(doc["eth"]["ip"].as<String>());
      gCfg.eth.gw.fromString(doc["eth"]["gw"].as<String>());
      gCfg.eth.mask.fromString(doc["eth"]["mask"].as<String>());
      gCfg.eth.dns1.fromString(doc["eth"]["dns1"].as<String>());
      gCfg.eth.dns2.fromString(doc["eth"]["dns2"].as<String>());
    }

    if (doc.containsKey("oscListenPort")) {
      gCfg.oscListenPort = doc["oscListenPort"];
    }

    gStore.save(gCfg);
    gWeb.send(200, "text/plain", "OK");
    
    LOG_INFO("WEB", "Network config updated (restart required)");
  });

  // 🔄 API: POST /api/config/network/reload - RECHARGEMENT RÉSEAU SANS REBOOT
  // ✨ Nouvelle fonctionnalité ! Permet de changer l'IP sans redémarrer
  gWeb.on("/api/config/network/reload", HTTP_POST, []() {
    gWeb.send(200, "text/plain", "Reloading network...");
    
    // Donner le temps au client de recevoir la réponse
    delay(500);
    
    // Recharger la config réseau (dans NetworkManager)
    NETMGR.hotReloadNetwork(&gCfg);
    
    LOG_WARN("WEB", "Network hot-reloaded");
  });

  // 📡 API: POST /api/config/ap - Sauvegarder config WiFi AP
  gWeb.on("/api/config/ap", HTTP_POST, []() {
    String body = gWeb.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      gWeb.send(400, "text/plain", "Invalid JSON");
      return;
    }

    gCfg.wifiApAllowed = doc["wifiApAllowed"];
    strlcpy(gCfg.apSsid, doc["apSsid"], sizeof(gCfg.apSsid));
    strlcpy(gCfg.apPass, doc["apPass"], sizeof(gCfg.apPass));
    gCfg.apIp.fromString(doc["apIp"].as<String>());
    gCfg.apMask.fromString(doc["apMask"].as<String>());
    gCfg.apGw.fromString(doc["apGw"].as<String>());
    gCfg.apTimeoutMin = doc["apTimeoutMin"] | 5;

    gStore.save(gCfg);

    // Répondre AVANT de toucher à l'AP (la connexion TCP doit rester ouverte)
    if (NETMGR.isWiFiAPActive()) {
      gApReloadPending = true;
      gApReloadTime = millis();
      LOG_INFO("WEB", "WiFi AP reload scheduled (SSID=%s)", gCfg.apSsid);
      gWeb.send(200, "text/plain", "OK_RELOADED");
    } else {
      LOG_INFO("WEB", "WiFi AP config saved (AP inactive, takes effect on next start)");
      gWeb.send(200, "text/plain", "OK");
    }
  });

  // � API: GET /api/system/status - État système verbose
  gWeb.on("/api/system/status", HTTP_GET, []() {
    StaticJsonDocument<512> doc;
    unsigned long sec = millis() / 1000;
    char uptime[32];
    snprintf(uptime, sizeof(uptime), "%lud %02lu:%02lu:%02lu", sec/86400, (sec%86400)/3600, (sec%3600)/60, sec%60);
    doc["uptime"] = uptime;
    doc["ethConnected"] = NETMGR.isEthernetConnected();
    doc["ethIp"] = NETMGR.getEthernetIP().toString();
    doc["wifiApActive"] = NETMGR.isWiFiAPActive();
    doc["wifiApIp"] = WiFi.softAPIP().toString();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["minFreeHeap"] = ESP.getMinFreeHeap();
    doc["oscPort"] = gCfg.oscListenPort;
    doc["apSsid"] = gCfg.apSsid;
    doc["apTimeoutMin"] = gCfg.apTimeoutMin;
    doc["cpuTemp"] = temperatureRead();
    JsonArray relays = doc.createNestedArray("relays");
    for (int i = 0; i < 8; i++) relays.add(gRelayLogical[i]);
    doc["apClients"] = WiFi.softAPgetStationNum();
    String response;
    serializeJson(doc, response);
    gWeb.send(200, "application/json", response);
  });

  // 📨 API: GET /api/osc/log - Derniers messages OSC reçus
  gWeb.on("/api/osc/log", HTTP_GET, []() {
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.createNestedArray("messages");
    // Iterate ring buffer from oldest to newest
    int start = (gOscLogCount < OSC_LOG_SIZE) ? 0 : gOscLogHead;
    for (int i = 0; i < gOscLogCount; i++) {
      int idx = (start + i) % OSC_LOG_SIZE;
      JsonObject msg = arr.createNestedObject();
      msg["ts"] = gOscLog[idx].ts;
      msg["addr"] = gOscLog[idx].address;
      msg["type"] = gOscLog[idx].typeTag;
      msg["val"] = gOscLog[idx].value;
    }
    String response;
    serializeJson(doc, response);
    gWeb.send(200, "application/json", response);
  });

  // 🔄 API: POST /api/system/reboot - Redémarrer l'ESP32
  gWeb.on("/api/system/reboot", HTTP_POST, []() {
    gWeb.send(200, "text/plain", "Rebooting...");
    gRebootPending = true;
    gPendingActionTime = millis();
    LOG_WARN("SYS", "User initiated reboot");
  });

  // 🗑️ API: POST /api/system/factoryreset - Réinitialisation usine
  gWeb.on("/api/system/factoryreset", HTTP_POST, []() {
    gWeb.send(200, "text/plain", "Factory reset...");
    gFactoryResetPending = true;
    gPendingActionTime = millis();
    LOG_ERROR("SYS", "Factory reset initiated - clearing all config");
  });

  // ⚡ API: GET /api/live - Endpoint combiné haute fréquence (snprintf, pas ArduinoJson)
  // Fusionne relay status + system status + OSC log en une seule requête
  // Polling à 100ms côté client : 10 req/sec au lieu de 5.5 avec 3 endpoints séparés
  gWeb.on("/api/live", HTTP_GET, []() {
    char buf[2048];
    int len = 0;

    // Relays
    len += snprintf(buf + len, sizeof(buf) - len,
      "{\"r\":[%d,%d,%d,%d,%d,%d,%d,%d],",
      gRelayLogical[0]?1:0, gRelayLogical[1]?1:0,
      gRelayLogical[2]?1:0, gRelayLogical[3]?1:0,
      gRelayLogical[4]?1:0, gRelayLogical[5]?1:0,
      gRelayLogical[6]?1:0, gRelayLogical[7]?1:0);

    // System (compact keys for minimal payload)
    unsigned long sec = millis() / 1000;
    len += snprintf(buf + len, sizeof(buf) - len,
      "\"up\":%lu,\"heap\":%u,\"hmin\":%u,"
      "\"eth\":%s,\"eip\":\"%s\","
      "\"ap\":%s,\"aip\":\"%s\",\"acl\":%d,"
      "\"t\":%.1f,\"oP\":%u,\"aSsid\":\"%s\",\"aTo\":%u,",
      sec, ESP.getFreeHeap(), ESP.getMinFreeHeap(),
      NETMGR.isEthernetConnected() ? "true" : "false",
      NETMGR.getEthernetIP().toString().c_str(),
      NETMGR.isWiFiAPActive() ? "true" : "false",
      WiFi.softAPIP().toString().c_str(),
      WiFi.softAPgetStationNum(),
      temperatureRead(),
      gCfg.oscListenPort,
      gCfg.apSsid,
      gCfg.apTimeoutMin);

    // OSC messages (delta: only new since "since" param)
    String sinceStr = gWeb.arg("since");
    unsigned long since = sinceStr.length() > 0 ? strtoul(sinceStr.c_str(), NULL, 10) : 0;

    len += snprintf(buf + len, sizeof(buf) - len, "\"osc\":[");
    bool first = true;
    int start = (gOscLogCount < OSC_LOG_SIZE) ? 0 : gOscLogHead;
    for (int i = 0; i < gOscLogCount; i++) {
      int idx = (start + i) % OSC_LOG_SIZE;
      if (gOscLog[idx].ts > since) {
        if (!first) { buf[len++] = ','; }
        len += snprintf(buf + len, sizeof(buf) - len,
          "{\"ts\":%lu,\"a\":\"%s\",\"t\":\"%s\",\"v\":\"%s\"}",
          gOscLog[idx].ts,
          gOscLog[idx].address,
          gOscLog[idx].typeTag,
          gOscLog[idx].value);
        first = false;
        if (len > (int)sizeof(buf) - 128) break;
      }
    }
    len += snprintf(buf + len, sizeof(buf) - len, "]}");

    gWeb.send(200, "application/json", buf);
  });

  // 📝 API: GET /api/changelog - Contenu de CHANGELOG.md embarqué à la compilation
  gWeb.on("/api/changelog", HTTP_GET, []() {
    gWeb.send_P(200, "text/plain; charset=utf-8", CHANGELOG_DATA);
  });

  gWeb.begin();
  LOG_INFO("WEB", "Web Server started on port 80");
}

// ========== SETUP ==========
void setup() {
  // 🎯 STEP 0: Clear LED (éviter état précédent)
  LedStatus::begin(PIN_RGB, 32);
  LedStatus::setRGB(0, 0, 0); // Éteindre d'abord
  delay(100);
  
  // 🎯 STEP 1: Initialiser le Logger (PRIORITÉ 1)
  Logger::instance().begin();
  Serial.println("[MAIN] Logger initialized");
  delay(2000);  // 2s delay to capture early boot logs
  Serial.println("[MAIN] Starting after delay...");
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  🚀 RelayOSC ESP32-S3-ETH-8DI-8RO   ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  LOG_INFO("SYS", "Firmware compiled: %s %s", __DATE__, __TIME__);
  LOG_INFO("SYS", "Starting initialization sequence...");

  // 🎯 STEP 2: Initialiser le Watchdog (surveille les crashs)
  WATCHDOG.begin(10000);  // 10 secondes de timeout
  WATCHDOG.feed();
  LOG_INFO("SYS", "Watchdog initialized: 10s timeout");
  Serial.println("[MAIN] Watchdog started (10s)");

  // 🎯 LED: statut au boot (bleu)
  LedStatus::booting();

  // 🎯 STEP 3: Initialiser I2C et PCA9554 (expander de relais)
  LOG_INFO("SYS", "Initializing I2C (SDA=42, SCL=41)...");
  Serial.println("\n========================================");
  Serial.println("  STEP 3: TCA9554 I2C INITIALIZATION");
  Serial.println("========================================");
  Serial.printf("[MAIN] Using addr=0x%02X, SDA=%d, SCL=%d, FREQ=%u Hz\n", 0x20, 42, 41, 100000u);
  
  if (!gRelayExpander.begin(Wire, 42, 41, 100000)) {
    LOG_ERROR("HW", "Failed to initialize PCA9554!");
    Serial.println("\n❌❌❌ FATAL: TCA9554 initialization failed! ❌❌❌\n");
    LedStatus::error();  // 🔴 Rouge = erreur critique
    while (1) {
      delay(1000);
      WATCHDOG.feed();
    }
  }
  LOG_INFO("HW", "PCA9554 relay expander initialized (8 channels)");
   Serial.println("\n✅✅✅ TCA9554 initialized successfully! ✅✅✅\n");
  WATCHDOG.feed();

  // 🎯 STEP 3b: Restaurer l'état des relais depuis NVS
  LOG_INFO("RELAY", "Loading relay states from NVS...");
  gStore.loadRelayStates(gRelayLogical);
  updateAllPhysicalRelays();
  for (int i = 0; i < 8; i++) {
    LOG_INFO("RELAY", "Relay %d restored: %s", i, gRelayLogical[i] ? "ON" : "OFF");
  }
  WATCHDOG.feed();

  // 🎯 STEP 4: Vérifier si le firmware a changé → factory reset automatique
  gStore.begin();
  Serial.println("[MAIN] NVS begin done");
  {
    static const char BUILD_STAMP[] = __DATE__ " " __TIME__;
    if (gStore.checkFirmwareBuild(BUILD_STAMP)) {
      LOG_WARN("CFG", "🆕 New firmware detected (%s) — factory reset applied", BUILD_STAMP);
      Serial.printf("[MAIN] New firmware: factory reset done\n");
    }
  }

  // Charger la configuration depuis NVS (stockage persistent)
  LOG_INFO("CFG", "Loading configuration from NVS...");
  if (!gStore.load(gCfg)) {
    LOG_WARN("CFG", "Loading defaults configuration");
    gCfg = ConfigStore::defaults();
    Serial.println("[MAIN] Config load failed -> defaults");
  }
  LOG_INFO("CFG", "Config loaded: OSC port=%d, Hostname=%s", 
    gCfg.oscListenPort, gCfg.hostname);
  Serial.printf("[MAIN] Config: OSC port=%d, Host=%s\n", gCfg.oscListenPort, gCfg.hostname);
  WATCHDOG.feed();

  // 🎯 STEP 5: Configurer les GPIO
  // RGB pilotée par WS2812 (pas besoin de OUTPUT ici)
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Initialiser tous les relais à OFF
  updateAllPhysicalRelays();
  LedStatus::booting(); // Bleu = démarrage
  LOG_INFO("HW", "GPIO configured: RGB=%d, Buzzer=%d", PIN_RGB, PIN_BUZZER);

  // 🎯 STEP 6: Initialiser le réseau (Ethernet W5500) + démarrer l'AP WiFi pour l'interface web
  LOG_INFO("NET", "Starting network initialization...");
  NETMGR.begin(&gCfg);

  // Démarrer l'AP WiFi pour servir l'interface Web (Ethernet reste dédié à l'OSC)
  NETMGR.startWiFiAP(&gCfg);

  // 🌐 Captive Portal DNS : redirige toutes les requêtes DNS vers l'IP de l'AP
  // Cela déclenche automatiquement le popup captive portal sur iOS/Android/Windows
  gDns.start(53, "*", WiFi.softAPIP());
  LOG_INFO("DNS", "Captive portal DNS started → %s", WiFi.softAPIP().toString().c_str());
  
  // Synchroniser les variables globales avec NetworkManager
  gEthLinked = NETMGR.isEthernetConnected();
  gEthIp = NETMGR.getEthernetIP();
  gWiFiApActive = NETMGR.isWiFiAPActive();
  LOG_INFO("NET", "Network state synced: ETH=%s IP=%s WiFiAP=%s", 
    gEthLinked ? "✓" : "✗", gEthIp.toString().c_str(), gWiFiApActive ? "UP" : "DOWN");
  
  WATCHDOG.feed();

  // 🎯 STEP 7: Initialiser le serveur Web
  setupWebServer();
  WATCHDOG.feed();

  // 🎯 STEP 8: Initialiser le routeur OSC (écoute UDP)
  LOG_INFO("OSC", "Starting OSC router on UDP port %d...", gCfg.oscListenPort);
  gOsc.begin(gCfg.oscListenPort, &gCfg, handleRelayCommand);
  gOsc.setSystemCallback(handleSystemOsc);
  gOsc.setLogCallback(recordOscMessage);
  LOG_INFO("OSC", "OSC router ready to receive messages");
  WATCHDOG.feed();

  LedStatus::ok();  // 🟢 Vert = système prêt
  gApLastClientSeen = millis();  // Démarrer le timer 5min AP
  LOG_INFO("SYS", "📡 AP will auto-shutdown after 5min without clients");
  LOG_INFO("SYS", "✅ System initialization complete!");
  Serial.println("[MAIN] ✅ Waiting for HTTP connections on port 80...\n");
  delay(500);  // Donne le temps à la LED de s'afficher
}

// ========== MAIN LOOP ==========
void loop() {
  unsigned long now = millis();

  // ⚡ OSC MESSAGE HANDLING — PRIORITÉ MAXIMALE — exécuté à chaque itération
  // Pas de throttle : chaque passage dans loop() vérifie les paquets UDP
  gOsc.loop();

  // � AP HOT-RELOAD différé (500ms après envoi HTTP pour laisser la réponse partir)
  if (gApReloadPending && (now - gApReloadTime >= 500)) {
    gApReloadPending = false;
    NETMGR.stopWiFiAP();
    delay(300);
    NETMGR.startWiFiAP(&gCfg);
    gApLastClientSeen = millis();
    LOG_INFO("WEB", "WiFi AP reloaded: SSID=%s", gCfg.apSsid);
  }
  // 🔄 REBOOT / FACTORY RESET différé (500ms après envoi HTTP)
  if ((gRebootPending || gFactoryResetPending) && (now - gPendingActionTime >= 500)) {
    if (gFactoryResetPending) {
      gStore.factoryReset();
    }
    ESP.restart();
  }
  // �💡 LED activity update (retour au vert après flash bleu OSC)
  LedStatus::update();

  // 🔄 WATCHDOG FEED (toutes les 100ms)
  if (now - gLastWatchdogFeed >= 100) {
    WATCHDOG.feed();
    gLastWatchdogFeed = now;
  }

  // 💾 DEFERRED NVS SAVE (500ms après dernière commande relais)
  // Évite les écritures flash bloquantes (~5-20ms) dans le chemin OSC critique
  if (gRelayStatesDirty && (now - gRelayDirtyTime >= NVS_SAVE_DEFER_MS)) {
    gStore.saveRelayStates(gRelayLogical);
    gRelayStatesDirty = false;
  }

  // 🌐 WEB + DNS HANDLING (toutes les 5ms — suffisant pour l'UI)
  if (now - gLastWebLoop >= 5) {
    gWeb.handleClient();
    gDns.processNextRequest();
    gLastWebLoop = now;
  }

  // � AP AUTO-MANAGEMENT (vérifier toutes les 5 secondes)
  if (!gApForcedOff && now - gLastStatusUpdate >= 5000) {
    int apClients = WiFi.softAPgetStationNum();
    if (apClients > 0) {
      // Client connecté → reset du timer
      gApLastClientSeen = now;
      // Rallumer l'AP si elle était éteinte (ne devrait pas arriver si client connecté)
    } else if (gCfg.apTimeoutMin > 0 && NETMGR.isWiFiAPActive() &&
               (now - gApLastClientSeen >= (unsigned long)gCfg.apTimeoutMin * 60UL * 1000UL)) {
      // Pas de client depuis le timeout configuré → éteindre l'AP
      LOG_WARN("AP", "📡 No clients for %u min, shutting down AP", gCfg.apTimeoutMin);
      NETMGR.stopWiFiAP();
      gWiFiApActive = false;
    }
  }

  // �📊 STATUS UPDATE (toutes les 5 secondes)
  // Afficher l'état du système pour diagnostic
  if (now - gLastStatusUpdate >= 5000) {
    // Sync network state from NetworkManager
    gEthLinked = NETMGR.isEthernetConnected();
    gEthIp = NETMGR.getEthernetIP();
    gWiFiApActive = NETMGR.isWiFiAPActive();
    
    LOG_INFO("STATUS",
      "ETH=%s IP=%s WiFiAP=%s RAM_free=%u bytes Relays=%d-%d-%d-%d-%d-%d-%d-%d",
      gEthLinked ? "✓" : "✗",
      gEthIp.toString().c_str(),
      gWiFiApActive ? "UP" : "DOWN",
      ESP.getFreeHeap(),
      gRelayLogical[0], gRelayLogical[1], gRelayLogical[2], gRelayLogical[3],
      gRelayLogical[4], gRelayLogical[5], gRelayLogical[6], gRelayLogical[7]
    );
    gLastStatusUpdate = now;
  }

  // Petite pause pour céder au scheduler FreeRTOS (yield sans bloquer)
  yield();
}
