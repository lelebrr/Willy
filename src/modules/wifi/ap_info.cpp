#include "ap_info.h"
#include "core/display.h"
#include "globals.h"
#include <WiFi.h>

void displayAPInfo(const wifi_ap_record_t &record) {
  // Otimizado: Validação de entrada
  if (!record.ssid[0] && record.primary == 0) {
    Serial.println("[APInfo] Registro AP inválido");
    return;
  }

  drawMainBorderWithTitle("INFORMACOES DO AP");
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.setTextSize(FM);

  char buf[64];
  padprintln("");

  // Otimizado: Sanitização de SSID
  String ssid = String((char *)record.ssid);
  if (ssid.length() == 0 || ssid.length() > 32) {
    ssid = "<SSID inválido>";
  }
  padprintln("SSID: " + ssid);

  // Otimizado: Formatação segura de BSSID
  if (snprintf(buf, sizeof(buf), "BSSID: %02X:%02X:%02X:%02X:%02X:%02X",
               record.bssid[0], record.bssid[1], record.bssid[2],
               record.bssid[3], record.bssid[4], record.bssid[5]) < 0) {
    padprintln("BSSID: <Erro de formatação>");
  } else {
    padprintln(buf);
  }

  // Otimizado: Validação de canal
  uint8_t channel = record.primary;
  if (channel < 1 || channel > 14) {
    padprintln("Canal: Inválido");
  } else {
    padprintln("Canal: " + String(channel));
  }

  // Otimizado: Validação de RSSI
  int rssi = record.rssi;
  if (rssi < -100 || rssi > 0) {
    padprintln("RSSI: Inválido");
  } else {
    padprintln("RSSI: " + String(rssi) + " dBm");
  }

  // Otimizado: Mapeamento de autenticação com validação
  String auth;
  switch (record.authmode) {
  case WIFI_AUTH_OPEN:
    auth = "Aberto";
    break;
  case WIFI_AUTH_WEP:
    auth = "WEP";
    break;
  case WIFI_AUTH_WPA_PSK:
    auth = "WPA/PSK";
    break;
  case WIFI_AUTH_WPA2_PSK:
    auth = "WPA2/PSK";
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:
    auth = "WPA/WPA2/PSK";
    break;
  case WIFI_AUTH_WPA2_ENTERPRISE:
    auth = "Enterprise";
    break;
  case WIFI_AUTH_WPA3_PSK:
    auth = "WPA3/PSK";
    break;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    auth = "WPA2/WPA3/PSK";
    break;
  default:
    auth = "Desconhecido (" + String((int)record.authmode) + ")";
    break;
  }
  padprintln("Seguranca: " + auth);

  // Otimizado: Informações adicionais se disponíveis
  if (record.second != WIFI_SECOND_CHAN_NONE) {
    padprintln("Canal Secundário: " + String(record.second));
  }

  padprintln("");
  padprintln("Pressione Sair para voltar");

  // Otimizado: Loop com timeout para evitar travamento
  uint32_t startTime = millis();
  const uint32_t TIMEOUT_MS = 30000; // 30 segundos

  while (!check(EscPress)) {
    if (millis() - startTime > TIMEOUT_MS) {
      Serial.println("[APInfo] Timeout ao aguardar saída");
      break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
