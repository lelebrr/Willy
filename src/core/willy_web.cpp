#include "willy_web.h"
#include <ArduinoJson.h>
#include <SD.h>
#include <WiFi.h>
#include "config.h"
#include "utils.h"
#include "willy_logger.h"

extern BruceConfig bruceConfig;

void setupWillyWeb(AsyncWebServer* server) {
    // Serve assets from SD card if they exist
    server->serveStatic("/Willy/", SD, "/WillyWebUI/");

    // Live device status endpoint (JSON)
    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<512> doc;
        doc["battery"] = 0;
        doc["uptime"] = millis() / 1000;
        doc["free_heap"] = ESP.getFreeHeap();
        doc["cpu_freq"] = ESP.getCpuFreqMHz();
        doc["wifi_rssi"] = WiFi.RSSI();
        doc["ip"] = WiFi.localIP().toString();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Device control endpoint
    server->on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Rebooting...");
        delay(500);
        ESP.restart();
    });

    // Log retrieval endpoint (last portion of the current CSV log)
    server->on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        String logFile = willyLogger.getCurrentLogFile();
        File file = SD.open(logFile, FILE_READ);
        if (!file) {
            request->send(404, "text/plain", "No logs found at " + logFile);
            return;
        }

        // Return only last portion if large
        String tail = "";
        if (file.size() > 5000) {
            file.seek(file.size() - 5000);
        }
        while (file.available()) {
            tail += (char)file.read();
        }
        file.close();
        request->send(200, "text/plain", tail);
    });
}
