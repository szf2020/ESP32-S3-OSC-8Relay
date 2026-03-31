#pragma once

static const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>RelayOSC - ESP32-S3-ETH-8DI-8RO</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #0d1117; color: #c9d1d9; line-height: 1.6; }
    header { background: #161b22; padding: 20px; border-bottom: 1px solid #30363d; }
    h1 { font-size: 24px; margin-bottom: 8px; }
    .status { font-size: 12px; color: #8b949e; }
    main { max-width: 1200px; margin: 0 auto; padding: 20px; }
    .tabs { display: flex; gap: 8px; margin-bottom: 20px; flex-wrap: wrap; }
    .tab-btn { padding: 10px 16px; background: #21262d; border: 1px solid #30363d; border-radius: 8px; color: #c9d1d9; cursor: pointer; transition: all 0.3s; }
    .tab-btn:hover { background: #30363d; }
    .tab-btn.active { background: #238636; border-color: #238636; }
    .tab-content { display: none; }
    .tab-content.active { display: block; }
    .card { background: #0d1117; border: 1px solid #30363d; border-radius: 8px; padding: 16px; margin-bottom: 16px; }
    .card h2 { font-size: 18px; margin-bottom: 12px; color: #e6edf3; }
    .form-group { margin-bottom: 16px; }
    label { display: block; margin-bottom: 6px; font-size: 12px; color: #8b949e; font-weight: 600; }
    input, select { width: 100%; padding: 8px 12px; background: #0d1117; border: 1px solid #30363d; border-radius: 6px; color: #c9d1d9; font-size: 12px; }
    input:focus, select:focus { outline: none; border-color: #238636; box-shadow: 0 0 0 3px rgba(35, 134, 54, 0.1); }
    input[type="checkbox"] { width: auto; margin-right: 8px; }
    .checkbox-label { display: flex; align-items: center; font-size: 14px; }
    .grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; }
    .grid-4 { display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px; }
    .relay-card { background: #161b22; padding: 12px; border-radius: 8px; border: 1px solid #30363d; }
    .relay-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
    .relay-name { font-weight: 600; color: #e6edf3; }
    .relay-status { font-size: 12px; color: #8b949e; }
    .relay-btn { width: 100%; padding: 10px; margin-bottom: 8px; border: none; border-radius: 6px; cursor: pointer; font-weight: 600; transition: all 0.2s; }
    .relay-btn.on { background: #238636; color: white; }
    .relay-btn.off { background: #21262d; color: #c9d1d9; border: 1px solid #30363d; }
    button { padding: 10px 16px; background: #238636; color: white; border: none; border-radius: 6px; cursor: pointer; font-weight: 600; transition: all 0.2s; }
    button:hover { background: #2ea043; }
    button:active { transform: translateY(1px); }
    .button-group { display: flex; gap: 8px; flex-wrap: wrap; }
    .info-box { background: #161b22; padding: 12px; border-radius: 6px; font-size: 12px; color: #8b949e; margin-bottom: 12px; border-left: 3px solid #238636; }
    .error { color: #f85149; }
    .success { color: #3fb950; }
    .loading { display: inline-block; width: 12px; height: 12px; border: 2px solid #30363d; border-top-color: #238636; border-radius: 50%; animation: spin 0.8s linear infinite; }
    @keyframes spin { to { transform: rotate(360deg); } }
    .message { padding: 8px 12px; border-radius: 6px; font-size: 12px; margin-bottom: 8px; }
    .message.error { background: rgba(248, 81, 73, 0.1); color: #f85149; }
    .message.success { background: rgba(63, 185, 80, 0.1); color: #3fb950; }
  </style>
</head>
<body>
  <header>
    <h1>⚡ RelayOSC</h1>
    <div id="statusLog" style="font-family:monospace; font-size:11px; color:#3fb950; margin-top:8px; background:#0d1117; border:1px solid #30363d; border-radius:6px; padding:8px 12px; max-height:150px; overflow-y:auto; white-space:pre; line-height:1.4;">Connexion...</div>
  </header>

  <main>
    <div class="tabs">
      <button class="tab-btn active" data-tab="relays">Relais & OSC</button>
      <button class="tab-btn" data-tab="network">Réseau</button>
      <button class="tab-btn" data-tab="system">Système</button>
    </div>

    <!-- ONGLET: RELAIS & OSC -->
    <div id="tab-relays" class="tab-content active">
      <div class="card">
        <h2>🎛️ Contrôle des Relais</h2>
        <div class="info-box">
          Écoute OSC sur port UDP: <strong id="oscPortView">8000</strong>
        </div>
        <div class="grid-4" id="relayControlGrid"></div>
        <div class="button-group" style="margin-top: 12px; justify-content: center;">
          <button style="background:#238636; min-width:140px;" onclick="setAllRelays(true)">✅ ALL ON</button>
          <button style="background:#da3633; min-width:140px;" onclick="setAllRelays(false)">⛔ ALL OFF</button>
        </div>
      </div>

      <div class="card">
        <h2>⚙️ Configuration OSC par Relais</h2>
        <div id="relayCfgGrid"></div>
        <div class="button-group" style="margin-top: 16px;">
          <button id="saveRelays">💾 Sauvegarder</button>
          <span id="saveRelaysMsg"></span>
        </div>
      </div>
    </div>

    <!-- ONGLET: RÉSEAU -->
    <div id="tab-network" class="tab-content">
      <div class="card">
        <h2>🌐 Ethernet (W5500)</h2>
        <div class="grid-2">
          <div class="form-group">
            <label class="checkbox-label">
              <input type="checkbox" id="ethDhcp">
              <span>Utiliser DHCP</span>
            </label>
            <p class="info-box" style="margin-top: 8px;">Si désactivé, utiliser les paramètres statiques ci-dessous.</p>
          </div>
          <div class="form-group">
            <label>Hostname</label>
            <input id="hostname" placeholder="esp32-relay-osc">
          </div>
          <div class="form-group">
            <label>Adresse IP</label>
            <input id="ethIp" placeholder="192.168.1.50">
          </div>
          <div class="form-group">
            <label>Masque de sous-réseau</label>
            <input id="ethMask" placeholder="255.255.255.0">
          </div>
          <div class="form-group">
            <label>Passerelle</label>
            <input id="ethGw" placeholder="192.168.1.1">
          </div>
          <div class="form-group">
            <label>DNS 1</label>
            <input id="ethDns1" placeholder="8.8.8.8">
          </div>
          <div class="form-group">
            <label>DNS 2</label>
            <input id="ethDns2" placeholder="1.1.1.1">
          </div>
          <div class="form-group">
            <label>Port d'écoute OSC (UDP)</label>
            <input id="oscPort" type="number" min="1024" max="65535" placeholder="8000">
          </div>
        </div>
        <div class="button-group">
          <button id="saveNet">💾 Sauvegarder</button>
          <span id="saveNetMsg"></span>
        </div>
      </div>

      <div class="card">
        <h2>📡 Wi-Fi / Point d'Accès (AP)</h2>
        <div class="info-box">
          L'AP WiFi n'est activé que si <strong>DI8 est actif au démarrage</strong> et si l'option ci-dessous est autorisée.
        </div>
        <div class="grid-2">
          <div class="form-group">
            <label class="checkbox-label">
              <input type="checkbox" id="apAllow">
              <span>Autoriser le mode AP</span>
            </label>
          </div>
          <div class="form-group">
            <label>SSID (nom du réseau WiFi)</label>
            <input id="apSsid" placeholder="RelayOSC">
          </div>
          <div class="form-group">
            <label>Mot de passe (≥8 caractères)</label>
            <input id="apPass" type="password" placeholder="relayosc123">
          </div>
          <div class="form-group">
            <label>IP du Point d'Accès</label>
            <input id="apIp" placeholder="192.168.4.1">
          </div>
          <div class="form-group">
            <label>Masque AP</label>
            <input id="apMask" placeholder="255.255.255.0">
          </div>
          <div class="form-group">
            <label>Passerelle AP</label>
            <input id="apGw" placeholder="192.168.4.1">
          </div>
        </div>
        <div class="button-group">
          <button id="saveAp">💾 Sauvegarder</button>
          <span id="saveApMsg"></span>
        </div>
      </div>
    </div>

    <!-- ONGLET: SYSTÈME -->
    <div id="tab-system" class="tab-content">
      <div class="card">
        <h2>ℹ️ Informations Système</h2>
        <div class="grid-2">
          <div><strong>Modèle:</strong> ESP32-S3-ETH-8DI-8RO</div>
          <div><strong>CPU:</strong> <span id="sysUptime">—</span></div>
          <div><strong>RAM libre:</strong> <span id="sysRam">—</span></div>
          <div><strong>IP Ethernet:</strong> <span id="sysEthIp">—</span></div>
          <div><strong>État WiFi:</strong> <span id="sysWifiState">—</span></div>
          <div><strong>SSID WiFi:</strong> <span id="sysWifiSsid">—</span></div>
        </div>
      </div>

      <div class="card">
        <h2>⚡ Actions</h2>
        <div class="button-group">
          <button id="rebootBtn" style="background: #f85149;">🔄 Redémarrer l'ESP32</button>
          <button id="factoryBtn" style="background: #d1242f;">🗑️ Réinitialiser aux paramètres d'usine</button>
        </div>
      </div>

      <div class="card">
        <h2>📝 Changelog</h2>
        <pre style="background: #0d1117; padding: 12px; border-radius: 6px; font-size: 12px; overflow: auto; max-height: 200px;">
v1.0.0 - Janvier 2025
- Interface Web complète
- Contrôle des 8 relais via OSC
- Configuration Ethernet (DHCP/statique)
- Support WiFi AP optionnel (via DI8)
- Stockage config en EEPROM
        </pre>
      </div>
    </div>
  </main>

  <script>
    const API_BASE = '/api';
    
    // Tab switching
    document.querySelectorAll('.tab-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const tab = btn.dataset.tab;
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        btn.classList.add('active');
        document.getElementById(`tab-${tab}`).classList.add('active');
      });
    });

    // Load configuration
    async function loadConfig() {
      try {
        const resp = await fetch(`${API_BASE}/config`);
        const cfg = await resp.json();
        
        // Network
        document.getElementById('ethDhcp').checked = cfg.eth.dhcp;
        document.getElementById('hostname').value = cfg.hostname;
        document.getElementById('ethIp').value = cfg.eth.ip;
        document.getElementById('ethMask').value = cfg.eth.mask;
        document.getElementById('ethGw').value = cfg.eth.gw;
        document.getElementById('ethDns1').value = cfg.eth.dns1;
        document.getElementById('ethDns2').value = cfg.eth.dns2;
        document.getElementById('oscPort').value = cfg.oscListenPort;
        document.getElementById('oscPortView').textContent = cfg.oscListenPort;

        // WiFi AP
        document.getElementById('apAllow').checked = cfg.wifiApAllowed;
        document.getElementById('apSsid').value = cfg.apSsid;
        document.getElementById('apPass').value = cfg.apPass;
        document.getElementById('apIp').value = cfg.apIp;
        document.getElementById('apMask').value = cfg.apMask;
        document.getElementById('apGw').value = cfg.apGw;

        // Relays
        const relayCtrlGrid = document.getElementById('relayControlGrid');
        relayCtrlGrid.innerHTML = '';
        const relayCfgGrid = document.getElementById('relayCfgGrid');
        relayCfgGrid.innerHTML = '';

        for (let i = 0; i < 8; i++) {
          const r = cfg.relays[i];

          // Control card
          const ctrlDiv = document.createElement('div');
          ctrlDiv.className = 'relay-card';
          ctrlDiv.innerHTML = `
            <div class="relay-header">
              <span class="relay-name">Relais ${i + 1}</span>
              <span class="relay-status" id="relay-status-${i}">OFF</span>
            </div>
            <button class="relay-btn off" id="relay-btn-${i}" onclick="toggleRelay(${i})">OFF</button>
          `;
          relayCtrlGrid.appendChild(ctrlDiv);

          // Config card
          const cfgDiv = document.createElement('div');
          cfgDiv.className = 'relay-card';
          cfgDiv.innerHTML = `
            <div class="relay-header">
              <span class="relay-name">Relais ${i + 1}</span>
            </div>
            <div class="form-group" style="margin: 0;">
              <label>Adresse OSC</label>
              <input class="relay-osc-addr" data-index="${i}" value="${r.oscAddress}" placeholder="/relay/${i + 1}">
            </div>
            <div class="form-group" style="margin: 8px 0 0 0;">
              <label class="checkbox-label">
                <input type="checkbox" class="relay-invert" data-index="${i}" ${r.invert ? 'checked' : ''}>
                <span>Inverser logique</span>
              </label>
            </div>
            <div class="form-group" style="margin: 8px 0 0 0;">
              <label>Mode</label>
              <select class="relay-mode" data-index="${i}">
                <option value="0" ${r.mode === 0 ? 'selected' : ''}>Latch (valeur directe)</option>
                <option value="1" ${r.mode === 1 ? 'selected' : ''}>Toggle (basculement)</option>
              </select>
            </div>
          `;
          relayCfgGrid.appendChild(cfgDiv);
        }

        updateRelayStatus();
      } catch (e) {
        console.error('Erreur chargement config:', e);
        showMessage('saveRelaysMsg', 'Erreur chargement', true);
      }
    }

    // Update relay status
    async function updateRelayStatus() {
      try {
        const resp = await fetch(`${API_BASE}/relays/status`);
        const status = await resp.json();
        for (let i = 0; i < 8; i++) {
          const btn = document.getElementById(`relay-btn-${i}`);
          const statusSpan = document.getElementById(`relay-status-${i}`);
          if (status[i]) {
            btn.classList.remove('off');
            btn.classList.add('on');
            btn.textContent = 'ON';
            statusSpan.textContent = 'ON';
          } else {
            btn.classList.remove('on');
            btn.classList.add('off');
            btn.textContent = 'OFF';
            statusSpan.textContent = 'OFF';
          }
        }
      } catch (e) {
        console.error('Erreur statut relais:', e);
      }
    }

    // Set all relays ON or OFF
    async function setAllRelays(state) {
      try {
        await fetch(`${API_BASE}/relays/all`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ state: state })
        });
        updateRelayStatus();
      } catch (e) {
        console.error('Erreur ALL relais:', e);
      }
    }

    // Toggle relay
    async function toggleRelay(idx) {
      try {
        const btn = document.getElementById(`relay-btn-${idx}`);
        const newState = !btn.classList.contains('on');
        await fetch(`${API_BASE}/relays/${idx}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ state: newState })
        });
        updateRelayStatus();
      } catch (e) {
        console.error('Erreur toggle relais:', e);
      }
    }

    // Save functions
    function showMessage(elemId, text, isError = false) {
      const el = document.getElementById(elemId);
      el.textContent = text;
      el.className = isError ? 'message error' : 'message success';
      setTimeout(() => el.textContent = '', 3000);
    }

    document.getElementById('saveRelays').addEventListener('click', async () => {
      const relays = [];
      for (let i = 0; i < 8; i++) {
        relays.push({
          oscAddress: document.querySelector(`.relay-osc-addr[data-index="${i}"]`).value,
          invert: document.querySelector(`.relay-invert[data-index="${i}"]`).checked,
          mode: parseInt(document.querySelector(`.relay-mode[data-index="${i}"]`).value)
        });
      }
      try {
        const resp = await fetch(`${API_BASE}/config/relays`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ relays })
        });
        if (resp.ok) {
          showMessage('saveRelaysMsg', '✓ Configuration sauvegardée');
        } else {
          showMessage('saveRelaysMsg', '✗ Erreur sauvegarde', true);
        }
      } catch (e) {
        showMessage('saveRelaysMsg', 'Erreur réseau', true);
      }
    });

    document.getElementById('saveNet').addEventListener('click', async () => {
      const cfg = {
        eth: {
          dhcp: document.getElementById('ethDhcp').checked,
          ip: document.getElementById('ethIp').value,
          mask: document.getElementById('ethMask').value,
          gw: document.getElementById('ethGw').value,
          dns1: document.getElementById('ethDns1').value,
          dns2: document.getElementById('ethDns2').value
        },
        hostname: document.getElementById('hostname').value,
        oscListenPort: parseInt(document.getElementById('oscPort').value)
      };
      try {
        const resp = await fetch(`${API_BASE}/config/network`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(cfg)
        });
        if (resp.ok) {
          showMessage('saveNetMsg', '✓ Configuration réseau sauvegardée');
        } else {
          showMessage('saveNetMsg', '✗ Erreur sauvegarde', true);
        }
      } catch (e) {
        showMessage('saveNetMsg', 'Erreur réseau', true);
      }
    });

    document.getElementById('saveAp').addEventListener('click', async () => {
      const cfg = {
        wifiApAllowed: document.getElementById('apAllow').checked,
        apSsid: document.getElementById('apSsid').value,
        apPass: document.getElementById('apPass').value,
        apIp: document.getElementById('apIp').value,
        apMask: document.getElementById('apMask').value,
        apGw: document.getElementById('apGw').value
      };
      try {
        const resp = await fetch(`${API_BASE}/config/ap`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(cfg)
        });
        if (resp.ok) {
          showMessage('saveApMsg', '✓ Configuration AP sauvegardée');
        } else {
          showMessage('saveApMsg', '✗ Erreur sauvegarde', true);
        }
      } catch (e) {
        showMessage('saveApMsg', 'Erreur réseau', true);
      }
    });

    document.getElementById('rebootBtn').addEventListener('click', () => {
      if (confirm('Redémarrer l\'ESP32 ?')) {
        fetch(`${API_BASE}/system/reboot`, { method: 'POST' });
        showMessage('saveApMsg', 'Redémarrage en cours...');
      }
    });

    document.getElementById('factoryBtn').addEventListener('click', () => {
      if (confirm('Réinitialiser aux paramètres d\'usine ? Cette action est irréversible.')) {
        fetch(`${API_BASE}/system/factoryreset`, { method: 'POST' });
        showMessage('saveApMsg', 'Réinitialisation en cours...');
      }
    });

    const MAX_LOG_LINES = 50;

    // Verbose status log in header (scrolling multi-line)
    async function updateStatusLog() {
      const el = document.getElementById('statusLog');
      try {
        const resp = await fetch(`${API_BASE}/system/status`);
        const s = await resp.json();
        const relayStr = s.relays.map((v,i) => `R${i+1}:${v?'\x1b[32mON\x1b[0m':'OFF'}`).join(' ');
        const heap = (s.freeHeap/1024).toFixed(0);
        const heapMin = (s.minFreeHeap/1024).toFixed(0);
        const now = new Date().toLocaleTimeString('fr-FR');
        const line = `[${now}] uptime=${s.uptime} ETH=${s.ethConnected?'✓':'✗'} IP=${s.ethIp} | AP=${s.wifiApActive?'UP':'DOWN'} SSID=${s.apSsid} Clients=${s.apClients} | OSC:${s.oscPort} | RAM=${heap}K (min:${heapMin}K) | ${relayStr}`;
        // Append new line, keep max lines
        let lines = el.textContent === 'Connexion...' ? [] : el.textContent.split('\n');
        lines.push(line);
        if (lines.length > MAX_LOG_LINES) lines = lines.slice(-MAX_LOG_LINES);
        el.textContent = lines.join('\n');
        el.style.color = s.ethConnected ? '#3fb950' : '#f85149';
        el.scrollTop = el.scrollHeight;
      } catch (e) {
        let lines = el.textContent === 'Connexion...' ? [] : el.textContent.split('\n');
        const now = new Date().toLocaleTimeString('fr-FR');
        lines.push(`[${now}] ⚠ Connexion perdue...`);
        if (lines.length > MAX_LOG_LINES) lines = lines.slice(-MAX_LOG_LINES);
        el.textContent = lines.join('\n');
        el.style.color = '#f85149';
        el.scrollTop = el.scrollHeight;
      }
    }

    // Initial load
    loadConfig();
    updateStatusLog();
    setInterval(updateRelayStatus, 2000);
    setInterval(updateStatusLog, 3000);
  </script>
</body>
</html>)HTML";
