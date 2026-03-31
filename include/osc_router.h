#pragma once
#include <Arduino.h>
#include <Ethernet_Generic.hpp>  // header-only declarations only; implementation pulled in once via network_mgr.cpp
#include <functional>
#include "config.h"

// Simple OSC Message Parser
// Format OSC: | null-terminated address string | null padding | type tags | data |

class OscRelayRouter {
public:
  using RelayCallback = std::function<void(uint8_t relayIndex, bool newLogicalState)>;
  using SystemCallback = std::function<void(const char* address, bool value)>;
  using LogCallback = std::function<void(const char* address, const char* typeTag, const char* valueStr)>;

  bool begin(uint16_t listenPort, const AppCfg* cfg, RelayCallback cb);
  void setSystemCallback(SystemCallback cb) { _sysCb = cb; }
  void setLogCallback(LogCallback cb) { _logCb = cb; }
  void setConfig(const AppCfg* cfg);
  void loop();
  void stop();

private:
  static constexpr size_t RX_BUF = 2048;

  EthernetUDP _udp;
  const AppCfg* _cfg = nullptr;
  RelayCallback _cb;
  SystemCallback _sysCb;
  LogCallback _logCb;
  uint16_t _port = 0;
  bool _running = false;
  uint8_t _rxBuffer[RX_BUF];

  // OSC parsing helpers
  bool parseOscMessage(const uint8_t* data, size_t len);
  int findAddressEnd(const uint8_t* data, size_t len);
  bool extractBoolValue(const uint8_t* data, size_t len, int tagsStart, bool& outValue);
};
