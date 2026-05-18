#include "web_server.h"
#include <Arduino.h>

String getTournamentConfigHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Tournament Configuration</title>
    <link rel='stylesheet' href='/common.css'>
    <script src='/translations.js'></script>
    <style>
      #participants-list {
        display: none;
      }
    </style>
    <script>
      window.currentLang = 'EN';
      
      function loadStatus() {
        const t = translations[window.currentLang];
        fetch('/tournament-status').then(r => r.json()).then(d => {
          let statusText = d.tournamentId ? t.tournamentId + ': ' + d.tournamentId : t.notConfigured;
          if (d.apiDomain) {
            statusText += ' | ' + t.domain + ': ' + d.apiDomain;
          }
          document.getElementById('current-tournament').textContent = statusText;
          
          if (d.tournamentId) {
            document.getElementById('tournamentId').value = d.tournamentId;
            loadParticipants();
          } else {
            document.getElementById('participants-list').style.display = 'none';
          }
          
          if (d.apiDomain) {
            document.getElementById('apiDomain').value = d.apiDomain;
          }
        }).catch(e => {
          console.error('Error loading status:', e);
          document.getElementById('current-tournament').textContent = t.errorLoadingStatus;
        });
      }
      
      function loadParticipants() {
        const t = translations[window.currentLang];
        document.getElementById('participants-status').textContent = t.loadingParticipants;
        document.getElementById('participants-list').style.display = 'block';
        document.getElementById('participants-container').innerHTML = '';
        
        fetch('/tournament-participants')
        .then(response => {
          if (!response.ok) {
            return response.json().then(err => {
              throw new Error(err.error || t.failedToLoadParticipants);
            });
          }
          return response.json();
        })
        .then(result => {
          if (result.data && result.data.length > 0) {
            document.getElementById('participants-status').textContent = t.participants + ' (' + result.data.length + '):';
            const container = document.getElementById('participants-container');
            result.data.forEach(participant => {
              const div = document.createElement('div');
              div.className = 'participant-item';
              div.innerHTML = '<span class="participant-name">' + participant.name + 
                             '</span><span class="participant-id">(ID: ' + 
                             participant.tournament_participant_id + ')</span>';
              container.appendChild(div);
            });
          } else {
            document.getElementById('participants-status').textContent = t.noParticipantsFound;
          }
        })
        .catch(error => {
          const t = translations[window.currentLang];
          document.getElementById('participants-status').textContent = t.errorMsg + ' ' + error.message;
          document.getElementById('participants-container').innerHTML = '';
        });
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const t = translations[window.currentLang];
        const tournamentId = document.getElementById('tournamentId').value;
        const apiDomain = document.getElementById('apiDomain').value;
        
        if (!tournamentId) {
          showMessage(t.pleaseEnterTournamentId, 'error');
          return;
        }
        
        if (!apiDomain) {
          showMessage(t.pleaseEnterApiDomain, 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('tournamentId', tournamentId);
        formData.append('apiDomain', apiDomain);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = t.saving;
        
        fetch('/tournament-save', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage(t.tournamentConfigSaved, 'success');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = t.save;
          loadStatus();
          loadParticipants();
        })
        .catch(error => {
          showMessage(t.errorSavingTournament + ' ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = t.save;
        });
      }
      
      function clearConfig() {
        const t = translations[window.currentLang];
        if (!confirm(t.clearTournamentConfirm)) {
          return;
        }
        
        fetch('/tournament-clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          showMessage(t.tournamentConfigCleared, 'success');
          document.getElementById('tournamentId').value = '';
          document.getElementById('apiDomain').value = 'archers.himmelix.ch';
          loadStatus();
        })
        .catch(error => {
          showMessage(t.errorClearingTournament + ' ' + error, 'error');
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
          document.title = t.tournamentConfigTitle;
          applyTranslations();
          loadStatus();
        }).catch(e => {
          console.error('Error loading device settings:', e);
          loadStatus();
        });
      };
    </script>
    </head><body>
    <h1 data-i18n='tournamentConfigTitle'>Tournament Configuration</h1>
    
    <div class='status-box'>
      <strong data-i18n='currentStatus'>Current Status:</strong>
      <p id='current-tournament'>Loading...</p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2 data-i18n='tournamentIntegration'>Tournament Integration</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='apiDomain' data-i18n='apiDomain'>API Domain:</label>
        <input type='text' id='apiDomain' name='apiDomain' placeholder='archers.himmelix.ch' value='archers.himmelix.ch'>
        <div class='info' data-i18n='enterApiDomain'>Enter the domain of your tournament API (without https://)</div>
      </div>
      
      <div class='form-group'>
        <label for='tournamentId' data-i18n='tournamentIdLabel'>Tournament ID:</label>
        <input type='number' id='tournamentId' name='tournamentId' data-i18n-placeholder='enterTournamentId' placeholder='Enter tournament ID' min='0' step='1'>
        <div class='info' data-i18n='enterNumericId'>Enter the numeric ID of your tournament</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn' data-i18n='save'>Save</button>
      <button type='button' class='btn-danger' onclick='clearConfig()' data-i18n='clearTournamentConfig'>Clear Tournament Config</button>
    </form>
    
    <div id='participants-list' class='participants-box'>
      <h3 id='participants-status' data-i18n='loadingParticipants'>Loading participants...</h3>
      <div id='participants-container'></div>
      <button type='button' class='btn-secondary' onclick='loadParticipants()' style='margin-top: 1em;' data-i18n='refreshParticipants'>Refresh Participants</button>
    </div>
    
    <div class='divider'>
      <a href='/' class='nav-btn' data-i18n='backToTimer'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:#e8f4ff;'>
      <strong data-i18n='noteTitle'>Note:</strong> <span data-i18n='tournamentNoteText'>Configure the API domain and tournament ID to integrate this timer with your tournament management application. The API domain should point to your tournament system (e.g., archers.himmelix.ch or a test environment). Enter the numeric ID provided by your tournament system to link this device with your tournament.</span>
      <br><br>
      <span data-i18n='tournamentNoteText2'>The configuration is stored persistently and will be retained after device restart or power loss.</span>
    </div>
    </body></html>
  )rawliteral";
  return html;
}
