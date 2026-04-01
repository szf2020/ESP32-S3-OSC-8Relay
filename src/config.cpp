#include "config.h"
#include <Preferences.h>

static Preferences prefs;

static void ipToBytes(const IPAddress& ip, uint8_t out[4]) {
  out[0] = ip[0]; out[1] = ip[1]; out[2] = ip[2]; out[3] = ip[3];
}

static IPAddress bytesToIp(const uint8_t in[4]) {
  return IPAddress(in[0], in[1], in[2], in[3]);
}

AppCfg ConfigStore::defaults() {
  AppCfg c{};

  c.oscListenPort = 8000;
  strlcpy(c.hostname, "esp32-relay-osc", sizeof(c.hostname));

  // Factory defaults: static IP 192.168.0.1/24 (no DHCP)
  c.eth.dhcp = false;
  c.eth.ip   = IPAddress(192,168,0,1);
  c.eth.gw   = IPAddress(192,168,0,254);
  c.eth.mask = IPAddress(255,255,255,0);
  c.eth.dns1 = IPAddress(1,1,1,1);
  c.eth.dns2 = IPAddress(8,8,8,8);

  c.wifiApAllowed = true;
  strlcpy(c.apSsid, "ESP32-S3-OSC-8RELAY", sizeof(c.apSsid));
  strlcpy(c.apPass, "S3Relay!", sizeof(c.apPass));
  c.apIp   = IPAddress(192,168,4,1);
  c.apGw   = IPAddress(192,168,4,1);
  c.apMask = IPAddress(255,255,255,0);
  c.apTimeoutMin = 5;              // 5 minutes par défaut, 0 = infini

  for (int i=0;i<8;i++) {
    snprintf(c.relays[i].oscAddress, sizeof(c.relays[i].oscAddress), "/relay/%d", i+1);
    c.relays[i].invert = false;
    c.relays[i].mode   = RelayMode::Latch;
  }
  return c;
}

bool ConfigStore::begin() {
  if (_begun) return true;
  _begun = prefs.begin(NAMESPACE, false);
  return _begun;
}

bool ConfigStore::load(AppCfg& out) {
  if (!begin()) return false;

  if (!prefs.isKey("v")) {
    out = defaults();
    return true;
  }

  uint32_t v = prefs.getUInt("v", 1);
  (void)v;

  AppCfg c = defaults();

  c.oscListenPort = prefs.getUShort("oscPort", c.oscListenPort);
  prefs.getString("host", c.hostname, sizeof(c.hostname));

  c.eth.dhcp = prefs.getBool("ethDhcp", c.eth.dhcp);

  uint8_t b[4];
  if (prefs.getBytes("ethIp", b, 4) > 0) c.eth.ip = bytesToIp(b);
  if (prefs.getBytes("ethGw", b, 4) > 0) c.eth.gw = bytesToIp(b);
  if (prefs.getBytes("ethMs", b, 4) > 0) c.eth.mask = bytesToIp(b);
  if (prefs.getBytes("ethD1", b, 4) > 0) c.eth.dns1 = bytesToIp(b);
  if (prefs.getBytes("ethD2", b, 4) > 0) c.eth.dns2 = bytesToIp(b);

  c.wifiApAllowed = prefs.getBool("apAllow", c.wifiApAllowed);
  prefs.getString("apSsid", c.apSsid, sizeof(c.apSsid));
  prefs.getString("apPass", c.apPass, sizeof(c.apPass));

  // If previous defaults are stored, upgrade to new SSID/pass automatically
  if (strlen(c.apSsid) == 0 || strcmp(c.apSsid, "RelayOSC") == 0) {
    strlcpy(c.apSsid, "ESP32-S3-OSC-8RELAY", sizeof(c.apSsid));
  }
  if (strcmp(c.apPass, "relayosc123") == 0) {
    strlcpy(c.apPass, "S3Relay!", sizeof(c.apPass));
  }
  if (prefs.getBytes("apIp", b, 4) > 0) c.apIp = bytesToIp(b);
  if (prefs.getBytes("apGw", b, 4) > 0) c.apGw = bytesToIp(b);
  if (prefs.getBytes("apMs", b, 4) > 0) c.apMask = bytesToIp(b);
  c.apTimeoutMin = prefs.getUShort("apTout", c.apTimeoutMin);

  for (int i=0;i<8;i++) {
    char key[16];

    snprintf(key, sizeof(key), "r%daddr", i);
    prefs.getString(key, c.relays[i].oscAddress, sizeof(c.relays[i].oscAddress));

    snprintf(key, sizeof(key), "r%dinv", i);
    c.relays[i].invert = prefs.getBool(key, c.relays[i].invert);

    snprintf(key, sizeof(key), "r%dmode", i);
    c.relays[i].mode = (RelayMode)prefs.getUChar(key, (uint8_t)c.relays[i].mode);
  }

  out = c;
  return true;
}

bool ConfigStore::save(const AppCfg& in) {
  if (!begin()) return false;

  prefs.putUInt("v", 1);

  prefs.putUShort("oscPort", in.oscListenPort);
  prefs.putString("host", in.hostname);

  prefs.putBool("ethDhcp", in.eth.dhcp);

  uint8_t b[4];
  ipToBytes(in.eth.ip, b);   prefs.putBytes("ethIp", b, 4);
  ipToBytes(in.eth.gw, b);   prefs.putBytes("ethGw", b, 4);
  ipToBytes(in.eth.mask, b); prefs.putBytes("ethMs", b, 4);
  ipToBytes(in.eth.dns1, b); prefs.putBytes("ethD1", b, 4);
  ipToBytes(in.eth.dns2, b); prefs.putBytes("ethD2", b, 4);

  prefs.putBool("apAllow", in.wifiApAllowed);
  prefs.putString("apSsid", in.apSsid);
  prefs.putString("apPass", in.apPass);
  ipToBytes(in.apIp, b);     prefs.putBytes("apIp", b, 4);
  ipToBytes(in.apGw, b);     prefs.putBytes("apGw", b, 4);
  ipToBytes(in.apMask, b);   prefs.putBytes("apMs", b, 4);
  prefs.putUShort("apTout", in.apTimeoutMin);

  for (int i=0;i<8;i++) {
    char key[16];

    snprintf(key, sizeof(key), "r%daddr", i);
    prefs.putString(key, in.relays[i].oscAddress);

    snprintf(key, sizeof(key), "r%dinv", i);
    prefs.putBool(key, in.relays[i].invert);

    snprintf(key, sizeof(key), "r%dmode", i);
    prefs.putUChar(key, (uint8_t)in.relays[i].mode);
  }

  return true;
}

void ConfigStore::factoryReset() {
  if (!begin()) return;
  prefs.clear();
}

bool ConfigStore::loadRelayStates(bool out[8]) {
  if (!begin()) return false;
  
  for (int i = 0; i < 8; i++) {
    char key[16];
    snprintf(key, sizeof(key), "relay%d_state", i);
    out[i] = prefs.getBool(key, false);
  }
  return true;
}

bool ConfigStore::saveRelayStates(const bool in[8]) {
  if (!begin()) return false;
  
  for (int i = 0; i < 8; i++) {
    char key[16];
    snprintf(key, sizeof(key), "relay%d_state", i);
    prefs.putBool(key, in[i]);
  }
  return true;
}
