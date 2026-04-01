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
  apTimeoutMin: 5,
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
const fakeLive = {
  r: [1, 0, 1, 0, 1, 0, 0, 1],
  up: 5025,
  heap: 281344,
  hmin: 265000,
  eth: true,
  eip: "192.168.0.1",
  ap: true,
  aip: "192.168.4.1",
  acl: 1,
  t: 42.3,
  oP: 8000,
  aSsid: "ESP32-S3-OSC-8RELAY",
  aTo: 5,
  osc: [
    {ts: 1711929601000, a: "/relay/1", t: "i", v: "1"},
    {ts: 1711929601300, a: "/relay/all", t: "i", v: "0"}
  ]
};

const fakeChangelog = [
  {tag_name:"v1.2.6",name:"Changelog lie aux GitHub Releases",html_url:"https://github.com/NeOdYmS/ESP32-S3-OSC-8Relay/releases/tag/v1.2.6",published_at:"2026-04-01T00:00:00Z",body:"### Ameliorations UI\\n- Changelog synchronise avec GitHub Releases\\n- Fallback firmware embarque si hors ligne"},
  {tag_name:"v1.2.5",name:"Drapeaux langue + portail captif dynamique",html_url:"https://github.com/NeOdYmS/ESP32-S3-OSC-8Relay/releases/tag/v1.2.5",published_at:"2026-04-01T00:00:00Z",body:"### Ameliorations UI\\n- Selecteur langue remplace par boutons drapeaux\\n- Portail captif : redirections dynamiques\\n### Robustesse API\\n- Validation JSON sur tous les endpoints POST"}
];
const origFetch = window.fetch;
window.fetch = async function(url, opts) {
  if (typeof url === 'string') {
    if (url.includes('/api/live')) return new Response(JSON.stringify(fakeLive), {status: 200});
    if (url.includes('/api/config')) return new Response(JSON.stringify(fakeConfig), {status: 200});
    if (url.includes('/api/relays/')) return new Response('{}', {status: 200});
    if (url.includes('api.github.com') && url.includes('releases')) return new Response(JSON.stringify(fakeChangelog), {status: 200});
    if (url.includes('/api/changelog')) return new Response('fallback local', {status: 200});
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

    errors = []
    page.on("console", lambda msg: errors.append(f"[{msg.type}] {msg.text}") if msg.type in ("error","warning") else None)
    page.on("pageerror", lambda e: errors.append(f"[pageerror] {e}"))

    page.goto(f"file://{tmp_html}")
    page.wait_for_timeout(2500)

    # Force populate relay grids via evaluate (bypasses any timing issue with loadConfig)
    page.evaluate("""() => {
      const fakeRelays = [
        {oscAddress:'/relay/1',invert:false,mode:0},{oscAddress:'/relay/2',invert:false,mode:0},
        {oscAddress:'/relay/3',invert:true, mode:0},{oscAddress:'/relay/4',invert:false,mode:1},
        {oscAddress:'/relay/5',invert:false,mode:0},{oscAddress:'/relay/6',invert:false,mode:0},
        {oscAddress:'/relay/7',invert:false,mode:0},{oscAddress:'/relay/8',invert:false,mode:0}
      ];
      const states = [1,0,1,0,1,0,0,1];
      const ctrlGrid = document.getElementById('relayControlGrid');
      const cfgGrid  = document.getElementById('relayCfgGrid');
      if (ctrlGrid) ctrlGrid.innerHTML = '';
      if (cfgGrid)  cfgGrid.innerHTML  = '';
      for (let i = 0; i < 8; i++) {
        const r = fakeRelays[i];
        const on = !!states[i];
        if (ctrlGrid) {
          const d = document.createElement('div');
          d.className = 'relay-card';
          d.innerHTML = '<div class="relay-header"><span class="relay-name">Relay '+(i+1)+'</span>'
            +'<span class="relay-status" id="relay-status-'+i+'">'+(on?'ON':'OFF')+'</span></div>'
            +'<button class="relay-btn '+(on?'on':'off')+'" id="relay-btn-'+i+'">'+(on?'ON':'OFF')+'</button>';
          ctrlGrid.appendChild(d);
        }
        if (cfgGrid) {
          const c = document.createElement('div');
          c.className = 'relay-card';
          c.innerHTML = '<div class="relay-header"><span class="relay-name">Relay '+(i+1)+'</span></div>'
            +'<div class="form-group" style="margin:0"><label>OSC Address</label>'
            +'<input value="'+r.oscAddress+'"></div>'
            +'<div class="form-group" style="margin:8px 0 0 0"><label class="checkbox-label">'
            +'<input type="checkbox"'+(r.invert?' checked':'')+'><span>Invert logic</span></label></div>'
            +'<div class="form-group" style="margin:8px 0 0 0"><label>Mode</label>'
            +'<select><option value="0"'+(r.mode===0?' selected':'')+'">Latch</option>'
            +'<option value="1"'+(r.mode===1?' selected':'')+'>Toggle</option></select></div>';
          cfgGrid.appendChild(c);
        }
      }
    }""")
    page.wait_for_timeout(300)

    if errors:
        print("  ⚠ JS errors captured:")
        for e in errors[:10]:
            print("   ", e)

    for fname, tab_id in tabs.items():
        page.click(f'button[data-tab="{tab_id}"]')
        page.wait_for_timeout(1200)
        page.screenshot(path=str(OUT / f"{fname}.png"), full_page=True)
        print(f"  ✅ {fname}.png")

    browser.close()

tmp_html.unlink()
print("Done!")
