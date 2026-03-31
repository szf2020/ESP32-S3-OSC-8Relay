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
    <div style="display:flex; align-items:center; gap:16px; flex-wrap:wrap;">
      <h1 style="margin:0;">⚡ RelayOSC</h1>
      <div id="oscTicker" style="flex:1; min-width:200px; font-family:monospace; font-size:11px; color:#58a6ff; background:#0d1117; border:1px solid #30363d; border-radius:6px; padding:6px 10px; max-height:60px; overflow-y:auto; white-space:pre; line-height:1.3;">En attente OSC...</div>
    </div>
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
          <div class="form-group">
            <label>Mise en veille AP (minutes)</label>
            <input id="apTimeout" type="number" min="0" placeholder="5">
            <small style="color:#888;">0 = toujours actif (pas de mise en veille)</small>
          </div>
        </div>
        <div class="button-group">
          <button id="saveAp">💾 Sauvegarder</button>
          <span id="saveApMsg"></span>
        </div>
        <div style="text-align:center; margin-top:20px; padding:16px; background:#161b22; border-radius:8px; border:1px solid #30363d;">
          <div style="font-size:14px; font-weight:600; margin-bottom:12px; color:#e6edf3;">📱 QR Code WiFi</div>
          <canvas id="qrCanvas" style="image-rendering:pixelated; image-rendering:-webkit-optimize-contrast;"></canvas>
          <div style="font-size:11px; color:#8b949e; margin-top:8px;">Scannez avec votre iPhone ou téléphone pour vous connecter</div>
        </div>
      </div>
    </div>

    <!-- ONGLET: SYSTÈME -->
    <div id="tab-system" class="tab-content">
      <div class="card">
        <h2>ℹ️ Informations Système</h2>
        <div class="grid-2">
          <div><strong>Modèle:</strong> ESP32-S3-ETH-8DI-8RO</div>
          <div><strong>Uptime:</strong> <span id="sysUptime">—</span></div>
          <div><strong>RAM libre:</strong> <span id="sysRam">—</span></div>
          <div><strong>RAM min:</strong> <span id="sysRamMin">—</span></div>
          <div><strong>Ethernet:</strong> <span id="sysEthState">—</span></div>
          <div><strong>IP Ethernet:</strong> <span id="sysEthIp">—</span></div>
          <div><strong>Port OSC:</strong> <span id="sysOscPort">—</span></div>
          <div><strong>WiFi AP:</strong> <span id="sysWifiState">—</span></div>
          <div><strong>SSID:</strong> <span id="sysWifiSsid">—</span></div>
          <div><strong>IP AP:</strong> <span id="sysWifiIp">—</span></div>
          <div><strong>Clients AP:</strong> <span id="sysApClients">—</span></div>
          <div><strong>Mise en veille AP:</strong> <span id="sysApTimeout">—</span></div>
          <div><strong>Relais actifs:</strong> <span id="sysRelays">—</span></div>
          <div><strong>Température CPU:</strong> <span id="sysCpuTemp">—</span></div>
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
        const relayStr = s.relays.map((v,i) => `R${i+1}:${v?'ON':'OFF'}`).join(' ');
        const heap = (s.freeHeap/1024).toFixed(0);
        const heapMin = (s.minFreeHeap/1024).toFixed(0);
        const now = new Date().toLocaleTimeString('fr-FR');
        const line = `[${now}] uptime=${s.uptime} ETH=${s.ethConnected?'✓':'✗'} IP=${s.ethIp} | AP=${s.wifiApActive?'UP':'DOWN'} Clients=${s.apClients} | OSC:${s.oscPort} | RAM=${heap}K (min:${heapMin}K) | T=${s.cpuTemp.toFixed(1)}°C | ${relayStr}`;
        // Update system info tab fields
        document.getElementById('sysUptime').textContent = s.uptime;
        document.getElementById('sysRam').textContent = heap + ' KB';
        document.getElementById('sysRamMin').textContent = heapMin + ' KB';
        document.getElementById('sysEthState').textContent = s.ethConnected ? '✅ Connecté' : '❌ Déconnecté';
        document.getElementById('sysEthIp').textContent = s.ethIp;
        document.getElementById('sysOscPort').textContent = s.oscPort;
        document.getElementById('sysWifiState').textContent = s.wifiApActive ? '✅ Actif' : '⏸️ Inactif';
        document.getElementById('sysWifiSsid').textContent = s.apSsid;
        document.getElementById('sysWifiIp').textContent = s.wifiApActive ? s.wifiApIp : '—';
        document.getElementById('sysApClients').textContent = s.apClients;
        document.getElementById('sysApTimeout').textContent = s.apTimeoutMin === 0 ? 'Désactivée (toujours actif)' : s.apTimeoutMin + ' min';
        const activeCount = s.relays.filter(v => v).length;
        document.getElementById('sysRelays').textContent = activeCount + ' / 8';
        document.getElementById('sysCpuTemp').textContent = s.cpuTemp.toFixed(1) + ' °C';
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

    // OSC message ticker in header
    var lastOscTs = 0;
    async function updateOscTicker() {
      try {
        const resp = await fetch(`${API_BASE}/osc/log`);
        const data = await resp.json();
        const el = document.getElementById('oscTicker');
        const msgs = data.messages;
        if (!msgs || msgs.length === 0) return;
        // Only show new messages (ts > lastOscTs)
        var newMsgs = msgs.filter(m => m.ts > lastOscTs);
        if (newMsgs.length === 0) return;
        lastOscTs = msgs[msgs.length - 1].ts;
        var lines = el.textContent === 'En attente OSC...' ? [] : el.textContent.split('\n');
        newMsgs.forEach(m => {
          var sec = Math.floor(m.ts / 1000);
          var h = Math.floor(sec / 3600) % 24;
          var mn = Math.floor(sec / 60) % 60;
          var s = sec % 60;
          var t = String(h).padStart(2,'0') + ':' + String(mn).padStart(2,'0') + ':' + String(s).padStart(2,'0');
          lines.push('[' + t + '] ' + m.addr + ' (' + m.type + ') = ' + m.val);
        });
        if (lines.length > 30) lines = lines.slice(-30);
        el.textContent = lines.join('\n');
        el.style.color = '#58a6ff';
        el.scrollTop = el.scrollHeight;
      } catch(e) {}
    }

    // Initial load
    loadConfig();
    updateStatusLog();
    updateOscTicker();
    updateWifiQR();
    document.getElementById('apSsid').addEventListener('input', updateWifiQR);
    document.getElementById('apPass').addEventListener('input', updateWifiQR);
    setInterval(updateRelayStatus, 2000);
    setInterval(updateStatusLog, 3000);
    setInterval(updateOscTicker, 1000);
  </script>
</body>
</html>)HTML";
