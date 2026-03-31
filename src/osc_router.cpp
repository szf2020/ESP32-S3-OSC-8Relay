#include "osc_router.h"
#include "logger.h"

bool OscRelayRouter::begin(uint16_t listenPort, const AppCfg* cfg, RelayCallback cb) {
  _port = listenPort;
  _cfg = cfg;
  _cb = cb;

  if (!_udp.begin(listenPort)) {
    LOG_OSC("OSC", "Failed to bind UDP port %d", listenPort);
    return false;
  }

  _running = true;
  LOG_OSC("OSC", "Router started on UDP port %d", listenPort);
  return true;
}

void OscRelayRouter::setConfig(const AppCfg* cfg) {
  _cfg = cfg;
  LOG_OSC("OSC", "Configuration updated");
}

int OscRelayRouter::findAddressEnd(const uint8_t* data, size_t len) {
  // Find null terminator for OSC address string
  for (size_t i = 0; i < len; i++) {
    if (data[i] == 0) {
      return i;
    }
  }
  return -1;
}

bool OscRelayRouter::extractBoolValue(const uint8_t* data, size_t len, int tagsStart, bool& outValue) {
  if (tagsStart < 0 || tagsStart >= (int)len) return false;

  // Skip past the tags string (starts with ',' and ends with null terminator)
  int dataPos = tagsStart;
  
  // Find the type tag character after ','
  if (dataPos < (int)len && data[dataPos] == ',') {
    dataPos++;
    if (dataPos < (int)len) {
      char typeChar = data[dataPos];
      dataPos++;
      
      // Align to 4-byte boundary
      while (dataPos % 4 != 0 && dataPos < (int)len) dataPos++;

      // Parse based on type
      if (typeChar == 'i' || typeChar == 'f') {
        // Integer or float
        if (dataPos + 4 <= (int)len) {
          uint32_t val = (data[dataPos] << 24) | (data[dataPos+1] << 16) | 
                        (data[dataPos+2] << 8) | data[dataPos+3];
          outValue = (val != 0);
          return true;
        }
      } else if (typeChar == 'T' || typeChar == 'F') {
        // True or False tag (no data)
        outValue = (typeChar == 'T');
        return true;
      }
    }
  }
  
  return false;
}

bool OscRelayRouter::parseOscMessage(const uint8_t* data, size_t len) {
  if (len < 8) return false;  // Minimum OSC message
  
  // Find address string end
  int addrEnd = findAddressEnd(data, len);
  if (addrEnd < 0 || addrEnd >= (int)len) return false;

  // Extract address string
  char address[64];
  int addrLen = addrEnd;
  if (addrLen >= (int)sizeof(address)) addrLen = sizeof(address) - 1;
  memcpy(address, data, addrLen);
  address[addrLen] = 0;

  // Find tags start (should be next 4-byte aligned position)
  int tagsStart = addrEnd + 1;
  while (tagsStart < (int)len && tagsStart % 4 != 0) tagsStart++;

  if (tagsStart >= (int)len || data[tagsStart] != ',') return false;

  // Extract type tag and value for logging
  char typeTag[2] = {0, 0};
  char valueStr[32] = "?";
  if (tagsStart + 1 < (int)len) {
    typeTag[0] = data[tagsStart + 1];
    int dataPos = tagsStart + 1;
    dataPos++;
    while (dataPos % 4 != 0 && dataPos < (int)len) dataPos++;
    if (typeTag[0] == 'i' && dataPos + 4 <= (int)len) {
      int32_t v = (int32_t)((data[dataPos]<<24)|(data[dataPos+1]<<16)|(data[dataPos+2]<<8)|data[dataPos+3]);
      snprintf(valueStr, sizeof(valueStr), "%d", v);
    } else if (typeTag[0] == 'f' && dataPos + 4 <= (int)len) {
      uint32_t raw = (data[dataPos]<<24)|(data[dataPos+1]<<16)|(data[dataPos+2]<<8)|data[dataPos+3];
      float fv; memcpy(&fv, &raw, 4);
      snprintf(valueStr, sizeof(valueStr), "%.2f", fv);
    } else if (typeTag[0] == 'T') {
      snprintf(valueStr, sizeof(valueStr), "True");
    } else if (typeTag[0] == 'F') {
      snprintf(valueStr, sizeof(valueStr), "False");
    }
  }

  // Fire log callback for all parsed messages (AFTER routing for lower latency)
  // Moved below relay dispatch to not delay relay actuation

  // Check system commands (/ap, /reboot)
  if (strncmp(address, "/ap", 3) == 0 || strcmp(address, "/reboot") == 0) {
    bool val = false;
    if (extractBoolValue(data, len, tagsStart, val)) {
      LOG_OSC("OSC", "System command: %s -> %d", address, val);
      if (_sysCb) _sysCb(address, val);
      if (_logCb) _logCb(address, typeTag, valueStr);
      return true;
    }
  }

  // Check ALL ON/OFF address
  if (strcmp(address, "/relay/all") == 0) {
    bool newState = false;
    if (extractBoolValue(data, len, tagsStart, newState)) {
      LOG_OSC("OSC", "Matched /relay/all -> state=%d (ALL relays)", newState);
      if (_cb) {
        for (int i = 0; i < 8; i++) {
          _cb(i, newState);
        }
      }
      if (_logCb) _logCb(address, typeTag, valueStr);
      return true;
    }
  }

  // Check each relay's address
  for (int i = 0; i < 8; i++) {
    if (strcmp(address, _cfg->relays[i].oscAddress) == 0) {
      // Address matches - extract bool value
      bool newState = false;
      if (extractBoolValue(data, len, tagsStart, newState)) {
        LOG_OSC("OSC", "Matched %s (relay %d) -> state=%d", address, i, newState);
        if (_cb) {
          _cb(i, newState);
        }
        // Log callback after relay actuation
        if (_logCb) _logCb(address, typeTag, valueStr);
        return true;
      }
    }
  }

  // No relay matched — still log unmatched messages
  if (_logCb) _logCb(address, typeTag, valueStr);
  return false;
}

void OscRelayRouter::loop() {
  if (!_running || !_cfg || !_cb) return;

  // Drain all pending packets in one pass for minimum latency
  for (;;) {
    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) break;

    if (packetSize > (int)sizeof(_rxBuffer)) {
      packetSize = sizeof(_rxBuffer);
    }

    int bytesRead = _udp.read(_rxBuffer, packetSize);
    if (bytesRead <= 0) continue;

    // Parse OSC message
    parseOscMessage(_rxBuffer, bytesRead);
  }
}

void OscRelayRouter::stop() {
  if (_running) {
    _udp.stop();
    _running = false;
    LOG_OSC("OSC", "Router stopped");
  }
}
