#include "wifi_heatmap.h"
#include <WiFi.h>
#include "core/display.h"
#include "globals.h"

void wifiHeatmap() {
    drawMainBorderWithTitle("WIFI HEATMAP");
    displayTextLine("Escaneando APs...");
    
    int n = WiFi.scanNetworks();
    if (n == 0) {
        displayTextLine("Nenhum AP encontrado.");
    } else {
        tft.fillScreen(bruceConfig.bgColor);
        drawMainBorderWithTitle("WIFI HEATMAP");
        for (int i = 0; i < n && i < 8; ++i) {
            int rssi = WiFi.RSSI(i);
            uint16_t color = (rssi > -50) ? TFT_GREEN : (rssi > -70) ? TFT_YELLOW : TFT_RED;
            
            tft.setCursor(10, 30 + (i * 20));
            tft.setTextColor(color, bruceConfig.bgColor);
            tft.printf("%s: %d dBm", WiFi.SSID(i).c_str(), rssi);
            
            // Draw a simple bar
            int barWidth = map(rssi, -100, -30, 0, 100);
            tft.fillRect(120, 35 + (i * 20), barWidth, 10, color);
        }
    }
    
    while (!check(EscPress)) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void encryptedTrafficFingerprint() {
    drawMainBorderWithTitle("TRAFFIC FINGERPRINT");
    displayTextLine("Monitorando trafego...");
    // Stub implementation: Just wait for Esc
    while (!check(EscPress)) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
