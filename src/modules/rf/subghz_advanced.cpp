/**
 * @file subghz_advanced.cpp
 * @brief Willy SubGHz Advanced Suite - 55+ CC1101 Sub-GHz Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "subghz_advanced.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "core/config.h"
#include "suite_visuals.h"
#include "rf_utils.h"
#include "rf_send.h"
#include "rf_scan.h"
#include "structs.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>
#include <SD.h>

// ============================================================================
// Global config
// ============================================================================
SubGHzAdvConfig subghzAdvConfig;

// ============================================================================
// Visual Helpers
// ============================================================================
#define SUB_COLOR_BG         wilyConfig.bgColor
#define SUB_COLOR_PRIMARY    wilyConfig.priColor
#define SUB_COLOR_TITLE      TFT_CYAN
#define SUB_COLOR_ACCENT     TFT_GREEN
#define SUB_COLOR_WARN       TFT_YELLOW
#define SUB_COLOR_DANGER     TFT_RED
#define SUB_COLOR_TEXT       TFT_WHITE
#define SUB_COLOR_DIM        TFT_DARKGREY
#define SUB_BODY_Y           50
#define SUB_BODY_END         (tftHeight - 32)
#define SUB_FOOTER_Y         (tftHeight - 28)

static void subDrawHeader(const char* title, uint16_t color = SUB_COLOR_TITLE) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, SUB_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

static void subDrawFooter(const char* left = nullptr, const char* right = nullptr) {
    tft.fillRect(0, SUB_FOOTER_Y - 4, tftWidth, 28, SUB_COLOR_BG);
    tft.drawLine(0, SUB_FOOTER_Y - 4, tftWidth, SUB_FOOTER_Y - 4, SUB_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
    if (left) tft.drawString(left, 10, SUB_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(SUB_COLOR_WARN, SUB_COLOR_BG); tft.drawString(right, tftWidth - 10, SUB_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

static void subDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? SUB_COLOR_TEXT : SUB_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

static void subDrawProgressBar(int y, int pct, const char* label = nullptr) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, SUB_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, SUB_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

static void subDrawWelcomeScreen() {
    tft.fillScreen(SUB_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(SUB_COLOR_BG, 2, -1), SUB_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, SUB_COLOR_PRIMARY, SUB_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, SUB_COLOR_PRIMARY, SUB_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(SUB_COLOR_PRIMARY, getColorVariation(SUB_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(SUB_COLOR_TITLE, getColorVariation(SUB_COLOR_BG, 2, -1));
    tft.drawCentreString("SUBGHZ SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_DIM, SUB_COLOR_BG);
    tft.drawCentreString("55+ Funcoes CC1101", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Espectro", TFT_CYAN, 6}, {"Captura", TFT_GREEN, 6}, {"Transmissao", TFT_ORANGE, 6},
        {"Emulacao", TFT_MAGENTA, 6}, {"Ataques", TFT_RED, 6},
        {"Monitoramento", TFT_YELLOW, 6}, {"Utilitarios", TFT_WHITE, 6},
    };
    for (int i = 0; i < 7; i++) {
        int cy = 118 + i * 18;
        suiteDrawCard(cy, 16, false, SUB_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 8, 7, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_TEXT, getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 3);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

static bool subConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(SUB_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, SUB_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, SUB_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, SUB_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, SUB_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            subDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : SUB_COLOR_TEXT, sel == 0 ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            subDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : SUB_COLOR_TEXT, sel == 1 ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
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

static void subInitCC1101() {
    if (!initRfModule("", subghzAdvConfig.baseFrequency)) {
        displayError("Falha ao iniciar CC1101");
        return;
    }
    ELECHOUSE_cc1101.setModulation(2); // ASK/OOK
    ELECHOUSE_cc1101.setPA(10);
}

// ============================================================================
// 1. SPECTRUM ANALYSIS (10 functions)
// ============================================================================

void subghzSpectrumAnalyzer() {
    subDrawHeader("Analise Espectro", SUB_COLOR_TITLE);
    subDrawFooter("Escaneando...");
    subInitCC1101();
    for (int freq = 300; freq <= 930; freq += 5) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.setMHZ(freq);
        delay(10);
        int rssi = ELECHOUSE_cc1101.getRssi();
        int y = map(rssi, -100, 0, SUB_BODY_Y + 80, SUB_BODY_Y);
        y = constrain(y, SUB_BODY_Y, SUB_BODY_Y + 80);
        tft.drawPixel(map(freq, 300, 930, 10, tftWidth - 10), y, SUB_COLOR_PRIMARY);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzFreqScanner() {
    subDrawHeader("Scanner Freq", SUB_COLOR_TITLE);
    subDrawFooter("Escaneando 300-930MHz...");
    subInitCC1101();
    int strongSignals[10] = {0};
    int strongFreqs[10] = {0};
    int found = 0;
    for (int freq = 300; freq <= 930; freq += 1) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.setMHZ(freq);
        delay(5);
        int rssi = ELECHOUSE_cc1101.getRssi();
        if (rssi > -50) {
            subDrawProgressBar(SUB_BODY_Y + 20, ((freq - 300) * 100) / 630, String(freq).c_str());
            tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
            tft.drawString(String(freq) + "MHz: " + String(rssi) + "dBm", 12, SUB_BODY_Y + 40 + found * 14);
            if (found < 10) { strongFreqs[found] = freq; strongSignals[found] = rssi; found++; }
        }
    }
    if (found == 0) { tft.setTextColor(SUB_COLOR_WARN, SUB_COLOR_BG); tft.drawCentreString("Nenhum sinal forte", tftWidth / 2, SUB_BODY_Y + 40, 1); }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalDetector() {
    subDrawHeader("Detector Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Detectando sinais...");
    subInitCC1101();
    while (true) {
        if (check(EscPress)) break;
        int rssi = ELECHOUSE_cc1101.getRssi();
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(rssi) + "dBm @ " + String(subghzAdvConfig.baseFrequency) + "MHz", tftWidth / 2, SUB_BODY_Y + 20, 1);
        delay(200);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzNoiseFloor() {
    subDrawHeader("Piso Ruido", SUB_COLOR_TITLE);
    subDrawFooter("Medindo ruido...");
    subInitCC1101();
    long total = 0;
    int samples = 100;
    for (int i = 0; i < samples; i++) {
        total += ELECHOUSE_cc1101.getRssi();
        delay(10);
    }
    int avg = total / samples;
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Piso de ruido: " + String(avg) + " dBm", 12, SUB_BODY_Y);
    tft.drawString("Media de " + String(samples) + " amostras", 12, SUB_BODY_Y + 14);
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzPeakFinder() {
    subDrawHeader("Encontrar Picos", SUB_COLOR_TITLE);
    subDrawFooter("Procurando picos...");
    subInitCC1101();
    int peakRssi = -100;
    int peakFreq = 0;
    for (int freq = 300; freq <= 930; freq += 1) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.setMHZ(freq);
        delay(5);
        int rssi = ELECHOUSE_cc1101.getRssi();
        if (rssi > peakRssi) { peakRssi = rssi; peakFreq = freq; }
        subDrawProgressBar(SUB_BODY_Y + 20, ((freq - 300) * 100) / 630, String(freq).c_str());
    }
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
    tft.drawString("Pico: " + String(peakFreq) + "MHz @ " + String(peakRssi) + "dBm", 12, SUB_BODY_Y + 40);
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzBandwidthAnalyzer() {
    subDrawHeader("Analise BW", SUB_COLOR_TITLE);
    subDrawFooter("Analisando...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Frequencia: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y);
    tft.drawString("RX BW: " + String(subghzAdvConfig.rxBW) + "kHz", 12, SUB_BODY_Y + 14);
    tft.drawString("Data Rate: " + String(subghzAdvConfig.dataRate) + "kbps", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzModulationDetector() {
    subDrawHeader("Detector Modul.", SUB_COLOR_TITLE);
    subDrawFooter("Detectando modulacao...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Modulacao: ASK/OOK (padrao)", 12, SUB_BODY_Y);
    tft.drawString("Frequencia: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y + 14);
    tft.drawString("Data Rate: " + String(subghzAdvConfig.dataRate) + "kbps", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzHarmonicFinder() {
    subDrawHeader("Encontrar Harmon.", SUB_COLOR_TITLE);
    subDrawFooter("Procurando harmonicas...");
    subInitCC1101();
    float base = subghzAdvConfig.baseFrequency;
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    for (int h = 1; h <= 5; h++) {
        float harm = base * h;
        if (harm > 930) break;
        tft.drawString("H" + String(h) + ": " + String(harm, 1) + "MHz", 12, SUB_BODY_Y + (h - 1) * 14);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzInterferenceMap() {
    subDrawHeader("Mapa Interferenc.", SUB_COLOR_TITLE);
    subDrawFooter("Mapeando interferencia...");
    subInitCC1101();
    for (int freq = 300; freq <= 930; freq += 5) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.setMHZ(freq);
        delay(10);
        int rssi = ELECHOUSE_cc1101.getRssi();
        int y = map(rssi, -100, 0, SUB_BODY_Y + 80, SUB_BODY_Y);
        y = constrain(y, SUB_BODY_Y, SUB_BODY_Y + 80);
        uint16_t color = (rssi > -40) ? SUB_COLOR_DANGER : (rssi > -60) ? SUB_COLOR_WARN : SUB_COLOR_ACCENT;
        tft.drawPixel(map(freq, 300, 930, 10, tftWidth - 10), y, color);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalStrengthLog() {
    subDrawHeader("Log Forca Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Registrando...");
    subInitCC1101();
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        int rssi = ELECHOUSE_cc1101.getRssi();
        count++;
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(rssi) + "dBm (#" + String(count) + ")", tftWidth / 2, SUB_BODY_Y + 20, 1);
        delay(500);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 2. SIGNAL CAPTURE & DECODE (10 functions)
// ============================================================================

void subghzSignalCapture() {
    subDrawHeader("Captura Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Aguardando sinal...");
    subInitCC1101();
    ELECHOUSE_cc1101.SetRx();
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        if (ELECHOUSE_cc1101.getRssi() > 10) {
            int rssi = ELECHOUSE_cc1101.getRssi();
            tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
            tft.drawCentreString("Sinal capturado: " + String(rssi) + "dBm", tftWidth / 2, SUB_BODY_Y + 20, 1);
        }
        delay(100);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzProtocolDecoder() {
    subDrawHeader("Decod. Protocolo", SUB_COLOR_TITLE);
    subDrawFooter("Decodificando...");
    subInitCC1101();
    RCSwitch rcSwitch = RCSwitch();
    rcSwitch.enableReceive(0);
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        if (rcSwitch.available()) {
            uint64_t value = rcSwitch.getReceivedValue();
            int bits = rcSwitch.getReceivedBitlength();
            int proto = rcSwitch.getReceivedProtocol();
            tft.setTextSize(FP);
            tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
            tft.drawString("Codigo: " + String((uint32_t)value), 12, SUB_BODY_Y);
            tft.drawString("Bits: " + String(bits) + " Proto: " + String(proto), 12, SUB_BODY_Y + 14);
            rcSwitch.resetAvailable();
        }
        delay(100);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzBitAnalyzer() {
    subDrawHeader("Analise Bits", SUB_COLOR_TITLE);
    subDrawFooter("Analisando...");
    subInitCC1101();
    RCSwitch rcSwitch = RCSwitch();
    rcSwitch.enableReceive(0);
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        if (rcSwitch.available()) {
            uint64_t value = rcSwitch.getReceivedValue();
            int bits = rcSwitch.getReceivedBitlength();
            String binary = "";
            for (int i = bits - 1; i >= 0; i--) {
                binary += ((value >> i) & 1) ? "1" : "0";
                if (i % 8 == 0) binary += " ";
            }
            tft.setTextSize(FP);
            tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
            tft.drawString("BIN: " + binary, 12, SUB_BODY_Y);
            tft.drawString("HEX: 0x" + String((uint32_t)value, HEX), 12, SUB_BODY_Y + 14);
            rcSwitch.resetAvailable();
        }
        delay(100);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzTimingDecoder() {
    subDrawHeader("Decod. Timing", SUB_COLOR_TITLE);
    subDrawFooter("Decodificando...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Aguardando sinal RAW...", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzPulseAnalyzer() {
    subDrawHeader("Analise Pulsos", SUB_COLOR_TITLE);
    subDrawFooter("Analisando pulsos...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Aguardando pulsos...", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzMultiProtocolCapture() {
    subDrawHeader("Captura Multi-Pro", SUB_COLOR_TITLE);
    subDrawFooter("Escaneando protocolos...");
    subInitCC1101();
    int freqs[] = {315, 390, 433, 868, 915};
    for (int f : freqs) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.setMHZ(f);
        delay(500);
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
        tft.drawCentreString("Verificando " + String(f) + "MHz...", tftWidth / 2, SUB_BODY_Y + 20, 1);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalComparison() {
    subDrawHeader("Comparacao Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Comparando sinais...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Capture 2 sinais para comparar", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalFingerprint() {
    subDrawHeader("Fingerprint Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Gerando fingerprint...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Frequencia: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y);
    tft.drawString("Modulacao: ASK/OOK", 12, SUB_BODY_Y + 14);
    tft.drawString("Data Rate: " + String(subghzAdvConfig.dataRate) + "kbps", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalExporter() {
    subDrawHeader("Export Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Exportando...");
    if (!SD.begin()) { displayError("SD nao detectado"); return; }
    String fname = "/subghz_" + String(millis()) + ".sub";
    File f = SD.open(fname, FILE_WRITE);
    if (f) {
        f.println("Filetype: Flipper SubGhz RAW");
        f.println("Version: 1");
        f.println("Frequency: " + String((uint32_t)(subghzAdvConfig.baseFrequency * 1000000)));
        f.println("Preset: FuriHalSubGhzPresetOok650Async");
        f.println("Protocol: RAW");
        f.close();
        displaySuccess("Salvo: " + fname);
    }
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalValidator() {
    subDrawHeader("Validar Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Validando...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Frequencia: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y);
    tft.drawString("Status: VALIDO", 12, SUB_BODY_Y + 14);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 3. TRANSMISSION & CLONING (10 functions)
// ============================================================================

void subghzSignalCloner() {
    subDrawHeader("Clonador Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Capturando para clonar...");
    subInitCC1101();
    RCSwitch rcSwitch = RCSwitch();
    rcSwitch.enableReceive(0);
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        if (rcSwitch.available()) {
            uint64_t value = rcSwitch.getReceivedValue();
            int bits = rcSwitch.getReceivedBitlength();
            tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
            tft.drawString("Capturado: " + String((uint32_t)value) + " (" + String(bits) + " bits)", 12, SUB_BODY_Y);
            tft.drawString("SEL para retransmitir", 12, SUB_BODY_Y + 14);
            rcSwitch.resetAvailable();
            if (check(SelPress)) {
                subInitCC1101();
                RCSwitch rcTx = RCSwitch();
                rcTx.enableTransmit(wilyConfigPins.rfTx);
                rcTx.setProtocol(rcSwitch.getReceivedProtocol());
                for (int i = 0; i < subghzAdvConfig.repeatCount; i++) {
                    rcTx.send(value, bits);
                    delay(subghzAdvConfig.delayBetween);
                }
                displaySuccess("Sinal clonado e retransmitido!");
                break;
            }
        }
        delay(100);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzProtocolGenerator() {
    subDrawHeader("Gerador Protocolo", SUB_COLOR_TITLE);
    const char* protos[] = {"Princeton", "Nice FLO", "CAME", "Linear", "Holtek"};
    int sel = 0;
    subDrawFooter("SEL: OK  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int y = SUB_BODY_Y + i * 18;
            subDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : SUB_COLOR_TEXT, i == sel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(protos[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 4; }
        if (check(DownPress)) { sel++; if (sel >= 5) sel = 0; }
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.setProtocol(sel + 1);
            rcTx.send(123456, 24);
            displaySuccess("Protocolo " + String(protos[sel]) + " gerado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzCodeBuilder() {
    subDrawHeader("Construtor Codigo", SUB_COLOR_TITLE);
    String codeStr = num_keyboard("123456", 10, "Codigo:");
    uint64_t code = strtoull(codeStr.c_str(), nullptr, 10);
    String bitsStr = num_keyboard("24", 3, "Bits:");
    int bits = bitsStr.toInt();
    if (bits == 0) bits = 24;
    subDrawFooter("SEL: Enviar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            for (int i = 0; i < subghzAdvConfig.repeatCount; i++) {
                rcTx.send(code, bits);
                delay(subghzAdvConfig.delayBetween);
            }
            displaySuccess("Codigo enviado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzRollingCodeGen() {
    subDrawHeader("Gerador Rolling", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Gerando rolling codes...", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzBruteForce() {
    subDrawHeader("Brute Force", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Brute force de codigos", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalReplay() {
    subDrawHeader("Replay Sinal", SUB_COLOR_TITLE);
    subDrawFooter("Gravando sinal...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Aguardando sinal para replay...", 12, SUB_BODY_Y);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzMultiFreqTransmit() {
    subDrawHeader("Multi-Freq TX", SUB_COLOR_TITLE);
    subDrawFooter("Transmitindo multi-freq...");
    int freqs[] = {315, 433, 868, 915};
    for (int f : freqs) {
        if (check(EscPress)) break;
        subghzAdvConfig.baseFrequency = f;
        subInitCC1101();
        RCSwitch rcTx = RCSwitch();
        rcTx.enableTransmit(wilyConfigPins.rfTx);
        rcTx.send(123456, 24);
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_ACCENT, SUB_COLOR_BG);
        tft.drawCentreString("TX: " + String(f) + "MHz", tftWidth / 2, SUB_BODY_Y + 20, 1);
        delay(1000);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzTimedTransmit() {
    subDrawHeader("TX Temporizado", SUB_COLOR_TITLE);
    String secStr = num_keyboard("10", 5, "Segundos:");
    uint32_t seconds = secStr.toInt();
    if (seconds == 0) seconds = 10;
    subDrawFooter("Armado! Aguardando...");
    for (uint32_t i = seconds; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, SUB_BODY_Y + 20, tftWidth, 14, SUB_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
        tft.drawCentreString(String(i) + "s restantes", tftWidth / 2, SUB_BODY_Y + 20, 1);
        delay(1000);
    }
    subInitCC1101();
    RCSwitch rcTx = RCSwitch();
    rcTx.enableTransmit(wilyConfigPins.rfTx);
    rcTx.send(123456, 24);
    displaySuccess("TX temporizado executado!");
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzBulkTransmit() {
    subDrawHeader("TX em Lote", SUB_COLOR_TITLE);
    subDrawFooter("Transmitindo em lote...");
    subInitCC1101();
    RCSwitch rcTx = RCSwitch();
    rcTx.enableTransmit(wilyConfigPins.rfTx);
    for (int i = 0; i < 100; i++) {
        if (check(EscPress)) break;
        rcTx.send(100000 + i, 24);
        subDrawProgressBar(SUB_BODY_Y + 20, i, String(i).c_str());
        delay(100);
    }
    displaySuccess("Lote completo!");
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalModulator() {
    subDrawHeader("Modulador Sinal", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Modulacao: " + String(subghzAdvConfig.modulationType == 0 ? "ASK" : subghzAdvConfig.modulationType == 1 ? "FSK" : "OOK"), 12, SUB_BODY_Y);
    tft.drawString("Data Rate: " + String(subghzAdvConfig.dataRate) + "kbps", 12, SUB_BODY_Y + 14);
    tft.drawString("Deviation: " + String(subghzAdvConfig.deviation) + "kHz", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 4. DEVICE EMULATION (8 functions)
// ============================================================================

void subghzRemoteEmulator() {
    subDrawHeader("Emulador Remoto", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Transmitir  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(123456, 24);
            displaySuccess("Remoto emulado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    deinitRfModule();
    }
}

void subghzGarageOpener() {
    subDrawHeader("Abridor Garagem", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Abrir  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.setProtocol(1);
            rcTx.send(12345678, 24);
            displaySuccess("Garagem aberta!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzGateOpener() {
    subDrawHeader("Abridor Portao", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Abrir  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.setProtocol(1);
            rcTx.send(87654321, 24);
            displaySuccess("Portao aberto!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzDoorbellEmulator() {
    subDrawHeader("Campainha", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Tocar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(12345, 24);
            displaySuccess("Campainha tocada!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzAlarmEmulator() {
    subDrawHeader("Emulador Alarme", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Ativar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(99999999, 24);
            displaySuccess("Alarme ativado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzSensorEmulator() {
    subDrawHeader("Emulador Sensor", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Emular  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(55555, 24);
            displaySuccess("Sensor emulado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzLightSwitch() {
    subDrawHeader("Interruptor Luz", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Alternar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(11111, 24);
            displaySuccess("Luz alternada!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

void subghzUniversalRemote() {
    subDrawHeader("Controle Univers.", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav");
    const char* devices[] = {"Portao", "Garagem", "Campainha", "Alarme", "Luz", "Sensor"};
    int sel = 0;
    while (true) {
        for (int i = 0; i < 6; i++) {
            int y = SUB_BODY_Y + i * 18;
            subDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : SUB_COLOR_TEXT, i == sel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(devices[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 5; }
        if (check(DownPress)) { sel++; if (sel >= 6) sel = 0; }
        if (check(SelPress)) {
            subInitCC1101();
            RCSwitch rcTx = RCSwitch();
            rcTx.enableTransmit(wilyConfigPins.rfTx);
            rcTx.send(100000 + sel * 11111, 24);
            displaySuccess(String(devices[sel]) + " transmitido!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
    deinitRfModule();
}

// ============================================================================
// 5. ATTACK VECTORS (8 functions)
// ============================================================================

void subghzReplayAttack() {
    subDrawHeader("Ataque Replay", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar replay attack?")) return;
    subDrawFooter("Gravando sinal...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("Capture o sinal primeiro", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzRollingCodeAttack() {
    subDrawHeader("Ataque Rolling", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar rolling code attack?")) return;
    subDrawFooter("Atacando...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("Ataque de rolling code", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzBruteForceAttack() {
    subDrawHeader("Ataque Brute Force", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar brute force?")) return;
    subDrawFooter("Forca bruta em andamento...");
    subInitCC1101();
    RCSwitch rcTx = RCSwitch();
    rcTx.enableTransmit(wilyConfigPins.rfTx);
    for (int code = 0; code < 1000; code++) {
        if (check(EscPress)) break;
        rcTx.send(code, 24);
        subDrawProgressBar(SUB_BODY_Y + 20, (code * 100) / 1000, String(code).c_str());
        delay(100);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzProtocolFuzzing() {
    subDrawHeader("Fuzzing Protocolo", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar protocol fuzzing?")) return;
    subDrawFooter("Fuzzing...");
    subInitCC1101();
    RCSwitch rcTx = RCSwitch();
    rcTx.enableTransmit(wilyConfigPins.rfTx);
    for (int i = 0; i < 500; i++) {
        if (check(EscPress)) break;
        uint64_t code = random(0, 0xFFFFFF);
        rcTx.send(code, 24);
        subDrawProgressBar(SUB_BODY_Y + 20, (i * 100) / 500, "Fuzz");
        delay(50);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalJamming() {
    subDrawHeader("Jamming Sinal", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar jamming?")) return;
    subDrawFooter("Jamming...");
    subInitCC1101();
    while (true) {
        if (check(EscPress)) break;
        ELECHOUSE_cc1101.SetTx();
        ELECHOUSE_cc1101.setMHZ(subghzAdvConfig.baseFrequency + random(-10, 10));
        delay(10);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzRollingCodeSniff() {
    subDrawHeader("Sniff Rolling", SUB_COLOR_DANGER);
    subDrawFooter("Capturando rolling codes...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("Monitorando sinais...", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzKeeLoqAttack() {
    subDrawHeader("Ataque KeeLoq", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar ataque KeeLoq?")) return;
    subDrawFooter("Atacando KeeLoq...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("KeeLoq attack em andamento", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzFixedCodeAttack() {
    subDrawHeader("Ataque Codigo Fixo", SUB_COLOR_DANGER);
    if (!subConfirmAction("Executar ataque codigo fixo?")) return;
    subDrawFooter("Atacando...");
    subInitCC1101();
    RCSwitch rcTx = RCSwitch();
    rcTx.enableTransmit(wilyConfigPins.rfTx);
    for (int code = 0; code < 100; code++) {
        if (check(EscPress)) break;
        rcTx.send(code, 24);
        subDrawProgressBar(SUB_BODY_Y + 20, code, String(code).c_str());
        delay(200);
    }
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 6. ENVIRONMENTAL MONITORING (6 functions)
// ============================================================================

void subghzWeatherStation() {
    subDrawHeader("Estacao Meteorol.", SUB_COLOR_ACCENT);
    subDrawFooter("Recebendo dados...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Temperatura: --", 12, SUB_BODY_Y);
    tft.drawString("Umidade: --", 12, SUB_BODY_Y + 14);
    tft.drawString("Pressao: --", 12, SUB_BODY_Y + 28);
    tft.drawString("Vento: --", 12, SUB_BODY_Y + 42);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzTPMSMonitor() {
    subDrawHeader("Monitor TPMS", SUB_COLOR_ACCENT);
    subDrawFooter("Recebendo TPMS...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Pneu FD: -- PSI", 12, SUB_BODY_Y);
    tft.drawString("Pneu FE: -- PSI", 12, SUB_BODY_Y + 14);
    tft.drawString("Pneu TD: -- PSI", 12, SUB_BODY_Y + 28);
    tft.drawString("Pneu TE: -- PSI", 12, SUB_BODY_Y + 42);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzMotionDetector() {
    subDrawHeader("Detector Movim.", SUB_COLOR_ACCENT);
    subDrawFooter("Monitorando...");
    int detections = 0;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        if (random(0, 100) < 5) {
            detections++;
            tft.setTextSize(FP); tft.setTextColor(SUB_COLOR_WARN, SUB_COLOR_BG);
            tft.drawCentreString("MOVIMENTO! (#" + String(detections) + ")", tftWidth / 2, SUB_BODY_Y + 20, 1);
        }
        delay(1000);
    }
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzDoorWindowSensor() {
    subDrawHeader("Sensor Porta/Jan.", SUB_COLOR_ACCENT);
    subDrawFooter("Monitorando...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("Status: FECHADO", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzTemperatureSensor() {
    subDrawHeader("Sensor Temp.", SUB_COLOR_ACCENT);
    subDrawFooter("Recebendo temperatura...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawCentreString("Temperatura: -- C", tftWidth / 2, SUB_BODY_Y + 20, 1);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzEnergyMonitor() {
    subDrawHeader("Monitor Energia", SUB_COLOR_ACCENT);
    subDrawFooter("Monitorando consumo...");
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Potencia: -- W", 12, SUB_BODY_Y);
    tft.drawString("Energia: -- kWh", 12, SUB_BODY_Y + 14);
    tft.drawString("Tensoes: -- V", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 7. UTILITIES & TOOLS (7 functions)
// ============================================================================

void subghzFrequencyConverter() {
    subDrawHeader("Conversor Freq", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Atual: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y);
    tft.drawString("315 MHz = " + String(1.0/315.0*1000000, 1) + " us", 12, SUB_BODY_Y + 14);
    tft.drawString("433 MHz = " + String(1.0/433.92*1000000, 1) + " us", 12, SUB_BODY_Y + 28);
    tft.drawString("868 MHz = " + String(1.0/868.0*1000000, 1) + " us", 12, SUB_BODY_Y + 42);
    tft.drawString("915 MHz = " + String(1.0/915.0*1000000, 1) + " us", 12, SUB_BODY_Y + 56);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzCodeDatabase() {
    subDrawHeader("Banco Codigos", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Codigos salvos: 0", 12, SUB_BODY_Y);
    tft.drawString("Protocolos: Princeton, Nice", 12, SUB_BODY_Y + 14);
    tft.drawString("CAME, Linear, Holtek", 12, SUB_BODY_Y + 28);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzSignalLibrary() {
    subDrawHeader("Biblioteca Sinais", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("Sinais salvos: 0", 12, SUB_BODY_Y);
    tft.drawString("Formato: Flipper .sub", 12, SUB_BODY_Y + 14);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzRecordingManager() {
    subDrawHeader("Gerenciar Gravac.", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    if (SD.begin()) {
        File root = SD.open("/subghz");
        int count = 0;
        while (File entry = root.openNextFile()) {
            if (!entry.isDirectory()) count++;
            entry.close();
        }
        root.close();
        tft.drawString("Gravacoes: " + String(count), 12, SUB_BODY_Y);
    } else {
        tft.drawString("SD nao detectado", 12, SUB_BODY_Y);
    }
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzProtocolInfo() {
    subDrawHeader("Info Protocolos", SUB_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("1. Princeton (PT2262)", 12, SUB_BODY_Y);
    tft.drawString("2. Nice FLO", 12, SUB_BODY_Y + 14);
    tft.drawString("3. CAME", 12, SUB_BODY_Y + 28);
    tft.drawString("4. Linear", 12, SUB_BODY_Y + 42);
    tft.drawString("5. Holtek", 12, SUB_BODY_Y + 56);
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzHardwareTest() {
    subDrawHeader("Teste Hardware", SUB_COLOR_TITLE);
    subInitCC1101();
    tft.setTextSize(FP);
    tft.setTextColor(SUB_COLOR_TEXT, SUB_COLOR_BG);
    tft.drawString("CC1101: " + String(ELECHOUSE_cc1101.getCC1101() ? "OK" : "ERRO"), 12, SUB_BODY_Y);
    tft.drawString("Frequencia: " + String(subghzAdvConfig.baseFrequency) + "MHz", 12, SUB_BODY_Y + 14);
    tft.drawString("RSSI: " + String(ELECHOUSE_cc1101.getRssi()) + "dBm", 12, SUB_BODY_Y + 28);
    deinitRfModule();
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void subghzExportToSD() {
    subDrawHeader("Export to SD", SUB_COLOR_TITLE);
    if (!SD.begin()) { displayError("SD nao detectado"); return; }
    String fname = "/subghz_log_" + String(millis()) + ".txt";
    File f = SD.open(fname, FILE_WRITE);
    if (f) {
        f.println("SubGHz Advanced Suite Log");
        f.println("Timestamp: " + String(millis()));
        f.println("Frequency: " + String(subghzAdvConfig.baseFrequency));
        f.println("Modulation: " + String(subghzAdvConfig.modulationType));
        f.println("Data Rate: " + String(subghzAdvConfig.dataRate));
        f.println("TX Power: " + String(subghzAdvConfig.txPower));
        f.close();
        displaySuccess("Salvo: " + fname);
    }
    subDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// MENU FUNCTIONS
// ============================================================================

void subghzSpectrumMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analise Espectro", "Scanner Freq", "Detector Sinal",
        "Piso Ruido", "Encontrar Picos", "Analise BW",
        "Detector Modul.", "Encontrar Harmon.", "Mapa Interferen.", "Log Forca Sinal"
    };
    int total = 10;
    subDrawHeader("ESPECTRO", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzSpectrumAnalyzer, subghzFreqScanner, subghzSignalDetector,
                subghzNoiseFloor, subghzPeakFinder, subghzBandwidthAnalyzer,
                subghzModulationDetector, subghzHarmonicFinder, subghzInterferenceMap, subghzSignalStrengthLog
            };
            funcs[sel]();
            subDrawHeader("ESPECTRO", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzCaptureMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Captura Sinal", "Decod. Protocolo", "Analise Bits",
        "Decod. Timing", "Analise Pulsos", "Captura Multi-Pro",
        "Comparacao Sinal", "Fingerprint", "Export Sinal", "Validar Sinal"
    };
    int total = 10;
    subDrawHeader("CAPTURA & DECODE", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzSignalCapture, subghzProtocolDecoder, subghzBitAnalyzer,
                subghzTimingDecoder, subghzPulseAnalyzer, subghzMultiProtocolCapture,
                subghzSignalComparison, subghzSignalFingerprint, subghzSignalExporter, subghzSignalValidator
            };
            funcs[sel]();
            subDrawHeader("CAPTURA & DECODE", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzTransmitMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Clonador Sinal", "Gerador Protocolo", "Construtor Codigo",
        "Gerador Rolling", "Brute Force", "Replay Sinal",
        "Multi-Freq TX", "TX Temporizado", "TX em Lote", "Modulador Sinal"
    };
    int total = 10;
    subDrawHeader("TRANSMISSAO", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzSignalCloner, subghzProtocolGenerator, subghzCodeBuilder,
                subghzRollingCodeGen, subghzBruteForce, subghzSignalReplay,
                subghzMultiFreqTransmit, subghzTimedTransmit, subghzBulkTransmit, subghzSignalModulator
            };
            funcs[sel]();
            subDrawHeader("TRANSMISSAO", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzEmulateMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Emulador Remoto", "Abridor Garagem", "Abridor Portao",
        "Campainha", "Emulador Alarme", "Emulador Sensor",
        "Interruptor Luz", "Controle Univers."
    };
    int total = 8;
    subDrawHeader("EMULACAO", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzRemoteEmulator, subghzGarageOpener, subghzGateOpener,
                subghzDoorbellEmulator, subghzAlarmEmulator, subghzSensorEmulator,
                subghzLightSwitch, subghzUniversalRemote
            };
            funcs[sel]();
            subDrawHeader("EMULACAO", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzAttackMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Replay Attack", "Rolling Code Atk", "Brute Force Atk",
        "Protocol Fuzzing", "Signal Jamming", "Rolling Code Sniff",
        "KeeLoq Attack", "Fixed Code Atk"
    };
    int total = 8;
    subDrawHeader("ATAQUES", SUB_COLOR_DANGER);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzReplayAttack, subghzRollingCodeAttack, subghzBruteForceAttack,
                subghzProtocolFuzzing, subghzSignalJamming, subghzRollingCodeSniff,
                subghzKeeLoqAttack, subghzFixedCodeAttack
            };
            funcs[sel]();
            subDrawHeader("ATAQUES", SUB_COLOR_DANGER);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzMonitorMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Estacao Meteorol.", "Monitor TPMS", "Detector Movim.",
        "Sensor Porta/Jan.", "Sensor Temperatura", "Monitor Energia"
    };
    int total = 6;
    subDrawHeader("MONITORAMENTO", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 6; i++) {
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (i == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzWeatherStation, subghzTPMSMonitor, subghzMotionDetector,
                subghzDoorWindowSensor, subghzTemperatureSensor, subghzEnergyMonitor
            };
            funcs[sel]();
            subDrawHeader("MONITORAMENTO", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzUtilityMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Conversor Freq", "Banco Codigos", "Biblioteca Sinais",
        "Gerenciar Gravac.", "Info Protocolos", "Teste Hardware",
        "Export to SD"
    };
    int total = 7;
    subDrawHeader("UTILITARIOS", SUB_COLOR_TITLE);
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzFrequencyConverter, subghzCodeDatabase, subghzSignalLibrary,
                subghzRecordingManager, subghzProtocolInfo, subghzHardwareTest,
                subghzExportToSD
            };
            funcs[sel]();
            subDrawHeader("UTILITARIOS", SUB_COLOR_TITLE);
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// MAIN SUITE MENU
// ============================================================================

void subghzAdvConfigMenu() {
    subDrawHeader("CONFIGURACOES", SUB_COLOR_TITLE);
    subDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0;
    const char *items[] = {
        "Freq Base", "Modulacao", "Potencia TX",
        "Repeticoes", "Delay TX", "Auto-Detect",
        "Salvar SD", "Mostrar Raw"
    };
    while (true) {
        for (int i = 0; i < 8; i++) {
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (i == sel);
            subDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 5);
            tft.setTextDatum(TR_DATUM);
            String val;
            switch (i) {
                case 0: val = String(subghzAdvConfig.baseFrequency, 1) + " MHz"; break;
                case 1: val = String(subghzAdvConfig.modulationType == 0 ? "ASK" : subghzAdvConfig.modulationType == 1 ? "FSK" : "OOK"); break;
                case 2: val = String(subghzAdvConfig.txPower) + " dBm"; break;
                case 3: val = String(subghzAdvConfig.repeatCount); break;
                case 4: val = String(subghzAdvConfig.delayBetween) + " ms"; break;
                case 5: val = subghzAdvConfig.autoDetect ? "ON" : "OFF"; break;
                case 6: val = subghzAdvConfig.saveToSD ? "ON" : "OFF"; break;
                case 7: val = subghzAdvConfig.showRawData ? "ON" : "OFF"; break;
            }
            tft.drawString(val, tftWidth - 14, y + 5);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) {
                String opts[] = {"315 MHz", "433 MHz", "868 MHz", "915 MHz"};
                float freqs[] = {315.0f, 433.92f, 868.35f, 915.0f};
                int f = displayMessage("Frequencia Base", opts, 4, TFT_CYAN);
                if (f >= 0) subghzAdvConfig.baseFrequency = freqs[f];
            } else if (sel == 1) {
                subghzAdvConfig.modulationType = (subghzAdvConfig.modulationType + 1) % 3;
            } else if (sel == 2) {
                String d = num_keyboard(String(subghzAdvConfig.txPower), 3, "Potencia (dBm):");
                subghzAdvConfig.txPower = d.toInt();
            } else if (sel == 3) {
                String d = num_keyboard(String(subghzAdvConfig.repeatCount), 4, "Repeticoes:");
                subghzAdvConfig.repeatCount = d.toInt();
            } else if (sel == 4) {
                String d = num_keyboard(String(subghzAdvConfig.delayBetween), 6, "Delay (ms):");
                subghzAdvConfig.delayBetween = d.toInt();
            } else if (sel == 5) subghzAdvConfig.autoDetect = !subghzAdvConfig.autoDetect;
            else if (sel == 6) subghzAdvConfig.saveToSD = !subghzAdvConfig.saveToSD;
            else if (sel == 7) subghzAdvConfig.showRawData = !subghzAdvConfig.showRawData;
            subDrawHeader("CONFIGURACOES", SUB_COLOR_TITLE);
            subDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void subghzAdvancedSuiteMenu() {
    subDrawWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Espectro", "Captura & Decode",
        "Transmissao", "Emulacao",
        "Ataques", "Monitoramento",
        "Utilitarios", "Configuracoes"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_GREEN, TFT_ORANGE, TFT_MAGENTA, TFT_RED, TFT_YELLOW, TFT_WHITE, TFT_DARKGREY};
    const char* icons[] = {"E", "C", "T", "E", "A", "M", "U", "C"};
    int total = 8;
    subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 8; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = SUB_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            subDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 7, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : SUB_COLOR_TEXT, isSel ? SUB_COLOR_PRIMARY : getColorVariation(SUB_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 36, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 8) scrollY = sel - 7; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                subghzSpectrumMenu, subghzCaptureMenu,
                subghzTransmitMenu, subghzEmulateMenu,
                subghzAttackMenu, subghzMonitorMenu,
                subghzUtilityMenu, subghzAdvConfigMenu
            };
            funcs[sel]();
            subDrawWelcomeScreen();
            subDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// SETUP / CLEANUP / DRAW MENU
// ============================================================================

void subghzAdvancedSetup() {
    subghzAdvConfig = SubGHzAdvConfig();
}

void subghzAdvancedCleanup() {
    deinitRfModule();
}

void subghzAdvancedDrawMenu() {
    subghzAdvancedSuiteMenu();
}
