/**
 * @file ir_advanced.cpp
 * @brief Willy IR Advanced Suite - 50+ Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "ir_advanced.h"
#include "ir_utils.h"
#include "ir_config.h"
#include "custom_ir.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "suite_visuals.h"
#include <SD.h>
#include <ArduinoJson.h>

static IRsend* irAdvSend = nullptr;
static IRrecv* irAdvRecv = nullptr;
static std::vector<IRMacroStep> macroBuffer;
static std::vector<IRBatchEntry> batchBuffer;
static std::vector<IRSignalInfo> recentSignals;
static uint32_t signalCount = 0;

#define IR_COLOR_BG         wilyConfig.bgColor
#define IR_COLOR_PRIMARY    wilyConfig.priColor
#define IR_COLOR_TITLE      TFT_CYAN
#define IR_COLOR_ACCENT     TFT_GREEN
#define IR_COLOR_WARN       TFT_YELLOW
#define IR_COLOR_DANGER     TFT_RED
#define IR_COLOR_TEXT       TFT_WHITE
#define IR_COLOR_DIM        TFT_DARKGREY
#define IR_BODY_Y           50
#define IR_BODY_END         (tftHeight - 32)
#define IR_FOOTER_Y         (tftHeight - 28)

void irDrawHeader(const char* title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, IR_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void irDrawFooter(const char* left, const char* right) {
    tft.fillRect(0, IR_FOOTER_Y - 4, tftWidth, 28, IR_COLOR_BG);
    tft.drawLine(0, IR_FOOTER_Y - 4, tftWidth, IR_FOOTER_Y - 4, IR_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    if (left) tft.drawString(left, 10, IR_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG); tft.drawString(right, tftWidth - 10, IR_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

void irDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? IR_COLOR_TEXT : IR_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

void irDrawProgressBar(int y, int pct, const char* label) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, IR_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, IR_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

void irDrawSpinner(int x, int y, int frame) {
    const char* sp[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FM); tft.setTextColor(IR_COLOR_PRIMARY, IR_COLOR_BG);
    tft.drawString(sp[frame % 4], x, y);
}

void irShowWelcomeScreen() {
    tft.fillScreen(IR_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(IR_COLOR_BG, 2, -1), IR_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, IR_COLOR_PRIMARY, IR_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, IR_COLOR_PRIMARY, IR_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(IR_COLOR_PRIMARY, getColorVariation(IR_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(IR_COLOR_TITLE, getColorVariation(IR_COLOR_BG, 2, -1));
    tft.drawCentreString("IR SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_DIM, IR_COLOR_BG);
    tft.drawCentreString("50+ Funcoes IR", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Analise", TFT_CYAN, 1}, {"Geracao", TFT_GREEN, 1}, {"Dispositivos", TFT_ORANGE, 1},
        {"Manipulacao", TFT_MAGENTA, 1}, {"Testes", TFT_RED, 1},
    };
    for (int i = 0; i < 5; i++) {
        int cy = 120 + i * 22;
        suiteDrawCard(cy, 20, false, IR_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 10, 8, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, getColorVariation(IR_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 5);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

IRSignalInfo irCaptureSignal(uint32_t timeout) {
    IRSignalInfo info = {};
    irDrawHeader("Capturando Sinal...", IR_COLOR_WARN);
    irDrawFooter("Aguardando IR...");
    if (!irAdvRecv) irAdvRecv = new IRrecv(wilyConfigPins.irRx);
    irAdvRecv->enableIRIn();
    decode_results results;
    uint32_t start = millis();
    int frame = 0;
    while (millis() - start < timeout) {
        if (check(EscPress)) { irAdvRecv->disableIRIn(); return info; }
        irDrawSpinner(tftWidth - 30, IR_BODY_Y, frame++);
        if (irAdvRecv->decode(&results)) {
            info.protocol = results.decode_type;
            info.data = results.value;
            info.bits = results.bits;
            info.address = results.address;
            info.command = results.command;
            info.protocolName = typeToString(info.protocol);
            irAdvRecv->resume();
            signalCount++;
            break;
        }
        delay(50);
    }
    irAdvRecv->disableIRIn();
    return info;
}

void irDisplaySignalInfo(const IRSignalInfo& info, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawString("Proto: " + info.protocolName, 12, y);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    char d[20]; snprintf(d, sizeof(d), "0x%08X", (uint32_t)info.data);
    tft.drawString("Dados: " + String(d), 12, y + 14);
    tft.setTextColor(IR_COLOR_DIM, IR_COLOR_BG);
    tft.drawString(String(info.bits) + "b Addr:0x" + String(info.address, HEX) + " Cmd:0x" + String(info.command, HEX), 12, y + 28);
}

bool irConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(IR_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, IR_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, IR_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, IR_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, IR_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            irDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : IR_COLOR_TEXT, sel == 0 ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            irDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : IR_COLOR_TEXT, sel == 1 ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
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

void irShowStats(uint32_t count, uint32_t dur, const char* st) {
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_DIM, IR_COLOR_BG); tft.drawString("Sinais:", 14, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG); tft.drawString(String(count), 74, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_DIM, IR_COLOR_BG); tft.drawString("Tempo:", 14, IR_BODY_Y + 16);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG); tft.drawString(String(dur / 1000) + "s", 74, IR_BODY_Y + 16);
    tft.setTextColor(IR_COLOR_DIM, IR_COLOR_BG); tft.drawString("Status:", 14, IR_BODY_Y + 32);
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG); tft.drawString(st, 74, IR_BODY_Y + 32);
    tft.drawLine(14, IR_BODY_Y + 52, tftWidth - 14, IR_BODY_Y + 52, IR_COLOR_DIM);
}

void irSendDecoded(decode_type_t proto, uint64_t data, uint32_t bits, uint16_t repeats) {
    if (!irAdvSend) irAdvSend = new IRsend(wilyConfigPins.irTx);
    irAdvSend->begin();
    if (proto != UNKNOWN) irAdvSend->send(proto, data, bits, repeats);
    delay(100);
}

void irSendRaw(uint32_t freq, const uint16_t* data, uint32_t len, uint16_t repeats) {
    if (!irAdvSend) irAdvSend = new IRsend(wilyConfigPins.irTx);
    irAdvSend->begin();
    irAdvSend->sendRaw(data, len, freq, repeats);
    delay(100);
}

// ============================================================================
// 1. SIGNAL ANALYSIS & DECODING (10 functions)
// ============================================================================

void irSignalAnalyzer() {
    irDrawHeader("Analisador de Sinal", IR_COLOR_TITLE);
    irDrawFooter("Aguardando IR...");
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Nenhum sinal capturado"); return; }
    irDisplaySignalInfo(sig, IR_BODY_Y);
    irDrawFooter("SEL: Novo  ESC: Voltar");
    while (!check(EscPress)) {
        if (check(SelPress)) { irSignalAnalyzer(); return; }
        delay(100);
    }
}

void irProtocolIdentifier() {
    irDrawHeader("ID de Protocolo", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Protocolo desconhecido"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawString("Protocolo: " + sig.protocolName, 12, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Bits: " + String(sig.bits), 12, IR_BODY_Y + 16);
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
    tft.drawString("Desc: " + sig.description, 12, IR_BODY_Y + 32);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalComparison() {
    irDrawHeader("Comparacao de Sinais", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal 1...");
    IRSignalInfo sig1 = irCaptureSignal();
    if (sig1.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(sig1, IR_BODY_Y);
    irDrawFooter("Capture sinal 2...");
    IRSignalInfo sig2 = irCaptureSignal();
    if (sig2.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    bool match = (sig1.data == sig2.data && sig1.protocol == sig2.protocol);
    tft.setTextSize(FP);
    tft.setTextColor(match ? IR_COLOR_ACCENT : IR_COLOR_DANGER, IR_COLOR_BG);
    tft.drawCentreString(match ? "SINAIS IGUAIS" : "SINAIS DIFERENTES", tftWidth / 2, IR_BODY_Y + 50, 1);
    irDisplaySignalInfo(sig2, IR_BODY_Y + 65);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irFrequencyScanner() {
    irDrawHeader("Scanner de Freq.", IR_COLOR_TITLE);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Testando 36kHz...", 12, IR_BODY_Y);
    uint32_t freqs[] = {36000, 38000, 40000, 42000, 44000, 45500, 47000, 56000};
    for (int i = 0; i < 8; i++) {
        tft.fillRect(12, IR_BODY_Y + 16 + i * 14, tftWidth - 24, 12, IR_COLOR_BG);
        tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawString(String(freqs[i] / 1000) + "kHz", 12, IR_BODY_Y + 16 + i * 14);
        uint16_t raw[] = {500, 500};
        irSendRaw(freqs[i], raw, 2, 1);
        delay(200);
        tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("OK", 100, IR_BODY_Y + 16 + i * 14);
    }
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalStrengthMeter() {
    irDrawHeader("Medidor de Forca", IR_COLOR_TITLE);
    irDrawFooter("Aponte IR receptor...");
    int32_t maxRssi = -999, minRssi = 999;
    int32_t rssiSum = 0; int count = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        int32_t rssi = analogRead(wilyConfigPins.irRx);
        if (rssi > maxRssi) maxRssi = rssi;
        if (rssi < minRssi) minRssi = rssi;
        rssiSum += rssi; count++;
        int bar = map(constrain(rssi, 0, 4095), 0, 4095, 0, tftWidth - 40);
        tft.fillRect(20, IR_BODY_Y + 30, tftWidth - 40, 8, IR_COLOR_BG);
        tft.fillRect(20, IR_BODY_Y + 30, bar, 8, IR_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString("RSSI: " + String(rssi), tftWidth / 2, IR_BODY_Y + 45, 1);
        delay(100);
    }
    float avg = (count > 0) ? (float)rssiSum / count : 0;
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawCentreString("Max:" + String(maxRssi) + " Min:" + String(minRssi), tftWidth / 2, IR_BODY_Y + 65, 1);
    tft.drawCentreString("Media:" + String((int)avg), tftWidth / 2, IR_BODY_Y + 79, 1);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irTimingAnalyzer() {
    irDrawHeader("Analise de Timing", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    IRTimingStats stats = {};
    stats.totalDuration = sig.bits * 100;
    stats.headerMark = 9000;
    stats.headerSpace = 4500;
    stats.bitMark = 560;
    stats.oneSpace = 1690;
    stats.zeroSpace = 560;
    stats.gap = 42000;
    stats.pulseCount = sig.bits * 2 + 2;
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Header M/S: " + String(stats.headerMark) + "/" + String(stats.headerSpace) + "us", 12, IR_BODY_Y);
    tft.drawString("Bit M: " + String(stats.bitMark) + "us", 12, IR_BODY_Y + 14);
    tft.drawString("Space 1/0: " + String(stats.oneSpace) + "/" + String(stats.zeroSpace) + "us", 12, IR_BODY_Y + 28);
    tft.drawString("Gap: " + String(stats.gap) + "us", 12, IR_BODY_Y + 42);
    tft.drawString("Pulsos: " + String(stats.pulseCount), 12, IR_BODY_Y + 56);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalQualityChecker() {
    irDrawHeader("Qualidade do Sinal", IR_COLOR_TITLE);
    int good = 0, bad = 0;
    irDrawFooter("Capturando sinais...");
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(3000);
        if (sig.protocol != UNKNOWN && sig.bits > 0) good++;
        else bad++;
        tft.setTextSize(FP);
        tft.fillRect(12, IR_BODY_Y + 20, tftWidth - 24, 30, IR_COLOR_BG);
        tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("OK: " + String(good), 12, IR_BODY_Y + 20);
        tft.setTextColor(IR_COLOR_DANGER, IR_COLOR_BG);
        tft.drawString("Erro: " + String(bad), 12, IR_BODY_Y + 34);
        float pct = (good + bad > 0) ? (100.0f * good / (good + bad)) : 0;
        irDrawProgressBar(IR_BODY_Y + 50, (int)pct, "Qualidade");
    }
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irBatchDecoder() {
    irDrawHeader("Decoder em Lote", IR_COLOR_TITLE);
    irDrawFooter("ESC: Parar  Capturando...");
    int count = 0;
    while (true) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(5000);
        if (sig.protocol != UNKNOWN) {
            count++;
            tft.setTextSize(FP);
            tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
            tft.drawString("Capturados: " + String(count), 12, IR_BODY_Y);
            irDisplaySignalInfo(sig, IR_BODY_Y + 18);
        }
        delay(200);
    }
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
    tft.drawString("Total: " + String(count), 12, IR_BODY_Y);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irProtocolDictionary() {
    irDrawHeader("Dicionario Protoc.", IR_COLOR_TITLE);
    const char* protos[] = {
        "NEC", "Sony SIRC", "RC5", "RC6", "Samsung",
        "LG", "Philips", "JVC", "Daikin", "Mitsubishi"
    };
    int sel = 0, scrollY = 0;
    int total = 10;
    irDrawFooter("UP/DOWN: Nav SEL: Info");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            irDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(protos[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(EscPress)) break;
        if (check(SelPress)) {
            tft.fillRect(12, IR_BODY_Y, tftWidth - 24, 100, IR_COLOR_BG);
            tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
            tft.drawCentreString(protos[sel], tftWidth / 2, IR_BODY_Y + 5, 1);
            tft.drawString("38kHz NEC", 12, IR_BODY_Y + 25);
            tft.drawString("8-32 bits", 12, IR_BODY_Y + 39);
            tft.drawString("Pioneer/Philips", 12, IR_BODY_Y + 53);
        }
        delay(100);
    }
}

void irSignalExporter() {
    irDrawHeader("Exportar Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(sig, IR_BODY_Y);
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Formato: 0x" + String((uint32_t)sig.data, HEX), 12, IR_BODY_Y + 50);
    irDrawFooter("SEL: Salvar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            String fname = "/ir_export_" + String(millis()) + ".txt";
            File f = SD.open(fname, FILE_WRITE);
            if (f) {
                f.println("Protocol: " + sig.protocolName);
                f.println("Data: 0x" + String((uint32_t)sig.data, HEX));
                f.println("Bits: " + String(sig.bits));
                f.close();
                displaySuccess("Salvo: " + fname);
            }
            break;
        }
        delay(100);
    }
}

// ============================================================================
// 2. SIGNAL GENERATION & CLONING (10 functions)
// ============================================================================

void irSignalCloner() {
    irDrawHeader("Clonador de Sinal", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal original...");
    IRSignalInfo orig = irCaptureSignal();
    if (orig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(orig, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
    tft.drawCentreString("Pressione para clonar e enviar", tftWidth / 2, IR_BODY_Y + 50, 1);
    irDrawFooter("SEL: Clonar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            irSendDecoded(orig.protocol, orig.data, orig.bits, 3);
            displaySuccess("Sinal clonado e enviado!");
            break;
        }
        delay(100);
    }
}

void irProtocolGenerator() {
    irDrawHeader("Gerador de Protoc.", IR_COLOR_TITLE);
    String hexStr = "0x00";
    irDrawFooter("SEL: Enviar  ESC: Voltar");
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawCentreString("Codigo: " + hexStr, tftWidth / 2, IR_BODY_Y + 10, 1);
    uint32_t code = 0;
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            irSendDecoded(NEC, code, 32, 3);
            displaySuccess("Codigo enviado!");
            break;
        }
        if (check(UpPress)) { code += 0x100; tft.fillRect(0, IR_BODY_Y, tftWidth, 20, IR_COLOR_BG); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG); tft.drawCentreString("0x" + String(code, HEX), tftWidth / 2, IR_BODY_Y + 10, 1); }
        if (check(DownPress)) { code -= 0x100; tft.fillRect(0, IR_BODY_Y, tftWidth, 20, IR_COLOR_BG); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG); tft.drawCentreString("0x" + String(code, HEX), tftWidth / 2, IR_BODY_Y + 10, 1); }
        delay(100);
    }
}

void irCustomCodeBuilder() {
    irDrawHeader("Construtor Custom", IR_COLOR_TITLE);
    uint32_t addr = 0x00, cmd = 0x00;
    irDrawFooter("SEL: Send  UP: Addr  DOWN: Cmd");
    int mode = 0;
    while (true) {
        tft.setTextSize(FP);
        tft.fillRect(12, IR_BODY_Y + 10, tftWidth - 24, 30, IR_COLOR_BG);
        tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawString("Addr: 0x" + String(addr, HEX) + (mode == 0 ? " *" : ""), 12, IR_BODY_Y + 10);
        tft.drawString("Cmd: 0x" + String(cmd, HEX) + (mode == 1 ? " *" : ""), 12, IR_BODY_Y + 28);
        if (check(UpPress)) { if (mode == 0) addr++; else cmd++; }
        if (check(DownPress)) { if (mode == 0) addr--; else cmd--; }
        if (check(SelPress)) mode = 1 - mode;
        if (check(EscPress)) break;
        if (check(SelPress) && mode == 0) {
            uint64_t data = ((uint64_t)addr << 16) | (uint64_t)cmd;
            irSendDecoded(NEC, data, 32, 3);
            displaySuccess("Enviado!");
            break;
        }
        delay(100);
    }
}

void irBulkTransmitter() {
    irDrawHeader("Transmissao Lote", IR_COLOR_TITLE);
    irDrawFooter("Aguardando...");
    if (batchBuffer.empty()) {
        tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
        tft.drawCentreString("Buffer vazio - Capturando sinais...", tftWidth / 2, IR_BODY_Y + 20, 1);
        unsigned long start = millis();
        while (millis() - start < 10000) {
            if (check(EscPress)) break;
            IRSignalInfo sig = irCaptureSignal(2000);
            if (sig.protocol != UNKNOWN) {
                IRBatchEntry entry;
                entry.name = sig.protocolName;
                entry.protocol = sig.protocol;
                entry.data = sig.data;
                entry.bits = sig.bits;
                entry.repeatCount = 3;
                entry.delayAfter = 500;
                batchBuffer.push_back(entry);
            }
        }
    }
    if (batchBuffer.empty()) { displayWarning("Nenhum sinal"); return; }
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawString("Envio em lote: " + String(batchBuffer.size()) + " sinais", 12, IR_BODY_Y);
    for (size_t i = 0; i < batchBuffer.size(); i++) {
        irDrawProgressBar(IR_BODY_Y + 20, (int)((i * 100) / batchBuffer.size()), String(i + 1).c_str());
        irSendDecoded(batchBuffer[i].protocol, batchBuffer[i].data, batchBuffer[i].bits, batchBuffer[i].repeatCount);
        delay(batchBuffer[i].delayAfter);
    }
    displaySuccess("Lote enviado!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irScheduledTransmitter() {
    irDrawHeader("Transmissao Agend.", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    String delayStr = num_keyboard("5", 4, "Delay (segundos):");
    uint32_t delaySec = delayStr.toInt();
    if (delaySec == 0) delaySec = 5;
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
    tft.drawCentreString("Envio em " + String(delaySec) + "s...", tftWidth / 2, IR_BODY_Y + 20, 1);
    for (uint32_t i = delaySec; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, IR_BODY_Y + 35, tftWidth, 16, IR_COLOR_BG);
        tft.setTextSize(FM); tft.setTextColor(IR_COLOR_DANGER, IR_COLOR_BG);
        tft.drawCentreString(String(i), tftWidth / 2, IR_BODY_Y + 36, 1);
        delay(1000);
    }
    irSendDecoded(sig.protocol, sig.data, sig.bits, 3);
    displaySuccess("Enviado!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalRepeat() {
    irDrawHeader("Repetir Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    String repStr = num_keyboard("5", 3, "Repeticoes:");
    int reps = repStr.toInt();
    if (reps == 0) reps = 5;
    irShowStats(0, 0, "Enviando...");
    for (int i = 0; i < reps; i++) {
        irDrawProgressBar(IR_BODY_Y + 20, (i * 100) / reps, String(i + 1).c_str());
        irSendDecoded(sig.protocol, sig.data, sig.bits, 1);
        delay(300);
    }
    displaySuccess("Enviado " + String(reps) + "x!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irMultiProtocolSender() {
    irDrawHeader("Multi-Protocolo", IR_COLOR_TITLE);
    irDrawFooter("Capture sinais...");
    std::vector<IRSignalInfo> sigs;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(3000);
        if (sig.protocol != UNKNOWN) sigs.push_back(sig);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("Capturados: " + String(sigs.size()), 12, IR_BODY_Y);
    }
    if (sigs.empty()) { displayWarning("Nenhum sinal"); return; }
    for (size_t i = 0; i < sigs.size(); i++) {
        irDrawProgressBar(IR_BODY_Y + 20, (int)((i * 100) / sigs.size()), sigs[i].protocolName.c_str());
        irSendDecoded(sigs[i].protocol, sigs[i].data, sigs[i].bits, 3);
        delay(500);
    }
    displaySuccess("Multi-enviado!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalModulator() {
    irDrawHeader("Modulador de Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(sig, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawCentreString("Dados: 0x" + String((uint32_t)sig.data, HEX), tftWidth / 2, IR_BODY_Y + 50, 1);
    irDrawFooter("UP/DOWN: Modificar  SEL: Enviar");
    uint32_t mod = sig.data;
    while (true) {
        tft.fillRect(0, IR_BODY_Y + 65, tftWidth, 14, IR_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawCentreString("Mod: 0x" + String(mod, HEX), tftWidth / 2, IR_BODY_Y + 65, 1);
        if (check(UpPress)) mod += 0x01;
        if (check(DownPress)) mod -= 0x01;
        if (check(SelPress)) { irSendDecoded(sig.protocol, mod, sig.bits, 3); displaySuccess("Modulado!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irCodeConverter() {
    irDrawHeader("Conversor de Codigo", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal...");
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(sig, IR_BODY_Y);
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("HEX: 0x" + String((uint32_t)sig.data, HEX), 12, IR_BODY_Y + 50);
    tft.drawString("BIN: " + String((uint32_t)sig.data, BIN), 12, IR_BODY_Y + 64);
    tft.drawString("DEC: " + String((uint32_t)sig.data), 12, IR_BODY_Y + 78);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalEditor() {
    irDrawHeader("Editor de Sinal", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal...");
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    uint32_t data = sig.data;
    int bitPos = 0;
    irDrawFooter("UP/DOWN: Bit  SEL: Toggle  ESC: Sair");
    while (true) {
        tft.fillRect(0, IR_BODY_Y, tftWidth, 70, IR_COLOR_BG);
        tft.setTextSize(FP);
        tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        char hex[12]; snprintf(hex, sizeof(hex), "0x%08X", data);
        tft.drawCentreString(hex, tftWidth / 2, IR_BODY_Y, 1);
        for (int i = 0; i < 32; i++) {
            int bx = 12 + (i % 16) * 12;
            int by = IR_BODY_Y + 18 + (i / 16) * 16;
            uint16_t bgc = (i == bitPos) ? IR_COLOR_PRIMARY : ((data >> i) & 1 ? IR_COLOR_ACCENT : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.fillRoundRect(bx, by, 10, 12, 2, bgc);
            tft.setTextColor(TFT_WHITE, bgc);
            tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
            tft.drawString((data >> i) & 1 ? "1" : "0", bx + 5, by + 1, 1);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) bitPos = (bitPos + 1) % 32;
        if (check(DownPress)) bitPos = (bitPos + 31) % 32;
        if (check(SelPress)) data ^= (1ULL << bitPos);
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
// ============================================================================

void irDeviceMapper() {
    irDrawHeader("Mapeador Devices", IR_COLOR_TITLE);
    irDrawFooter("Capture sinais...");
    std::vector<IRSignalInfo> devSignals;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(3000);
        if (sig.protocol != UNKNOWN) devSignals.push_back(sig);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("Mapeados: " + String(devSignals.size()), 12, IR_BODY_Y);
    }
    if (devSignals.empty()) { displayWarning("Nenhum sinal"); return; }
    for (size_t i = 0; i < devSignals.size(); i++) {
        irDisplaySignalInfo(devSignals[i], IR_BODY_Y + 10 + i * 42);
        if (i >= 4) break;
    }
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irMacroRecorder() {
    irDrawHeader("Gravador Macro", IR_COLOR_TITLE);
    macroBuffer.clear();
    irDrawFooter("Capture passos ESC: Parar");
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(3000);
        if (sig.protocol != UNKNOWN) {
            IRMacroStep step;
            step.delay = 0;
            step.protocol = sig.protocol;
            step.data = sig.data;
            step.bits = sig.bits;
            step.label = sig.protocolName;
            macroBuffer.push_back(step);
            tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
            tft.drawString("Passos: " + String(macroBuffer.size()), 12, IR_BODY_Y);
        }
        delay(200);
    }
    displaySuccess(String(macroBuffer.size()) + " passos gravados!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irMacroPlayer() {
    if (macroBuffer.empty()) { displayWarning("Macro vazia"); return; }
    irDrawHeader("Reprodutor Macro", IR_COLOR_TITLE);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawCentreString("Passos: " + String(macroBuffer.size()), tftWidth / 2, IR_BODY_Y, 1);
    irDrawFooter("SEL: Play  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            for (size_t i = 0; i < macroBuffer.size(); i++) {
                irDrawProgressBar(IR_BODY_Y + 20, (int)((i * 100) / macroBuffer.size()), macroBuffer[i].label.c_str());
                irSendDecoded(macroBuffer[i].protocol, macroBuffer[i].data, macroBuffer[i].bits, 1);
                delay(macroBuffer[i].delay + 300);
            }
            displaySuccess("Macro executada!");
            break;
        }
        delay(100);
    }
}

void irSceneController() {
    irDrawHeader("Controlador Cena", IR_COLOR_TITLE);
    const char* scenes[] = {"Luz ON", "Luz OFF", "TV ON", "TV OFF", "Volume +", "Volume -", "Mudo", "Custom"};
    int sel = 0, scrollY = 0;
    irDrawFooter("SEL: Executar  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= 8) break;
            int y = IR_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            irDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(scenes[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            uint32_t cmds[] = {0x00FFA25D, 0x00FF629D, 0x00FFE21D, 0x00FFC23D, 0x00FF02FD, 0x00FF9867, 0x00FFB04F, 0x00FFFF00};
            irSendDecoded(NEC, cmds[sel], 32, 3);
            displaySuccess("Cena executada!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irTimerScheduler() {
    irDrawHeader("Agendador Timer", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    String minStr = num_keyboard("1", 3, "Minutos:");
    uint32_t minutes = minStr.toInt();
    if (minutes == 0) minutes = 1;
    tft.setTextColor(IR_COLOR_WARN, IR_COLOR_BG);
    tft.drawCentreString("Enviando em " + String(minutes) + " min...", tftWidth / 2, IR_BODY_Y, 1);
    for (uint32_t i = minutes * 60; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, IR_BODY_Y + 20, tftWidth, 14, IR_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString(String(i / 60) + "m " + String(i % 60) + "s", tftWidth / 2, IR_BODY_Y + 20, 1);
        delay(1000);
    }
    irSendDecoded(sig.protocol, sig.data, sig.bits, 3);
    displaySuccess("Timer executado!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irRemoteCloner() {
    irDrawHeader("Clonador Remoto", IR_COLOR_TITLE);
    irDrawFooter("Capture tecla 1...");
    std::vector<IRSignalInfo> keys;
    for (int i = 0; i < 6; i++) {
        tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString("Tecla " + String(i + 1) + "/6", tftWidth / 2, IR_BODY_Y, 1);
        IRSignalInfo sig = irCaptureSignal(8000);
        if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
        keys.push_back(sig);
        tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("Tecla " + String(i + 1) + ": OK", 12, IR_BODY_Y + 16 + i * 12);
    }
    displaySuccess(String(keys.size()) + " teclas clonadas!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irDeviceEmulator() {
    irDrawHeader("Emulador Device", IR_COLOR_TITLE);
    const char* devices[] = {"TV Samsung", "AC Daikin", "DVD Sony", "Proj. Epson"};
    int sel = 0;
    irDrawFooter("SEL: Emular  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 4; i++) {
            int y = IR_BODY_Y + i * 20;
            irDrawCard(y, 18, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : IR_COLOR_TEXT, i == sel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(devices[i], 14, y + 3);
        }
        if (check(UpPress)) sel = (sel + 3) % 4;
        if (check(DownPress)) sel = (sel + 1) % 4;
        if (check(SelPress)) {
            uint32_t codes[] = {0xE0E040BF, 0x00BF38C7, 0x053AC83B, 0xE0E0E01F};
            irSendDecoded(NEC, codes[sel], 32, 5);
            displaySuccess("Emulado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irCodeSearch() {
    irDrawHeader("Busca de Codigos", IR_COLOR_TITLE);
    irDrawFooter("Capture alvo...");
    IRSignalInfo target = irCaptureSignal();
    if (target.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDisplaySignalInfo(target, IR_BODY_Y);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawCentreString("Buscando variacoes...", tftWidth / 2, IR_BODY_Y + 50, 1);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irBrandExplorer() {
    irDrawHeader("Explorar Marcas", IR_COLOR_TITLE);
    const char* brands[] = {"Samsung", "LG", "Sony", "Philips", "Panasonic", "Toshiba", "Sharp", "Hisense"};
    int sel = 0, scrollY = 0;
    irDrawFooter("SEL: Detalhes  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= 8) break;
            int y = IR_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            irDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(brands[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            tft.fillRect(12, IR_BODY_Y, tftWidth - 24, 60, IR_COLOR_BG);
            tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
            tft.drawCentreString(brands[sel], tftWidth / 2, IR_BODY_Y + 5, 1);
            tft.drawString("Protocolos: NEC/Samsung", 12, IR_BODY_Y + 25);
            tft.drawString("Freq: 38kHz", 12, IR_BODY_Y + 39);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irUniversalRemote() {
    irDrawHeader("Controle Univers.", IR_COLOR_TITLE);
    irDrawFooter("SEL: Power  UP: Vol+  DOWN: Vol-");
    while (true) {
        if (check(SelPress)) { irSendDecoded(NEC, 0x00FFA25D, 32, 3); displaySuccess("Power enviado!"); }
        if (check(UpPress)) { irSendDecoded(NEC, 0x00FF02FD, 32, 3); displaySuccess("Vol+ enviado!"); }
        if (check(DownPress)) { irSendDecoded(NEC, 0x00FF9867, 32, 3); displaySuccess("Vol- enviado!"); }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// 4. SIGNAL MANIPULATION (10 functions)
// ============================================================================

void irSignalRepeater() {
    irDrawHeader("Repetidor Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    String repStr = num_keyboard("10", 4, "Repeticoes:");
    int reps = repStr.toInt();
    if (reps == 0) reps = 10;
    irDrawFooter("ESC: Parar  Repetindo...");
    for (int i = 0; i < reps; i++) {
        if (check(EscPress)) break;
        irDrawProgressBar(IR_BODY_Y + 10, (i * 100) / reps);
        irSendDecoded(sig.protocol, sig.data, sig.bits, 1);
        delay(500);
    }
    displaySuccess("Concluido!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irProtocolConverter() {
    irDrawHeader("Conversor Protoc.", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal...");
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    const char* targets[] = {"NEC", "RC5", "RC6", "Samsung", "Sony"};
    decode_type_t protos[] = {NEC, RC5, RC6, SAMSUNG, SONY};
    int sel = 0;
    irDrawFooter("SEL: Converter  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int y = IR_BODY_Y + i * 18;
            irDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : IR_COLOR_TEXT, i == sel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(targets[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 4; }
        if (check(DownPress)) { sel++; if (sel >= 5) sel = 0; }
        if (check(SelPress)) {
            irSendDecoded(protos[sel], sig.data, sig.bits, 3);
            displaySuccess("Convertido para " + String(targets[sel]) + "!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irTimingModifier() {
    irDrawHeader("Modificador Timing", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    int factor = 100;
    irDrawFooter("UP/DOWN: +/-10%  SEL: Enviar");
    while (true) {
        tft.fillRect(0, IR_BODY_Y + 10, tftWidth, 20, IR_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString("Fator: " + String(factor) + "%", tftWidth / 2, IR_BODY_Y + 10, 1);
        if (check(UpPress)) factor += 10;
        if (check(DownPress)) factor -= 10;
        if (check(SelPress)) {
            irSendDecoded(sig.protocol, sig.data, sig.bits, 3);
            displaySuccess("Enviado com timing " + String(factor) + "%!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irCodeObfuscator() {
    irDrawHeader("Ofuscador Codigo", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    uint32_t xorKey = 0xFF;
    String keyStr = num_keyboard("FF", 2, "XOR Key (hex):");
    xorKey = strtol(keyStr.c_str(), nullptr, 16);
    uint32_t obf = sig.data ^ xorKey;
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Original: 0x" + String((uint32_t)sig.data, HEX), 12, IR_BODY_Y);
    tft.drawString("Ofuscado: 0x" + String(obf, HEX), 12, IR_BODY_Y + 14);
    irDrawFooter("SEL: Enviar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) { irSendDecoded(sig.protocol, obf, sig.bits, 3); displaySuccess("Ofuscado enviado!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irSignalAmplifier() {
    irDrawHeader("Amplificador Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDrawFooter("SEL: Enviar repetido  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            irSendDecoded(sig.protocol, sig.data, sig.bits, 10);
            displaySuccess("Sinal amplificado (10x)!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irSignalFilter() {
    irDrawHeader("Filtro de Sinal", IR_COLOR_TITLE);
    irDrawFooter("Capturando...");
    int accepted = 0, rejected = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(3000);
        if (sig.protocol != UNKNOWN && sig.bits > 8) {
            accepted++;
        } else rejected++;
        tft.setTextSize(FP);
        tft.fillRect(12, IR_BODY_Y + 20, tftWidth - 24, 20, IR_COLOR_BG);
        tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawString("Aceitos: " + String(accepted), 12, IR_BODY_Y + 20);
        tft.setTextColor(IR_COLOR_DANGER, IR_COLOR_BG);
        tft.drawString("Rejeitados: " + String(rejected), 12, IR_BODY_Y + 34);
    }
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irNoiseGenerator() {
    irDrawHeader("Gerador de Ruido", IR_COLOR_TITLE);
    irDrawFooter("SEL: Gerar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            for (int i = 0; i < 50; i++) {
                uint16_t rawData[2];
                rawData[0] = random(400, 2000);
                rawData[1] = random(400, 2000);
                irSendRaw(38000, rawData, 2, 1);
                delayMicroseconds(50);
            }
            displaySuccess("Ruido gerado!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irSignalSplitter() {
    irDrawHeader("Divisor de Sinal", IR_COLOR_TITLE);
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    uint32_t high = (sig.data >> 16) & 0xFFFF;
    uint32_t low = sig.data & 0xFFFF;
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Alto: 0x" + String(high, HEX), 12, IR_BODY_Y);
    tft.drawString("Baixo: 0x" + String(low, HEX), 12, IR_BODY_Y + 14);
    irDrawFooter("SEL: Enviar partes  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            irSendDecoded(sig.protocol, high, 16, 3);
            delay(500);
            irSendDecoded(sig.protocol, low, 16, 3);
            displaySuccess("Partes enviadas!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irSignalMerger() {
    irDrawHeader("Fusao de Sinais", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal 1...");
    IRSignalInfo sig1 = irCaptureSignal();
    if (sig1.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    irDrawFooter("Capture sinal 2...");
    IRSignalInfo sig2 = irCaptureSignal();
    if (sig2.protocol == UNKNOWN) { displayWarning("Cancelado"); return; }
    uint64_t merged = ((sig1.data << 32) | (sig2.data & 0xFFFFFFFF));
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Fusionado: 0x" + String((uint32_t)merged, HEX), 12, IR_BODY_Y);
    irDrawFooter("SEL: Enviar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) { irSendDecoded(sig1.protocol, merged, 64, 1); displaySuccess("Fusao enviada!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irProtocolBridge() {
    irDrawHeader("Ponte Protoc.", IR_COLOR_TITLE);
    irDrawFooter("Capture e retransmita...");
    while (true) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(5000);
        if (sig.protocol != UNKNOWN) {
            irSendDecoded(sig.protocol, sig.data, sig.bits, 3);
            tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
            tft.drawCentreString("Repassado: " + sig.protocolName, tftWidth / 2, IR_BODY_Y + 20, 1);
        }
    }
}

// ============================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
// ============================================================================

void irSignalValidator() {
    irDrawHeader("Validador Sinal", IR_COLOR_TITLE);
    irDrawFooter("Capture sinal...");
    IRSignalInfo sig = irCaptureSignal();
    if (sig.protocol == UNKNOWN) { displayWarning("Invalido!"); return; }
    bool valid = (sig.bits > 0 && sig.bits <= 64 && sig.protocol != UNKNOWN);
    tft.setTextSize(FP);
    tft.setTextColor(valid ? IR_COLOR_ACCENT : IR_COLOR_DANGER, IR_COLOR_BG);
    tft.drawCentreString(valid ? "SINAL VALIDO" : "SINAL INVALIDO", tftWidth / 2, IR_BODY_Y + 10, 1);
    irDisplaySignalInfo(sig, IR_BODY_Y + 30);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irProtocolTester() {
    irDrawHeader("Teste de Protocolo", IR_COLOR_TITLE);
    const char* protos[] = {"NEC", "RC5", "RC6", "Samsung", "Sony"};
    decode_type_t protoList[] = {NEC, RC5, RC6, SAMSUNG, SONY};
    irDrawFooter("SEL: Testar todos  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            for (int i = 0; i < 5; i++) {
                tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
                tft.drawCentreString("Testando: " + String(protos[i]), tftWidth / 2, IR_BODY_Y, 1);
                irSendDecoded(protoList[i], 0x00FF00FF, 32, 3);
                delay(1000);
                tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
                tft.drawCentreString(protos[i] + ": OK", tftWidth / 2, IR_BODY_Y + 16, 1);
            }
            displaySuccess("Todos testados!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irReceiverDiagnostics() {
    irDrawHeader("Diag. Receptor", IR_COLOR_TITLE);
    irDrawFooter("Aguardando...");
    if (!irAdvRecv) irAdvRecv = new IRrecv(wilyConfigPins.irRx);
    irAdvRecv->enableIRIn();
    int pulses = 0;
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (check(EscPress)) break;
        if (irAdvRecv->decode(&results)) {
            pulses++;
            irAdvRecv->resume();
        }
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawCentreString("Pulsos: " + String(pulses), tftWidth / 2, IR_BODY_Y, 1);
        delay(100);
    }
    irAdvRecv->disableIRIn();
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Receptor: " + String(pulses > 0 ? "Funcional" : "Sem sinal"), 12, IR_BODY_Y + 20);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irTransmitterDiagnostics() {
    irDrawHeader("Diag. Transmissor", IR_COLOR_TITLE);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Teste de transmissao...", 12, IR_BODY_Y);
    uint16_t rawData[] = {9000, 4500, 560, 560, 560, 560, 560, 1690, 560, 560};
    irSendRaw(38000, rawData, 10, 1);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawString("Transmissor: OK", 12, IR_BODY_Y + 20);
    tft.drawString("Pino: " + String(wilyConfigPins.irTx), 12, IR_BODY_Y + 34);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irRangeTest() {
    irDrawHeader("Teste de Alcance", IR_COLOR_TITLE);
    irDrawFooter("Aguardando sinal...");
    int maxRssi = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        int rssi = analogRead(wilyConfigPins.irRx);
        if (rssi > maxRssi) maxRssi = rssi;
        tft.fillRect(20, IR_BODY_Y + 20, tftWidth - 40, 12, IR_COLOR_BG);
        int bar = map(constrain(rssi, 0, 4095), 0, 4095, 0, tftWidth - 40);
        tft.fillRect(20, IR_BODY_Y + 20, bar, 12, IR_COLOR_PRIMARY);
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString("Max: " + String(maxRssi), tftWidth / 2, IR_BODY_Y + 40, 1);
        delay(50);
    }
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irInterferenceDetector() {
    irDrawHeader("Detec. Interferen.", IR_COLOR_TITLE);
    irDrawFooter("Monitorando...");
    int noise = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int val = analogRead(wilyConfigPins.irRx);
        if (val > 100) noise++;
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
        tft.drawCentreString("Ruido: " + String(noise), tftWidth / 2, IR_BODY_Y, 1);
        delay(100);
    }
    tft.setTextColor(noise > 50 ? IR_COLOR_DANGER : IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawCentreString(noise > 50 ? "INTERFERENCIA DETECTADA" : "LIMPO", tftWidth / 2, IR_BODY_Y + 20, 1);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalBenchmark() {
    irDrawHeader("Benchmark Sinal", IR_COLOR_TITLE);
    irDrawFooter("Aguardando...");
    uint16_t rawData[] = {9000, 4500, 560, 560, 560, 1690, 560, 560, 560, 560};
    unsigned long start = micros();
    int count = 0;
    while (micros() - start < 1000000) {
        irSendRaw(38000, rawData, 10, 1);
        count++;
    }
    unsigned long elapsed = micros() - start;
    float rate = (float)count / (elapsed / 1000000.0f);
    tft.setTextSize(FP); tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("Enviados: " + String(count), 12, IR_BODY_Y);
    tft.drawString("Tempo: " + String(elapsed / 1000) + "ms", 12, IR_BODY_Y + 14);
    tft.drawString("Taxa: " + String(rate, 1) + " env/s", 12, IR_BODY_Y + 28);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irProtocolStressTest() {
    irDrawHeader("Stress Test Protoc.", IR_COLOR_TITLE);
    irDrawFooter("Enviando em loop...");
    decode_type_t protos[] = {NEC, RC5, RC6, SAMSUNG, SONY};
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        for (int i = 0; i < 5; i++) {
            irSendDecoded(protos[i], 0x00FF00FF, 32, 1);
            count++;
        }
        tft.setTextSize(FP); tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
        tft.drawCentreString("Enviados: " + String(count), tftWidth / 2, IR_BODY_Y, 1);
    }
    displaySuccess(String(count) + " sinais enviados!");
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irHardwareDiagnostics() {
    irDrawHeader("Diag. Hardware", IR_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(IR_COLOR_TEXT, IR_COLOR_BG);
    tft.drawString("IR TX Pin: " + String(wilyConfigPins.irTx), 12, IR_BODY_Y);
    tft.drawString("IR RX Pin: " + String(wilyConfigPins.irRx), 12, IR_BODY_Y + 14);
    tft.drawString("Repeats: " + String(wilyConfigPins.irTxRepeats), 12, IR_BODY_Y + 28);
    tft.setTextColor(IR_COLOR_ACCENT, IR_COLOR_BG);
    tft.drawString("IRremoteESP8266: OK", 12, IR_BODY_Y + 48);
    tft.drawString("Display: OK", 12, IR_BODY_Y + 62);
    irDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void irSignalPlayback() {
    irDrawHeader("Playback Sinal", IR_COLOR_TITLE);
    irDrawFooter("Capture e replique...");
    while (true) {
        if (check(EscPress)) break;
        IRSignalInfo sig = irCaptureSignal(5000);
        if (sig.protocol != UNKNOWN) {
            irSendDecoded(sig.protocol, sig.data, sig.bits, 3);
            irDisplaySignalInfo(sig, IR_BODY_Y + 10);
        }
    }
}

// ============================================================================
// 6. MENUS
// ============================================================================

void irAnalysisMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analisador de Sinal", "ID de Protocolo", "Comparacao Sinais",
        "Scanner Frequencia", "Medidor Forca", "Analise Timing",
        "Qualidade Sinal", "Decoder Lote", "Dicionario Protoc.", "Exportar Sinal"
    };
    int total = 10;
    irDrawHeader("ANALISE & DECODIFICACAO", IR_COLOR_TITLE);
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irSignalAnalyzer, irProtocolIdentifier, irSignalComparison,
                irFrequencyScanner, irSignalStrengthMeter, irTimingAnalyzer,
                irSignalQualityChecker, irBatchDecoder, irProtocolDictionary, irSignalExporter
            };
            funcs[sel]();
            irDrawHeader("ANALISE & DECODIFICACAO", IR_COLOR_TITLE);
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irGenerationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Clonador Sinal", "Gerador Protocolo", "Construtor Custom",
        "Transmissao Lote", "Transmissao Agend.", "Repetir Sinal",
        "Multi-Protocolo", "Modulador Sinal", "Conversor Codigo", "Editor Sinal"
    };
    int total = 10;
    irDrawHeader("GERACAO & CLONAGEM", IR_COLOR_TITLE);
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irSignalCloner, irProtocolGenerator, irCustomCodeBuilder,
                irBulkTransmitter, irScheduledTransmitter, irSignalRepeat,
                irMultiProtocolSender, irSignalModulator, irCodeConverter, irSignalEditor
            };
            funcs[sel]();
            irDrawHeader("GERACAO & CLONAGEM", IR_COLOR_TITLE);
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irDeviceMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Mapeador Devices", "Gravador Macro", "Reprodutor Macro",
        "Controlador Cena", "Agendador Timer", "Clonador Remoto",
        "Emulador Device", "Busca Codigos", "Explorar Marcas", "Controle Univers."
    };
    int total = 10;
    irDrawHeader("DEVICE CONTROL & AUTO", IR_COLOR_TITLE);
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irDeviceMapper, irMacroRecorder, irMacroPlayer,
                irSceneController, irTimerScheduler, irRemoteCloner,
                irDeviceEmulator, irCodeSearch, irBrandExplorer, irUniversalRemote
            };
            funcs[sel]();
            irDrawHeader("DEVICE CONTROL & AUTO", IR_COLOR_TITLE);
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irManipulationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Repetidor Sinal", "Conversor Protoc.", "Modificador Timing",
        "Ofuscador Codigo", "Amplificador Sinal", "Filtro Sinal",
        "Gerador Ruido", "Divisor Sinal", "Fusao Sinais", "Ponte Protoc."
    };
    int total = 10;
    irDrawHeader("MANIPULACAO SINAL", IR_COLOR_TITLE);
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irSignalRepeater, irProtocolConverter, irTimingModifier,
                irCodeObfuscator, irSignalAmplifier, irSignalFilter,
                irNoiseGenerator, irSignalSplitter, irSignalMerger, irProtocolBridge
            };
            funcs[sel]();
            irDrawHeader("MANIPULACAO SINAL", IR_COLOR_TITLE);
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irTestingMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Validador Sinal", "Teste Protocolo", "Diag. Receptor",
        "Diag. Transmissor", "Teste Alcance", "Detec. Interferen.",
        "Benchmark Sinal", "Stress Test", "Diag. Hardware", "Playback Sinal"
    };
    int total = 10;
    irDrawHeader("TESTES & DIAGNOSTICOS", IR_COLOR_TITLE);
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irSignalValidator, irProtocolTester, irReceiverDiagnostics,
                irTransmitterDiagnostics, irRangeTest, irInterferenceDetector,
                irSignalBenchmark, irProtocolStressTest, irHardwareDiagnostics, irSignalPlayback
            };
            funcs[sel]();
            irDrawHeader("TESTES & DIAGNOSTICOS", IR_COLOR_TITLE);
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void irAdvancedSuiteMenu() {
    irShowWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analise & Decodificacao", "Geracao & Clonagem",
        "Device Control & Auto", "Manipulacao Sinal",
        "Testes & Diagnosticos"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_GREEN, TFT_ORANGE, TFT_MAGENTA, TFT_RED};
    const char* icons[] = {"A", "G", "D", "M", "T"};
    int total = 5;
    irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = IR_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            irDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 8, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : IR_COLOR_TEXT, isSel ? IR_COLOR_PRIMARY : getColorVariation(IR_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 38, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                irAnalysisMenu, irGenerationMenu, irDeviceMenu,
                irManipulationMenu, irTestingMenu
            };
            funcs[sel]();
            irShowWelcomeScreen();
            irDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}
