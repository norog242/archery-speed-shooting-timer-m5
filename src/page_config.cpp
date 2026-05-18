#include "web_server.h"
#include <Arduino.h>

String getConfigHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>WiFi Configuration</title>
    <link rel='stylesheet' href='/common.css'>
    <script src='/translations.js'></script>
    <script>
      window.currentLang = 'EN';
      
      function loadStatus() {
        fetch('/wifi-status').then(r => r.json()).then(d => {
          const t = translations[window.currentLang];
          document.getElementById('current-mode').textContent = d.mode === 'station' ? t.connectedToWifi : t.accessPointMode;
          document.getElementById('current-ssid').textContent = d.ssid;
          document.getElementById('current-ip').textContent = d.ip;
          
          // Show hostname if in station mode
          const hostnameRow = document.getElementById('hostname-row');
          if (d.mode === 'station' && d.hostname) {
            hostnameRow.style.display = 'block';
            document.getElementById('current-hostname').textContent = d.hostname;
          } else {
            hostnameRow.style.display = 'none';
          }
        }).catch(e => {
          console.error('Error loading status:', e);
        });
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const t = translations[window.currentLang];
        const ssid = document.getElementById('ssid').value;
        const password = document.getElementById('password').value;
        
        if (!ssid) {
          showMessage(t.pleaseEnterSSID, 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('ssid', ssid);
        formData.append('password', password);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = t.saving;
        
        fetch('/wifi-config', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage(t.configSaved, 'success');
        })
        .catch(error => {
          showMessage(t.errorSavingConfig + ' ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = t.saveRestart;
        });
      }
      
      function clearConfig() {
        const t = translations[window.currentLang];
        if (!confirm(t.clearWifiConfirm)) {
          return;
        }
        
        fetch('/wifi-clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          showMessage(t.wifiConfigCleared, 'success');
        })
        .catch(error => {
          showMessage(t.errorClearingConfig + ' ' + error, 'error');
        });
      }
      
      function showMessage(text, type) {
        const msgDiv = document.getElementById('message');
        msgDiv.textContent = text;
        msgDiv.className = 'message ' + type;
        msgDiv.style.display = 'block';
      }
      
      window.onload = function() {
        // Load device settings including language
        fetch('/device-settings-status').then(r => r.json()).then(d => {
          window.currentLang = d.language || 'EN';
          const t = translations[window.currentLang];
          document.title = t.wifiConfigTitle;
          applyTranslations();
          loadStatus();
        }).catch(e => {
          console.error('Error loading device settings:', e);
          loadStatus();
        });
      };
    </script>
    </head><body>
    <h1 data-i18n='wifiConfigTitle'>WiFi Configuration</h1>
    
    <div class='status-box'>
      <strong data-i18n='currentStatus'>Current Status:</strong>
      <p><strong data-i18n='mode'>Mode:</strong> <span id='current-mode'>Loading...</span></p>
      <p><strong data-i18n='network'>Network:</strong> <span id='current-ssid'>-</span></p>
      <p><strong data-i18n='ipAddress'>IP Address:</strong> <span id='current-ip'>-</span></p>
      <p id='hostname-row' style='display:none;'><strong data-i18n='hostname'>Hostname:</strong> <span id='current-hostname'>-</span></p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2 data-i18n='connectToWifi'>Connect to WiFi Network</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='ssid' data-i18n='wifiNetworkName'>WiFi Network Name (SSID):</label>
        <input type='text' id='ssid' name='ssid' data-i18n-placeholder='enterWifiSSID' placeholder='Enter WiFi SSID' required>
        <div class='info' data-i18n='enterNetworkName'>Enter the name of your WiFi network</div>
      </div>
      
      <div class='form-group'>
        <label for='password' data-i18n='wifiPassword'>WiFi Password:</label>
        <input type='password' id='password' name='password' data-i18n-placeholder='enterWifiPassword' placeholder='Enter WiFi password'>
        <div class='info' data-i18n='leaveEmptyOpen'>Leave empty for open networks</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn' data-i18n='saveRestart'>Save & Restart</button>
      <button type='button' class='btn-danger' onclick='clearConfig()' data-i18n='clearConfiguration'>Clear Configuration</button>
    </form>
    
    <div class='divider'>
      <a href='/' class='nav-btn' data-i18n='backToTimer'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:var(--info-bg);'>
      <strong data-i18n='noteTitle'>Note:</strong> <span data-i18n='wifiNoteText'>After saving, the device will restart and attempt to connect to the specified WiFi network. If connection fails, it will automatically fall back to Access Point mode (ArrowTimerAP). The configuration is stored persistently and will be used after power loss.</span>
      <br><br>
      <strong data-i18n='easyAccessTitle'>Easy Access:</strong> <span data-i18n='easyAccessText'>When connected to WiFi, access the device at</span> <strong>http://arrowtimer.local</strong> 
      <span data-i18n='mdnsNote'>(works on most devices with mDNS/Bonjour support). The IP address and hostname are also displayed on the device screen.</span>
      <br><br>
      <strong data-i18n='hardwareResetTitle'>Hardware Reset:</strong> <span data-i18n='hardwareResetText'>To reset WiFi credentials without web access, press and hold the device button for 10 seconds. The device will show a countdown and then restart in Access Point mode.</span>
    </div>
    </body></html>
  )rawliteral";
  return html;
}
