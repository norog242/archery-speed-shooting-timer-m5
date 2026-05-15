#include <M5AtomS3.h>
#include <math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
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
  // Shift and add new duration and points to the end
  if (lastDurationsCount < MAX_ATTEMPTS) {
    lastDurations[lastDurationsCount] = d;
    lastDurationsCount++;
  } else {
    for (int i = 1; i < MAX_ATTEMPTS; ++i) {
      lastDurations[i-1] = lastDurations[i];
      lastPoints[i-1] = lastPoints[i];
    }
    lastDurations[MAX_ATTEMPTS-1] = d;
  }
  // points and score will be set in reset handler
}
// Auto-rotation state
int lastRotation = -1;

// WiFi and Web server setup
const char* ap_ssid = "ArrowTimerAP";
const char* ap_password = "12345678";
const char* mdns_hostname = "arrowtimer";
WebServer server(80);
Preferences preferences;

// WiFi mode tracking
bool isStationMode = false;
String currentSSID = "";

// Tournament ID and API domain
int tournamentId = -1;  // -1 means not set
String apiDomain = "archers.himmelix.ch";  // Default API domain

// Button long press for WiFi reset
unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;
bool wifiResetInProgress = false;
const unsigned long WIFI_RESET_DURATION = 10000; // 10 seconds



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

bool connectToWiFi(const String& ssid, const String& password) {
  if (ssid.length() == 0) return false;
  
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Connecting to:\n%s", ssid.c_str());
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    M5.Lcd.printf(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    isStationMode = true;
    currentSSID = ssid;
    IPAddress IP = WiFi.localIP();
    
    // Start mDNS
    if (MDNS.begin(mdns_hostname)) {
      MDNS.addService("http", "tcp", 80);
    }
    
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("WiFi: %s\nIP: %s\n%s.local", ssid.c_str(), IP.toString().c_str(), mdns_hostname);
    M5.Lcd.setTextSize(2);
    return true;
  }
  return false;
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("AP: %s\nIP: %s", ap_ssid, IP.toString().c_str());
  M5.Lcd.setTextSize(2);
  isStationMode = false;
  currentSSID = ap_ssid;
}

void setup() {
    // Setup static web server routes
    setupWebServer(server);
  M5.begin();
  M5.Lcd.setBrightness(100); 
  M5.Lcd.setTextSize(2);
  reset();

  // Initialize preferences
  preferences.begin("wifi-config", false);
  
  // Try to load and connect to saved WiFi
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  
  // Load tournament ID and API domain
  tournamentId = preferences.getInt("tournamentId", -1);
  apiDomain = preferences.getString("apiDomain", "archers.himmelix.ch");
  
  bool connected = false;
  if (savedSSID.length() > 0) {
    connected = connectToWiFi(savedSSID, savedPassword);
  }
  
  // Fall back to AP mode if connection failed
  if (!connected) {
    startAPMode();
  }

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
  server.on("/wifi-config", HTTP_POST, []() {
    if (server.hasArg("ssid")) {
      String newSSID = server.arg("ssid");
      String newPassword = server.arg("password");
      
      // Save to preferences
      preferences.putString("ssid", newSSID);
      preferences.putString("password", newPassword);
      
      server.send(200, "text/plain", "WiFi configuration saved. Device will restart...");
      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Missing SSID");
    }
  });
  
  server.on("/wifi-status", HTTP_GET, []() {
    String json = "{";
    json += "\"mode\":\"" + String(isStationMode ? "station" : "ap") + "\",";
    json += "\"ssid\":\"" + currentSSID + "\",";
    json += "\"ip\":\"" + (isStationMode ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "\",";
    json += "\"hostname\":\"" + String(mdns_hostname) + ".local\"";
    json += "}";
    server.send(200, "application/json", json);
  });
  
  server.on("/wifi-clear", HTTP_POST, []() {
    preferences.remove("ssid");
    preferences.remove("password");
    server.send(200, "text/plain", "WiFi configuration cleared. Device will restart in AP mode...");
    delay(1000);
    ESP.restart();
  });
  
  // Tournament endpoints
  server.on("/tournament-status", HTTP_GET, []() {
    String json = "{";
    if (tournamentId >= 0) {
      json += "\"tournamentId\":" + String(tournamentId);
    } else {
      json += "\"tournamentId\":null";
    }
    json += ",\"apiDomain\":\"" + apiDomain + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });
  
  server.on("/tournament-save", HTTP_POST, []() {
    if (server.hasArg("tournamentId")) {
      tournamentId = server.arg("tournamentId").toInt();
      
      // Save API domain if provided
      if (server.hasArg("apiDomain")) {
        apiDomain = server.arg("apiDomain");
        if (apiDomain.length() == 0) {
          apiDomain = "archers.himmelix.ch";  // Default if empty
        }
        preferences.putString("apiDomain", apiDomain);
      }
      
      // Save tournament ID to preferences
      preferences.putInt("tournamentId", tournamentId);
      
      server.send(200, "text/plain", "Tournament configuration saved successfully");
    } else {
      server.send(400, "text/plain", "Missing tournament ID");
    }
  });
  
  server.on("/tournament-clear", HTTP_POST, []() {
    tournamentId = -1;
    apiDomain = "archers.himmelix.ch";
    preferences.remove("tournamentId");
    preferences.remove("apiDomain");
    server.send(200, "text/plain", "Tournament configuration cleared successfully");
  });
  
  server.on("/tournament-participants", HTTP_GET, []() {
    // Check if tournament ID is configured
    if (tournamentId < 0) {
      server.send(400, "application/json", "{\"error\":\"Tournament ID not configured\"}");
      return;
    }
    
    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
      server.send(503, "application/json", "{\"error\":\"WiFi not connected\"}");
      return;
    }
    
    HTTPClient http;
    String url = "https://" + apiDomain + "/api/v1/tournaments/" + String(tournamentId) + "/participants";
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer paBQQVhY4soHrzBkJRwD6tBpMvTgGtYhSWT1keV82EC89vbXwWwVq7Vh11SWDDSb");
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        server.send(200, "application/json", payload);
      } else {
        String error = "{\"error\":\"API returned status code " + String(httpCode) + "\"}";
        server.send(httpCode, "application/json", error);
      }
    } else {
      String error = "{\"error\":\"Failed to connect to API: " + http.errorToString(httpCode) + "\"}";
      server.send(500, "application/json", error);
    }
    
    http.end();
  });
  
  server.on("/tournament-save-result", HTTP_POST, []() {
    // Check if tournament ID is configured
    if (tournamentId < 0) {
      server.send(400, "application/json", "{\"error\":\"Tournament ID not configured\"}");
      return;
    }
    
    // Check if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
      server.send(503, "application/json", "{\"error\":\"WiFi not connected\"}");
      return;
    }
    
    // Parse request parameters
    if (!server.hasArg("participantId") || !server.hasArg("points") || !server.hasArg("duration")) {
      server.send(400, "application/json", "{\"error\":\"Missing required parameters\"}");
      return;
    }
    
    int participantId = server.arg("participantId").toInt();
    int rawScore = server.arg("points").toInt();
    float timeInSeconds = server.arg("duration").toFloat();
    
    // Build JSON payload
    String jsonPayload = "{";
    jsonPayload += "\"tournament_participant_id\":" + String(participantId) + ",";
    jsonPayload += "\"raw_score\":" + String(rawScore) + ",";
    jsonPayload += "\"time_in_seconds\":" + String(timeInSeconds, 2);
    jsonPayload += "}";
    
    HTTPClient http;
    String url = "https://" + apiDomain + "/api/v1/tournaments/" + String(tournamentId) + "/speed-attempts";
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer paBQQVhY4soHrzBkJRwD6tBpMvTgGtYhSWT1keV82EC89vbXwWwVq7Vh11SWDDSb");
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(jsonPayload);
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String payload = http.getString();
        server.send(200, "application/json", payload);
      } else {
        String error = "{\"error\":\"API returned status code " + String(httpCode) + "\"}";
        server.send(httpCode, "application/json", error);
      }
    } else {
      String error = "{\"error\":\"Failed to connect to API: " + http.errorToString(httpCode) + "\"}";
      server.send(500, "application/json", error);
    }
    
    http.end();
  });
  
  server.on("/reset", HTTP_POST, []() {
    static unsigned long lastResetTime = 0;
    unsigned long now = millis();
    if (now - lastResetTime < 500) {
      server.send(429, "text/plain", "Debounced");
      return;
    }
    lastResetTime = now;
    int inputPoints = points;
    float inputScore = 0.0;
    if (server.hasArg("points")) {
      inputPoints = server.arg("points").toInt();
    }
    if (server.hasArg("score")) {
      inputScore = server.arg("score").toFloat();
    } else if (duration > 0) {
      inputScore = ((float)inputPoints / duration);
    }
    // Only store if finished (after 10 arrows)
    if (finished && duration > 0) {
      if (lastDurationsCount < MAX_ATTEMPTS) {
        lastDurations[lastDurationsCount] = duration;
        lastPoints[lastDurationsCount] = inputPoints;
        lastDurationsCount++;
      } else {
        for (int i = 1; i < MAX_ATTEMPTS; ++i) {
          lastDurations[i-1] = lastDurations[i];
          lastPoints[i-1] = lastPoints[i];
        }
        lastDurations[MAX_ATTEMPTS-1] = duration;
        lastPoints[MAX_ATTEMPTS-1] = inputPoints;
      }
    }
    points = inputPoints;
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

  // Check for button long press to reset WiFi credentials
  if (M5.BtnA.isPressed()) {
    if (!buttonPressed) {
      // Button just pressed
      buttonPressed = true;
      buttonPressStartTime = millis();
      wifiResetInProgress = false;
    } else {
      // Button is being held
      unsigned long pressDuration = millis() - buttonPressStartTime;
      
      if (pressDuration >= WIFI_RESET_DURATION && !wifiResetInProgress) {
        // 10 seconds reached - reset WiFi
        wifiResetInProgress = true;
        M5.Lcd.clear();
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.printf("WiFi Reset!\n\nClearing\nconfig...");
        
        // Clear WiFi credentials
        preferences.remove("ssid");
        preferences.remove("password");
        
        delay(1000);
        M5.Lcd.clear();
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.printf("Restarting\nin AP mode...");
        delay(1000);
        ESP.restart();
      } else if (pressDuration >= 3000 && pressDuration < WIFI_RESET_DURATION) {
        // Show progress indicator after 3 seconds
        if (!wifiResetInProgress && (pressDuration / 500) % 2 == 0) {
          M5.Lcd.clear();
          M5.Lcd.setTextSize(2);
          M5.Lcd.setCursor(0, 20);
          M5.Lcd.printf("Hold for\nWiFi reset\n\n%d sec", (WIFI_RESET_DURATION - pressDuration) / 1000);
        }
      }
    }
  } else {
    if (buttonPressed) {
      // Button released
      unsigned long pressDuration = millis() - buttonPressStartTime;
      buttonPressed = false;
      
      // Only trigger normal reset if press was short (< 3 seconds) and not during a run
      if (pressDuration < 3000 && !wifiResetInProgress) {
        reset();
      } else if (pressDuration >= 3000 && !wifiResetInProgress) {
        // Long press released before 10 seconds - restore display
        M5.Lcd.clear();
        if (finished) {
          M5.Lcd.setTextSize(3);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("Time");
          M5.Lcd.setTextSize(6);
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.printf("%.2f", duration);
          M5.Lcd.setTextSize(2);
        } else {
          M5.Lcd.setTextSize(3);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("Arrow");
          M5.Lcd.setTextSize(6);
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.printf("%d", arrows);
          M5.Lcd.setTextSize(2);
        }
        // Restore WiFi info at bottom
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.setTextSize(1);
        if (isStationMode) {
          M5.Lcd.printf("WiFi: %s\nIP: %s\n%s.local", currentSSID.c_str(), WiFi.localIP().toString().c_str(), mdns_hostname);
        } else {
          M5.Lcd.printf("AP: %s\nIP: %s", ap_ssid, WiFi.softAPIP().toString().c_str());
        }
        M5.Lcd.setTextSize(2);
      }
    }
  }

  // Button A pressed (AtomS3 has only one button) - now handled above
  // Commenting out old button handling
  // if (M5.BtnA.wasPressed()) {
  //   reset();
  // }

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
          // Do not store attempt yet, wait for points input and reset
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
    // After 10 arrows, do nothing until reset (no repeated LCD updates)
    // Wait for reset, but do not block
  }
  delay(20); // Main loop delay
}

