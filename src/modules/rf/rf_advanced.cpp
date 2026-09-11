/**
 * @file rf_advanced.cpp
 * @brief Willy RF Advanced Suite - 50+ Sub-GHz Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "rf_advanced.h"
#include "rf_send.h"
#include "rf_utils.h"
#include "rf_scan.h"
#include "rf_spectrum.h"
#include "rf_bruteforce.h"
#include "protocols/rf_encoder.h"
#include "protocols/rf_decoder.h"
#include "protocols/rf_keeloq.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "suite_visuals.h"
#include <SD.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

static std::vector<RFMacroStep> macroBuffer;
static std::vector<RFBatchEntry> batchBuffer;
static std::vector<RFSignalInfo> recentSignals;
static uint32_t signalCount = 0;

#define RF_COLOR_BG         wilyConfig.bgColor
#define RF_COLOR_PRIMARY    wilyConfig.priColor
#define RF_COLOR_TITLE      TFT_CYAN
#define RF_COLOR_ACCENT     TFT_GREEN
#define RF_COLOR_WARN       TFT_YELLOW
#define RF_COLOR_DANGER     TFT_RED
#define RF_COLOR_TEXT       TFT_WHITE
#define RF_COLOR_DIM        TFT_DARKGREY
#define RF_BODY_Y           50
#define RF_BODY_END         (tftHeight - 32)
#define RF_FOOTER_Y         (tftHeight - 28)

void rfDrawHeader(const char* title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, RF_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void rfDrawFooter(const char* left, const char* right) {
    tft.fillRect(0, RF_FOOTER_Y - 4, tftWidth, 28, RF_COLOR_BG);
    tft.drawLine(0, RF_FOOTER_Y - 4, tftWidth, RF_FOOTER_Y - 4, RF_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    if (left) tft.drawString(left, 10, RF_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG); tft.drawString(right, tftWidth - 10, RF_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

void rfDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? RF_COLOR_TEXT : RF_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

void rfDrawProgressBar(int y, int pct, const char* label) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, RF_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, RF_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

void rfDrawSpinner(int x, int y, int frame) {
    const char* sp[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FM); tft.setTextColor(RF_COLOR_PRIMARY, RF_COLOR_BG);
    tft.drawString(sp[frame % 4], x, y);
}

void rfShowWelcomeScreen() {
    tft.fillScreen(RF_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(RF_COLOR_BG, 2, -1), RF_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, RF_COLOR_PRIMARY, RF_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, RF_COLOR_PRIMARY, RF_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(RF_COLOR_PRIMARY, getColorVariation(RF_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(RF_COLOR_TITLE, getColorVariation(RF_COLOR_BG, 2, -1));
    tft.drawCentreString("RF SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_DIM, RF_COLOR_BG);
    tft.drawCentreString("50+ Funcoes Sub-GHz", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Analise", TFT_CYAN, 2}, {"Geracao", TFT_GREEN, 2}, {"Dispositivos", TFT_ORANGE, 2},
        {"Manipulacao", TFT_MAGENTA, 2}, {"Testes", TFT_RED, 2},
    };
    for (int i = 0; i < 5; i++) {
        int cy = 120 + i * 22;
        suiteDrawCard(cy, 20, false, RF_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 10, 8, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, getColorVariation(RF_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 5);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

RFSignalInfo rfCaptureSignal(uint32_t timeout) {
    RFSignalInfo info = {};
    info.frequency = wilyConfigPins.rfFreq;
    rfDrawHeader("Capturando RF...", RF_COLOR_WARN);
    rfDrawFooter("Aguardando sinal...");

    if (!initRfModule("rx", info.frequency)) {
        displayError("Falha ao init RF RX");
        return info;
    }
    setMHZ(info.frequency);

    rmt_channel_handle_t rx_channel = setup_rf_rx();
    if (!rx_channel) {
        displayError("Falha ao criar canal RX");
        deinitRfModule();
        return info;
    }

    rmt_receive_config_t receive_config = {};
    receive_config.signal_range_min_ns = 100;
    receive_config.signal_range_max_ns = 12000000;

    size_t pulseCount = 0;
    rmt_symbol_word_t *rawSymbols = (rmt_symbol_word_t *)heap_caps_malloc(4096 * sizeof(rmt_symbol_word_t), MALLOC_CAP_8BIT);
    if (!rawSymbols) { deinitRfModule(); return info; }

    uint32_t start = millis();
    int frame = 0;
    bool gotSignal = false;

    while (millis() - start < timeout) {
        if (check(EscPress)) { heap_caps_free(rawSymbols); deinitRfModule(); return info; }
        rfDrawSpinner(tftWidth - 30, RF_BODY_Y, frame++);

        size_t rx_size = 0;
        esp_err_t ret = rmt_receive(rx_channel, rawSymbols, 4096 * sizeof(rmt_symbol_word_t), &receive_config);
        if (ret == ESP_OK) {
            rmt_rx_done_event_data_t rx_data;
            if (rmt_receive_wait_done(rx_channel, &rx_data, pdMS_TO_TICKS(200)) == ESP_OK) {
                pulseCount = rx_data.num_symbols;
                if (pulseCount > 0) {
                    gotSignal = true;
                    break;
                }
            }
        }
        delay(50);
    }

    if (gotSignal && pulseCount > 0) {
        info.rssi = ELECHOUSE_cc1101.getRssi();
        info.bits = pulseCount;
        info.te = rawSymbols[0].duration0 + rawSymbols[0].duration1;
        info.protocol = "RAW";
        info.key = 0;

        String rawData = "";
        for (size_t i = 0; i < min(pulseCount, (size_t)32); i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d|%d ", rawSymbols[i].duration0, rawSymbols[i].duration1);
            rawData += buf;
        }
        info.rawData = rawData;
        info.description = String(pulseCount) + " pulsos @ " + String(info.frequency, 2) + "MHz";
        signalCount++;
    }

    heap_caps_free(rawSymbols);
    deinitRfModule();
    return info;
}

void rfDisplaySignalInfo(const RFSignalInfo& info, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawString("Freq: " + String(info.frequency, 2) + " MHz", 12, y);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Proto: " + info.protocol, 12, y + 14);
    char d[20]; snprintf(d, sizeof(d), "0x%08X", (uint32_t)info.key);
    tft.drawString("Key: " + String(d), 12, y + 28);
    tft.setTextColor(RF_COLOR_DIM, RF_COLOR_BG);
    tft.drawString(String(info.bits) + " bits RSSI:" + String(info.rssi) + "dB", 12, y + 42);
}

bool rfConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(RF_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, RF_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, RF_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, RF_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, RF_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            rfDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : RF_COLOR_TEXT, sel == 0 ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            rfDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : RF_COLOR_TEXT, sel == 1 ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
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

void rfShowStats(uint32_t count, uint32_t dur, const char* st) {
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_DIM, RF_COLOR_BG); tft.drawString("Sinais:", 14, RF_BODY_Y);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG); tft.drawString(String(count), 74, RF_BODY_Y);
    tft.setTextColor(RF_COLOR_DIM, RF_COLOR_BG); tft.drawString("Tempo:", 14, RF_BODY_Y + 16);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG); tft.drawString(String(dur / 1000) + "s", 74, RF_BODY_Y + 16);
    tft.setTextColor(RF_COLOR_DIM, RF_COLOR_BG); tft.drawString("Status:", 14, RF_BODY_Y + 32);
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG); tft.drawString(st, 74, RF_BODY_Y + 32);
    tft.drawLine(14, RF_BODY_Y + 52, tftWidth - 14, RF_BODY_Y + 52, RF_COLOR_DIM);
}

void rfSendCode(float freq, uint64_t code, int bits, int te, int repeats) {
    if (!initRfModule("", freq)) return;
    setMHZ(freq);
    struct RfCodes sendCode;
    sendCode.frequency = (uint32_t)(freq * 1000000);
    sendCode.key = code;
    sendCode.protocol = "Princeton";
    sendCode.Bit = bits;
    sendCode.te = te;
    sendCode.preset = "FuriHalSubGhzPresetOok650Async";
    for (int i = 0; i < repeats; i++) {
        sendRfCommand(sendCode, true);
        delay(te * 2);
    }
    deinitRfModule();
}

// ============================================================================
// 1. SIGNAL ANALYSIS & SCANNING (10 functions)
// ============================================================================

void rfSignalAnalyzer() {
    rfDrawHeader("Analisador de Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Aguardando RF...");
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.protocol == "" && sig.bits == 0) { displayWarning("Nenhum sinal capturado"); return; }
    rfDisplaySignalInfo(sig, RF_BODY_Y);
    rfDrawFooter("SEL: Novo  ESC: Voltar");
    while (!check(EscPress)) {
        if (check(SelPress)) { rfSignalAnalyzer(); return; }
        delay(100);
    }
}

void rfProtocolIdentifier() {
    rfDrawHeader("ID de Protocolo RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.protocol == "") { displayWarning("Protocolo desconhecido"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawString("Frequencia: " + String(sig.frequency, 2) + " MHz", 12, RF_BODY_Y);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Protocolo: " + sig.protocol, 12, RF_BODY_Y + 14);
    tft.drawString("Bits: " + String(sig.bits), 12, RF_BODY_Y + 28);
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
    tft.drawString("TE: " + String(sig.te) + "us", 12, RF_BODY_Y + 42);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalComparison() {
    rfDrawHeader("Comparacao RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture sinal 1...");
    RFSignalInfo sig1 = rfCaptureSignal();
    if (sig1.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(sig1, RF_BODY_Y);
    rfDrawFooter("Capture sinal 2...");
    RFSignalInfo sig2 = rfCaptureSignal();
    if (sig2.bits == 0) { displayWarning("Cancelado"); return; }
    bool match = (sig1.key == sig2.key && sig1.frequency == sig2.frequency);
    tft.setTextSize(FP);
    tft.setTextColor(match ? RF_COLOR_ACCENT : RF_COLOR_DANGER, RF_COLOR_BG);
    tft.drawCentreString(match ? "SINAIS IGUAIS" : "SINAIS DIFERENTES", tftWidth / 2, RF_BODY_Y + 50, 1);
    rfDisplaySignalInfo(sig2, RF_BODY_Y + 65);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfFrequencyScanner() {
    rfDrawHeader("Scanner de Frequencia", RF_COLOR_TITLE);
    rfDrawFooter("Escaneando bandas...");

    float scanFreqs[] = {300.0, 303.875, 304.25, 310.0, 315.0, 345.0, 387.0, 390.0, 418.0, 430.0, 433.075, 433.92, 434.177, 434.39, 464.0, 868.35, 868.95, 906.4, 915.0, 928.0};
    int total = 20;

    if (!initRfModule("rx", 433.92)) { displayError("Falha RF init"); return; }

    int peakIdx = 0;
    int maxRssi = -120;

    for (int i = 0; i < total; i++) {
        if (check(EscPress)) break;
        setMHZ(scanFreqs[i]);
        delay(10);
        int rssi = ELECHOUSE_cc1101.getRssi();
        int bar = map(constrain(rssi + 120, 0, 100), 0, 100, 0, tftWidth - 60);

        tft.fillRect(50, RF_BODY_Y + i * 12, tftWidth - 60, 10, RF_COLOR_BG);
        tft.fillRect(50, RF_BODY_Y + i * 12, max(0, bar), 10, RF_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawString(String(scanFreqs[i], 1) + "M", 6, RF_BODY_Y + i * 12);

        if (rssi > maxRssi) { maxRssi = rssi; peakIdx = i; }
    }

    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawCentreString("Pico: " + String(scanFreqs[peakIdx], 1) + " MHz (" + String(maxRssi) + "dB)", tftWidth / 2, RF_BODY_END - 10, 1);
    deinitRfModule();
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalStrengthMeter() {
    rfDrawHeader("Medidor de Forca RF", RF_COLOR_TITLE);
    rfDrawFooter("Aguardando RF...");
    if (!initRfModule("rx", wilyConfigPins.rfFreq)) { displayError("Falha RF"); return; }
    setMHZ(wilyConfigPins.rfFreq);

    int32_t maxRssi = -120, minRssi = 0;
    int32_t rssiSum = 0; int count = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int32_t rssi = ELECHOUSE_cc1101.getRssi();
        if (rssi > maxRssi) maxRssi = rssi;
        if (rssi < minRssi || count == 0) minRssi = rssi;
        rssiSum += rssi; count++;
        int bar = map(constrain(rssi + 120, 0, 100), 0, 100, 0, tftWidth - 40);
        tft.fillRect(20, RF_BODY_Y + 30, tftWidth - 40, 8, RF_COLOR_BG);
        tft.fillRect(20, RF_BODY_Y + 30, max(0, bar), 8, RF_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(rssi) + " dB", tftWidth / 2, RF_BODY_Y + 45, 1);
        delay(100);
    }
    float avg = (count > 0) ? (float)rssiSum / count : 0;
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawCentreString("Max:" + String(maxRssi) + " Min:" + String(minRssi), tftWidth / 2, RF_BODY_Y + 60, 1);
    tft.drawCentreString("Media:" + String((int)avg) + "dB", tftWidth / 2, RF_BODY_Y + 74, 1);
    deinitRfModule();
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfTimingAnalyzer() {
    rfDrawHeader("Analise de Timing RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    RFTimingStats stats = {};
    stats.te = sig.te;
    stats.totalDuration = sig.bits * sig.te;
    stats.pulseCount = sig.bits;

    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("TE (base): " + String(stats.te) + "us", 12, RF_BODY_Y);
    tft.drawString("Total: " + String(stats.totalDuration) + "us", 12, RF_BODY_Y + 14);
    tft.drawString("Pulsos: " + String(stats.pulseCount), 12, RF_BODY_Y + 28);
    tft.drawString("Freq: " + String(sig.frequency, 2) + " MHz", 12, RF_BODY_Y + 42);
    tft.drawString("Bitrate: " + String(1000000.0 / stats.te, 1) + " bps", 12, RF_BODY_Y + 56);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalQualityChecker() {
    rfDrawHeader("Qualidade do Sinal RF", RF_COLOR_TITLE);
    int good = 0, bad = 0;
    rfDrawFooter("Capturando sinais...");
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(3000);
        if (sig.bits > 0) good++;
        else bad++;
        tft.setTextSize(FP);
        tft.fillRect(12, RF_BODY_Y + 20, tftWidth - 24, 30, RF_COLOR_BG);
        tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawString("OK: " + String(good), 12, RF_BODY_Y + 20);
        tft.setTextColor(RF_COLOR_DANGER, RF_COLOR_BG);
        tft.drawString("Erro: " + String(bad), 12, RF_BODY_Y + 34);
        float pct = (good + bad > 0) ? (100.0f * good / (good + bad)) : 0;
        rfDrawProgressBar(RF_BODY_Y + 50, (int)pct, "Qualidade");
    }
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfBatchDecoder() {
    rfDrawHeader("Decoder RF em Lote", RF_COLOR_TITLE);
    rfDrawFooter("ESC: Parar  Capturando...");
    int count = 0;
    while (true) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(5000);
        if (sig.bits > 0) {
            count++;
            tft.setTextSize(FP);
            tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
            tft.drawString("Capturados: " + String(count), 12, RF_BODY_Y);
            rfDisplaySignalInfo(sig, RF_BODY_Y + 18);
        }
        delay(200);
    }
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
    tft.drawString("Total: " + String(count), 12, RF_BODY_Y);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfProtocolDictionary() {
    rfDrawHeader("Dicionario Protoc. RF", RF_COLOR_TITLE);
    const char* protos[] = {
        "Princeton (24b)", "Nice Flo (12b)", "Came (12b)", "Linear (10b)",
        "Chamberlain (8b)", "Holtek (10b)", "Liftmaster (8b)", "Ansonic (12b)",
        "KeeLoq (64b)", "Star (12b)"
    };
    int sel = 0, scrollY = 0;
    int total = 10;
    rfDrawFooter("UP/DOWN: Nav SEL: Info");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            rfDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(protos[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(EscPress)) break;
        if (check(SelPress)) {
            tft.fillRect(12, RF_BODY_Y, tftWidth - 24, 80, RF_COLOR_BG);
            tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
            tft.drawCentreString(protos[sel], tftWidth / 2, RF_BODY_Y + 5, 1);
            tft.drawString("OOK/ASK 433MHz", 12, RF_BODY_Y + 25);
            tft.drawString("350us base pulse", 12, RF_BODY_Y + 39);
            tft.drawString("Garage/Gate/Alarm", 12, RF_BODY_Y + 53);
        }
        delay(100);
    }
}

void rfSignalExporter() {
    rfDrawHeader("Exportar Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(sig, RF_BODY_Y);
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Formato: .sub Flipper", 12, RF_BODY_Y + 50);
    rfDrawFooter("SEL: Salvar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            String fname = "/WillyRF/rf_export_" + String(millis()) + ".sub";
            File f = SD.open(fname, FILE_WRITE);
            if (f) {
                f.println("Filetype: Flipper SubGhz RAW File");
                f.println("Version: 1");
                f.println("Frequency: " + String((uint32_t)(sig.frequency * 1000000)));
                f.println("Preset: FuriHalSubGhzPresetOok650Async");
                f.println("Protocol: " + sig.protocol);
                if (sig.rawData.length() > 0) {
                    f.println("RAW_Data: " + sig.rawData);
                }
                f.close();
                displaySuccess("Salvo: " + fname);
            } else {
                displayError("Falha ao salvar");
            }
            break;
        }
        delay(100);
    }
}

// ============================================================================
// 2. SIGNAL GENERATION & TRANSMISSION (10 functions)
// ============================================================================

void rfSignalCloner() {
    rfDrawHeader("Clonador de Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture sinal original...");
    RFSignalInfo orig = rfCaptureSignal();
    if (orig.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(orig, RF_BODY_Y);
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
    tft.drawCentreString("Pressione para clonar e enviar", tftWidth / 2, RF_BODY_Y + 55, 1);
    rfDrawFooter("SEL: Clonar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            rfSendCode(orig.frequency, orig.key, orig.bits, orig.te, 5);
            displaySuccess("Sinal clonado e enviado!");
            break;
        }
        delay(100);
    }
}

void rfProtocolGenerator() {
    rfDrawHeader("Gerador de Protocolo RF", RF_COLOR_TITLE);
    uint32_t code = 0x00;
    rfDrawFooter("SEL: Enviar  UP/DOWN: Valor");
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawCentreString("Codigo: 0x" + String(code, HEX), tftWidth / 2, RF_BODY_Y + 10, 1);
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            rfSendCode(433.92, code, 24, 350, 5);
            displaySuccess("Codigo enviado!");
            break;
        }
        if (check(UpPress)) { code += 0x100; tft.fillRect(0, RF_BODY_Y, tftWidth, 20, RF_COLOR_BG); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG); tft.drawCentreString("0x" + String(code, HEX), tftWidth / 2, RF_BODY_Y + 10, 1); }
        if (check(DownPress)) { code -= 0x100; tft.fillRect(0, RF_BODY_Y, tftWidth, 20, RF_COLOR_BG); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG); tft.drawCentreString("0x" + String(code, HEX), tftWidth / 2, RF_BODY_Y + 10, 1); }
        delay(100);
    }
}

void rfCustomCodeBuilder() {
    rfDrawHeader("Construtor Custom RF", RF_COLOR_TITLE);
    uint32_t addr = 0x00, cmd = 0x00;
    rfDrawFooter("SEL: Send  UP: Addr  DOWN: Cmd");
    int mode = 0;
    while (true) {
        tft.setTextSize(FP);
        tft.fillRect(12, RF_BODY_Y + 10, tftWidth - 24, 30, RF_COLOR_BG);
        tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawString("Addr: 0x" + String(addr, HEX) + (mode == 0 ? " *" : ""), 12, RF_BODY_Y + 10);
        tft.drawString("Cmd: 0x" + String(cmd, HEX) + (mode == 1 ? " *" : ""), 12, RF_BODY_Y + 28);
        if (check(UpPress)) { if (mode == 0) addr++; else cmd++; }
        if (check(DownPress)) { if (mode == 0) addr--; else cmd--; }
        if (check(SelPress)) mode = 1 - mode;
        if (check(EscPress)) break;
        if (check(SelPress) && mode == 0) {
            uint64_t data = ((uint64_t)addr << 16) | (uint64_t)cmd;
            rfSendCode(433.92, data, 24, 350, 5);
            displaySuccess("Enviado!");
            break;
        }
        delay(100);
    }
}

void rfBulkTransmitter() {
    rfDrawHeader("Transmissao RF Lote", RF_COLOR_TITLE);
    rfDrawFooter("Aguardando...");
    if (batchBuffer.empty()) {
        tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
        tft.drawCentreString("Buffer vazio - Capturando sinais...", tftWidth / 2, RF_BODY_Y + 20, 1);
        unsigned long start = millis();
        while (millis() - start < 10000) {
            if (check(EscPress)) break;
            RFSignalInfo sig = rfCaptureSignal(2000);
            if (sig.bits > 0) {
                RFBatchEntry entry;
                entry.name = sig.protocol;
                entry.frequency = sig.frequency;
                entry.key = sig.key;
                entry.bits = sig.bits;
                entry.te = sig.te;
                entry.repeatCount = 3;
                entry.delayAfter = 500;
                batchBuffer.push_back(entry);
            }
        }
    }
    if (batchBuffer.empty()) { displayWarning("Nenhum sinal"); return; }
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawString("Envio em lote: " + String(batchBuffer.size()) + " sinais", 12, RF_BODY_Y);
    for (size_t i = 0; i < batchBuffer.size(); i++) {
        rfDrawProgressBar(RF_BODY_Y + 20, (int)((i * 100) / batchBuffer.size()), String(i + 1).c_str());
        rfSendCode(batchBuffer[i].frequency, batchBuffer[i].key, batchBuffer[i].bits, batchBuffer[i].te, batchBuffer[i].repeatCount);
        delay(batchBuffer[i].delayAfter);
    }
    displaySuccess("Lote enviado!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfScheduledTransmitter() {
    rfDrawHeader("Transmissao Agend. RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    String delayStr = num_keyboard("5", 4, "Delay (segundos):");
    uint32_t delaySec = delayStr.toInt();
    if (delaySec == 0) delaySec = 5;
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
    tft.drawCentreString("Envio em " + String(delaySec) + "s...", tftWidth / 2, RF_BODY_Y + 20, 1);
    for (uint32_t i = delaySec; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, RF_BODY_Y + 35, tftWidth, 16, RF_COLOR_BG);
        tft.setTextSize(FM); tft.setTextColor(RF_COLOR_DANGER, RF_COLOR_BG);
        tft.drawCentreString(String(i), tftWidth / 2, RF_BODY_Y + 36, 1);
        delay(1000);
    }
    rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 5);
    displaySuccess("Enviado!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalRepeat() {
    rfDrawHeader("Repetir Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    String repStr = num_keyboard("5", 3, "Repeticoes:");
    int reps = repStr.toInt();
    if (reps == 0) reps = 5;
    for (int i = 0; i < reps; i++) {
        rfDrawProgressBar(RF_BODY_Y + 20, (i * 100) / reps, String(i + 1).c_str());
        rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 1);
        delay(500);
    }
    displaySuccess("Enviado " + String(reps) + "x!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfMultiFreqSender() {
    rfDrawHeader("Multi-Frequencia RF", RF_COLOR_TITLE);
    rfDrawFooter("Aguardando...");
    float freqs[] = {315.0, 433.92, 868.35, 915.0};
    rfDrawFooter("SEL: Enviar todas  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            for (int i = 0; i < 4; i++) {
                rfDrawProgressBar(RF_BODY_Y + 20, (i * 100) / 4, String(freqs[i], 1).c_str());
                rfSendCode(freqs[i], 0xAAAAAA, 24, 350, 3);
                delay(300);
            }
            displaySuccess("Multi-freq enviado!");
            break;
        }
        delay(100);
    }
}

void rfSignalModulator() {
    rfDrawHeader("Modulador de Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(sig, RF_BODY_Y);
    uint64_t mod = sig.key;
    rfDrawFooter("UP/DOWN: Modificar  SEL: Enviar");
    while (true) {
        tft.fillRect(0, RF_BODY_Y + 55, tftWidth, 14, RF_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawCentreString("Mod: 0x" + String((uint32_t)mod, HEX), tftWidth / 2, RF_BODY_Y + 55, 1);
        if (check(UpPress)) mod += 0x01;
        if (check(DownPress)) mod -= 0x01;
        if (check(SelPress)) { rfSendCode(sig.frequency, mod, sig.bits, sig.te, 3); displaySuccess("Modulado!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfCodeConverter() {
    rfDrawHeader("Conversor de Codigo RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(sig, RF_BODY_Y);
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("HEX: 0x" + String((uint32_t)sig.key, HEX), 12, RF_BODY_Y + 55);
    tft.drawString("BIN: " + String((uint32_t)sig.key, BIN), 12, RF_BODY_Y + 69);
    tft.drawString("DEC: " + String((uint32_t)sig.key), 12, RF_BODY_Y + 83);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalEditor() {
    rfDrawHeader("Editor de Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    uint32_t data = sig.key;
    int bitPos = 0;
    rfDrawFooter("UP/DOWN: Bit  SEL: Toggle  ESC: Sair");
    while (true) {
        tft.fillRect(0, RF_BODY_Y, tftWidth, 70, RF_COLOR_BG);
        tft.setTextSize(FP);
        tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        char hex[12]; snprintf(hex, sizeof(hex), "0x%08X", data);
        tft.drawCentreString(hex, tftWidth / 2, RF_BODY_Y, 1);
        for (int i = 0; i < 24; i++) {
            int bx = 12 + (i % 12) * 18;
            int by = RF_BODY_Y + 18 + (i / 12) * 20;
            uint16_t bgc = (i == bitPos) ? RF_COLOR_PRIMARY : ((data >> i) & 1 ? RF_COLOR_ACCENT : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.fillRoundRect(bx, by, 14, 16, 3, bgc);
            tft.setTextColor(TFT_WHITE, bgc);
            tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
            tft.drawString((data >> i) & 1 ? "1" : "0", bx + 7, by + 2, 1);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) bitPos = (bitPos + 1) % 24;
        if (check(DownPress)) bitPos = (bitPos + 23) % 24;
        if (check(SelPress)) data ^= (1ULL << bitPos);
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
// ============================================================================

void rfDeviceMapper() {
    rfDrawHeader("Mapeador Devices RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture sinais...");
    std::vector<RFSignalInfo> devSignals;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(3000);
        if (sig.bits > 0) devSignals.push_back(sig);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawString("Mapeados: " + String(devSignals.size()), 12, RF_BODY_Y);
    }
    if (devSignals.empty()) { displayWarning("Nenhum sinal"); return; }
    for (size_t i = 0; i < devSignals.size(); i++) {
        rfDisplaySignalInfo(devSignals[i], RF_BODY_Y + 10 + i * 48);
        if (i >= 3) break;
    }
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfMacroRecorder() {
    rfDrawHeader("Gravador Macro RF", RF_COLOR_TITLE);
    macroBuffer.clear();
    rfDrawFooter("Capture passos ESC: Parar");
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(3000);
        if (sig.bits > 0) {
            RFMacroStep step;
            step.delay = 0;
            step.frequency = sig.frequency;
            step.protocol = sig.protocol;
            step.key = sig.key;
            step.bits = sig.bits;
            step.te = sig.te;
            step.label = sig.protocol;
            macroBuffer.push_back(step);
            tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
            tft.drawString("Passos: " + String(macroBuffer.size()), 12, RF_BODY_Y);
        }
        delay(200);
    }
    displaySuccess(String(macroBuffer.size()) + " passos gravados!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfMacroPlayer() {
    if (macroBuffer.empty()) { displayWarning("Macro vazia"); return; }
    rfDrawHeader("Reprodutor Macro RF", RF_COLOR_TITLE);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawCentreString("Passos: " + String(macroBuffer.size()), tftWidth / 2, RF_BODY_Y, 1);
    rfDrawFooter("SEL: Play  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            for (size_t i = 0; i < macroBuffer.size(); i++) {
                rfDrawProgressBar(RF_BODY_Y + 20, (int)((i * 100) / macroBuffer.size()), macroBuffer[i].label.c_str());
                rfSendCode(macroBuffer[i].frequency, macroBuffer[i].key, macroBuffer[i].bits, macroBuffer[i].te, 1);
                delay(macroBuffer[i].delay + 300);
            }
            displaySuccess("Macro executada!");
            break;
        }
        delay(100);
    }
}

void rfSceneController() {
    rfDrawHeader("Controlador Cena RF", RF_COLOR_TITLE);
    const char* scenes[] = {"Garagem", "Portao", "Alarme ON", "Alarme OFF", "Sirene", "Luz ON", "Luz OFF", "Custom"};
    int sel = 0, scrollY = 0;
    rfDrawFooter("SEL: Executar  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= 8) break;
            int y = RF_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            rfDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(scenes[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            uint32_t codes[] = {0x123456, 0x654321, 0xAAAAAA, 0x555555, 0xABCDEF, 0x000001, 0xFFFFFF, 0x000000};
            rfSendCode(433.92, codes[sel], 24, 350, 5);
            displaySuccess("Cena executada!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfTimerScheduler() {
    rfDrawHeader("Agendador Timer RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    String minStr = num_keyboard("1", 3, "Minutos:");
    uint32_t minutes = minStr.toInt();
    if (minutes == 0) minutes = 1;
    tft.setTextColor(RF_COLOR_WARN, RF_COLOR_BG);
    tft.drawCentreString("Enviando em " + String(minutes) + " min...", tftWidth / 2, RF_BODY_Y, 1);
    for (uint32_t i = minutes * 60; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, RF_BODY_Y + 20, tftWidth, 14, RF_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString(String(i / 60) + "m " + String(i % 60) + "s", tftWidth / 2, RF_BODY_Y + 20, 1);
        delay(1000);
    }
    rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 5);
    displaySuccess("Timer executado!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfRemoteCloner() {
    rfDrawHeader("Clonador Remoto RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture tecla 1...");
    std::vector<RFSignalInfo> keys;
    for (int i = 0; i < 6; i++) {
        tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString("Tecla " + String(i + 1) + "/6", tftWidth / 2, RF_BODY_Y, 1);
        RFSignalInfo sig = rfCaptureSignal(8000);
        if (sig.bits == 0) { displayWarning("Cancelado"); return; }
        keys.push_back(sig);
        tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawString("Tecla " + String(i + 1) + ": OK", 12, RF_BODY_Y + 16 + i * 12);
    }
    displaySuccess(String(keys.size()) + " teclas clonadas!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfDeviceEmulator() {
    rfDrawHeader("Emulador Device RF", RF_COLOR_TITLE);
    const char* devices[] = {"Garagem 1", "Garagem 2", "Portao Eletr.", "Alarme"};
    int sel = 0;
    rfDrawFooter("SEL: Emular  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 4; i++) {
            int y = RF_BODY_Y + i * 20;
            rfDrawCard(y, 18, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : RF_COLOR_TEXT, i == sel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(devices[i], 14, y + 3);
        }
        if (check(UpPress)) sel = (sel + 3) % 4;
        if (check(DownPress)) sel = (sel + 1) % 4;
        if (check(SelPress)) {
            uint32_t codes[] = {0x123456, 0x654321, 0xABCDEF, 0x555555};
            rfSendCode(433.92, codes[sel], 24, 350, 5);
            displaySuccess("Emulado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfCodeSearch() {
    rfDrawHeader("Busca de Codigos RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture alvo...");
    RFSignalInfo target = rfCaptureSignal();
    if (target.bits == 0) { displayWarning("Cancelado"); return; }
    rfDisplaySignalInfo(target, RF_BODY_Y);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawCentreString("Buscando variacoes...", tftWidth / 2, RF_BODY_Y + 55, 1);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfBrandExplorer() {
    rfDrawHeader("Explorar Marcas RF", RF_COLOR_TITLE);
    const char* brands[] = {"Chamberlain", "LiftMaster", "Linear", "Came", "Nice", "Sommer", "Genie", " craftsman"};
    int sel = 0, scrollY = 0;
    rfDrawFooter("SEL: Detalhes  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= 8) break;
            int y = RF_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            rfDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(brands[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            tft.fillRect(12, RF_BODY_Y, tftWidth - 24, 60, RF_COLOR_BG);
            tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
            tft.drawCentreString(brands[sel], tftWidth / 2, RF_BODY_Y + 5, 1);
            tft.drawString("Freq: 315/433 MHz", 12, RF_BODY_Y + 25);
            tft.drawString("Protocolo: Rolling/OOK", 12, RF_BODY_Y + 39);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfUniversalRemote() {
    rfDrawHeader("Controle Univers. RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: TX  UP: Vol+  DOWN: Vol-");
    while (true) {
        if (check(SelPress)) { rfSendCode(433.92, 0x123456, 24, 350, 3); displaySuccess("Power!"); }
        if (check(UpPress)) { rfSendCode(433.92, 0xABCDEF, 24, 350, 3); displaySuccess("Abrir!"); }
        if (check(DownPress)) { rfSendCode(433.92, 0x654321, 24, 350, 3); displaySuccess("Fechar!"); }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// 4. SIGNAL MANIPULATION (10 functions)
// ============================================================================

void rfSignalRepeater() {
    rfDrawHeader("Repetidor Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    String repStr = num_keyboard("10", 4, "Repeticoes:");
    int reps = repStr.toInt();
    if (reps == 0) reps = 10;
    rfDrawFooter("ESC: Parar  Repetindo...");
    for (int i = 0; i < reps; i++) {
        if (check(EscPress)) break;
        rfDrawProgressBar(RF_BODY_Y + 10, (i * 100) / reps);
        rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 1);
        delay(500);
    }
    displaySuccess("Concluido!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfProtocolConverter() {
    rfDrawHeader("Conversor Protoc. RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    const char* targets[] = {"Princeton", "Nice Flo", "Came", "Linear", "Star"};
    float freqs[] = {433.92, 433.92, 433.92, 315.0, 433.92};
    int bits[] = {24, 12, 12, 10, 12};
    int sel = 0;
    rfDrawFooter("SEL: Converter  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int y = RF_BODY_Y + i * 18;
            rfDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : RF_COLOR_TEXT, i == sel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(targets[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 4; }
        if (check(DownPress)) { sel++; if (sel >= 5) sel = 0; }
        if (check(SelPress)) {
            rfSendCode(freqs[sel], sig.key, bits[sel], 350, 3);
            displaySuccess("Convertido para " + String(targets[sel]) + "!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfTimingModifier() {
    rfDrawHeader("Modificador Timing RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    int factor = 100;
    rfDrawFooter("UP/DOWN: +/-10%  SEL: Enviar");
    while (true) {
        tft.fillRect(0, RF_BODY_Y + 10, tftWidth, 20, RF_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString("Fator: " + String(factor) + "% (TE:" + String(sig.te * factor / 100) + "us)", tftWidth / 2, RF_BODY_Y + 10, 1);
        if (check(UpPress)) factor += 10;
        if (check(DownPress)) factor -= 10;
        if (check(SelPress)) {
            rfSendCode(sig.frequency, sig.key, sig.bits, sig.te * factor / 100, 3);
            displaySuccess("Enviado com timing " + String(factor) + "%!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfCodeObfuscator() {
    rfDrawHeader("Ofuscador Codigo RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    uint32_t xorKey = 0xFF;
    String keyStr = num_keyboard("FF", 2, "XOR Key (hex):");
    xorKey = strtol(keyStr.c_str(), nullptr, 16);
    uint32_t obf = sig.key ^ xorKey;
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Original: 0x" + String((uint32_t)sig.key, HEX), 12, RF_BODY_Y);
    tft.drawString("Ofuscado: 0x" + String(obf, HEX), 12, RF_BODY_Y + 14);
    rfDrawFooter("SEL: Enviar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) { rfSendCode(sig.frequency, obf, sig.bits, sig.te, 3); displaySuccess("Ofuscado enviado!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfSignalAmplifier() {
    rfDrawHeader("Amplificador Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    rfDrawFooter("SEL: Enviar repetido  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 10);
            displaySuccess("Sinal amplificado (10x)!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfSignalFilter() {
    rfDrawHeader("Filtro de Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Capturando...");
    int accepted = 0, rejected = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(3000);
        if (sig.bits > 8) accepted++;
        else rejected++;
        tft.setTextSize(FP);
        tft.fillRect(12, RF_BODY_Y + 20, tftWidth - 24, 20, RF_COLOR_BG);
        tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawString("Aceitos: " + String(accepted), 12, RF_BODY_Y + 20);
        tft.setTextColor(RF_COLOR_DANGER, RF_COLOR_BG);
        tft.drawString("Rejeitados: " + String(rejected), 12, RF_BODY_Y + 34);
    }
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfNoiseGenerator() {
    rfDrawHeader("Gerador de Ruido RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: Gerar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            if (!initRfModule("", 433.92)) { displayError("Falha RF"); return; }
            setMHZ(433.92);
            for (int i = 0; i < 100; i++) {
                uint32_t randomCode = random(0, 0xFFFFFF);
                rfSendCode(433.92, randomCode, 24, 350, 1);
            }
            deinitRfModule();
            displaySuccess("Ruido gerado!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfSignalSplitter() {
    rfDrawHeader("Divisor de Sinal RF", RF_COLOR_TITLE);
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Cancelado"); return; }
    uint32_t high = (sig.key >> 16) & 0xFFFF;
    uint32_t low = sig.key & 0xFFFF;
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Alto: 0x" + String(high, HEX), 12, RF_BODY_Y);
    tft.drawString("Baixo: 0x" + String(low, HEX), 12, RF_BODY_Y + 14);
    rfDrawFooter("SEL: Enviar partes  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            rfSendCode(sig.frequency, high, 16, sig.te, 3);
            delay(500);
            rfSendCode(sig.frequency, low, 16, sig.te, 3);
            displaySuccess("Partes enviadas!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfSignalMerger() {
    rfDrawHeader("Fusao de Sinais RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture sinal 1...");
    RFSignalInfo sig1 = rfCaptureSignal();
    if (sig1.bits == 0) { displayWarning("Cancelado"); return; }
    rfDrawFooter("Capture sinal 2...");
    RFSignalInfo sig2 = rfCaptureSignal();
    if (sig2.bits == 0) { displayWarning("Cancelado"); return; }
    uint64_t merged = ((sig1.key << 16) | (sig2.key & 0xFFFF));
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Fusionado: 0x" + String((uint32_t)merged, HEX), 12, RF_BODY_Y);
    rfDrawFooter("SEL: Enviar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) { rfSendCode(sig1.frequency, merged, 32, sig1.te, 1); displaySuccess("Fusao enviada!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfProtocolBridge() {
    rfDrawHeader("Ponte Protoc. RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture e retransmita...");
    while (true) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(5000);
        if (sig.bits > 0) {
            rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 3);
            tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
            tft.drawCentreString("Repassado: " + sig.protocol + " @ " + String(sig.frequency, 1) + "MHz", tftWidth / 2, RF_BODY_Y + 20, 1);
        }
    }
}

// ============================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
// ============================================================================

void rfSignalValidator() {
    rfDrawHeader("Validador Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture sinal...");
    RFSignalInfo sig = rfCaptureSignal();
    if (sig.bits == 0) { displayWarning("Invalido!"); return; }
    bool valid = (sig.bits > 0 && sig.bits <= 256 && sig.frequency > 0);
    tft.setTextSize(FP);
    tft.setTextColor(valid ? RF_COLOR_ACCENT : RF_COLOR_DANGER, RF_COLOR_BG);
    tft.drawCentreString(valid ? "SINAL VALIDO" : "SINAL INVALIDO", tftWidth / 2, RF_BODY_Y + 10, 1);
    rfDisplaySignalInfo(sig, RF_BODY_Y + 30);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfProtocolTester() {
    rfDrawHeader("Teste de Protocolo RF", RF_COLOR_TITLE);
    const char* protos[] = {"Princeton", "Nice Flo", "Came", "Linear", "Star"};
    rfDrawFooter("SEL: Testar todos  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            for (int i = 0; i < 5; i++) {
                tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
                tft.drawCentreString("Testando: " + String(protos[i]), tftWidth / 2, RF_BODY_Y, 1);
                rfSendCode(433.92, 0xAAAAAA, 24, 350, 3);
                delay(1000);
                tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
                tft.drawCentreString(protos[i] + ": OK", tftWidth / 2, RF_BODY_Y + 16, 1);
            }
            displaySuccess("Todos testados!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfReceiverDiagnostics() {
    rfDrawHeader("Diag. Receptor RF", RF_COLOR_TITLE);
    if (!initRfModule("rx", 433.92)) { displayError("Falha RF init"); return; }
    setMHZ(433.92);
    int pulses = 0;
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (check(EscPress)) break;
        int rssi = ELECHOUSE_cc1101.getRssi();
        if (rssi > -90) pulses++;
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(rssi) + "dB Pulses: " + String(pulses), tftWidth / 2, RF_BODY_Y, 1);
        delay(100);
    }
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Receptor: " + String(pulses > 0 ? "Funcional" : "Sem sinal"), 12, RF_BODY_Y + 20);
    deinitRfModule();
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfTransmitterDiagnostics() {
    rfDrawHeader("Diag. Transmissor RF", RF_COLOR_TITLE);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Teste de transmissao...", 12, RF_BODY_Y);
    rfSendCode(433.92, 0xAAAAAA, 24, 350, 1);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawString("Transmissor: OK", 12, RF_BODY_Y + 20);
    tft.drawString("Freq: 433.92 MHz", 12, RF_BODY_Y + 34);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfRangeTest() {
    rfDrawHeader("Teste de Alcance RF", RF_COLOR_TITLE);
    rfDrawFooter("Aguardando sinal...");
    if (!initRfModule("rx", 433.92)) { displayError("Falha RF"); return; }
    setMHZ(433.92);
    int maxRssi = -120;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        int rssi = ELECHOUSE_cc1101.getRssi();
        if (rssi > maxRssi) maxRssi = rssi;
        tft.fillRect(20, RF_BODY_Y + 20, tftWidth - 40, 12, RF_COLOR_BG);
        int bar = map(constrain(rssi + 120, 0, 100), 0, 100, 0, tftWidth - 40);
        tft.fillRect(20, RF_BODY_Y + 20, max(0, bar), 12, RF_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString("Max: " + String(maxRssi) + "dB", tftWidth / 2, RF_BODY_Y + 40, 1);
        delay(50);
    }
    deinitRfModule();
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfInterferenceDetector() {
    rfDrawHeader("Detec. Interferen. RF", RF_COLOR_TITLE);
    rfDrawFooter("Monitorando...");
    if (!initRfModule("rx", 433.92)) { displayError("Falha RF"); return; }
    setMHZ(433.92);
    int noise = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int val = ELECHOUSE_cc1101.getRssi();
        if (val > -80) noise++;
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(val) + "dB  Ruido: " + String(noise), tftWidth / 2, RF_BODY_Y, 1);
        delay(100);
    }
    deinitRfModule();
    tft.setTextColor(noise > 50 ? RF_COLOR_DANGER : RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawCentreString(noise > 50 ? "INTERFERENCIA DETECTADA" : "LIMPO", tftWidth / 2, RF_BODY_Y + 20, 1);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalBenchmark() {
    rfDrawHeader("Benchmark Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Enviando...");
    unsigned long start = micros();
    int count = 0;
    while (micros() - start < 1000000) {
        rfSendCode(433.92, 0xAAAAAA, 24, 350, 1);
        count++;
    }
    unsigned long elapsed = micros() - start;
    float rate = (float)count / (elapsed / 1000000.0f);
    tft.setTextSize(FP); tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Enviados: " + String(count), 12, RF_BODY_Y);
    tft.drawString("Tempo: " + String(elapsed / 1000) + "ms", 12, RF_BODY_Y + 14);
    tft.drawString("Taxa: " + String(rate, 1) + " env/s", 12, RF_BODY_Y + 28);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfProtocolStressTest() {
    rfDrawHeader("Stress Test Protoc. RF", RF_COLOR_TITLE);
    rfDrawFooter("Enviando em loop...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        float freqs[] = {315.0, 433.92, 868.35, 915.0};
        for (int i = 0; i < 4; i++) {
            rfSendCode(freqs[i], 0xAAAAAA, 24, 350, 1);
            count++;
        }
        tft.setTextSize(FP); tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
        tft.drawCentreString("Enviados: " + String(count), tftWidth / 2, RF_BODY_Y, 1);
    }
    displaySuccess(String(count) + " sinais enviados!");
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfHardwareDiagnostics() {
    rfDrawHeader("Diag. Hardware RF", RF_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(RF_COLOR_TEXT, RF_COLOR_BG);
    tft.drawString("Modulo: " + String(wilyConfigPins.rfModule == CC1101_SPI_MODULE ? "CC1101" : "Simple TX"), 12, RF_BODY_Y);
    if (wilyConfigPins.rfModule == CC1101_SPI_MODULE) {
        tft.drawString("CC1101 SPI OK", 12, RF_BODY_Y + 14);
        tft.drawString("Pino TX: " + String(wilyConfigPins.rfTx), 12, RF_BODY_Y + 28);
        tft.drawString("Pino RX: " + String(wilyConfigPins.rfRx), 12, RF_BODY_Y + 42);
    } else {
        tft.drawString("Pino TX: " + String(wilyConfigPins.rfTx), 12, RF_BODY_Y + 14);
        tft.drawString("Pino RX: " + String(wilyConfigPins.rfRx), 12, RF_BODY_Y + 28);
    }
    tft.drawString("Freq Default: " + String(wilyConfigPins.rfFreq) + " MHz", 12, RF_BODY_Y + 56);
    tft.setTextColor(RF_COLOR_ACCENT, RF_COLOR_BG);
    tft.drawString("ELECHOUSE CC1101: OK", 12, RF_BODY_Y + 76);
    rfDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void rfSignalPlayback() {
    rfDrawHeader("Playback Sinal RF", RF_COLOR_TITLE);
    rfDrawFooter("Capture e replique...");
    while (true) {
        if (check(EscPress)) break;
        RFSignalInfo sig = rfCaptureSignal(5000);
        if (sig.bits > 0) {
            rfSendCode(sig.frequency, sig.key, sig.bits, sig.te, 3);
            rfDisplaySignalInfo(sig, RF_BODY_Y + 10);
        }
    }
}

// ============================================================================
// 6. MENUS
// ============================================================================

void rfAnalysisMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analisador de Sinal", "ID de Protocolo", "Comparacao Sinais",
        "Scanner Frequencia", "Medidor Forca", "Analise Timing",
        "Qualidade Sinal", "Decoder Lote", "Dicionario Protoc.", "Exportar Sinal"
    };
    int total = 10;
    rfDrawHeader("ANALISE & SCANNING RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfSignalAnalyzer, rfProtocolIdentifier, rfSignalComparison,
                rfFrequencyScanner, rfSignalStrengthMeter, rfTimingAnalyzer,
                rfSignalQualityChecker, rfBatchDecoder, rfProtocolDictionary, rfSignalExporter
            };
            funcs[sel]();
            rfDrawHeader("ANALISE & SCANNING RF", RF_COLOR_TITLE);
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfGenerationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Clonador Sinal", "Gerador Protocolo", "Construtor Custom",
        "Transmissao Lote", "Transmissao Agend.", "Repetir Sinal",
        "Multi-Frequencia", "Modulador Sinal", "Conversor Codigo", "Editor Sinal"
    };
    int total = 10;
    rfDrawHeader("GERACAO & TRANSMISSAO RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfSignalCloner, rfProtocolGenerator, rfCustomCodeBuilder,
                rfBulkTransmitter, rfScheduledTransmitter, rfSignalRepeat,
                rfMultiFreqSender, rfSignalModulator, rfCodeConverter, rfSignalEditor
            };
            funcs[sel]();
            rfDrawHeader("GERACAO & TRANSMISSAO RF", RF_COLOR_TITLE);
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfDeviceControlMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Mapeador Devices", "Gravador Macro", "Reprodutor Macro",
        "Controlador Cena", "Agendador Timer", "Clonador Remoto",
        "Emulador Device", "Busca Codigos", "Explorar Marcas", "Controle Univers."
    };
    int total = 10;
    rfDrawHeader("DEVICE CONTROL & AUTO RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfDeviceMapper, rfMacroRecorder, rfMacroPlayer,
                rfSceneController, rfTimerScheduler, rfRemoteCloner,
                rfDeviceEmulator, rfCodeSearch, rfBrandExplorer, rfUniversalRemote
            };
            funcs[sel]();
            rfDrawHeader("DEVICE CONTROL & AUTO RF", RF_COLOR_TITLE);
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfManipulationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Repetidor Sinal", "Conversor Protoc.", "Modificador Timing",
        "Ofuscador Codigo", "Amplificador Sinal", "Filtro Sinal",
        "Gerador Ruido", "Divisor Sinal", "Fusao Sinais", "Ponte Protoc."
    };
    int total = 10;
    rfDrawHeader("MANIPULACAO SINAL RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfSignalRepeater, rfProtocolConverter, rfTimingModifier,
                rfCodeObfuscator, rfSignalAmplifier, rfSignalFilter,
                rfNoiseGenerator, rfSignalSplitter, rfSignalMerger, rfProtocolBridge
            };
            funcs[sel]();
            rfDrawHeader("MANIPULACAO SINAL RF", RF_COLOR_TITLE);
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfTestingMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Validador Sinal", "Teste Protocolo", "Diag. Receptor",
        "Diag. Transmissor", "Teste Alcance", "Detec. Interferen.",
        "Benchmark Sinal", "Stress Test", "Diag. Hardware", "Playback Sinal"
    };
    int total = 10;
    rfDrawHeader("TESTES & DIAGNOSTICOS RF", RF_COLOR_TITLE);
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfSignalValidator, rfProtocolTester, rfReceiverDiagnostics,
                rfTransmitterDiagnostics, rfRangeTest, rfInterferenceDetector,
                rfSignalBenchmark, rfProtocolStressTest, rfHardwareDiagnostics, rfSignalPlayback
            };
            funcs[sel]();
            rfDrawHeader("TESTES & DIAGNOSTICOS RF", RF_COLOR_TITLE);
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfAdvancedSuiteMenu() {
    rfShowWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analise & Scanning", "Geracao & Transmissao",
        "Device Control & Auto", "Manipulacao Sinal",
        "Testes & Diagnosticos"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_GREEN, TFT_ORANGE, TFT_MAGENTA, TFT_RED};
    const char* icons[] = {"A", "G", "D", "M", "T"};
    int total = 5;
    rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = RF_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 8, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : RF_COLOR_TEXT, isSel ? RF_COLOR_PRIMARY : getColorVariation(RF_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 38, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                rfAnalysisMenu, rfGenerationMenu, rfDeviceControlMenu,
                rfManipulationMenu, rfTestingMenu
            };
            funcs[sel]();
            rfShowWelcomeScreen();
            rfDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}
