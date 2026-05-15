#pragma once
#include <WebServer.h>

void setupWebServer(WebServer& server);
String getHtml();
String getConfigHtml();
String getTournamentConfigHtml();
void updateWebData(int arrows, float duration, int points, float score, float* lastDurations, int* lastPoints, int lastDurationsCount);
