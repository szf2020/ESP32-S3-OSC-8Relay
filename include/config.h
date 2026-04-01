#pragma once
#include <Arduino.h>
#include <IPAddress.h>

enum class RelayMode : uint8_t {
  Latch = 0,   // value-driven (0/1)
  Toggle = 1   // rising edge / "true" toggles
};

struct RelayCfg {
  char oscAddress[48];   // e.g. "/relay/1"
  bool invert;           // invert logical->physical
  RelayMode mode;        // latch / toggle
};

struct NetCfg {
  bool dhcp;
  IPAddress ip;
  IPAddress gw;
  IPAddress mask;
  IPAddress dns1;
  IPAddress dns2;
};

struct AppCfg {
  uint16_t oscListenPort;      // UDP port for incoming OSC
  char hostname[32];

  // Ethernet config (W5500)
  NetCfg eth;

  // WiFi AP settings
  bool wifiApAllowed;          // "master enable" from UI
  char apSsid[32];
  char apPass[64];
  IPAddress apIp;
  IPAddress apGw;
  IPAddress apMask;
  uint16_t apTimeoutMin;           // auto-shutdown AP sans client (minutes, 0 = infini)

  RelayCfg relays[8];
};

class ConfigStore {
public:
  static constexpr const char* NAMESPACE = "relayosc";
  static AppCfg defaults();

  bool begin();
  bool load(AppCfg& out);
  bool save(const AppCfg& in);
  void factoryReset();
  // Retourne true si un factory reset a été effectué (nouveau firmware détecté)
  bool checkFirmwareBuild(const char* buildStamp);
  
  // Relay state persistence
  bool loadRelayStates(bool out[8]);
  bool saveRelayStates(const bool in[8]);

private:
  bool _begun = false;
};
