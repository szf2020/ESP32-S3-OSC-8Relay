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
    header { position: relative; background: #161b22; padding: 20px; border-bottom: 1px solid #30363d; }
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

    /* Responsive mobile */
    @media (max-width: 768px) {
      header { padding: 12px; }
      h1 { font-size: 20px; }
      main { padding: 10px; }
      .tabs { gap: 4px; }
      .tab-btn { padding: 8px 10px; font-size: 13px; flex: 1; text-align: center; min-width: 0; }
      .grid-4 { grid-template-columns: repeat(2, 1fr); gap: 8px; }
      .grid-2 { grid-template-columns: 1fr; gap: 10px; }
      .card { padding: 12px; margin-bottom: 10px; }
      .card h2 { font-size: 16px; }
      .relay-card { padding: 10px; }
      .button-group { flex-direction: column; }
      .button-group button { width: 100%; }
      #unifiedLog { height: 80px; font-size: 10px; padding: 6px 8px; }
      .relay-btn { padding: 12px; font-size: 14px; }
      input, select { font-size: 14px; padding: 10px 12px; }
      button { padding: 12px 16px; font-size: 14px; }
      #langWrap { top:8px; right:8px; gap:2px; }
      .lang-btn { font-size:15px; padding:1px 3px; }
    }
    @media (max-width: 480px) {
      .grid-4 { grid-template-columns: 1fr 1fr; gap: 6px; }
      .tab-btn { padding: 8px 6px; font-size: 12px; }
      #unifiedLog { max-height: 100px; font-size: 9px; }
      h1 { font-size: 18px; }
    }
    #langWrap { position:absolute; top:10px; right:14px; opacity:0.85; display:flex; gap:3px; }
    .lang-btn { background:none; border:1px solid transparent; border-radius:6px; padding:2px 4px; font-size:18px; cursor:pointer; line-height:1; transition:border-color 0.2s, background 0.2s; }
    .lang-btn:hover { border-color:#3d444d; background:#21262d; }
    .lang-btn.active { border-color:#238636; background:#0d2318; }
  </style>
</head>
<body>
  <header>
    <div id="langWrap">
      <button class="lang-btn" title="Français" onclick="setLang('fr')">🇫🇷</button>
      <button class="lang-btn" title="English" onclick="setLang('en')">🇬🇧</button>
      <button class="lang-btn" title="Español" onclick="setLang('es')">🇪🇸</button>
      <button class="lang-btn" title="Deutsch" onclick="setLang('de')">🇩🇪</button>
      <button class="lang-btn" title="中文" onclick="setLang('zh')">🇨🇳</button>
    </div>
    <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:8px;">
      <h1>⚡ RelayOSC</h1>
    </div>
    <div id="unifiedLog" style="font-family:monospace; font-size:11px; margin-top:8px; background:#0d1117; border:1px solid #30363d; border-radius:6px 6px 0 0; padding:8px 12px; height:102px; overflow-y:auto; line-height:1.5; min-height:40px; max-height:400px;"><span style="color:#8b949e;">Connexion...</span></div>
    <div id="logDragBar" style="height:8px; background:#161b22; border-left:1px solid #30363d; border-right:1px solid #30363d; cursor:ns-resize; display:flex; align-items:center; justify-content:center; user-select:none; -webkit-user-select:none;">
      <div style="width:40px; height:3px; background:#30363d; border-radius:2px;"></div>
    </div>
    <div style="display:flex; gap:6px; justify-content:flex-end; background:#161b22; border:1px solid #30363d; border-top:none; border-radius:0 0 6px 6px; padding:3px 8px;">
      <button id="logAutoScrollBtn" onclick="toggleAutoScroll()" style="padding:2px 8px; font-size:10px; background:#238636; border:none; border-radius:4px; color:#fff; cursor:pointer;">⬇ Auto</button>
      <button onclick="clearLog()" style="padding:2px 8px; font-size:10px; background:#da3633; border:none; border-radius:4px; color:#fff; cursor:pointer;">✕ Clear</button>
    </div>
  </header>

  <main>
    <div class="tabs">
      <button class="tab-btn active" data-tab="relays" data-i18n="tab_relays">Relais & OSC</button>
      <button class="tab-btn" data-tab="network" data-i18n="tab_network">Réseau</button>
      <button class="tab-btn" data-tab="system" data-i18n="tab_system">Système</button>
    </div>

    <!-- ONGLET: RELAIS & OSC -->
    <div id="tab-relays" class="tab-content active">
      <div class="card">
        <h2>🏛️ <span data-i18n="relay_control">Contrôle des Relais</span></h2>
        <div class="info-box">
          <span data-i18n="osc_listen">Écoute OSC sur port UDP:</span> <strong id="oscPortView">8000</strong>
        </div>
        <div class="grid-4" id="relayControlGrid"></div>
        <div class="button-group" style="margin-top: 12px; justify-content: center;">
          <button style="background:#238636; min-width:140px;" onclick="setAllRelays(true)" data-i18n="all_on">✅ ALL ON</button>
          <button style="background:#da3633; min-width:140px;" onclick="setAllRelays(false)" data-i18n="all_off">⛔ ALL OFF</button>
        </div>
      </div>

      <div class="card">
        <h2>⚙️ <span data-i18n="relay_osc_config">Configuration OSC par Relais</span></h2>
        <div id="relayCfgGrid"></div>
        <div class="button-group" style="margin-top: 16px;">
          <button id="saveRelays" data-i18n="save_btn">💾 Sauvegarder</button>
          <span id="saveRelaysMsg"></span>
        </div>
      </div>
    </div>

    <!-- ONGLET: RÉSEAU -->
    <div id="tab-network" class="tab-content">
      <div class="card">
        <h2>🌐 <span data-i18n="eth_title">Ethernet (W5500)</span></h2>
        <div class="grid-2">
          <div class="form-group">
            <label class="checkbox-label">
              <input type="checkbox" id="ethDhcp">
              <span data-i18n="use_dhcp">Utiliser DHCP</span>
            </label>
            <p class="info-box" style="margin-top: 8px;" data-i18n="dhcp_info">Si désactivé, utiliser les paramètres statiques ci-dessous.</p>
          </div>
          <div class="form-group">
            <label data-i18n="hostname">Hostname</label>
            <input id="hostname" placeholder="esp32-relay-osc">
          </div>
          <div class="form-group">
            <label data-i18n="ip_addr">Adresse IP</label>
            <input id="ethIp" placeholder="192.168.1.50">
          </div>
          <div class="form-group">
            <label data-i18n="subnet">Masque de sous-réseau</label>
            <input id="ethMask" placeholder="255.255.255.0">
          </div>
          <div class="form-group">
            <label data-i18n="gateway">Passerelle</label>
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
            <label data-i18n="osc_port">Port d'écoute OSC (UDP)</label>
            <input id="oscPort" type="number" min="1024" max="65535" placeholder="8000">
          </div>
        </div>
        <div class="button-group">
          <button id="saveNet" data-i18n="save_btn">💾 Sauvegarder</button>
          <span id="saveNetMsg"></span>
        </div>
      </div>

      <div class="card">
        <h2>📡 <span data-i18n="ap_title">Wi-Fi / Point d'Accès (AP)</span></h2>
        <div class="info-box" data-i18n-html="ap_info">
          L'AP WiFi démarre automatiquement et s'éteint après le délai configuré (0 = toujours actif).
        </div>
        <div class="grid-2">
          <div class="form-group">
            <label class="checkbox-label">
              <input type="checkbox" id="apAllow">
              <span data-i18n="allow_ap">Autoriser le mode AP</span>
            </label>
          </div>
          <div class="form-group">
            <label data-i18n="ssid_label">SSID (nom du réseau WiFi)</label>
            <input id="apSsid" placeholder="RelayOSC">
          </div>
          <div class="form-group">
            <label data-i18n="password">Mot de passe (≥8 caractères)</label>
            <input id="apPass" type="password" placeholder="relayosc123">
          </div>
          <div class="form-group">
            <label data-i18n="ap_ip">IP du Point d'Accès</label>
            <input id="apIp" placeholder="192.168.4.1">
          </div>
          <div class="form-group">
            <label data-i18n="ap_mask">Masque AP</label>
            <input id="apMask" placeholder="255.255.255.0">
          </div>
          <div class="form-group">
            <label data-i18n="ap_gw">Passerelle AP</label>
            <input id="apGw" placeholder="192.168.4.1">
          </div>
          <div class="form-group">
            <label data-i18n="ap_timeout">Mise en veille AP (minutes)</label>
            <input id="apTimeout" type="number" min="0" placeholder="5">
            <small style="color:#888;" data-i18n="ap_timeout_info">0 = toujours actif (pas de mise en veille)</small>
          </div>
        </div>
        <div class="button-group">
          <button id="saveAp" data-i18n="save_btn">💾 Sauvegarder</button>
          <span id="saveApMsg"></span>
        </div>
        <div style="text-align:center; margin-top:20px; padding:16px; background:#161b22; border-radius:8px; border:1px solid #30363d;">
          <div style="font-size:14px; font-weight:600; margin-bottom:12px; color:#e6edf3;" data-i18n="qr_title">📱 QR Code WiFi</div>
          <canvas id="qrCanvas" style="image-rendering:pixelated; image-rendering:-webkit-optimize-contrast;"></canvas>
          <div style="font-size:11px; color:#8b949e; margin-top:8px;" data-i18n="qr_scan">Scannez avec votre iPhone ou téléphone pour vous connecter</div>
        </div>
      </div>
    </div>

    <!-- ONGLET: SYSTÈME -->
    <div id="tab-system" class="tab-content">
      <div class="card">
        <h2>ℹ️ <span data-i18n="sys_title">Informations Système</span></h2>
        <div class="grid-2">
          <div><strong data-i18n="model">Modèle:</strong> ESP32-S3-ETH-8DI-8RO</div>
          <div><strong data-i18n="uptime">Uptime:</strong> <span id="sysUptime">—</span></div>
          <div><strong data-i18n="free_ram">RAM libre:</strong> <span id="sysRam">—</span></div>
          <div><strong data-i18n="min_ram">RAM min:</strong> <span id="sysRamMin">—</span></div>
          <div><strong data-i18n="ethernet_lbl">Ethernet:</strong> <span id="sysEthState">—</span></div>
          <div><strong data-i18n="eth_ip">IP Ethernet:</strong> <span id="sysEthIp">—</span></div>
          <div><strong data-i18n="osc_port_sys">Port OSC:</strong> <span id="sysOscPort">—</span></div>
          <div><strong data-i18n="wifi_ap_lbl">WiFi AP:</strong> <span id="sysWifiState">—</span></div>
          <div><strong>SSID:</strong> <span id="sysWifiSsid">—</span></div>
          <div><strong data-i18n="ap_ip_sys">IP AP:</strong> <span id="sysWifiIp">—</span></div>
          <div><strong data-i18n="ap_clients">Clients AP:</strong> <span id="sysApClients">—</span></div>
          <div><strong data-i18n="ap_sleep_lbl">Mise en veille AP:</strong> <span id="sysApTimeout">—</span></div>
          <div><strong data-i18n="active_relays">Relais actifs:</strong> <span id="sysRelays">—</span></div>
          <div><strong data-i18n="cpu_temp">Température CPU:</strong> <span id="sysCpuTemp">—</span></div>
        </div>
      </div>

      <div class="card">
        <h2>⚡ <span data-i18n="actions">Actions</span></h2>
        <div class="button-group">
          <button id="rebootBtn" style="background: #f85149;" data-i18n="reboot">🔄 Redémarrer l'ESP32</button>
          <button id="factoryBtn" style="background: #d1242f;" data-i18n="factory_reset">🗑️ Réinitialiser aux paramètres d'usine</button>
        </div>
      </div>

      <div class="card">
        <h2>📝 <span data-i18n="changelog">Changelog</span></h2>
        <div class="info-box" style="margin-bottom:10px;display:flex;align-items:center;justify-content:space-between;">
          <span id="changelogSrc" style="font-size:12px;color:#8b949e;"></span>
          <a href="https://github.com/NeOdYmS/ESP32-S3-OSC-8Relay/releases" target="_blank" rel="noopener noreferrer" style="color:#58a6ff;font-size:12px;">GitHub Releases ↗</a>
        </div>
        <div id="changelogContent" style="max-height:340px;overflow:auto;"></div>
      </div>
    </div>
  </main>

  <script>
    const API_BASE = '/api';

    // ===== i18n =====
    var LANG = localStorage.getItem('lang') || navigator.language.slice(0,2) || 'fr';
    var LOCALES = {fr:'fr-FR',en:'en-GB',es:'es-ES',de:'de-DE',zh:'zh-CN'};
    var I18N = {
      fr:{tab_relays:'Relais & OSC',tab_network:'Réseau',tab_system:'Système',relay_control:'Contrôle des Relais',osc_listen:'Écoute OSC sur port UDP:',all_on:'✅ ALL ON',all_off:'⛔ ALL OFF',relay_osc_config:'Configuration OSC par Relais',save_btn:'💾 Sauvegarder',eth_title:'Ethernet (W5500)',use_dhcp:'Utiliser DHCP',dhcp_info:'Si désactivé, utiliser les paramètres statiques ci-dessous.',hostname:'Hostname',ip_addr:'Adresse IP',subnet:'Masque de sous-réseau',gateway:'Passerelle',osc_port:"Port d'écoute OSC (UDP)",ap_title:"Wi-Fi / Point d'Accès (AP)",ap_info:"L'AP WiFi démarre automatiquement et s'éteint après le délai configuré (0 = toujours actif).",allow_ap:'Autoriser le mode AP',ssid_label:'SSID (nom du réseau WiFi)',password:'Mot de passe (≥8 caractères)',ap_ip:"IP du Point d'Accès",ap_mask:'Masque AP',ap_gw:'Passerelle AP',ap_timeout:'Mise en veille AP (minutes)',ap_timeout_info:'0 = toujours actif (pas de mise en veille)',qr_title:'📱 QR Code WiFi',qr_scan:'Scannez avec votre iPhone ou téléphone pour vous connecter',sys_title:'Informations Système',model:'Modèle:',uptime:'Uptime:',free_ram:'RAM libre:',min_ram:'RAM min:',ethernet_lbl:'Ethernet:',eth_ip:'IP Ethernet:',osc_port_sys:'Port OSC:',wifi_ap_lbl:'WiFi AP:',ap_ip_sys:'IP AP:',ap_clients:'Clients AP:',ap_sleep_lbl:'Mise en veille AP:',active_relays:'Relais actifs:',cpu_temp:'Température CPU:',actions:'Actions',reboot:"🔄 Redémarrer l'ESP32",factory_reset:"🗑️ Réinitialiser aux paramètres d'usine",changelog:'Changelog',relay:'Relais',osc_addr:'Adresse OSC',invert:'Inverser logique',mode:'Mode',mode_latch:'Latch (valeur directe)',mode_toggle:'Toggle (basculement)',config_saved:'Configuration sauvegardée',save_error:'Erreur sauvegarde',net_error:'Erreur réseau',net_saved:'Configuration réseau sauvegardée',ap_saved:'Configuration AP sauvegardée',confirm_reboot:"Redémarrer l'ESP32 ?",rebooting:'Redémarrage en cours...',confirm_factory:"Réinitialiser aux paramètres d'usine ? Cette action est irréversible.",resetting:'Réinitialisation en cours...',log_cleared:'Log vidé',connecting:'Connexion...',connected:'✅ Connecté',disconnected:'❌ Déconnecté',ap_active:'✅ Actif',ap_inactive:'⏸️ Inactif',disabled_always:'Désactivée (toujours actif)',conn_lost:'Connexion perdue...',ram_critical:'⚠ RAM critique'},
      en:{tab_relays:'Relays & OSC',tab_network:'Network',tab_system:'System',relay_control:'Relay Control',osc_listen:'OSC listening on UDP port:',all_on:'✅ ALL ON',all_off:'⛔ ALL OFF',relay_osc_config:'OSC Configuration per Relay',save_btn:'💾 Save',eth_title:'Ethernet (W5500)',use_dhcp:'Use DHCP',dhcp_info:'If disabled, use static settings below.',hostname:'Hostname',ip_addr:'IP Address',subnet:'Subnet Mask',gateway:'Gateway',osc_port:'OSC Listen Port (UDP)',ap_title:'Wi-Fi / Access Point (AP)',ap_info:'WiFi AP starts automatically and shuts down after the configured timeout (0 = always active).',allow_ap:'Allow AP mode',ssid_label:'SSID (WiFi network name)',password:'Password (≥8 characters)',ap_ip:'Access Point IP',ap_mask:'AP Mask',ap_gw:'AP Gateway',ap_timeout:'AP Sleep Timeout (minutes)',ap_timeout_info:'0 = always active (no sleep)',qr_title:'📱 WiFi QR Code',qr_scan:'Scan with your phone to connect',sys_title:'System Information',model:'Model:',uptime:'Uptime:',free_ram:'Free RAM:',min_ram:'Min RAM:',ethernet_lbl:'Ethernet:',eth_ip:'Ethernet IP:',osc_port_sys:'OSC Port:',wifi_ap_lbl:'WiFi AP:',ap_ip_sys:'AP IP:',ap_clients:'AP Clients:',ap_sleep_lbl:'AP Sleep:',active_relays:'Active Relays:',cpu_temp:'CPU Temperature:',actions:'Actions',reboot:'🔄 Reboot ESP32',factory_reset:'🗑️ Factory Reset',changelog:'Changelog',relay:'Relay',osc_addr:'OSC Address',invert:'Invert logic',mode:'Mode',mode_latch:'Latch (direct value)',mode_toggle:'Toggle',config_saved:'Configuration saved',save_error:'Save error',net_error:'Network error',net_saved:'Network configuration saved',ap_saved:'AP configuration saved',confirm_reboot:'Reboot ESP32?',rebooting:'Rebooting...',confirm_factory:'Factory reset? This action is irreversible.',resetting:'Resetting...',log_cleared:'Log cleared',connecting:'Connecting...',connected:'✅ Connected',disconnected:'❌ Disconnected',ap_active:'✅ Active',ap_inactive:'⏸️ Inactive',disabled_always:'Disabled (always active)',conn_lost:'Connection lost...',ram_critical:'⚠ Critical RAM'},
      es:{tab_relays:'Relés & OSC',tab_network:'Red',tab_system:'Sistema',relay_control:'Control de Relés',osc_listen:'Escucha OSC en puerto UDP:',all_on:'✅ TODO ON',all_off:'⛔ TODO OFF',relay_osc_config:'Configuración OSC por Relé',save_btn:'💾 Guardar',eth_title:'Ethernet (W5500)',use_dhcp:'Usar DHCP',dhcp_info:'Si está desactivado, usar los parámetros estáticos.',hostname:'Hostname',ip_addr:'Dirección IP',subnet:'Máscara de subred',gateway:'Puerta de enlace',osc_port:'Puerto de escucha OSC (UDP)',ap_title:'Wi-Fi / Punto de Acceso (AP)',ap_info:'El AP WiFi se inicia automáticamente y se apaga tras el tiempo configurado (0 = siempre activo).',allow_ap:'Permitir modo AP',ssid_label:'SSID (nombre de red WiFi)',password:'Contraseña (≥8 caracteres)',ap_ip:'IP del Punto de Acceso',ap_mask:'Máscara AP',ap_gw:'Puerta de enlace AP',ap_timeout:'Tiempo de espera AP (minutos)',ap_timeout_info:'0 = siempre activo (sin suspensión)',qr_title:'📱 Código QR WiFi',qr_scan:'Escanee con su teléfono para conectarse',sys_title:'Información del Sistema',model:'Modelo:',uptime:'Uptime:',free_ram:'RAM libre:',min_ram:'RAM mín:',ethernet_lbl:'Ethernet:',eth_ip:'IP Ethernet:',osc_port_sys:'Puerto OSC:',wifi_ap_lbl:'WiFi AP:',ap_ip_sys:'IP AP:',ap_clients:'Clientes AP:',ap_sleep_lbl:'Suspensión AP:',active_relays:'Relés activos:',cpu_temp:'Temperatura CPU:',actions:'Acciones',reboot:'🔄 Reiniciar ESP32',factory_reset:'🗑️ Restablecer valores de fábrica',changelog:'Changelog',relay:'Relé',osc_addr:'Dirección OSC',invert:'Invertir lógica',mode:'Modo',mode_latch:'Latch (valor directo)',mode_toggle:'Toggle (alternancia)',config_saved:'Configuración guardada',save_error:'Error al guardar',net_error:'Error de red',net_saved:'Configuración de red guardada',ap_saved:'Configuración AP guardada',confirm_reboot:'¿Reiniciar ESP32?',rebooting:'Reiniciando...',confirm_factory:'¿Restablecer valores de fábrica? Esta acción es irreversible.',resetting:'Reinicializando...',log_cleared:'Log borrado',connecting:'Conectando...',connected:'✅ Conectado',disconnected:'❌ Desconectado',ap_active:'✅ Activo',ap_inactive:'⏸️ Inactivo',disabled_always:'Desactivado (siempre activo)',conn_lost:'Conexión perdida...',ram_critical:'⚠ RAM crítica'},
      de:{tab_relays:'Relais & OSC',tab_network:'Netzwerk',tab_system:'System',relay_control:'Relaissteuerung',osc_listen:'OSC-Empfang auf UDP-Port:',all_on:'✅ ALLE AN',all_off:'⛔ ALLE AUS',relay_osc_config:'OSC-Konfiguration pro Relais',save_btn:'💾 Speichern',eth_title:'Ethernet (W5500)',use_dhcp:'DHCP verwenden',dhcp_info:'Falls deaktiviert, statische Einstellungen unten verwenden.',hostname:'Hostname',ip_addr:'IP-Adresse',subnet:'Subnetzmaske',gateway:'Gateway',osc_port:'OSC-Empfangsport (UDP)',ap_title:'Wi-Fi / Zugangspunkt (AP)',ap_info:'Der WiFi-AP startet automatisch und schaltet sich nach dem konfigurierten Timeout ab (0 = immer aktiv).',allow_ap:'AP-Modus erlauben',ssid_label:'SSID (WiFi-Netzwerkname)',password:'Passwort (≥8 Zeichen)',ap_ip:'IP des Zugangspunkts',ap_mask:'AP-Maske',ap_gw:'AP-Gateway',ap_timeout:'AP-Ruhezustand (Minuten)',ap_timeout_info:'0 = immer aktiv (kein Ruhezustand)',qr_title:'📱 WiFi-QR-Code',qr_scan:'Scannen Sie mit Ihrem Telefon',sys_title:'Systeminformationen',model:'Modell:',uptime:'Uptime:',free_ram:'Freier RAM:',min_ram:'Min. RAM:',ethernet_lbl:'Ethernet:',eth_ip:'Ethernet-IP:',osc_port_sys:'OSC-Port:',wifi_ap_lbl:'WiFi AP:',ap_ip_sys:'AP-IP:',ap_clients:'AP-Clients:',ap_sleep_lbl:'AP-Ruhezustand:',active_relays:'Aktive Relais:',cpu_temp:'CPU-Temperatur:',actions:'Aktionen',reboot:'🔄 ESP32 neustarten',factory_reset:'🗑️ Werkseinstellungen',changelog:'Changelog',relay:'Relais',osc_addr:'OSC-Adresse',invert:'Logik invertieren',mode:'Modus',mode_latch:'Latch (Direktwert)',mode_toggle:'Toggle (Umschaltung)',config_saved:'Konfiguration gespeichert',save_error:'Speicherfehler',net_error:'Netzwerkfehler',net_saved:'Netzwerkkonfiguration gespeichert',ap_saved:'AP-Konfiguration gespeichert',confirm_reboot:'ESP32 neustarten?',rebooting:'Neustart läuft...',confirm_factory:'Werkseinstellungen? Dies ist nicht rückgängig zu machen.',resetting:'Zurücksetzen...',log_cleared:'Log gelöscht',connecting:'Verbindung...',connected:'✅ Verbunden',disconnected:'❌ Getrennt',ap_active:'✅ Aktiv',ap_inactive:'⏸️ Inaktiv',disabled_always:'Deaktiviert (immer aktiv)',conn_lost:'Verbindung verloren...',ram_critical:'⚠ Kritischer RAM'},
      zh:{tab_relays:'继电器 & OSC',tab_network:'网络',tab_system:'系统',relay_control:'继电器控制',osc_listen:'OSC 监听 UDP 端口:',all_on:'✅ 全部开启',all_off:'⛔ 全部关闭',relay_osc_config:'各继电器 OSC 配置',save_btn:'💾 保存',eth_title:'以太网 (W5500)',use_dhcp:'使用 DHCP',dhcp_info:'禁用时，使用以下静态设置。',hostname:'主机名',ip_addr:'IP 地址',subnet:'子网掩码',gateway:'网关',osc_port:'OSC 监听端口 (UDP)',ap_title:'Wi-Fi / 接入点 (AP)',ap_info:'WiFi AP 自动启动，并在配置的超时后关闭（0 = 始终活动）。',allow_ap:'允许 AP 模式',ssid_label:'SSID（WiFi 网络名称）',password:'密码（≥8 个字符）',ap_ip:'接入点 IP',ap_mask:'AP 掩码',ap_gw:'AP 网关',ap_timeout:'AP 休眠超时（分钟）',ap_timeout_info:'0 = 始终活动（不休眠）',qr_title:'📱 WiFi 二维码',qr_scan:'用手机扫描连接',sys_title:'系统信息',model:'型号:',uptime:'运行时间:',free_ram:'空闲 RAM:',min_ram:'最低 RAM:',ethernet_lbl:'以太网:',eth_ip:'以太网 IP:',osc_port_sys:'OSC 端口:',wifi_ap_lbl:'WiFi AP:',ap_ip_sys:'AP IP:',ap_clients:'AP 客户端:',ap_sleep_lbl:'AP 休眠:',active_relays:'活动继电器:',cpu_temp:'CPU 温度:',actions:'操作',reboot:'🔄 重启 ESP32',factory_reset:'🗑️ 恢复出厂设置',changelog:'更新日志',relay:'继电器',osc_addr:'OSC 地址',invert:'反转逻辑',mode:'模式',mode_latch:'锁存（直接值）',mode_toggle:'切换',config_saved:'配置已保存',save_error:'保存错误',net_error:'网络错误',net_saved:'网络配置已保存',ap_saved:'AP 配置已保存',confirm_reboot:'重启 ESP32？',rebooting:'正在重启...',confirm_factory:'恢复出厂设置？此操作不可逆。',resetting:'正在重置...',log_cleared:'日志已清除',connecting:'连接中...',connected:'✅ 已连接',disconnected:'❌ 已断开',ap_active:'✅ 活动',ap_inactive:'⏸️ 非活动',disabled_always:'已禁用（始终活动）',conn_lost:'连接丢失...',ram_critical:'⚠ RAM 严重不足'}
    };
    if (!I18N[LANG]) LANG = 'fr';
    function t(k) { return (I18N[LANG] && I18N[LANG][k]) || I18N.fr[k] || k; }
    function setLang(l) { LANG = l; localStorage.setItem('lang', l); applyLang(); loadConfig(); }
    function applyLang() {
      document.querySelectorAll('[data-i18n]').forEach(function(el) { el.textContent = t(el.dataset.i18n); });
      document.querySelectorAll('[data-i18n-html]').forEach(function(el) { el.innerHTML = t(el.dataset.i18nHtml); });
      document.querySelectorAll('.lang-btn').forEach(function(btn) {
        var l = btn.getAttribute('onclick').match(/'([a-z]+)'/);
        btn.classList.toggle('active', l && l[1] === LANG);
      });
    }

    // QR Code generator - qrcode-generator v1.4.4 by Kazuhiko Arase (MIT License)
    var qrcode=function(){var t=function(t,r){var e=t,n=g[r],o=null,i=0,a=null,u=[],f={},c=function(t,r){o=function(t){for(var r=new Array(t),e=0;e<t;e+=1){r[e]=new Array(t);for(var n=0;n<t;n+=1)r[e][n]=null}return r}(i=4*e+17),l(0,0),l(i-7,0),l(0,i-7),s(),h(),d(t,r),e>=7&&v(t),null==a&&(a=p(e,n,u)),w(a,r)},l=function(t,r){for(var e=-1;e<=7;e+=1)if(!(t+e<=-1||i<=t+e))for(var n=-1;n<=7;n+=1)r+n<=-1||i<=r+n||(o[t+e][r+n]=0<=e&&e<=6&&(0==n||6==n)||0<=n&&n<=6&&(0==e||6==e)||2<=e&&e<=4&&2<=n&&n<=4)},h=function(){for(var t=8;t<i-8;t+=1)null==o[t][6]&&(o[t][6]=t%2==0);for(var r=8;r<i-8;r+=1)null==o[6][r]&&(o[6][r]=r%2==0)},s=function(){for(var t=B.getPatternPosition(e),r=0;r<t.length;r+=1)for(var n=0;n<t.length;n+=1){var i=t[r],a=t[n];if(null==o[i][a])for(var u=-2;u<=2;u+=1)for(var f=-2;f<=2;f+=1)o[i+u][a+f]=-2==u||2==u||-2==f||2==f||0==u&&0==f}},v=function(t){for(var r=B.getBCHTypeNumber(e),n=0;n<18;n+=1){var a=!t&&1==(r>>n&1);o[Math.floor(n/3)][n%3+i-8-3]=a}for(n=0;n<18;n+=1){a=!t&&1==(r>>n&1);o[n%3+i-8-3][Math.floor(n/3)]=a}},d=function(t,r){for(var e=n<<3|r,a=B.getBCHTypeInfo(e),u=0;u<15;u+=1){var f=!t&&1==(a>>u&1);u<6?o[u][8]=f:u<8?o[u+1][8]=f:o[i-15+u][8]=f}for(u=0;u<15;u+=1){f=!t&&1==(a>>u&1);u<8?o[8][i-u-1]=f:u<9?o[8][15-u-1+1]=f:o[8][15-u-1]=f}o[i-8][8]=!t},w=function(t,r){for(var e=-1,n=i-1,a=7,u=0,f=B.getMaskFunction(r),c=i-1;c>0;c-=2)for(6==c&&(c-=1);;){for(var g=0;g<2;g+=1)if(null==o[n][c-g]){var l=!1;u<t.length&&(l=1==(t[u]>>>a&1)),f(n,c-g)&&(l=!l),o[n][c-g]=l,-1==(a-=1)&&(u+=1,a=7)}if((n+=e)<0||i<=n){n-=e,e=-e;break}}},p=function(t,r,e){for(var n=A.getRSBlocks(t,r),o=b(),i=0;i<e.length;i+=1){var a=e[i];o.put(a.getMode(),4),o.put(a.getLength(),B.getLengthInBits(a.getMode(),t)),a.write(o)}var u=0;for(i=0;i<n.length;i+=1)u+=n[i].dataCount;if(o.getLengthInBits()>8*u)throw"code length overflow. ("+o.getLengthInBits()+">"+8*u+")";for(o.getLengthInBits()+4<=8*u&&o.put(0,4);o.getLengthInBits()%8!=0;)o.putBit(!1);for(;!(o.getLengthInBits()>=8*u||(o.put(236,8),o.getLengthInBits()>=8*u));)o.put(17,8);return function(t,r){for(var e=0,n=0,o=0,i=new Array(r.length),a=new Array(r.length),u=0;u<r.length;u+=1){var f=r[u].dataCount,c=r[u].totalCount-f;n=Math.max(n,f),o=Math.max(o,c),i[u]=new Array(f);for(var g=0;g<i[u].length;g+=1)i[u][g]=255&t.getBuffer()[g+e];e+=f;var l=B.getErrorCorrectPolynomial(c),h=k(i[u],l.getLength()-1).mod(l);for(a[u]=new Array(l.getLength()-1),g=0;g<a[u].length;g+=1){var s=g+h.getLength()-a[u].length;a[u][g]=s>=0?h.getAt(s):0}}var v=0;for(g=0;g<r.length;g+=1)v+=r[g].totalCount;var d=new Array(v),w=0;for(g=0;g<n;g+=1)for(u=0;u<r.length;u+=1)g<i[u].length&&(d[w]=i[u][g],w+=1);for(g=0;g<o;g+=1)for(u=0;u<r.length;u+=1)g<a[u].length&&(d[w]=a[u][g],w+=1);return d}(o,n)};f.addData=function(t,r){var e=null;switch(r=r||"Byte"){case"Numeric":e=M(t);break;case"Alphanumeric":e=x(t);break;case"Byte":e=m(t);break;case"Kanji":e=L(t);break;default:throw"mode:"+r}u.push(e),a=null},f.isDark=function(t,r){if(t<0||i<=t||r<0||i<=r)throw t+","+r;return o[t][r]},f.getModuleCount=function(){return i},f.make=function(){if(e<1){for(var t=1;t<40;t++){for(var r=A.getRSBlocks(t,n),o=b(),i=0;i<u.length;i++){var a=u[i];o.put(a.getMode(),4),o.put(a.getLength(),B.getLengthInBits(a.getMode(),t)),a.write(o)}var g=0;for(i=0;i<r.length;i++)g+=r[i].dataCount;if(o.getLengthInBits()<=8*g)break}e=t}c(!1,function(){for(var t=0,r=0,e=0;e<8;e+=1){c(!0,e);var n=B.getLostPoint(f);(0==e||t>n)&&(t=n,r=e)}return r}())},f.createTableTag=function(t,r){t=t||2;var e="";e+='<table style="',e+=" border-width: 0px; border-style: none;",e+=" border-collapse: collapse;",e+=" padding: 0px; margin: "+(r=void 0===r?4*t:r)+"px;",e+='">',e+="<tbody>";for(var n=0;n<f.getModuleCount();n+=1){e+="<tr>";for(var o=0;o<f.getModuleCount();o+=1)e+='<td style="',e+=" border-width: 0px; border-style: none;",e+=" border-collapse: collapse;",e+=" padding: 0px; margin: 0px;",e+=" width: "+t+"px;",e+=" height: "+t+"px;",e+=" background-color: ",e+=f.isDark(n,o)?"#000000":"#ffffff",e+=";",e+='"/>';e+="</tr>"}return e+="</tbody>",e+="</table>"},f.renderTo2dContext=function(t,r){r=r||2;for(var e=f.getModuleCount(),n=0;n<e;n++)for(var o=0;o<e;o++)t.fillStyle=f.isDark(n,o)?"black":"white",t.fillRect(n*r,o*r,r,r)};return f};t.stringToBytes=(t.stringToBytesFuncs={default:function(t){for(var r=[],e=0;e<t.length;e+=1){var n=t.charCodeAt(e);r.push(255&n)}return r}}).default;var r,e,n,o,i,a=1,u=2,f=4,c=8,g={L:1,M:0,Q:3,H:2},l=0,h=1,s=2,v=3,d=4,w=5,p=6,y=7,B=(r=[[],[6,18],[6,22],[6,26],[6,30],[6,34],[6,22,38],[6,24,42],[6,26,46],[6,28,50],[6,30,54],[6,32,58],[6,34,62],[6,26,46,66],[6,26,48,70],[6,26,50,74],[6,30,54,78],[6,30,56,82],[6,30,58,86],[6,34,62,90],[6,28,50,72,94],[6,26,50,74,98],[6,30,54,78,102],[6,28,54,80,106],[6,32,58,84,110],[6,30,58,86,114],[6,34,62,90,118],[6,26,50,74,98,122],[6,30,54,78,102,126],[6,26,52,78,104,130],[6,30,56,82,108,134],[6,34,60,86,112,138],[6,30,58,86,114,142],[6,34,62,90,118,146],[6,30,54,78,102,126,150],[6,24,50,76,102,128,154],[6,28,54,80,106,132,158],[6,32,58,84,110,136,162],[6,26,54,82,110,138,166],[6,30,58,86,114,142,170]],e=1335,n=7973,i=function(t){for(var r=0;0!=t;)r+=1,t>>>=1;return r},(o={}).getBCHTypeInfo=function(t){for(var r=t<<10;i(r)-i(e)>=0;)r^=e<<i(r)-i(e);return 21522^(t<<10|r)},o.getBCHTypeNumber=function(t){for(var r=t<<12;i(r)-i(n)>=0;)r^=n<<i(r)-i(n);return t<<12|r},o.getPatternPosition=function(t){return r[t-1]},o.getMaskFunction=function(t){switch(t){case l:return function(t,r){return(t+r)%2==0};case h:return function(t,r){return t%2==0};case s:return function(t,r){return r%3==0};case v:return function(t,r){return(t+r)%3==0};case d:return function(t,r){return(Math.floor(t/2)+Math.floor(r/3))%2==0};case w:return function(t,r){return t*r%2+t*r%3==0};case p:return function(t,r){return(t*r%2+t*r%3)%2==0};case y:return function(t,r){return(t*r%3+(t+r)%2)%2==0};default:throw"bad maskPattern:"+t}},o.getErrorCorrectPolynomial=function(t){for(var r=k([1],0),e=0;e<t;e+=1)r=r.multiply(k([1,C.gexp(e)],0));return r},o.getLengthInBits=function(t,r){if(1<=r&&r<10)switch(t){case a:return 10;case u:return 9;case f:case c:return 8;default:throw"mode:"+t}else if(r<27)switch(t){case a:return 12;case u:return 11;case f:return 16;case c:return 10;default:throw"mode:"+t}else{if(!(r<41))throw"type:"+r;switch(t){case a:return 14;case u:return 13;case f:return 16;case c:return 12;default:throw"mode:"+t}}},o.getLostPoint=function(t){for(var r=t.getModuleCount(),e=0,n=0;n<r;n+=1)for(var o=0;o<r;o+=1){for(var i=0,a=t.isDark(n,o),u=-1;u<=1;u+=1)if(!(n+u<0||r<=n+u))for(var f=-1;f<=1;f+=1)o+f<0||r<=o+f||0==u&&0==f||a==t.isDark(n+u,o+f)&&(i+=1);i>5&&(e+=3+i-5)}for(n=0;n<r-1;n+=1)for(o=0;o<r-1;o+=1){var c=0;t.isDark(n,o)&&(c+=1),t.isDark(n+1,o)&&(c+=1),t.isDark(n,o+1)&&(c+=1),t.isDark(n+1,o+1)&&(c+=1),0!=c&&4!=c||(e+=3)}for(n=0;n<r;n+=1)for(o=0;o<r-6;o+=1)t.isDark(n,o)&&!t.isDark(n,o+1)&&t.isDark(n,o+2)&&t.isDark(n,o+3)&&t.isDark(n,o+4)&&!t.isDark(n,o+5)&&t.isDark(n,o+6)&&(e+=40);for(o=0;o<r;o+=1)for(n=0;n<r-6;n+=1)t.isDark(n,o)&&!t.isDark(n+1,o)&&t.isDark(n+2,o)&&t.isDark(n+3,o)&&t.isDark(n+4,o)&&!t.isDark(n+5,o)&&t.isDark(n+6,o)&&(e+=40);var g=0;for(o=0;o<r;o+=1)for(n=0;n<r;n+=1)t.isDark(n,o)&&(g+=1);return e+=Math.abs(100*g/r/r-50)/5*10},o),C=function(){for(var t=new Array(256),r=new Array(256),e=0;e<8;e+=1)t[e]=1<<e;for(e=8;e<256;e+=1)t[e]=t[e-4]^t[e-5]^t[e-6]^t[e-8];for(e=0;e<255;e+=1)r[t[e]]=e;var n={glog:function(t){if(t<1)throw"glog("+t+")";return r[t]},gexp:function(r){for(;r<0;)r+=255;for(;r>=256;)r-=255;return t[r]}};return n}();function k(t,r){if(void 0===t.length)throw t.length+"/"+r;var e=function(){for(var e=0;e<t.length&&0==t[e];)e+=1;for(var n=new Array(t.length-e+r),o=0;o<t.length-e;o+=1)n[o]=t[o+e];return n}(),n={getAt:function(t){return e[t]},getLength:function(){return e.length},multiply:function(t){for(var r=new Array(n.getLength()+t.getLength()-1),e=0;e<n.getLength();e+=1)for(var o=0;o<t.getLength();o+=1)r[e+o]^=C.gexp(C.glog(n.getAt(e))+C.glog(t.getAt(o)));return k(r,0)},mod:function(t){if(n.getLength()-t.getLength()<0)return n;for(var r=C.glog(n.getAt(0))-C.glog(t.getAt(0)),e=new Array(n.getLength()),o=0;o<n.getLength();o+=1)e[o]=n.getAt(o);for(o=0;o<t.getLength();o+=1)e[o]^=C.gexp(C.glog(t.getAt(o))+r);return k(e,0).mod(t)}};return n}var A=function(){var t=[[1,26,19],[1,26,16],[1,26,13],[1,26,9],[1,44,34],[1,44,28],[1,44,22],[1,44,16],[1,70,55],[1,70,44],[2,35,17],[2,35,13],[1,100,80],[2,50,32],[2,50,24],[4,25,9],[1,134,108],[2,67,43],[2,33,15,2,34,16],[2,33,11,2,34,12],[2,86,68],[4,43,27],[4,43,19],[4,43,15],[2,98,78],[4,49,31],[2,32,14,4,33,15],[4,39,13,1,40,14],[2,121,97],[2,60,38,2,61,39],[4,40,18,2,41,19],[4,40,14,2,41,15],[2,146,116],[3,58,36,2,59,37],[4,36,16,4,37,17],[4,36,12,4,37,13],[2,86,68,2,87,69],[4,69,43,1,70,44],[6,43,19,2,44,20],[6,43,15,2,44,16],[4,101,81],[1,80,50,4,81,51],[4,50,22,4,51,23],[3,36,12,8,37,13],[2,116,92,2,117,93],[6,58,36,2,59,37],[4,46,20,6,47,21],[7,42,14,4,43,15],[4,133,107],[8,59,37,1,60,38],[8,44,20,4,45,21],[12,33,11,4,34,12],[3,145,115,1,146,116],[4,64,40,5,65,41],[11,36,16,5,37,17],[11,36,12,5,37,13],[5,109,87,1,110,88],[5,65,41,5,66,42],[5,54,24,7,55,25],[11,36,12,7,37,13],[5,122,98,1,123,99],[7,73,45,3,74,46],[15,43,19,2,44,20],[3,45,15,13,46,16],[1,135,107,5,136,108],[10,74,46,1,75,47],[1,50,22,15,51,23],[2,42,14,17,43,15],[5,150,120,1,151,121],[9,69,43,4,70,44],[17,50,22,1,51,23],[2,42,14,19,43,15],[3,141,113,4,142,114],[3,70,44,11,71,45],[17,47,21,4,48,22],[9,39,13,16,40,14],[3,135,107,5,136,108],[3,67,41,13,68,42],[15,54,24,5,55,25],[15,43,15,10,44,16],[4,144,116,4,145,117],[17,68,42],[17,50,22,6,51,23],[19,46,16,6,47,17],[2,139,111,7,140,112],[17,74,46],[7,54,24,16,55,25],[34,37,13],[4,151,121,5,152,122],[4,75,47,14,76,48],[11,54,24,14,55,25],[16,45,15,14,46,16],[6,147,117,4,148,118],[6,73,45,14,74,46],[11,54,24,16,55,25],[30,46,16,2,47,17],[8,132,106,4,133,107],[8,75,47,13,76,48],[7,54,24,22,55,25],[22,45,15,13,46,16],[10,142,114,2,143,115],[19,74,46,4,75,47],[28,50,22,6,51,23],[33,46,16,4,47,17],[8,152,122,4,153,123],[22,73,45,3,74,46],[8,53,23,26,54,24],[12,45,15,28,46,16],[3,147,117,10,148,118],[3,73,45,23,74,46],[4,54,24,31,55,25],[11,45,15,31,46,16],[7,146,116,7,147,117],[21,73,45,7,74,46],[1,53,23,37,54,24],[19,45,15,26,46,16],[5,145,115,10,146,116],[19,75,47,10,76,48],[15,54,24,25,55,25],[23,45,15,25,46,16],[13,145,115,3,146,116],[2,74,46,29,75,47],[42,54,24,1,55,25],[23,45,15,28,46,16],[17,145,115],[10,74,46,23,75,47],[10,54,24,35,55,25],[19,45,15,35,46,16],[17,145,115,1,146,116],[14,74,46,21,75,47],[29,54,24,19,55,25],[11,45,15,46,46,16],[13,145,115,6,146,116],[14,74,46,23,75,47],[44,54,24,7,55,25],[59,46,16,1,47,17],[12,151,121,7,152,122],[12,75,47,26,76,48],[39,54,24,14,55,25],[22,45,15,41,46,16],[6,151,121,14,152,122],[6,75,47,34,76,48],[46,54,24,10,55,25],[2,45,15,64,46,16],[17,152,122,4,153,123],[29,74,46,14,75,47],[49,54,24,10,55,25],[24,45,15,46,46,16],[4,152,122,18,153,123],[13,74,46,32,75,47],[48,54,24,14,55,25],[42,45,15,32,46,16],[20,147,117,4,148,118],[40,75,47,7,76,48],[43,54,24,22,55,25],[10,45,15,67,46,16],[19,148,118,6,149,119],[18,75,47,31,76,48],[34,54,24,34,55,25],[20,45,15,61,46,16]],r=function(t,r){var e={};return e.totalCount=t,e.dataCount=r,e},e={};return e.getRSBlocks=function(e,n){var o=function(r,e){switch(e){case g.L:return t[4*(r-1)+0];case g.M:return t[4*(r-1)+1];case g.Q:return t[4*(r-1)+2];case g.H:return t[4*(r-1)+3];default:return}}(e,n);if(void 0===o)throw"bad rs block @ typeNumber:"+e+"/errorCorrectionLevel:"+n;for(var i=o.length/3,a=[],u=0;u<i;u+=1)for(var f=o[3*u+0],c=o[3*u+1],l=o[3*u+2],h=0;h<f;h+=1)a.push(r(c,l));return a},e}(),b=function(){var t=[],r=0,e={getBuffer:function(){return t},getAt:function(r){var e=Math.floor(r/8);return 1==(t[e]>>>7-r%8&1)},put:function(t,r){for(var n=0;n<r;n+=1)e.putBit(1==(t>>>r-n-1&1))},getLengthInBits:function(){return r},putBit:function(e){var n=Math.floor(r/8);t.length<=n&&t.push(0),e&&(t[n]|=128>>>r%8),r+=1}};return e},M=function(t){var r=a,e=t,n={getMode:function(){return r},getLength:function(t){return e.length},write:function(t){for(var r=e,n=0;n+2<r.length;)t.put(o(r.substring(n,n+3)),10),n+=3;n<r.length&&(r.length-n==1?t.put(o(r.substring(n,n+1)),4):r.length-n==2&&t.put(o(r.substring(n,n+2)),7))}},o=function(t){for(var r=0,e=0;e<t.length;e+=1)r=10*r+i(t.charAt(e));return r},i=function(t){if("0"<=t&&t<="9")return t.charCodeAt(0)-"0".charCodeAt(0);throw"illegal char :"+t};return n},x=function(t){var r=u,e=t,n={getMode:function(){return r},getLength:function(t){return e.length},write:function(t){for(var r=e,n=0;n+1<r.length;)t.put(45*o(r.charAt(n))+o(r.charAt(n+1)),11),n+=2;n<r.length&&t.put(o(r.charAt(n)),6)}},o=function(t){if("0"<=t&&t<="9")return t.charCodeAt(0)-"0".charCodeAt(0);if("A"<=t&&t<="Z")return t.charCodeAt(0)-"A".charCodeAt(0)+10;switch(t){case" ":return 36;case"$":return 37;case"%":return 38;case"*":return 39;case"+":return 40;case"-":return 41;case".":return 42;case"/":return 43;case":":return 44;default:throw"illegal char :"+t}};return n},m=function(r){var e=f,n=t.stringToBytes(r),o={getMode:function(){return e},getLength:function(t){return n.length},write:function(t){for(var r=0;r<n.length;r+=1)t.put(n[r],8)}};return o};function k(t,r){if(void 0===t.length)throw t.length+"/"+r;var e=function(){for(var e=0;e<t.length&&0==t[e];)e+=1;for(var n=new Array(t.length-e+r),o=0;o<t.length-e;o+=1)n[o]=t[o+e];return n}(),n={getAt:function(t){return e[t]},getLength:function(){return e.length},multiply:function(t){for(var r=new Array(n.getLength()+t.getLength()-1),e=0;e<n.getLength();e+=1)for(var o=0;o<t.getLength();o+=1)r[e+o]^=C.gexp(C.glog(n.getAt(e))+C.glog(t.getAt(o)));return k(r,0)},mod:function(t){if(n.getLength()-t.getLength()<0)return n;for(var r=C.glog(n.getAt(0))-C.glog(t.getAt(0)),e=new Array(n.getLength()),o=0;o<n.getLength();o+=1)e[o]=n.getAt(o);for(o=0;o<t.getLength();o+=1)e[o]^=C.gexp(C.glog(t.getAt(o))+r);return k(e,0).mod(t)}};return n}var C=function(){for(var t=new Array(256),r=new Array(256),e=0;e<8;e+=1)t[e]=1<<e;for(e=8;e<256;e+=1)t[e]=t[e-4]^t[e-5]^t[e-6]^t[e-8];for(e=0;e<255;e+=1)r[t[e]]=e;var n={glog:function(t){if(t<1)throw"glog("+t+")";return r[t]},gexp:function(r){for(;r<0;)r+=255;for(;r>=256;)r-=255;return t[r]}};return n}();return t}();

    function updateWifiQR() {
      var ssid = document.getElementById('apSsid').value;
      var pass = document.getElementById('apPass').value;
      var cvs = document.getElementById('qrCanvas');
      if (!ssid) { cvs.width = cvs.height = 0; return; }
      var esc = function(s) { return s.replace(/[\\;,":]/g, '\\$&'); };
      var text = 'WIFI:T:WPA;S:' + esc(ssid) + ';P:' + esc(pass) + ';;';
      var qr = qrcode(0, 'L');
      qr.addData(text);
      qr.make();
      var sz = qr.getModuleCount();
      var cs = 6, q = 4, total = sz + q * 2;
      cvs.width = cvs.height = total * cs;
      var ctx = cvs.getContext('2d');
      ctx.fillStyle = '#fff'; ctx.fillRect(0, 0, cvs.width, cvs.height);
      ctx.fillStyle = '#000';
      for (var r = 0; r < sz; r++)
        for (var c = 0; c < sz; c++)
          if (qr.isDark(r, c)) ctx.fillRect((c + q) * cs, (r + q) * cs, cs, cs);
    }
    
    // Tab switching
    document.querySelectorAll('.tab-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const tab = btn.dataset.tab;
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        btn.classList.add('active');
        document.getElementById(`tab-${tab}`).classList.add('active');
        if (tab === 'system') loadChangelog();
      });
    });

    // Changelog: GitHub Releases (via browser) avec fallback /api/changelog
    var changelogLoaded = false;
    function renderGithubReleases(releases) {
      var html = '';
      releases.forEach(function(rel) {
        var body = rel.body ? rel.body
          .replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')
          .replace(/### ([^\n]+)/g,'<strong>$1</strong>')
          .replace(/## ([^\n]+)/g,'<strong style="color:#c9d1d9">$1</strong>')
          .replace(/^[-*] (.+)$/gm,'<span style="color:#8b949e">▸</span> $1')
          .replace(/\n/g,'<br>') : '<em style="color:#8b949e">No description.</em>';
        html += '<div style="border-bottom:1px solid #21262d;padding:10px 0;">';
        html += '<div style="display:flex;align-items:center;gap:8px;margin-bottom:6px;">';
        html += '<a href="'+rel.html_url+'" target="_blank" rel="noopener noreferrer" style="color:#58a6ff;font-size:13px;font-weight:bold;">'+rel.tag_name+'</a>';
        if(rel.name && rel.name !== rel.tag_name) html += '<span style="color:#c9d1d9;font-size:13px;">'+rel.name.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')+'</span>';
        var d = new Date(rel.published_at);
        html += '<span style="color:#8b949e;font-size:11px;margin-left:auto;">'+d.toLocaleDateString()+'</span>';
        html += '</div>';
        html += '<div style="font-size:12px;line-height:1.6;">'+body+'</div>';
        html += '</div>';
      });
      document.getElementById('changelogContent').innerHTML = html || '<em style="color:#8b949e">Aucune release trouvée.</em>';
      document.getElementById('changelogSrc').textContent = '✅ Synchronisé avec GitHub Releases';
    }
    async function loadChangelog() {
      if (changelogLoaded) return;
      var el = document.getElementById('changelogContent');
      el.innerHTML = '<em style="color:#8b949e">Chargement...</em>';
      // Tentative GitHub Releases API (navigateur a internet)
      try {
        const r = await fetch('https://api.github.com/repos/NeOdYmS/ESP32-S3-OSC-8Relay/releases',
          {headers:{Accept:'application/vnd.github+json'},signal:AbortSignal.timeout(5000)});
        if (!r.ok) throw new Error('GitHub '+r.status);
        const releases = await r.json();
        renderGithubReleases(releases);
        changelogLoaded = true;
        return;
      } catch(_) {}
      // Fallback: CHANGELOG.md embarqué dans le firmware
      try {
        const r2 = await fetch(`${API_BASE}/changelog`);
        if (!r2.ok) throw new Error(r2.status);
        el.innerHTML = '<pre style="background:#0d1117;padding:12px;border-radius:6px;font-size:12px;margin:0;">' +
          (await r2.text()).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;') + '</pre>';
        document.getElementById('changelogSrc').textContent = '📦 Source: firmware embarqué (hors ligne)';
        changelogLoaded = true;
      } catch(e) {
        el.innerHTML = '<em style="color:#f85149">Erreur: '+e+'</em>';
      }
    }

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
        document.getElementById('apTimeout').value = cfg.apTimeoutMin;
        updateWifiQR();

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
              <span class="relay-name">${t('relay')} ${i + 1}</span>
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
              <span class="relay-name">${t('relay')} ${i + 1}</span>
            </div>
            <div class="form-group" style="margin: 0;">
              <label>${t('osc_addr')}</label>
              <input class="relay-osc-addr" data-index="${i}" value="${r.oscAddress}" placeholder="/relay/${i + 1}">
            </div>
            <div class="form-group" style="margin: 8px 0 0 0;">
              <label class="checkbox-label">
                <input type="checkbox" class="relay-invert" data-index="${i}" ${r.invert ? 'checked' : ''}>
                <span>${t('invert')}</span>
              </label>
            </div>
            <div class="form-group" style="margin: 8px 0 0 0;">
              <label>${t('mode')}</label>
              <select class="relay-mode" data-index="${i}">
                <option value="0" ${r.mode === 0 ? 'selected' : ''}>${t('mode_latch')}</option>
                <option value="1" ${r.mode === 1 ? 'selected' : ''}>${t('mode_toggle')}</option>
              </select>
            </div>
          `;
          relayCfgGrid.appendChild(cfgDiv);
        }

        updateRelayStatus();
      } catch (e) {
        console.error('loadConfig error:', e);
        showMessage('saveRelaysMsg', t('net_error'), true);
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
          showMessage('saveRelaysMsg', '✓ ' + t('config_saved'));
        } else {
          showMessage('saveRelaysMsg', '✗ ' + t('save_error'), true);
        }
      } catch (e) {
        showMessage('saveRelaysMsg', t('net_error'), true);
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
          showMessage('saveNetMsg', '✓ ' + t('net_saved'));
        } else {
          showMessage('saveNetMsg', '✗ ' + t('save_error'), true);
        }
      } catch (e) {
        showMessage('saveNetMsg', t('net_error'), true);
      }
    });

    document.getElementById('saveAp').addEventListener('click', async () => {
      const cfg = {
        wifiApAllowed: document.getElementById('apAllow').checked,
        apSsid: document.getElementById('apSsid').value,
        apPass: document.getElementById('apPass').value,
        apIp: document.getElementById('apIp').value,
        apMask: document.getElementById('apMask').value,
        apGw: document.getElementById('apGw').value,
        apTimeoutMin: parseInt(document.getElementById('apTimeout').value) || 0
      };
      try {
        const resp = await fetch(`${API_BASE}/config/ap`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(cfg)
        });
        if (resp.ok) {
          const txt = await resp.text();
          if (txt === 'OK_RELOADED') {
            showMessage('saveApMsg', '✓ ' + t('ap_saved') + ' — AP redémarré');
          } else {
            showMessage('saveApMsg', '✓ ' + t('ap_saved'));
          }
        } else {
          showMessage('saveApMsg', '✗ ' + t('save_error'), true);
        }
      } catch (e) {
        showMessage('saveApMsg', t('net_error'), true);
      }
    });

    document.getElementById('rebootBtn').addEventListener('click', () => {
      if (confirm(t('confirm_reboot'))) {
        fetch(`${API_BASE}/system/reboot`, { method: 'POST' });
        showMessage('saveApMsg', t('rebooting'));
      }
    });

    document.getElementById('factoryBtn').addEventListener('click', () => {
      if (confirm(t('confirm_factory'))) {
        fetch(`${API_BASE}/system/factoryreset`, { method: 'POST' });
        showMessage('saveApMsg', t('resetting'));
      }
    });

    const MAX_LOG_LINES = 200;
    var logLines = [];
    var autoScroll = true;

    function appendLog(html) {
      var el = document.getElementById('unifiedLog');
      var savedTop = el.scrollTop;
      var wasAtBottom = (el.scrollTop + el.clientHeight >= el.scrollHeight - 4);
      var removed = 0;
      logLines.push(html);
      if (logLines.length > MAX_LOG_LINES) {
        removed = logLines.length - MAX_LOG_LINES;
        logLines = logLines.slice(-MAX_LOG_LINES);
      }
      el.innerHTML = logLines.join('<br>');
      if (autoScroll) {
        el.scrollTop = el.scrollHeight;
      } else {
        // Compenser les lignes supprimées (~17px par ligne)
        el.scrollTop = Math.max(0, savedTop - removed * 17);
      }
    }

    function toggleAutoScroll() {
      autoScroll = !autoScroll;
      var btn = document.getElementById('logAutoScrollBtn');
      btn.style.background = autoScroll ? '#238636' : '#21262d';
      btn.textContent = autoScroll ? '⬇ Auto' : '⏸ Auto';
      if (autoScroll) document.getElementById('unifiedLog').scrollTop = document.getElementById('unifiedLog').scrollHeight;
    }

    function clearLog() {
      logLines = [];
      document.getElementById('unifiedLog').innerHTML = '<span style="color:#8b949e;">' + t('log_cleared') + '</span>';
    }

    // Drag resize for log panel
    (function() {
      var bar = document.getElementById('logDragBar');
      var log = document.getElementById('unifiedLog');
      var startY, startH;
      function onMove(e) {
        var y = e.touches ? e.touches[0].clientY : e.clientY;
        var h = Math.min(400, Math.max(40, startH + (y - startY)));
        log.style.height = h + 'px';
      }
      function onUp() {
        document.removeEventListener('mousemove', onMove);
        document.removeEventListener('mouseup', onUp);
        document.removeEventListener('touchmove', onMove);
        document.removeEventListener('touchend', onUp);
      }
      function onDown(e) {
        startY = e.touches ? e.touches[0].clientY : e.clientY;
        startH = log.offsetHeight;
        document.addEventListener('mousemove', onMove);
        document.addEventListener('mouseup', onUp);
        document.addEventListener('touchmove', onMove, {passive: false});
        document.addEventListener('touchend', onUp);
        e.preventDefault();
      }
      bar.addEventListener('mousedown', onDown);
      bar.addEventListener('touchstart', onDown, {passive: false});
    })();

    function esc(s) { var d = document.createElement('span'); d.textContent = s; return d.innerHTML; }

    // ===== LIVE UPDATE (combined endpoint /api/live) =====
    var lastOscTs = 0;
    var liveCount = 0;
    var liveBusy = false;

    async function updateLive() {
      if (liveBusy) return;
      liveBusy = true;
      try {
        const resp = await fetch(`${API_BASE}/live?since=${lastOscTs}`);
        const d = await resp.json();

        // 🎛️ Relay buttons (every tick)
        for (let i = 0; i < 8; i++) {
          const btn = document.getElementById(`relay-btn-${i}`);
          const st = document.getElementById(`relay-status-${i}`);
          if (!btn) continue;
          if (d.r[i]) {
            btn.classList.remove('off'); btn.classList.add('on');
            btn.textContent = 'ON'; st.textContent = 'ON';
          } else {
            btn.classList.remove('on'); btn.classList.add('off');
            btn.textContent = 'OFF'; st.textContent = 'OFF';
          }
        }

        // 📨 OSC messages (immediate)
        if (d.osc && d.osc.length > 0) {
          d.osc.forEach(m => {
            if (m.ts > lastOscTs) lastOscTs = m.ts;
            var sec = Math.floor(m.ts / 1000);
            var h = Math.floor(sec / 3600) % 24;
            var mn = Math.floor(sec / 60) % 60;
            var s = sec % 60;
            var t = String(h).padStart(2,'0') + ':' + String(mn).padStart(2,'0') + ':' + String(s).padStart(2,'0');
            var txt = '[' + t + '] 📨 ' + m.a + ' (' + m.t + ') = ' + m.v;
            appendLog('<span style="color:#58a6ff">' + esc(txt) + '</span>');
          });
        }

        // 📊 System status (every ~2s = 40 ticks at ~50ms)
        liveCount++;
        if (liveCount % 40 === 1) {
          const sec = d.up;
          const days = Math.floor(sec/86400);
          const h = Math.floor((sec%86400)/3600);
          const mn = Math.floor((sec%3600)/60);
          const s = sec%60;
          const upStr = days + 'd ' + String(h).padStart(2,'0') + ':' + String(mn).padStart(2,'0') + ':' + String(s).padStart(2,'0');
          const heap = (d.heap/1024).toFixed(0);
          const heapMin = (d.hmin/1024).toFixed(0);
          const relayStr = d.r.map((v,i) => `R${i+1}:${v?'ON':'OFF'}`).join(' ');
          const now = new Date().toLocaleTimeString(LOCALES[LANG] || 'fr-FR');
          const color = d.eth ? '#3fb950' : '#f85149';
          const txt = `[${now}] uptime=${upStr} ETH=${d.eth?'✓':'✗'} IP=${d.eip} | AP=${d.ap?'UP':'DOWN'} Clients=${d.acl} | OSC:${d.oP} | T=${d.t.toFixed(1)}°C | ${relayStr}`;
          appendLog('<span style="color:' + color + '">' + esc(txt) + '</span>');
          // RAM critique : alerte si < 50 KB
          if (d.heap < 51200) {
            appendLog('<span style="color:#f85149">' + t('ram_critical') + ' : ' + heap + ' KB !</span>');
          }

          document.getElementById('sysUptime').textContent = upStr;
          document.getElementById('sysRam').textContent = heap + ' KB / ' + (d.heap > 102400 ? '✅' : d.heap > 51200 ? '⚠️' : '🔴');
          document.getElementById('sysRamMin').textContent = heapMin + ' KB';
          document.getElementById('sysEthState').textContent = d.eth ? t('connected') : t('disconnected');
          document.getElementById('sysEthIp').textContent = d.eip;
          document.getElementById('sysOscPort').textContent = d.oP;
          document.getElementById('sysWifiState').textContent = d.ap ? t('ap_active') : t('ap_inactive');
          document.getElementById('sysWifiSsid').textContent = d.aSsid;
          document.getElementById('sysWifiIp').textContent = d.ap ? d.aip : '—';
          document.getElementById('sysApClients').textContent = d.acl;
          document.getElementById('sysApTimeout').textContent = d.aTo === 0 ? t('disabled_always') : d.aTo + ' min';
          const activeCount = d.r.filter(v => v).length;
          document.getElementById('sysRelays').textContent = activeCount + ' / 8';
          document.getElementById('sysCpuTemp').textContent = d.t.toFixed(1) + ' °C';
        }
      } catch (e) {
        const now = new Date().toLocaleTimeString(LOCALES[LANG] || 'fr-FR');
        appendLog('<span style="color:#f85149">' + esc('[' + now + '] ⚠ ' + t('conn_lost')) + '</span>');
      }
      liveBusy = false;
    }

    // Chained polling: relance dès que la précédente est terminée (min 30ms de pause)
    function liveLoop() {
      updateLive().then(() => setTimeout(liveLoop, 30));
    }

    // Initial load
    loadConfig();
    updateWifiQR();
    document.getElementById('apSsid').addEventListener('input', updateWifiQR);
    document.getElementById('apPass').addEventListener('input', updateWifiQR);
    applyLang();
    liveLoop();
  </script>
</body>
</html>)HTML";
