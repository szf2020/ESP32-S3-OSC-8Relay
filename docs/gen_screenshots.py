#!/usr/bin/env python3
"""Generate screenshots of the web UI for the README."""
import re, pathlib
from playwright.sync_api import sync_playwright

ROOT = pathlib.Path(__file__).resolve().parent.parent
WEB_UI = ROOT / "include" / "web_ui.h"
OUT = ROOT / "docs" / "screenshots"
OUT.mkdir(parents=True, exist_ok=True)

# Extract HTML from PROGMEM string
raw = WEB_UI.read_text(encoding="utf-8")
m = re.search(r'R"HTML\((.*?)\)HTML"', raw, re.DOTALL)
if not m:
    raise RuntimeError("Could not find HTML in web_ui.h")
html = m.group(1)

# Inject fake data so the UI looks populated
fake_js = """
<script>
// Override fetch to return fake data
const fakeConfig = {
  oscListenPort: 8000,
  hostname: "esp32-relay-osc",
  eth: { dhcp: false, ip: "192.168.0.1", gw: "192.168.0.254", mask: "255.255.255.0", dns1: "1.1.1.1", dns2: "8.8.8.8" },
  wifiApAllowed: true,
  apSsid: "ESP32-S3-OSC-8RELAY",
  apPass: "S3Relay!",
  apIp: "192.168.4.1",
  apGw: "192.168.4.1",
  apMask: "255.255.255.0",
  relays: [
    {oscAddress: "/relay/1", invert: false, mode: 0},
    {oscAddress: "/relay/2", invert: false, mode: 0},
    {oscAddress: "/relay/3", invert: true, mode: 0},
    {oscAddress: "/relay/4", invert: false, mode: 1},
    {oscAddress: "/relay/5", invert: false, mode: 0},
    {oscAddress: "/relay/6", invert: false, mode: 0},
    {oscAddress: "/relay/7", invert: false, mode: 0},
    {oscAddress: "/relay/8", invert: false, mode: 0}
  ]
};
const fakeStatus = [true, false, true, false, true, false, false, true];
const fakeSystemStatus = {
  uptime: "0d 01:23:45",
  ethConnected: true,
  ethIp: "192.168.0.1",
  wifiApActive: true,
  wifiApIp: "192.168.4.1",
  freeHeap: 281344,
  minFreeHeap: 265000,
  oscPort: 8000,
  apSsid: "ESP32-S3-OSC-8RELAY",
  relays: [true, false, true, false, true, false, false, true],
  apClients: 1
};

const origFetch = window.fetch;
window.fetch = async function(url, opts) {
  if (typeof url === 'string') {
    if (url.includes('/api/system/status')) return new Response(JSON.stringify(fakeSystemStatus), {status: 200});
    if (url.includes('/api/config')) return new Response(JSON.stringify(fakeConfig), {status: 200});
    if (url.includes('/api/relays/status')) return new Response(JSON.stringify(fakeStatus), {status: 200});
    if (url.includes('/api/relays/')) return new Response('{}', {status: 200});
  }
  return origFetch(url, opts);
};
</script>
"""
html = html.replace("</head>", fake_js + "</head>")

tmp_html = OUT / "_temp_ui.html"
tmp_html.write_text(html, encoding="utf-8")

tabs = {
    "tab_relays": "relays",
    "tab_network": "network",
    "tab_system": "system",
}

with sync_playwright() as p:
    browser = p.chromium.launch()
    page = browser.new_page(viewport={"width": 1280, "height": 900})
    page.goto(f"file://{tmp_html}")
    page.wait_for_timeout(1500)  # let JS load

    for fname, tab_id in tabs.items():
        # Click the tab
        page.click(f'button[data-tab="{tab_id}"]')
        page.wait_for_timeout(500)
        page.screenshot(path=str(OUT / f"{fname}.png"), full_page=True)
        print(f"  ✅ {fname}.png")

    browser.close()

tmp_html.unlink()
print("Done!")
