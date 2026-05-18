#include "web_server.h"
#include <Arduino.h>

String getDeviceSettingsHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Device Settings</title>
    <link rel='stylesheet' href='/common.css'>
    <script src='/translations.js'></script>
    <script>
      window.currentLang = 'EN';
      
      function loadStatus() {
        fetch('/device-settings-status').then(r => r.json()).then(d => {
          window.currentLang = d.language || 'EN';
          applyTranslations();
          const t = translations[window.currentLang];
          document.title = t.title;
          
          document.getElementById('current-arrows').textContent = d.maxArrows;
          document.getElementById('current-sensitivity').textContent = d.sensitivity.toFixed(0) + ' milli-g';
          document.getElementById('current-language').textContent = window.currentLang === 'EN' ? 'English' : 'Deutsch';
          
          document.getElementById('maxArrows').value = d.maxArrows;
          document.getElementById('sensitivity').value = d.sensitivity;
          document.getElementById('sensitivityValue').textContent = d.sensitivity.toFixed(0) + ' milli-g';
          document.getElementById('language').value = d.language || 'EN';
        }).catch(e => {
          console.error('Error loading status:', e);
          const t = translations[window.currentLang];
          document.getElementById('current-arrows').textContent = t.error;
          document.getElementById('current-sensitivity').textContent = t.error;
        });
      }
      
      function updateSensitivityDisplay() {
        const value = document.getElementById('sensitivity').value;
        document.getElementById('sensitivityValue').textContent = value + ' milli-g';
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const t = translations[window.currentLang];
        const maxArrows = document.getElementById('maxArrows').value;
        const sensitivity = document.getElementById('sensitivity').value;
        const language = document.getElementById('language').value;
        
        if (!maxArrows || maxArrows < 1) {
          showMessage(t.validArrows, 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('maxArrows', maxArrows);
        formData.append('sensitivity', sensitivity);
        formData.append('language', language);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = t.saving;
        
        fetch('/device-settings-save', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          window.currentLang = language;
          const t = translations[window.currentLang];
          document.title = t.title;
          applyTranslations();
          showMessage(t.savedSuccess, 'success');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = t.save;
          loadStatus();
        })
        .catch(error => {
          const t = translations[window.currentLang];
          showMessage(t.errorSaving + ' ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = t.save;
        });
      }
      
      function resetToDefaults() {
        const t = translations[window.currentLang];
        if (!confirm(t.resetConfirm)) {
          return;
        }
        
        document.getElementById('maxArrows').value = 10;
        document.getElementById('sensitivity').value = 1200;
        document.getElementById('language').value = 'EN';
        updateSensitivityDisplay();
        showMessage(t.resetMessage, 'success');
      }
      
      function showMessage(text, type) {
        const msgDiv = document.getElementById('message');
        msgDiv.textContent = text;
        msgDiv.className = 'message ' + type;
        msgDiv.style.display = 'block';
      }
      
      window.onload = loadStatus;
    </script>
    </head><body>
    <h1 data-i18n='title'>Device Settings</h1>
    
    <div class='status-box'>
      <strong data-i18n='currentSettings'>Current Settings:</strong>
      <p><strong data-i18n='numArrows'>Number of Arrows:</strong> <span id='current-arrows'>Loading...</span></p>
      <p><strong data-i18n='sensitivityThreshold'>Sensitivity Threshold:</strong> <span id='current-sensitivity'>Loading...</span></p>
      <p><strong data-i18n='language'>Language:</strong> <span id='current-language'>Loading...</span></p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2 data-i18n='configureSettings'>Configure Timer Settings</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='maxArrows' data-i18n='numArrowsLabel'>Number of Arrows:</label>
        <input type='number' id='maxArrows' name='maxArrows' placeholder='10' min='1' max='50' step='1' value='10'>
        <div class='info' data-i18n='numArrowsInfo'>Set how many arrows to count before stopping the timer (default: 10)</div>
      </div>
      
      <div class='form-group'>
        <label for='sensitivity' data-i18n='sensitivityLabel'>Sensitivity Threshold:</label>
        <div class='slider-container'>
          <input type='range' id='sensitivity' name='sensitivity' min='1010' max='1500' step='10' value='1200' oninput='updateSensitivityDisplay()'>
          <span class='slider-value' id='sensitivityValue'>1200 milli-g</span>
        </div>
        <div class='info' data-i18n='sensitivityInfo'>Adjust the acceleration threshold for detecting arrow impacts (1010-1500 milli-g, default: 1200)</div>
        <div class='info' style='margin-top: 0.5em;'>
          <strong data-i18n='lowerValues'>Lower values</strong> = <span data-i18n='moreSensitive'>more sensitive (may detect false positives)</span><br>
          <strong data-i18n='higherValues'>Higher values</strong> = <span data-i18n='lessSensitive'>less sensitive (may miss softer impacts)</span>
        </div>
      </div>
      
      <div class='form-group'>
        <label for='language' data-i18n='languageLabel'>Language:</label>
        <select id='language' name='language' style='width:100%;padding:0.8em;font-size:1em;border:1px solid var(--border-color);border-radius:var(--radius-sm);box-sizing:border-box;'>
          <option value='EN'>English</option>
          <option value='DE'>Deutsch</option>
        </select>
        <div class='info' data-i18n='languageInfo'>Select interface language</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn' data-i18n='save'>Save</button>
      <button type='button' class='btn-secondary' onclick='resetToDefaults()' data-i18n='resetDefaults'>Reset to Defaults</button>
    </form>
    
    <div class='divider'>
      <a href='/' class='nav-btn' data-i18n='backToTimer'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:#fff3cd;'>
      <strong data-i18n='noteTitle'>Note:</strong> <span data-i18n='noteText'>These settings control the basic operation of the arrow timer device. The number of arrows determines when the timer stops counting, and the sensitivity threshold controls how strong an impact must be to register as an arrow hit.</span>
      <br><br>
      <span data-i18n='noteText2'>All settings are stored persistently and will be retained after device restart or power loss.</span>
      <br><br>
      <strong data-i18n='tipTitle'>Tip:</strong> <span data-i18n='tipText'>If the timer is too sensitive (counting accidental bumps) or not sensitive enough (missing arrow impacts), adjust the sensitivity threshold. Start with small adjustments of ±50-100 milli-g.</span>
    </div>
    </body></html>
  )rawliteral";
  return html;
}
