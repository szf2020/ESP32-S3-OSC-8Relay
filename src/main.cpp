#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "config.h"
#include "pca9554.h"
#include "osc_router.h"
#include "web_ui.h"
#include "logger.h"
#include "watchdog.h"
#include "mutex.h"
#include "network_mgr.h"
#include "led_status.h"

// ========== HARDWARE PINOUT ==========
// Ethernet (W5500 SPI): SCK=15, MISO=14, MOSI=13, CS=16, INT=12, RST=-1
// I2C (PCA9554 + RTC): SDA=42, SCL=41
// DI8 = GPIO11 (active when LOW = pulled to ground by optocoupler)
// RGB LED = GPIO38, Buzzer = GPIO46

static constexpr int PIN_DI8_WIFI = 11;
static constexpr int PIN_RGB = 38;
static constexpr int PIN_BUZZER = 46;

// ========== GLOBALS ==========
static ConfigStore gStore;
static AppCfg gCfg;
static WebServer gWeb(80);
static PCA9554 gRelayExpander(0x20);
static OscRelayRouter gOsc;

// 🔒 MUTEX PROTECTION : Protège l'accès concurrent aux relais
// Raison : Le thread Web ET le thread OSC accèdent à gRelayLogical simultanément
//          Sans mutex = race condition = corruption de données !
static Mutex gRelayMutex;

static bool gRelayLogical[8] = {false};
static bool gEthLinked = false;
static IPAddress gEthIp(0, 0, 0, 0);
static bool gWiFiApActive = false;
static unsigned long gLastOscLoop = 0;
static unsigned long gLastStatusUpdate = 0;
static unsigned long gLastWatchdogFeed = 0;

// ========== FORWARD DECLARATIONS ==========
void setupWebServer();
void handleRelayCommand(uint8_t relayIdx, bool newState);
void updatePhysicalRelay(uint8_t idx);
void updateAllPhysicalRelays();
void setupRGB(uint8_t r, uint8_t g, uint8_t b);

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

  // � Sauvegarder l'état en NVS pour persistance
  gStore.saveRelayStates(gRelayLogical);

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

  // Log 404
  gWeb.onNotFound([]() {
    LOG_WARN("WEB", "🔴 404 Not Found: %s from %s", 
      gWeb.uri().c_str(), gWeb.client().remoteIP().toString().c_str());
    gWeb.send(404, "text/plain", "Not Found");
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
      deserializeJson(doc, body);
      
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
    deserializeJson(doc, body);

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
    deserializeJson(doc, body);

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
    deserializeJson(doc, body);

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
    deserializeJson(doc, body);

    gCfg.wifiApAllowed = doc["wifiApAllowed"];
    strlcpy(gCfg.apSsid, doc["apSsid"], sizeof(gCfg.apSsid));
    strlcpy(gCfg.apPass, doc["apPass"], sizeof(gCfg.apPass));
    gCfg.apIp.fromString(doc["apIp"].as<String>());
    gCfg.apMask.fromString(doc["apMask"].as<String>());
    gCfg.apGw.fromString(doc["apGw"].as<String>());

    gStore.save(gCfg);
    gWeb.send(200, "text/plain", "OK");
    
    LOG_INFO("WEB", "WiFi AP config updated");
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
    JsonArray relays = doc.createNestedArray("relays");
    for (int i = 0; i < 8; i++) relays.add(gRelayLogical[i]);
    doc["apClients"] = WiFi.softAPgetStationNum();
    String response;
    serializeJson(doc, response);
    gWeb.send(200, "application/json", response);
  });

  // �🔄 API: POST /api/system/reboot - Redémarrer l'ESP32
  gWeb.on("/api/system/reboot", HTTP_POST, []() {
    gWeb.send(200, "text/plain", "Rebooting...");
    delay(1000);
    LOG_WARN("SYS", "User initiated reboot");
    ESP.restart();
  });

  // 🗑️ API: POST /api/system/factoryreset - Réinitialisation usine
  gWeb.on("/api/system/factoryreset", HTTP_POST, []() {
    gWeb.send(200, "text/plain", "Factory reset...");
    
    LOG_ERROR("SYS", "Factory reset initiated - clearing all config");
    gStore.factoryReset();
    
    delay(1000);
    ESP.restart();
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

  // 🎯 STEP 4: Charger la configuration depuis NVS (stockage persistent)
  LOG_INFO("CFG", "Loading configuration from NVS...");
  gStore.begin();
  Serial.println("[MAIN] NVS begin done");
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
  pinMode(PIN_DI8_WIFI, INPUT_PULLUP);
  // RGB pilotée par WS2812 (pas besoin de OUTPUT ici)
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Initialiser tous les relais à OFF
  updateAllPhysicalRelays();
  LedStatus::booting(); // � Bleu = démarrage
  LOG_INFO("HW", "GPIO configured: DI8=%d, RGB=%d, Buzzer=%d", 
    PIN_DI8_WIFI, PIN_RGB, PIN_BUZZER);

  // 🎯 STEP 6: Initialiser le réseau (Ethernet W5500) + démarrer l'AP WiFi pour l'interface web
  LOG_INFO("NET", "Starting network initialization...");
  NETMGR.begin(&gCfg);

  // Démarrer l'AP WiFi pour servir l'interface Web (Ethernet reste dédié à l'OSC)
  NETMGR.startWiFiAP(&gCfg);
  
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
  LOG_INFO("OSC", "OSC router ready to receive messages");
  WATCHDOG.feed();

  LedStatus::ok();  // 🟢 Vert = système prêt
  LOG_INFO("SYS", "✅ System initialization complete!");
  Serial.println("[MAIN] ✅ Waiting for HTTP connections on port 80...\n");
  delay(500);  // Donne le temps à la LED de s'afficher
}

// ========== MAIN LOOP ==========
void loop() {
  unsigned long now = millis();

  // 🔄 WATCHDOG FEED (toutes les 100ms)
  // Alimente le chien de garde pour éviter un redémarrage
  if (now - gLastWatchdogFeed >= 100) {
    WATCHDOG.feed();
    gLastWatchdogFeed = now;
  }

  // 📡 OSC MESSAGE HANDLING (toutes les 10ms)
  // Écoute les messages OSC entrants et déclenche les relais
  if (now - gLastOscLoop >= 10) {
    try {
      gOsc.loop();
    } catch (...) {
      LOG_ERROR("OSC", "❌ Exception in OSC loop!");
      LedStatus::error();
    }
    gLastOscLoop = now;
  }

  // 🌐 WEB SERVER HANDLING (serveur synchrone - nécessite handleClient())
  gWeb.handleClient();

  // 📊 STATUS UPDATE (toutes les 5 secondes)
  // Afficher l'état du système pour diagnostic
  if (now - gLastStatusUpdate >= 5000) {
    try {
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
    } catch (...) {
      LOG_ERROR("STATUS", "❌ Exception in STATUS update!");
      LedStatus::error();
    }
  }

  // Petite pause pour éviter un loop trop agressif
  delay(1);
}
