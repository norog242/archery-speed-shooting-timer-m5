#include "web_server.h"
#include <Arduino.h>

String getCommonCss() {
  String css = R"rawliteral(
:root {
  --primary: #0066cc;
  --primary-hover: #0052a3;
  --secondary: #666;
  --danger: #cc0000;
  --success-bg: #d4edda;
  --success-text: #155724;
  --error-bg: #f8d7da;
  --error-text: #721c24;
  --info-bg: #fffbcc;
  --info-text: #666;
  --border-color: #ccc;
  --box-bg: #f0f0f0;
  --radius-sm: 0.3em;
  --radius-md: 0.5em;
}

body {
  font-family: sans-serif;
  max-width: 600px;
  margin: 2em auto;
  padding: 1em;
}

h1 {
  color: #333;
}

.status-box {
  background: var(--box-bg);
  padding: 1em;
  border-radius: var(--radius-md);
  margin-bottom: 1.5em;
}

.status-box strong {
  display: block;
  margin-bottom: 0.5em;
}

.form-group {
  margin-bottom: 1.5em;
}

label {
  display: block;
  margin-bottom: 0.5em;
  font-weight: bold;
}

input[type='text'],
input[type='password'],
input[type='number'] {
  width: 100%;
  padding: 0.8em;
  font-size: 1em;
  border: 1px solid var(--border-color);
  border-radius: var(--radius-sm);
  box-sizing: border-box;
}

input[type='range'] {
  width: 100%;
  padding: 0;
}

button {
  padding: 0.8em 2em;
  font-size: 1.1em;
  border: none;
  border-radius: var(--radius-sm);
  cursor: pointer;
  margin-right: 1em;
  margin-top: 0.5em;
}

.btn-primary {
  background: var(--primary);
  color: white;
}

.btn-primary:hover {
  background: var(--primary-hover);
}

.btn-secondary {
  background: var(--secondary);
  color: white;
}

.btn-danger {
  background: var(--danger);
  color: white;
}

.message {
  padding: 1em;
  margin: 1em 0;
  border-radius: var(--radius-sm);
}

.message.success {
  background: var(--success-bg);
  color: var(--success-text);
}

.message.error {
  background: var(--error-bg);
  color: var(--error-text);
}

.info {
  color: var(--info-text);
  font-size: 0.9em;
  margin-top: 0.5em;
}

.info-box {
  margin-top: 2em;
  padding: 1em;
  border-radius: var(--radius-sm);
}

.nav-btn {
  display: inline-block;
  padding: 0.8em 1.5em;
  background: var(--secondary);
  color: white;
  text-decoration: none;
  border-radius: var(--radius-sm);
  font-size: 1.1em;
  margin-right: 1em;
  margin-bottom: 1em;
}

.divider {
  margin-top: 2em;
  padding-top: 2em;
  border-top: 1px solid var(--border-color);
}

.participants-box {
  background: #f9f9f9;
  padding: 1em;
  border-radius: var(--radius-md);
  margin: 1.5em 0;
}

.participant-item {
  padding: 0.5em;
  border-bottom: 1px solid #ddd;
}

.participant-item:last-child {
  border-bottom: none;
}

.participant-name {
  font-weight: bold;
}

.participant-id {
  color: var(--info-text);
  font-size: 0.9em;
  margin-left: 0.5em;
}

.slider-container {
  display: flex;
  align-items: center;
  gap: 1em;
}

.slider-value {
  font-size: 1.5em;
  font-weight: bold;
  color: var(--primary);
  min-width: 100px;
  text-align: center;
}
  )rawliteral";
  return css;
}

String getHtml() {
  String html = R"rawliteral(
    <html><head>
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
    <script>
      let maxArrows = 10; // Default, will be loaded dynamically
      
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
        <h2>Arrow Count:</h2>
        <div id='arrows' class='segment'>-</div>
    </div>
    <div id='duration-container' class='status-container'>
        <h2>Time:</h2>
        <div id='duration' class='segment'>-</div>
    </div>
    <div id='points-container' class='status-container'>
      <h4>Score:</h4>
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
      <label for='points' class='points-label'>Points: </label>
      <button type='button' class='points-btn' onclick='changePoints(-10)'>-10</button>
      <button type='button' class='points-btn' onclick='changePoints(-1)'>-1</button>
      <input type='number' id='points' name='points' min='0' value='0' class='points-input'>
      <button type='button' class='points-btn' onclick='changePoints(1)'>+1</button>
      <button type='button' class='points-btn' onclick='changePoints(10)'>+10</button>
      
        <!-- Tournament Integration Section -->
      <div id='tournament-section' class='tournament-section'>
        <h4>Save Tournament result (ID: <strong id='tournament-id-display'>-</strong>)</h4>
        <div class='tournament-form'>
          <select id='username-select' style='min-width:250px;'>
            <option value=''>-- Select Archer --</option>
          </select>
          <button id='save-tournament-btn' onclick='saveToTournament()' disabled>Save</button>
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
        if (btn) btn.textContent = (arrows === maxArrows) ? 'Start' : 'Reset';
      }
    </script>
    <div style='margin:1em 0;'>
      <h4>Last 5 Attempts</h4>
      <ul id='lastDurations'>
        <li>-</li>
      </ul>
    </div>
    
    <div style='margin-top:2em;'>
      <a href='/device-settings' class='nav-btn'>Device Settings</a>
      <a href='/config' class='nav-btn'>WiFi Settings</a>
      <a href='/tournament-config' class='nav-btn'>Tournament Settings</a>
    </div>
    
    <p class='footer-text'>Updates every 0.5 seconds</p>
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
          select.innerHTML = '<option value="">-- Select Archer --</option>';
          
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
        
        if (!participantId) {
          showTournamentMessage('Please select an archer', 'error');
          return;
        }
        
        const points = parseInt(document.getElementById('points').value) || 0;
        const durationText = document.getElementById('duration').textContent;
        const duration = parseFloat(durationText) || 0;
        const arrows = parseInt(document.getElementById('arrows').textContent) || 0;
        
        if (arrows !== maxArrows) {
          showTournamentMessage('Please complete ' + maxArrows + ' arrows before saving to tournament', 'error');
          return;
        }
        
        if (duration <= 0) {
          showTournamentMessage('Invalid duration', 'error');
          return;
        }
        
        const btn = document.getElementById('save-tournament-btn');
        btn.disabled = true;
        btn.textContent = 'Saving...';
        
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
          showTournamentMessage('Result saved successfully for ' + participantName + '!', 'success');
          btn.disabled = false;
          btn.textContent = 'Save';
        })
        .catch(error => {
          showTournamentMessage('Error: ' + error.message, 'error');
          btn.disabled = false;
          btn.textContent = 'Save';
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
        // Load device settings
        fetch('/device-settings-status').then(r => r.json()).then(d => {
          maxArrows = d.maxArrows;
        }).catch(e => {
          console.error('Error loading device settings:', e);
        });
      };
    </script>
    </body></html>
  )rawliteral";
  return html;
}

String getConfigHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>WiFi Configuration</title>
    <link rel='stylesheet' href='/common.css'>
    <script>
      function loadStatus() {
        fetch('/wifi-status').then(r => r.json()).then(d => {
          document.getElementById('current-mode').textContent = d.mode === 'station' ? 'Connected to WiFi' : 'Access Point Mode';
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
        const ssid = document.getElementById('ssid').value;
        const password = document.getElementById('password').value;
        
        if (!ssid) {
          showMessage('Please enter a WiFi SSID', 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('ssid', ssid);
        formData.append('password', password);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = 'Saving...';
        
        fetch('/wifi-config', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage('Configuration saved! Device will restart and connect to the new WiFi network. You may need to reconnect to the new network.', 'success');
        })
        .catch(error => {
          showMessage('Error saving configuration: ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save & Restart';
        });
      }
      
      function clearConfig() {
        if (!confirm('Are you sure you want to clear the WiFi configuration? Device will restart in Access Point mode.')) {
          return;
        }
        
        fetch('/wifi-clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          showMessage('WiFi configuration cleared! Device will restart in Access Point mode.', 'success');
        })
        .catch(error => {
          showMessage('Error clearing configuration: ' + error, 'error');
        });
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
    <h1>WiFi Configuration</h1>
    
    <div class='status-box'>
      <strong>Current Status:</strong>
      <p><strong>Mode:</strong> <span id='current-mode'>Loading...</span></p>
      <p><strong>Network:</strong> <span id='current-ssid'>-</span></p>
      <p><strong>IP Address:</strong> <span id='current-ip'>-</span></p>
      <p id='hostname-row' style='display:none;'><strong>Hostname:</strong> <span id='current-hostname'>-</span></p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2>Connect to WiFi Network</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='ssid'>WiFi Network Name (SSID):</label>
        <input type='text' id='ssid' name='ssid' placeholder='Enter WiFi SSID' required>
        <div class='info'>Enter the name of your WiFi network</div>
      </div>
      
      <div class='form-group'>
        <label for='password'>WiFi Password:</label>
        <input type='password' id='password' name='password' placeholder='Enter WiFi password'>
        <div class='info'>Leave empty for open networks</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn'>Save & Restart</button>
      <button type='button' class='btn-danger' onclick='clearConfig()'>Clear Configuration</button>
    </form>
    
    <div class='divider'>
      <a href='/' class='nav-btn'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:var(--info-bg);'>
      <strong>Note:</strong> After saving, the device will restart and attempt to connect to the specified WiFi network. 
      If connection fails, it will automatically fall back to Access Point mode (ArrowTimerAP).
      The configuration is stored persistently and will be used after power loss.
      <br><br>
      <strong>Easy Access:</strong> When connected to WiFi, access the device at <strong>http://arrowtimer.local</strong> 
      (works on most devices with mDNS/Bonjour support). The IP address and hostname are also displayed on the device screen.
      <br><br>
      <strong>Hardware Reset:</strong> To reset WiFi credentials without web access, press and hold the device button for 10 seconds. 
      The device will show a countdown and then restart in Access Point mode.
    </div>
    </body></html>
  )rawliteral";
  return html;
}

String getTournamentConfigHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Tournament Configuration</title>
    <link rel='stylesheet' href='/common.css'>
    <style>
      #participants-list {
        display: none;
      }
    </style>
    <script>
      function loadStatus() {
        fetch('/tournament-status').then(r => r.json()).then(d => {
          let statusText = d.tournamentId ? 'Tournament ID: ' + d.tournamentId : 'Not configured';
          if (d.apiDomain) {
            statusText += ' | Domain: ' + d.apiDomain;
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
          document.getElementById('current-tournament').textContent = 'Error loading status';
        });
      }
      
      function loadParticipants() {
        document.getElementById('participants-status').textContent = 'Loading participants...';
        document.getElementById('participants-list').style.display = 'block';
        document.getElementById('participants-container').innerHTML = '';
        
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
          if (result.data && result.data.length > 0) {
            document.getElementById('participants-status').textContent = 'Participants (' + result.data.length + '):';
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
            document.getElementById('participants-status').textContent = 'No participants found';
          }
        })
        .catch(error => {
          document.getElementById('participants-status').textContent = 'Error: ' + error.message;
          document.getElementById('participants-container').innerHTML = '';
        });
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const tournamentId = document.getElementById('tournamentId').value;
        const apiDomain = document.getElementById('apiDomain').value;
        
        if (!tournamentId) {
          showMessage('Please enter a tournament ID', 'error');
          return;
        }
        
        if (!apiDomain) {
          showMessage('Please enter an API domain', 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('tournamentId', tournamentId);
        formData.append('apiDomain', apiDomain);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = 'Saving...';
        
        fetch('/tournament-save', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage('Tournament configuration saved successfully!', 'success');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
          loadStatus();
          loadParticipants();
        })
        .catch(error => {
          showMessage('Error saving tournament configuration: ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
        });
      }
      
      function clearConfig() {
        if (!confirm('Are you sure you want to clear the tournament configuration?')) {
          return;
        }
        
        fetch('/tournament-clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          showMessage('Tournament configuration cleared successfully!', 'success');
          document.getElementById('tournamentId').value = '';
          document.getElementById('apiDomain').value = 'archers.himmelix.ch';
          loadStatus();
        })
        .catch(error => {
          showMessage('Error clearing tournament configuration: ' + error, 'error');
        });
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
    <h1>Tournament Configuration</h1>
    
    <div class='status-box'>
      <strong>Current Status:</strong>
      <p id='current-tournament'>Loading...</p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2>Tournament Integration</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='apiDomain'>API Domain:</label>
        <input type='text' id='apiDomain' name='apiDomain' placeholder='archers.himmelix.ch' value='archers.himmelix.ch'>
        <div class='info'>Enter the domain of your tournament API (without https://)</div>
      </div>
      
      <div class='form-group'>
        <label for='tournamentId'>Tournament ID:</label>
        <input type='number' id='tournamentId' name='tournamentId' placeholder='Enter tournament ID' min='0' step='1'>
        <div class='info'>Enter the numeric ID of your tournament</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn'>Save</button>
      <button type='button' class='btn-danger' onclick='clearConfig()'>Clear Tournament Config</button>
    </form>
    
    <div id='participants-list' class='participants-box'>
      <h3 id='participants-status'>Loading participants...</h3>
      <div id='participants-container'></div>
      <button type='button' class='btn-secondary' onclick='loadParticipants()' style='margin-top: 1em;'>Refresh Participants</button>
    </div>
    
    <div class='divider'>
      <a href='/' class='nav-btn'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:#e8f4ff;'>
      <strong>Note:</strong> Configure the API domain and tournament ID to integrate this timer with your tournament management application. 
      The API domain should point to your tournament system (e.g., archers.himmelix.ch or a test environment). 
      Enter the numeric tournament ID provided by your tournament system to link this device with your tournament.
      <br><br>
      The configuration is stored persistently and will be retained after device restart or power loss.
    </div>
    </body></html>
  )rawliteral";
  return html;
}

String getDeviceSettingsHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Device Settings</title>
    <link rel='stylesheet' href='/common.css'>
    <script>
      function loadStatus() {
        fetch('/device-settings-status').then(r => r.json()).then(d => {
          document.getElementById('current-arrows').textContent = d.maxArrows;
          document.getElementById('current-sensitivity').textContent = d.sensitivity.toFixed(0) + ' milli-g';
          
          document.getElementById('maxArrows').value = d.maxArrows;
          document.getElementById('sensitivity').value = d.sensitivity;
          document.getElementById('sensitivityValue').textContent = d.sensitivity.toFixed(0) + ' milli-g';
        }).catch(e => {
          console.error('Error loading status:', e);
          document.getElementById('current-arrows').textContent = 'Error';
          document.getElementById('current-sensitivity').textContent = 'Error';
        });
      }
      
      function updateSensitivityDisplay() {
        const value = document.getElementById('sensitivity').value;
        document.getElementById('sensitivityValue').textContent = value + ' milli-g';
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const maxArrows = document.getElementById('maxArrows').value;
        const sensitivity = document.getElementById('sensitivity').value;
        
        if (!maxArrows || maxArrows < 1) {
          showMessage('Please enter a valid number of arrows (minimum 1)', 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('maxArrows', maxArrows);
        formData.append('sensitivity', sensitivity);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = 'Saving...';
        
        fetch('/device-settings-save', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage('Device settings saved successfully!', 'success');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
          loadStatus();
        })
        .catch(error => {
          showMessage('Error saving device settings: ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
        });
      }
      
      function resetToDefaults() {
        if (!confirm('Are you sure you want to reset to default settings? (10 arrows, 1200 milli-g sensitivity)')) {
          return;
        }
        
        document.getElementById('maxArrows').value = 10;
        document.getElementById('sensitivity').value = 1200;
        updateSensitivityDisplay();
        showMessage('Settings reset to defaults. Click Save to apply.', 'success');
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
    <h1>Device Settings</h1>
    
    <div class='status-box'>
      <strong>Current Settings:</strong>
      <p><strong>Number of Arrows:</strong> <span id='current-arrows'>Loading...</span></p>
      <p><strong>Sensitivity Threshold:</strong> <span id='current-sensitivity'>Loading...</span></p>
    </div>
    
    <div id='message' style='display:none;'></div>
    
    <h2>Configure Timer Settings</h2>
    <form onsubmit='saveConfig(event)'>
      <div class='form-group'>
        <label for='maxArrows'>Number of Arrows:</label>
        <input type='number' id='maxArrows' name='maxArrows' placeholder='10' min='1' max='50' step='1' value='10'>
        <div class='info'>Set how many arrows to count before stopping the timer (default: 10)</div>
      </div>
      
      <div class='form-group'>
        <label for='sensitivity'>Sensitivity Threshold:</label>
        <div class='slider-container'>
          <input type='range' id='sensitivity' name='sensitivity' min='500' max='1500' step='10' value='1200' oninput='updateSensitivityDisplay()'>
          <span class='slider-value' id='sensitivityValue'>1200 milli-g</span>
        </div>
        <div class='info'>Adjust the acceleration threshold for detecting arrow impacts (500-1500 milli-g, default: 1200)</div>
        <div class='info' style='margin-top: 0.5em;'>
          <strong>Lower values</strong> = more sensitive (may detect false positives)<br>
          <strong>Higher values</strong> = less sensitive (may miss softer impacts)
        </div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn'>Save</button>
      <button type='button' class='btn-secondary' onclick='resetToDefaults()'>Reset to Defaults</button>
    </form>
    
    <div class='divider'>
      <a href='/' class='nav-btn'>Back to Timer</a>
    </div>
    
    <div class='info-box' style='background:#fff3cd;'>
      <strong>Note:</strong> These settings control the basic operation of the arrow timer device. 
      The number of arrows determines when the timer stops counting, and the sensitivity threshold controls 
      how strong an impact must be to register as an arrow hit.
      <br><br>
      All settings are stored persistently and will be retained after device restart or power loss.
      <br><br>
      <strong>Tip:</strong> If the timer is too sensitive (counting accidental bumps) or not sensitive enough 
      (missing arrow impacts), adjust the sensitivity threshold. Start with small adjustments of ±50-100 milli-g.
    </div>
    </body></html>
  )rawliteral";
  return html;
}
