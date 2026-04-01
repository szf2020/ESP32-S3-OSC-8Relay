#include "network_mgr.h"
#include "logger.h"
#include <WiFi.h>
#include <SPI.h>
#include <Ethernet_Generic.h>

static NetworkManager* gNetMgrPtr = nullptr;

void NetworkManager::onEthEvent(arduino_event_t* event) {
  if (!gNetMgrPtr) return;
  
  switch (event->event_id) {
    case ARDUINO_EVENT_ETH_START:
      LOG_NET("ETH", "Starting...");
      break;
    
    case ARDUINO_EVENT_ETH_CONNECTED:
      LOG_NET("ETH", "Connected to link");
      break;
    
    case ARDUINO_EVENT_ETH_GOT_IP:
      gNetMgrPtr->_ethIp = IPAddress(event->event_info.got_ip.ip_info.ip.addr);
      gNetMgrPtr->_ethConnected = true;
      LOG_NET("ETH", "Got IP: %s", gNetMgrPtr->_ethIp.toString().c_str());
      break;
    
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      LOG_NET("ETH", "Disconnected");
      gNetMgrPtr->_ethConnected = false;
      gNetMgrPtr->_ethIp = IPAddress(0, 0, 0, 0);
      break;
    
    default:
      break;
  }
}

void NetworkManager::begin(const AppCfg* cfg) {
  gNetMgrPtr = this;
  startEthernet(cfg);
}

void NetworkManager::startEthernet(const AppCfg* cfg) {
  LOG_INFO("NET", "Initializing Ethernet (W5500 native)...");
  
  // IMPORTANT: Enable WiFi stack (STA mode) for lwIP TCP/IP support
  // WebServer requires lwIP which is only available with WiFi subsystem active
  // WiFi radio stays OFF (no WiFi.begin() call), but TCP/IP stack is initialized
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  delay(500);  // Allow lwIP to initialize
  
  LOG_INFO("NET", "WiFi stack enabled (radio idle, lwIP active)");
  
  // Configure W5500 SPI pins
  // ESP32-S3-POE-ETH-8DI-8RO pinout: CS=16, SCK=15, MISO=14, MOSI=13
  #define ETH_SPI_HOST  SPI3_HOST
  #define PIN_ETH_MISO  14
  #define PIN_ETH_MOSI  13
  #define PIN_ETH_SCK   15
  #define PIN_ETH_CS    16
  
  LOG_INFO("NET", "W5500 pins: CS=%d, SCK=%d, MISO=%d, MOSI=%d", PIN_ETH_CS, PIN_ETH_SCK, PIN_ETH_MISO, PIN_ETH_MOSI);
  
  // Initialize SPI bus
  SPI.begin(PIN_ETH_SCK, PIN_ETH_MISO, PIN_ETH_MOSI, PIN_ETH_CS);
  
  // Set Ethernet CS pin
  Ethernet.init(PIN_ETH_CS);
  
  // Configure static MAC address
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, cfg->eth.ip[3] };  // Last byte = last IP octet
  
  // Start Ethernet with static IP configuration
  if (cfg->eth.dhcp) {
    LOG_INFO("NET", "Starting W5500 with DHCP (timeout 6s)...");
    if (Ethernet.begin(mac, 6000, 2000) == 0) {
      // Échec DHCP → fallback APIPA 169.254.mac[4].mac[5]
      uint8_t a = mac[4];  // 0xFE = 254
      uint8_t b = mac[5];  // dernier octet de l'IP statique configurée
      if (b == 0)   b = 1;    // éviter .0
      if (b == 255) b = 254;  // éviter .255
      IPAddress apipaIp(169, 254, a, b);
      IPAddress apipaGw(0, 0, 0, 0);
      IPAddress apipaMask(255, 255, 0, 0);
      IPAddress apipaDns(0, 0, 0, 0);
      LOG_WARN("NET", "⚠️ DHCP failed — APIPA fallback: %s/16", apipaIp.toString().c_str());
      Ethernet.begin(mac, apipaIp, apipaDns, apipaGw, apipaMask);
      _ethIp = apipaIp;
    } else {
      _ethIp = Ethernet.localIP();
    }
  } else {
    LOG_INFO("NET", "Starting W5500 with static IP: %s", cfg->eth.ip.toString().c_str());
    Ethernet.begin(mac, cfg->eth.ip, cfg->eth.dns1, cfg->eth.gw, cfg->eth.mask);
    _ethIp = cfg->eth.ip;
  }
  
  // Verify connection
  delay(500);  // Allow W5500 to initialize
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    LOG_ERROR("NET", "❌ W5500 hardware not found!");
    _ethConnected = false;
    return;
  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    LOG_WARN("NET", "⚠️ W5500 no link (cable unplugged?)");
  }
  
  _ethConnected = true;
  _ethMask = Ethernet.subnetMask();
  _ethGw   = Ethernet.gatewayIP();
  _ethDns1 = Ethernet.dnsServerIP();
  _ethDns2 = cfg->eth.dhcp ? IPAddress(0,0,0,0) : cfg->eth.dns2;
  LOG_INFO("NET", "✅ W5500 Ethernet initialized");
  LOG_INFO("NET", "   MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  LOG_INFO("NET", "   IP: %s", _ethIp.toString().c_str());
  LOG_INFO("NET", "   Mask: %s", _ethMask.toString().c_str());
  LOG_INFO("NET", "   Gateway: %s", _ethGw.toString().c_str());
  LOG_INFO("NET", "   DNS: %s", _ethDns1.toString().c_str());
}

void NetworkManager::startWiFiAP(const AppCfg* cfg) {
  if (!cfg->wifiApAllowed) {
    LOG_NET("WiFi", "AP not allowed by config");
    return;
  }
  
  LOG_INFO("WiFi", "Starting WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(cfg->apIp, cfg->apGw, cfg->apMask);  // config IP AVANT softAP
  WiFi.softAP(cfg->apSsid, cfg->apPass);
  
  _wifiApIp = WiFi.softAPIP();
  _wifiApActive = true;
  
  LOG_INFO("WiFi", "AP Started: SSID=%s, IP=%s", cfg->apSsid, _wifiApIp.toString().c_str());
}

void NetworkManager::stopWiFiAP() {
  if (!_wifiApActive) return;
  LOG_INFO("WiFi", "Stopping WiFi AP...");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);   // reset complet du driver pour forcer reload du SSID
  delay(100);
  WiFi.mode(WIFI_STA);
  _wifiApActive = false;
  LOG_INFO("WiFi", "AP stopped");
}

void NetworkManager::applyNetworkConfig(const AppCfg* cfg) {
  LOG_INFO("NET", "Applying network configuration");
  
  if (!cfg->eth.dhcp) {
    WiFi.config(cfg->eth.ip, cfg->eth.gw, cfg->eth.mask, cfg->eth.dns1, cfg->eth.dns2);
    LOG_INFO("NET", "New IP config: %s", cfg->eth.ip.toString().c_str());
  } else {
    LOG_INFO("NET", "Switching to DHCP");
  }
  
  WiFi.setHostname(cfg->hostname);
}

void NetworkManager::hotReloadNetwork(const AppCfg* cfg) {
  LOG_WARN("NET", "Hot-reloading network configuration");
  LOG_WARN("NET", "Note: Full hot-reload requires system reboot for Ethernet");
  
  // Apply new IP config without full restart
  if (!cfg->eth.dhcp) {
    WiFi.config(cfg->eth.ip, cfg->eth.gw, cfg->eth.mask, cfg->eth.dns1, cfg->eth.dns2);
    LOG_INFO("NET", "IP config updated: %s", cfg->eth.ip.toString().c_str());
  }
  
  WiFi.setHostname(cfg->hostname);
  LOG_INFO("NET", "Network config applied (reboot recommended)");
}

void NetworkManager::stop() {
  LOG_INFO("NET", "Stopping network");
  WiFi.mode(WIFI_OFF);
  _ethConnected = false;
  _wifiApActive = false;
}
