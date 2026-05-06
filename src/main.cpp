// Auto-rotation state
int lastRotation = -1;

// Determine rotation based on accelerometer data
void updateScreenRotation() {
  float accX, accY, accZ;
  M5.Imu.getAccelData(&accX, &accY, &accZ);
  int rotation = 0;
  if (fabs(accX) > fabs(accY)) {
    if (accX > 0.5) rotation = 1; // left
    else if (accX < -0.5) rotation = 3; // right
    else rotation = 0; // default
  } else {
    if (accY > 0.5) rotation = 2; // upside down
    else rotation = 0; // default
  }
  if (rotation != lastRotation) {
    M5.Lcd.setRotation(rotation);
    lastRotation = rotation;
  }
}

// M5 Atom S3 Step Counter/Timer Conversion from micro:bit

#include <M5AtomS3.h>
#include <math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <webfont/digital7_woff.h>


int arrows = 0;
unsigned long startTime = 0;
float duration = 0;
bool finished = false;

// WiFi and Web server setup
const char* ssid = "ArrowTimerAP";
const char* password = "12345678";
WebServer server(80);

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
    <form action='/reset' method='POST'><button style='font-size:1.5em;padding:0.5em 2em;margin:1em;' type='submit'>Reset</button></form>
    <p style='color:gray;font-size:12px;'>Updates every 0.5 seconds</p>
    <p style='color:gray;font-size:12px;'>M5 Atom S3 Arrow Timer by norog242</p>
    </body></html>
  )rawliteral";
  return html;
}

void reset() {
  arrows = 0;
  startTime = 0;
  duration = 0;
  finished = false;
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear();
  // Show 'Arrow' in size 3
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Arrow");
  // Show value in size 6 below
  M5.Lcd.setTextSize(6);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("%d", arrows);
  M5.Lcd.setTextSize(2); // Restore default size
  delay(500);//wait half a second to avoid false detection of the first step when resetting.
}

void setup() {
    // Serve the font file for offline use
    server.on("/digital7.woff", HTTP_GET, []() {
      server.sendHeader("Content-Type", "font/woff");
      server.send_P(200, "font/woff", (const char*)digital7_woff, digital7_woff_len);
    });
  M5.begin();
  M5.Lcd.setTextSize(2);
  reset();

  // WiFi Access Point setup
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("AP: %s\nIP: %s", ssid, IP.toString().c_str());
  M5.Lcd.setTextSize(2);

  // Web server route
  server.on("/", []() {
    server.send(200, "text/html", getHtml());
  });
  server.on("/data", HTTP_GET, []() {
    float currentDuration = duration;
    if (!finished && arrows > 0) {
      currentDuration = (millis() - startTime) / 1000.0;
    }
    String json = "{";
    json += "\"arrows\":" + String(arrows) + ",";
    json += "\"duration\":" + String(currentDuration, 2);
    json += "}";
    server.send(200, "application/json", json);
  });
  server.on("/reset", HTTP_POST, []() {
    reset();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.begin();
}

void loop() {

  M5.update();
  updateScreenRotation();
  server.handleClient();

  // Button A pressed (AtomS3 has only one button)
  if (M5.BtnA.wasPressed()) {
    reset();
  }

  if (!finished) {
    float accX, accY, accZ;
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    float strength = sqrt(accX * accX + accY * accY + accZ * accZ) * 1000; // Convert to milli-g

    if (arrows < 10) {
      if (strength > 1200) {
        arrows++;
        if (arrows == 1) {
          startTime = millis();
        }
        if (arrows == 10) {
          duration = (millis() - startTime) / 1000.0;
          finished = true;
          M5.Lcd.clear();
          M5.Lcd.setTextSize(3);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("Time");
          M5.Lcd.setTextSize(6);
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.printf("%.2f", duration);
          M5.Lcd.setTextSize(2); // Restore default size
        } else {
          M5.Lcd.clear();
          M5.Lcd.setTextSize(4);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("Arrow");
          M5.Lcd.setTextSize(6);
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.printf("%d", arrows);
          M5.Lcd.setTextSize(2); // Restore default size
        }
        delay(350); // Debounce for impact detection
      }
    }
  } else {
    // After 10 arrows, just show the result, keep server responsive
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Time");
    M5.Lcd.setTextSize(6);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("%.2f", duration);
    M5.Lcd.setTextSize(2);
    // Wait for reset, but do not block
  }
  delay(20); // Main loop delay
}