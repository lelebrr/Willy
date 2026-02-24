#include "ap_info.h"
#include <WiFi.h>
#include "core/display.h"
#include "globals.h"

void displayAPInfo(const wifi_ap_record_t &record) {
    drawMainBorderWithTitle("INFORMACOES DO AP");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FM);
    
    char buf[64];
    padprintln("");
    padprintln("SSID: " + String((char*)record.ssid));
    
    snprintf(buf, sizeof(buf), "BSSID: %02X:%02X:%02X:%02X:%02X:%02X", 
             record.bssid[0], record.bssid[1], record.bssid[2], 
             record.bssid[3], record.bssid[4], record.bssid[5]);
    padprintln(buf);
    
    padprintln("Canal: " + String(record.primary));
    padprintln("RSSI: " + String(record.rssi) + " dBm");
    
    String auth;
    switch (record.authmode) {
        case WIFI_AUTH_OPEN: auth = "Aberto"; break;
        case WIFI_AUTH_WEP: auth = "WEP"; break;
        case WIFI_AUTH_WPA_PSK: auth = "WPA/PSK"; break;
        case WIFI_AUTH_WPA2_PSK: auth = "WPA2/PSK"; break;
        case WIFI_AUTH_WPA_WPA2_PSK: auth = "WPA/WPA2/PSK"; break;
        case WIFI_AUTH_WPA2_ENTERPRISE: auth = "Enterprise"; break;
        default: auth = "Desconhecido"; break;
    }
    padprintln("Seguranca: " + auth);
    
    padprintln("");
    padprintln("Pressione Sair para voltar");
    
    while (!check(EscPress)) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
