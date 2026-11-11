#include "wifi_heatmap.h"
#include "core/display.h"
#include "globals.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>

static int packetCount = 0;

// Callback for promiscuous mode
void promiscuousCallback(void *buf, wifi_promiscuous_pkt_type_t type) {
  packetCount++;
}

void wifiHeatmap() {
  drawMainBorderWithTitle("WIFI HEATMAP");
  displayTextLine("Escaneando APs...");

  int n = WiFi.scanNetworks();
  if (n == 0) {
    displayTextLine("Nenhum AP encontrado.");
  } else {
    tft.fillScreen(willyConfig.bgColor);
    drawMainBorderWithTitle("WIFI HEATMAP");
    for (int i = 0; i < n && i < 8; ++i) {
      int rssi = WiFi.RSSI(i);
      uint16_t color = (rssi > -50)   ? TFT_GREEN
                       : (rssi > -70) ? TFT_YELLOW
                                      : TFT_RED;

      tft.setCursor(10, 30 + (i * 20));
      tft.setTextColor(color, willyConfig.bgColor);
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
  displayTextLine("Monitorando trafego encriptado...");

  // Enable promiscuous mode
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuousCallback);

  int packetCount = 0;
  unsigned long startTime = millis();

  while (!check(EscPress)) {
    tft.fillScreen(willyConfig.bgColor);
    drawMainBorderWithTitle("TRAFFIC FINGERPRINT");
    tft.setCursor(10, 30);
    tft.printf("Pacotes capturados: %d", packetCount);
    tft.setCursor(10, 50);
    tft.printf("Tempo: %lu s", (millis() - startTime) / 1000);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  esp_wifi_set_promiscuous(false);
  displayTextLine("Fingerprinting concluido.");
}
