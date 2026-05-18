#include "web_server.h"
#include <Arduino.h>
#include <WebServer.h>
#include <webfont/digital7_woff.h>

static int* g_arrows = nullptr;
static float* g_duration = nullptr;
static int* g_points = nullptr;
static float* g_score = nullptr;
static float* g_lastDurations = nullptr;
static int* g_lastPoints = nullptr;
static int* g_lastDurationsCount = nullptr;

String getHtml();

void updateWebData(int arrows, float duration, int points, float score, float* lastDurations, int* lastPoints, int lastDurationsCount) {
    g_arrows = &arrows;
    g_duration = &duration;
    g_points = &points;
    g_score = &score;
    g_lastDurations = lastDurations;
    g_lastPoints = lastPoints;
    g_lastDurationsCount = &lastDurationsCount;
}

void setupWebServer(WebServer& server) {
    server.on("/digital7.woff", HTTP_GET, [&]() {
        server.sendHeader("Content-Type", "font/woff");
        server.send_P(200, "font/woff", (const char*)digital7_woff, digital7_woff_len);
    });
    server.on("/common.css", HTTP_GET, [&]() {
        server.send(200, "text/css", getCommonCss());
    });
    server.on("/", [&]() {
        server.send(200, "text/html", getHtml());
    });
    server.on("/config", [&]() {
        server.send(200, "text/html", getConfigHtml());
    });
    server.on("/device-settings", [&]() {
        server.send(200, "text/html", getDeviceSettingsHtml());
    });
    server.on("/tournament-config", [&]() {
        server.send(200, "text/html", getTournamentConfigHtml());
    });
}

// The getHtml() function will be moved here from main.cpp
