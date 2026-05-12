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
        font-size: 4em;
        color: #000000;
        letter-spacing: 0.1em;
        background: #fcfcfc;
        padding: 0.2em 0.5em;
        border-radius: 0.2em;
        display: inline-block;
        margin: 0.2em 0;
      }
    </style>
    <script>
      function updateData() {
        fetch('/data').then(r => r.json()).then(d => {
          document.getElementById('arrows').textContent = d.arrows;
          document.getElementById('duration').textContent = d.duration.toFixed(2) + ' s';
          document.getElementById('score').textContent = d.score.toFixed(2) + ' pts/s';
          document.getElementById('points').value = d.points;
            updateResetBtnLabel(d.arrows);
        });
      }
      setInterval(updateData, 500);
      window.onload = updateData;
    </script>
    </head><body style='font-family:sans-serif;text-align:center;'>
    <h2>Arrow Count:</h2>
    <div id='arrows' class='segment'>-</div>
    <h2>Time:</h2>
    <div id='duration' class='segment'>-</div>
    <h2>Score:</h2>
    <div id='score' class='segment'>-</div>
    <form id='pointsForm' action='/reset' method='POST' style='margin-bottom:1em;display:flex;align-items:center;justify-content:center;gap:8px;'>
      <label for='points' style='font-size:1.2em;'>Points:</label>
      <button type='button' class='points-btn' onclick='changePoints(-10)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>-10</button>
      <button type='button' class='points-btn' onclick='changePoints(-1)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>-1</button>
      <input type='number' id='points' name='points' min='0' value='0' style='font-size:1.2em;width:5em;margin:0 8px;'>
      <button type='button' class='points-btn' onclick='changePoints(1)' style='font-size:1.5em;padding:0.5em 1em;margin-right:8px;'>+1</button>
      <button type='button' class='points-btn' onclick='changePoints(10)' style='font-size:1.5em;padding:0.5em 1em;'>+10</button>
      <button id='resetBtn' style='font-size:1.5em;padding:0.5em 2em;margin-left:16px;' type='submit'>Reset</button>
    </form>
    <script>
      function changePoints(delta) {
        const input = document.getElementById('points');
        let val = parseInt(input.value) || 0;
        val += delta;
        if (val < 0) val = 0;
        input.value = val;
      }
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
      setInterval(updateData, 500);
      window.onload = updateData;
    </script>
    </body></html>
  )rawliteral";
  return html;
}
