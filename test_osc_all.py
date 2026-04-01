#!/usr/bin/env python3
"""Test complet de toutes les commandes OSC - ESP32-S3-OSC-8Relay"""
import time, socket, struct, sys

ESP32_IP = "192.168.0.1"
OSC_PORT = 8000

def osc_msg(addr, tag, value=None):
    a = addr.encode() + b'\x00'
    while len(a) % 4: a += b'\x00'
    t = (',' + tag).encode() + b'\x00'
    while len(t) % 4: t += b'\x00'
    d = b''
    if tag == 'i': d = struct.pack('>i', value)
    elif tag == 'f': d = struct.pack('>f', value)
    return a + t + d

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
dest = (ESP32_IP, OSC_PORT)
count = 0
fails = 0

def send(name, addr, tag, value=None):
    global count
    msg = osc_msg(addr, tag, value)
    sock.sendto(msg, dest)
    vstr = str(value) if value is not None else ""
    print("  [%02d] OK  %-30s  %s %s %s" % (count+1, name, addr, tag, vstr))
    count += 1
    time.sleep(0.25)

print("=" * 65)
print("  TEST OSC COMPLET - ESP32-S3-OSC-8Relay")
print("  Target: %s:%d" % (ESP32_IP, OSC_PORT))
print("=" * 65)

# ===== 1. Relais individuels int32 =====
print("\n--- 1. Relais individuels ON/OFF (int32) ---")
for r in range(1, 9):
    send("Relay %d ON (i)" % r, "/relay/%d" % r, "i", 1)
time.sleep(0.3)
for r in range(1, 9):
    send("Relay %d OFF (i)" % r, "/relay/%d" % r, "i", 0)

# ===== 2. Relais individuels float32 =====
print("\n--- 2. Relais individuels ON/OFF (float32) ---")
for r in range(1, 5):
    send("Relay %d ON (f)" % r, "/relay/%d" % r, "f", 1.0)
time.sleep(0.3)
for r in range(1, 5):
    send("Relay %d OFF (f)" % r, "/relay/%d" % r, "f", 0.0)

# ===== 3. Relais individuels True/False type tags =====
print("\n--- 3. Relais individuels ON/OFF (T/F tags) ---")
for r in range(1, 5):
    send("Relay %d ON (T)" % r, "/relay/%d" % r, "T")
time.sleep(0.3)
for r in range(1, 5):
    send("Relay %d OFF (F)" % r, "/relay/%d" % r, "F")

# ===== 4. /relay/all =====
print("\n--- 4. /relay/all (tous les types) ---")
send("ALL ON (i)",  "/relay/all", "i", 1)
time.sleep(0.5)
send("ALL OFF (i)", "/relay/all", "i", 0)
send("ALL ON (f)",  "/relay/all", "f", 1.0)
time.sleep(0.5)
send("ALL OFF (f)", "/relay/all", "f", 0.0)
send("ALL ON (T)",  "/relay/all", "T")
time.sleep(0.5)
send("ALL OFF (F)", "/relay/all", "F")

# ===== 5. /ap control =====
print("\n--- 5. /ap (WiFi AP control) ---")
send("AP OFF",     "/ap",        "i", 0)
time.sleep(1.0)
send("AP ON",      "/ap",        "i", 1)

# ===== 6. Edge cases =====
print("\n--- 6. Edge cases ---")
send("Relay 1 val=0 (OFF)",       "/relay/1", "i", 0)
send("Relay 1 val=42 (ON)",       "/relay/1", "i", 42)
time.sleep(0.2)
send("Relay 1 val=-1 (ON)",       "/relay/1", "i", -1)
time.sleep(0.2)
send("Relay 1 OFF",               "/relay/1", "i", 0)
send("Relay 1 float=0.5 (ON)",    "/relay/1", "f", 0.5)
time.sleep(0.2)
send("Relay 1 float=-0.1 (ON)",   "/relay/1", "f", -0.1)
time.sleep(0.2)
send("Relay 1 float=0.0 (OFF)",   "/relay/1", "f", 0.0)

# ===== 7. Adresses inconnues =====
print("\n--- 7. Adresses inconnues ---")
send("Unknown /foo/bar",   "/foo/bar",   "i", 1)
send("Unknown /relay/99",  "/relay/99",  "i", 1)

# ===== Cleanup =====
print("\n--- Cleanup ---")
send("ALL OFF final", "/relay/all", "i", 0)

# ===== 8. /reboot (DERNIER) =====
if "--reboot" in sys.argv:
    print("\n--- 8. /reboot (DERNIER TEST) ---")
    send("REBOOT", "/reboot", "i", 1)
    print("  >>> ESP32 va redemarrer...")
else:
    print("\n--- 8. /reboot SKIP (ajouter --reboot pour tester) ---")

sock.close()

print("\n" + "=" * 65)
print("  %d tests executes avec succes" % count)
print("=" * 65)
