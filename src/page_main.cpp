#include "web_server.h"
#include <Arduino.h>

String getHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Arrow Timer</title>
    <link rel='stylesheet' href='/common.css'>
    <style>
      body {
        text-align: center;
        max-width: 800px;
      }
      
      @font-face {
        font-family: 'Digital7';
        src: url('/digital7.woff') format('woff');
        font-display: swap;
      }
      
      .segment {
        font-family: 'Digital7', monospace;
        font-weight: bold;
        font-size: 4em;
        color: #000000;
        letter-spacing: 0.1em;
        background: #fcfcfc;
        padding: 0.2em 0.5em;
        border-radius: 0.2em;
        display: inline-block;
        margin: 0.2em 0;
      }
      
      .segment-small {
        font-family: 'Digital7', monospace;
        font-weight: bold;
        font-size: 2em;
        color: #000000;
        letter-spacing: 0.1em;
        background: #fcfcfc;
        padding: 0.2em 0.5em;
        border-radius: 0.2em;
        display: inline-block;
        margin: 0.2em 0;
      }
      
      .status-container {
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        justify-content: space-around;
        align-items: center;
        gap: 2em;
        margin-top: 1em;
      }
      
      .tournament-section {
        background: #e8f4ff;
        padding: 1.0em;
        border-radius: 0.5em;
        margin: 8px;
        max-width: 90%;
        display: none;
      }
      
      .tournament-section h3 {
        margin-top: 0;
        color: var(--primary);
      }
      
      .tournament-form {
        display: flex;
        flex-direction: row;
        flex-wrap: wrap;
        gap: 1em;
        align-items: center;
        justify-content: center;
      }
      
      .tournament-form label {
        flex-basis: 100%;
        text-align: center;
      }
      
      .tournament-form select,
      .tournament-form button {
        font-size: 1.2em;
        padding: 0.8em 1.0em;
        border-radius: 0.3em;
        border: 1px solid var(--border-color);
      }
      
      .tournament-form button {
        background: var(--primary);
        color: white;
        border: none;
        cursor: pointer;
      }
      
      .tournament-form button:hover {
        background: var(--primary-hover);
      }
      
      .tournament-form button:disabled {
        background: var(--border-color);
        cursor: not-allowed;
      }
      
      .tournament-message {
        padding: 0.8em;
        margin-top: 1em;
        border-radius: 0.3em;
        display: none;
      }
      
      .tournament-message.success {
        background: var(--success-bg);
        color: var(--success-text);
      }
      
      .tournament-message.error {
        background: var(--error-bg);
        color: var(--error-text);
      }
      
      .points-form {
        margin-bottom: 1em;
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 8px;
        flex-wrap: wrap;
      }
      
      .points-btn {
        font-size: 1.5em;
        padding: 0.5em 1em;
        margin-right: 8px;
      }
      
      .points-label {
        font-size: 1.5em;
      }
      
      .points-input {
        font-size: 1.2em;
        width: 5em;
        margin: 0 8px;
      }
      
      .reset-btn {
        font-size: 1.5em;
        padding: 0.5em 2em;
        margin-left: 16px;
        margin-top: 16px;
        border: none;
        border-radius: 0.2em;
        width: 50%;
      }
      
      .footer-text {
        color: gray;
        font-size: 12px;
      }
    </style>
    <script src='/translations.js'></script>
    <script>
      let maxArrows = 10;
      window.currentLang = 'EN';
      
      function updateData() {
        fetch('/data').then(r => r.json()).then(d => {
          document.getElementById('arrows').textContent = d.arrows;
          document.getElementById('duration').textContent = d.duration.toFixed(2) + ' s';
          updateResetBtnLabel(d.arrows);
          // Reset points and score for new attempt
          if (d.arrows === 0) {
            document.getElementById('points').value = 0;
            document.getElementById('score').textContent = '0.00 pts/s';
          }
        });
      }
      setInterval(updateData, 500);
      window.onload = updateData;
    </script>
    </head><body>
    <div id='arrows-container' class='status-container'>
        <h2 data-i18n='arrowCount'>Arrow Count:</h2>
        <div id='arrows' class='segment'>-</div>
    </div>
    <div id='duration-container' class='status-container'>
        <h2 data-i18n='time'>Time:</h2>
        <div id='duration' class='segment'>-</div>
    </div>
    <div id='points-container' class='status-container'>
      <h4 data-i18n='score'>Score:</h4>
      <div id='score' class='segment-small'>-</div>
    </div>
    <form id='pointsForm' action='/reset' method='POST' class='points-form' onsubmit='return submitWithScore(event)'>
          <script>
            function submitWithScore(e) {
              // Add score as hidden input before submitting
              const form = document.getElementById('pointsForm');
              let scoreInput = document.getElementById('scoreInput');
              if (!scoreInput) {
                scoreInput = document.createElement('input');
                scoreInput.type = 'hidden';
                scoreInput.name = 'score';
                scoreInput.id = 'scoreInput';
                form.appendChild(scoreInput);
              }
              const points = parseInt(document.getElementById('points').value) || 0;
              const durationText = document.getElementById('duration').textContent;
              const duration = parseFloat(durationText) || 0;
              const score = (duration > 0) ? (points / duration) : 0;
              scoreInput.value = score;
              return true;
            }
          </script>
      <label for='points' class='points-label' data-i18n='points'>Points: </label>
      <button type='button' class='points-btn' onclick='changePoints(-10)'>-10</button>
      <button type='button' class='points-btn' onclick='changePoints(-1)'>-1</button>
      <input type='number' id='points' name='points' min='0' value='0' class='points-input'>
      <button type='button' class='points-btn' onclick='changePoints(1)'>+1</button>
      <button type='button' class='points-btn' onclick='changePoints(10)'>+10</button>
      
        <!-- Tournament Integration Section -->
      <div id='tournament-section' class='tournament-section'>
        <h4><span data-i18n='saveTournamentResult'>Save Tournament result (ID:</span> <strong id='tournament-id-display'>-</strong>)</h4>
        <div class='tournament-form'>
          <select id='username-select' style='min-width:250px;'>
            <option value='' data-i18n='selectArcher'>-- Select Archer --</option>
          </select>
          <button id='save-tournament-btn' onclick='saveToTournament()' disabled data-i18n='save'>Save</button>
          <div id='tournament-message' class='tournament-message'></div>
        </div>
      </div>

      <button id='resetBtn' class='reset-btn' type='submit'>Reset</button>
      </form>
    <script>
      function updateScoreField() {
        const points = parseInt(document.getElementById('points').value) || 0;
        const durationText = document.getElementById('duration').textContent;
        const duration = parseFloat(durationText) || 0;
        const score = (duration > 0) ? (points / duration) : 0;
        document.getElementById('score').textContent = score.toFixed(2) + ' pts/s';
      }
      function changePoints(delta) {
        const input = document.getElementById('points');
        let val = parseInt(input.value) || 0;
        val += delta;
        if (val < 0) val = 0;
        input.value = val;
        updateScoreField();
      }
      document.addEventListener('DOMContentLoaded', function() {
        document.getElementById('points').addEventListener('input', updateScoreField);
      });
      function updateResetBtnLabel(arrows) {
        const btn = document.getElementById('resetBtn');
        const t = translations[window.currentLang];
        if (btn) btn.textContent = (arrows === maxArrows) ? t.start : t.reset;
      }
    </script>
    <div style='margin:1em 0;'>
      <h4 data-i18n='lastAttempts'>Last 5 Attempts</h4>
      <ul id='lastDurations'>
        <li>-</li>
      </ul>
    </div>
    
    <div style='margin-top:2em;'>
      <a href='/device-settings' class='nav-btn' data-i18n='deviceSettings'>Device Settings</a>
      <a href='/config' class='nav-btn' data-i18n='wifiSettings'>WiFi Settings</a>
      <a href='/tournament-config' class='nav-btn' data-i18n='tournamentSettings'>Tournament Settings</a>
    </div>
    
    <p class='footer-text' data-i18n='updates'>Updates every 0.5 seconds</p>
    <p class='footer-text'>M5 Atom S3 Arrow Timer by norog242</p>
    <script>
      function updateLastDurations(arr) {
        const ul = document.getElementById('lastDurations');
        ul.innerHTML = '';
        if (!arr || arr.length === 0) {
          ul.innerHTML = '<li>-</li>';
          return;
        }
        for (let i = arr.length - 1; i >= 0; --i) {
          const li = document.createElement('li');
          const entry = arr[i];
          li.textContent = `${entry.duration.toFixed(2)} s - ${entry.points} pts => ${entry.score.toFixed(2)} pts/s`;
          ul.appendChild(li);
        }
      }
      function updateData() {
        fetch('/data').then(r => r.json()).then(d => {
          document.getElementById('arrows').textContent = d.arrows;
          document.getElementById('duration').textContent = d.duration.toFixed(2) + ' s';
          updateLastDurations(d.lastDurations);
        });
      }
      
      // Tournament functions
      function loadTournamentStatus() {
        fetch('/tournament-status').then(r => r.json()).then(d => {
          if (d.tournamentId !== null && d.tournamentId >= 0) {
            document.getElementById('tournament-section').style.display = 'block';
            document.getElementById('tournament-id-display').textContent = d.tournamentId;
            loadTournamentUsers();
          } else {
            document.getElementById('tournament-section').style.display = 'none';
          }
        }).catch(e => {
          console.error('Error loading tournament status:', e);
        });
      }
      
      function loadTournamentUsers() {
        const select = document.getElementById('username-select');
        select.innerHTML = '<option value="">-- Loading... --</option>';
        select.disabled = true;
        
        fetch('/tournament-participants')
        .then(response => {
          if (!response.ok) {
            return response.json().then(err => {
              throw new Error(err.error || 'Failed to load participants');
            });
          }
          return response.json();
        })
        .then(result => {
          const t = translations[window.currentLang];
          select.innerHTML = '<option value="">' + t.selectArcher + '</option>';
          
          if (result.data && result.data.length > 0) {
            result.data.forEach(participant => {
              const option = document.createElement('option');
              option.value = participant.tournament_participant_id;
              option.textContent = participant.name;
              select.appendChild(option);
            });
            select.disabled = false;
          } else {
            select.innerHTML = '<option value="">-- No participants found --</option>';
            select.disabled = true;
          }
        })
        .catch(error => {
          console.error('Error loading participants:', error);
          select.innerHTML = '<option value="">-- Error loading participants --</option>';
          select.disabled = true;
          showTournamentMessage('Failed to load participants: ' + error.message, 'error');
        });
      }
      
      function saveToTournament() {
        const participantId = document.getElementById('username-select').value;
        const participantName = document.getElementById('username-select').options[document.getElementById('username-select').selectedIndex].text;
        const t = translations[window.currentLang];
        
        if (!participantId) {
          showTournamentMessage(t.pleaseSelect, 'error');
          return;
        }
        
        const points = parseInt(document.getElementById('points').value) || 0;
        const durationText = document.getElementById('duration').textContent;
        const duration = parseFloat(durationText) || 0;
        const arrows = parseInt(document.getElementById('arrows').textContent) || 0;
        
        if (arrows !== maxArrows) {
          showTournamentMessage(t.pleaseComplete + ' ' + maxArrows + ' ' + t.arrowsBefore, 'error');
          return;
        }
        
        if (duration <= 0) {
          showTournamentMessage(t.invalidDuration, 'error');
          return;
        }
        
        const btn = document.getElementById('save-tournament-btn');
        btn.disabled = true;
        btn.textContent = t.saving;
        
        const formData = new URLSearchParams();
        formData.append('participantId', participantId);
        formData.append('points', points);
        formData.append('duration', duration);
        
        fetch('/tournament-save-result', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => {
          if (!response.ok) {
            return response.json().then(err => {
              throw new Error(err.error || 'Failed to save result');
            });
          }
          return response.json();
        })
        .then(result => {
          const t = translations[window.currentLang];
          showTournamentMessage(t.savedSuccessfully + ' ' + participantName + '!', 'success');
          btn.disabled = false;
          btn.textContent = t.save;
        })
        .catch(error => {
          const t = translations[window.currentLang];
          showTournamentMessage(t.errorMsg + ' ' + error.message, 'error');
          btn.disabled = false;
          btn.textContent = t.save;
        });
      }
      
      function showTournamentMessage(text, type) {
        const msgDiv = document.getElementById('tournament-message');
        msgDiv.textContent = text;
        msgDiv.className = 'tournament-message ' + type;
        msgDiv.style.display = 'block';
        setTimeout(() => {
          msgDiv.style.display = 'none';
        }, 5000);
      }
      
      // Enable/disable save button based on username selection
      document.addEventListener('DOMContentLoaded', function() {
        const select = document.getElementById('username-select');
        select.addEventListener('change', function() {
          const btn = document.getElementById('save-tournament-btn');
          btn.disabled = !this.value;
        });
      });
      
      setInterval(updateData, 500);
      window.onload = function() {
        updateData();
        loadTournamentStatus();
        // Load device settings including language
        fetch('/device-settings-status').then(r => r.json()).then(d => {
          maxArrows = d.maxArrows;
          window.currentLang = d.language || 'EN';
          applyTranslations();
        }).catch(e => {
          console.error('Error loading device settings:', e);
        });
      };
    </script>
    </body></html>
  )rawliteral";
  return html;
}
