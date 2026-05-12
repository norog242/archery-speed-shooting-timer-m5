#include <M5AtomS3.h>
#include <math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "web_server.h"


int arrows = 0;
int points = 0;
unsigned long startTime = 0;
float duration = 0;
bool finished = false;
// Store last 5 durations
#define MAX_ATTEMPTS 5
float lastDurations[MAX_ATTEMPTS] = {0};
int lastPoints[MAX_ATTEMPTS] = {0};
int lastDurationsCount = 0;

void addDuration(float d) {
  // Shift and add new duration to the end
  if (lastDurationsCount < MAX_ATTEMPTS) {
    lastDurations[lastDurationsCount] = d;
    lastPoints[lastDurationsCount] = points;
    lastDurationsCount++;
  } else {
    for (int i = 1; i < MAX_ATTEMPTS; ++i) {
      lastDurations[i-1] = lastDurations[i];
      lastPoints[i-1] = lastPoints[i];
    }
    lastDurations[MAX_ATTEMPTS-1] = d;
    lastPoints[MAX_ATTEMPTS-1] = points;
  }
}
// Auto-rotation state
int lastRotation = -1;

// WiFi and Web server setup
const char* ssid = "ArrowTimerAP";
const char* password = "12345678";
WebServer server(80);



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
    if (accY > 0.5) rotation = 0; // upside down
    else rotation = 2; // default
  }
  if (rotation != lastRotation) {
    M5.Lcd.setRotation(rotation);
    lastRotation = rotation;
  }
}

void setup() {
    // Setup static web server routes
    setupWebServer(server);
  M5.begin();
  M5.Lcd.setBrightness(100); 
  M5.Lcd.setTextSize(2);
  reset();

  // WiFi Access Point setup
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("AP: %s\nIP: %s", ssid, IP.toString().c_str());
  M5.Lcd.setTextSize(2);

  // ...static routes handled in web_server.cpp...
  server.on("/data", HTTP_GET, []() {
    float currentDuration = duration;
    if (!finished && arrows > 0) {
      currentDuration = (millis() - startTime) / 1000.0;
    }
    float score = (currentDuration > 0) ? ((float)points / currentDuration) : 0.0;
    String json = "{";
    json += "\"arrows\":" + String(arrows) + ",";
    json += "\"duration\":" + String(currentDuration, 2) + ",";
    json += "\"points\":" + String(points) + ",";
    json += "\"score\":" + String(score, 2) + ",";
    json += "\"lastDurations\":[";
    for (int i = 0; i < lastDurationsCount; ++i) {
      float d = lastDurations[i];
      int p = lastPoints[i];
      float s = (d > 0) ? ((float)p / d) : 0.0;
      json += "{\"duration\":" + String(d, 2) + ",\"points\":" + String(p) + ",\"score\":" + String(s, 2) + "}";
      if (i < lastDurationsCount - 1) json += ",";
    }
    json += "]}";
    server.send(200, "application/json", json);
  });
  server.on("/reset", HTTP_POST, []() {
    if (server.hasArg("points")) {
      points = server.arg("points").toInt();
    }
    reset();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.begin();
}

void loop() {

  M5.update();
  if (arrows < 10) {
    updateScreenRotation();
  }
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
          addDuration(duration);
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

