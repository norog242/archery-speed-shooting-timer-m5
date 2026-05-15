#include "web_server.h"
#include <Arduino.h>

String getHtml() {
  String html = R"rawliteral(
    <html><head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Arrow Timer</title>
    <style>
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
     .status-container{
      display:flex;
      flex-direction:row;
      flex-wrap:wrap;
      justify-content:space-around;
      align-items:center;
      gap:2em;
      margin-top:1em;
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
        color: #0066cc;
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
      .tournament-form select, .tournament-form button {
        font-size: 1.2em;
        padding: 0.8em 1.0em;
        border-radius: 0.3em;
        border: 1px solid #ccc;
      }
      .tournament-form button {
        background: #0066cc;
        color: white;
        border: none;
        cursor: pointer;
      }
      .tournament-form button:hover {
        background: #0052a3;
      }
      .tournament-form button:disabled {
        background: #ccc;
        cursor: not-allowed;
      }
      .tournament-message {
        padding: 0.8em;
        margin-top: 1em;
        border-radius: 0.3em;
        display: none;
      }
      .tournament-message.success {
        background: #d4edda;
        color: #155724;
      }
      .tournament-message.error {
        background: #f8d7da;
        color: #721c24;
      }
    </style>
    <script>
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
    </head><body style='font-family:sans-serif;text-align:center;'>
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
    <form id='pointsForm' action='/reset' method='POST' style='margin-bottom:1em;display:flex;align-items:center;justify-content:center;gap:8px; flex-wrap:wrap;' onsubmit='return submitWithScore(event)'>
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
      <label for='points' style='font-size:1.5em;'>Points: </label>
      <button type='button' class='points-btn' onclick='changePoints(-10)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>-10</button>
      <button type='button' class='points-btn' onclick='changePoints(-1)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>-1</button>
      <input type='number' id='points' name='points' min='0' value='0' style='font-size:1.2em;width:5em;margin:0 8px;'>
      <button type='button' class='points-btn' onclick='changePoints(1)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>+1</button>
      <button type='button' class='points-btn' onclick='changePoints(10)' style='font-size:1.5em;padding:0.5em 1em;'>+10</button>
      
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

      <button id='resetBtn' style='font-size:1.5em;padding:0.5em 2em;margin-left:16px; margin-top:16px; border:none; border-radius:0.2em; width:50%' type='submit'>Reset</button>
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
        if (btn) btn.textContent = (arrows === 10) ? 'Start' : 'Reset';
      }
    </script>
    <div style='margin:1em 0;'>
      <h4>Last 5 Attempts</h4>
      <ul id='lastDurations'>
        <li>-</li>
      </ul>
    </div>
    
    <div style='margin-top:2em;'>
      <a href='/config' style='display:inline-block;padding:0.8em 1.5em;background:grey;color:white;text-decoration:none;border-radius:0.3em;font-size:1.1em;margin-right:1em;'>WiFi Settings</a>
      <a href='/tournament-config' style='display:inline-block;padding:0.8em 1.5em;background:grey;color:white;text-decoration:none;border-radius:0.3em;font-size:1.1em;'>Tournament Settings</a>
    </div>
    
    <p style='color:gray;font-size:12px;'>Updates every 0.5 seconds</p>
    <p style='color:gray;font-size:12px;'>M5 Atom S3 Arrow Timer by norog242</p>
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
        // Placeholder: This will be replaced with actual API call to tournament system
        // For now, add some dummy users for testing
        const select = document.getElementById('username-select');
        select.innerHTML = '<option value="">-- Select Archer --</option>';
        
        // TODO: Replace with actual API call
        // fetch('/api/tournament/users').then(r => r.json()).then(users => {
        //   users.forEach(user => {
        //     const option = document.createElement('option');
        //     option.value = user.id;
        //     option.textContent = user.name;
        //     select.appendChild(option);
        //   });
        // });
        
        // Temporary placeholder users
        const dummyUsers = ['Archer 1', 'Archer 2', 'Archer 3'];
        dummyUsers.forEach(name => {
          const option = document.createElement('option');
          option.value = name;
          option.textContent = name;
          select.appendChild(option);
        });
      }
      
      function saveToTournament() {
        const username = document.getElementById('username-select').value;
        if (!username) {
          showTournamentMessage('Please select an archer', 'error');
          return;
        }
        
        const points = parseInt(document.getElementById('points').value) || 0;
        const durationText = document.getElementById('duration').textContent;
        const duration = parseFloat(durationText) || 0;
        const score = (duration > 0) ? (points / duration) : 0;
        const arrows = parseInt(document.getElementById('arrows').textContent) || 0;
        
        if (arrows !== 10) {
          showTournamentMessage('Please complete 10 arrows before saving to tournament', 'error');
          return;
        }
        
        const btn = document.getElementById('save-tournament-btn');
        btn.disabled = true;
        btn.textContent = 'Saving...';
        
        // Placeholder: This will be replaced with actual API call to tournament system
        // TODO: Replace with actual API endpoint
        /*
        const data = {
          username: username,
          tournamentId: document.getElementById('tournament-id-display').textContent,
          arrows: arrows,
          duration: duration,
          points: points,
          score: score
        };
        
        fetch('/api/tournament/save-result', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(data)
        }).then(r => r.json()).then(result => {
          if (result.success) {
            showTournamentMessage('Result saved to tournament successfully!', 'success');
          } else {
            showTournamentMessage('Error saving to tournament: ' + result.message, 'error');
          }
        }).catch(error => {
          showTournamentMessage('Error saving to tournament: ' + error, 'error');
        }).finally(() => {
          btn.disabled = false;
          btn.textContent = 'Save to Tournament';
        });
        */
        
        // Temporary simulation
        setTimeout(() => {
          showTournamentMessage('Tournament API not yet connected. Result would be saved for ' + username, 'success');
          btn.disabled = false;
          btn.textContent = 'Save to Tournament';
        }, 1000);
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
    <style>
      body { font-family: sans-serif; max-width: 600px; margin: 2em auto; padding: 1em; }
      h1 { color: #333; }
      .status-box { background: #f0f0f0; padding: 1em; border-radius: 0.5em; margin-bottom: 1.5em; }
      .status-box strong { display: block; margin-bottom: 0.5em; }
      .form-group { margin-bottom: 1.5em; }
      label { display: block; margin-bottom: 0.5em; font-weight: bold; }
      input[type='text'], input[type='password'] {
        width: 100%;
        padding: 0.8em;
        font-size: 1em;
        border: 1px solid #ccc;
        border-radius: 0.3em;
        box-sizing: border-box;
      }
      button {
        padding: 0.8em 2em;
        font-size: 1.1em;
        border: none;
        border-radius: 0.3em;
        cursor: pointer;
        margin-right: 1em;
        margin-top: 0.5em;
      }
      .btn-primary { background: #0066cc; color: white; }
      .btn-secondary { background: #666; color: white; }
      .btn-danger { background: #cc0000; color: white; }
      .message { padding: 1em; margin: 1em 0; border-radius: 0.3em; }
      .message.success { background: #d4edda; color: #155724; }
      .message.error { background: #f8d7da; color: #721c24; }
      .info { color: #666; font-size: 0.9em; margin-top: 0.5em; }
    </style>
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
    <h1>⚙️ WiFi Configuration</h1>
    
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
    
    <div style='margin-top: 2em; padding-top: 2em; border-top: 1px solid #ccc;'>
      <a href='/' style='display:inline-block;padding:0.8em 1.5em;background:#666;color:white;text-decoration:none;border-radius:0.3em;'>Back to Timer</a>
    </div>
    
    <div class='info' style='margin-top: 2em; padding: 1em; background: #fffbcc; border-radius: 0.3em;'>
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
    <style>
      body { font-family: sans-serif; max-width: 600px; margin: 2em auto; padding: 1em; }
      h1 { color: #333; }
      .status-box { background: #f0f0f0; padding: 1em; border-radius: 0.5em; margin-bottom: 1.5em; }
      .status-box strong { display: block; margin-bottom: 0.5em; }
      .form-group { margin-bottom: 1.5em; }
      label { display: block; margin-bottom: 0.5em; font-weight: bold; }
      input[type='number'] {
        width: 100%;
        padding: 0.8em;
        font-size: 1em;
        border: 1px solid #ccc;
        border-radius: 0.3em;
        box-sizing: border-box;
      }
      button {
        padding: 0.8em 2em;
        font-size: 1.1em;
        border: none;
        border-radius: 0.3em;
        cursor: pointer;
        margin-right: 1em;
        margin-top: 0.5em;
      }
      .btn-primary { background: #0066cc; color: white; }
      .btn-secondary { background: #666; color: white; }
      .btn-danger { background: #cc0000; color: white; }
      .message { padding: 1em; margin: 1em 0; border-radius: 0.3em; }
      .message.success { background: #d4edda; color: #155724; }
      .message.error { background: #f8d7da; color: #721c24; }
      .info { color: #666; font-size: 0.9em; margin-top: 0.5em; }
    </style>
    <script>
      function loadStatus() {
        fetch('/tournament-status').then(r => r.json()).then(d => {
          const statusText = d.tournamentId ? 'Tournament ID: ' + d.tournamentId : 'Not configured';
          document.getElementById('current-tournament').textContent = statusText;
          
          if (d.tournamentId) {
            document.getElementById('tournamentId').value = d.tournamentId;
          }
        }).catch(e => {
          console.error('Error loading status:', e);
          document.getElementById('current-tournament').textContent = 'Error loading status';
        });
      }
      
      function saveConfig(event) {
        event.preventDefault();
        const tournamentId = document.getElementById('tournamentId').value;
        
        if (!tournamentId) {
          showMessage('Please enter a tournament ID', 'error');
          return;
        }
        
        const formData = new URLSearchParams();
        formData.append('tournamentId', tournamentId);
        
        document.getElementById('save-btn').disabled = true;
        document.getElementById('save-btn').textContent = 'Saving...';
        
        fetch('/tournament-save', {
          method: 'POST',
          body: formData,
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
        })
        .then(response => response.text())
        .then(data => {
          showMessage('Tournament ID saved successfully!', 'success');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
          loadStatus();
        })
        .catch(error => {
          showMessage('Error saving tournament ID: ' + error, 'error');
          document.getElementById('save-btn').disabled = false;
          document.getElementById('save-btn').textContent = 'Save';
        });
      }
      
      function clearConfig() {
        if (!confirm('Are you sure you want to clear the tournament ID?')) {
          return;
        }
        
        fetch('/tournament-clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          showMessage('Tournament ID cleared successfully!', 'success');
          document.getElementById('tournamentId').value = '';
          loadStatus();
        })
        .catch(error => {
          showMessage('Error clearing tournament ID: ' + error, 'error');
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
        <label for='tournamentId'>Tournament ID:</label>
        <input type='number' id='tournamentId' name='tournamentId' placeholder='Enter tournament ID' min='0' step='1'>
        <div class='info'>Enter the numeric ID of your tournament</div>
      </div>
      
      <button type='submit' class='btn-primary' id='save-btn'>Save</button>
      <button type='button' class='btn-danger' onclick='clearConfig()'>Clear Tournament ID</button>
    </form>
    
    <div style='margin-top: 2em; padding-top: 2em; border-top: 1px solid #ccc;'>
      <a href='/' style='display:inline-block;padding:0.8em 1.5em;background:#666;color:white;text-decoration:none;border-radius:0.3em;'>Back to Timer</a>
    </div>
    
    <div class='info' style='margin-top: 2em; padding: 1em; background: #e8f4ff; border-radius: 0.3em;'>
      <strong>Note:</strong> The tournament ID is used to integrate this timer with your tournament management application. 
      Enter the numeric ID provided by your tournament system to link this device with your tournament.
      <br><br>
      The configuration is stored persistently and will be retained after device restart or power loss.
    </div>
    </body></html>
  )rawliteral";
  return html;
}
