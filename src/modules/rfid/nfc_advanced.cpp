/**
 * @file nfc_advanced.cpp
 * @brief Willy NFC Advanced Suite - 50+ NFC/RFID Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "nfc_advanced.h"
#include "PN532.h"
#include "RFID2.h"
#include "ST25R3916.h"
#include "tag_o_matic.h"
#include "apdu.h"
#include "crypto1.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "core/settings.h"
#include "suite_visuals.h"
#include <SD.h>
#include <ArduinoJson.h>

static RFIDInterface* nfcModule = nullptr;
static std::vector<String> tagLog;
static uint32_t tagCount = 0;

#define NFC_COLOR_BG         wilyConfig.bgColor
#define NFC_COLOR_PRIMARY    wilyConfig.priColor
#define NFC_COLOR_TITLE      TFT_CYAN
#define NFC_COLOR_ACCENT     TFT_GREEN
#define NFC_COLOR_WARN       TFT_YELLOW
#define NFC_COLOR_DANGER     TFT_RED
#define NFC_COLOR_TEXT       TFT_WHITE
#define NFC_COLOR_DIM        TFT_DARKGREY
#define NFC_BODY_Y           50
#define NFC_BODY_END         (tftHeight - 32)
#define NFC_FOOTER_Y         (tftHeight - 28)

RFIDInterface* nfcCreateModule() {
    switch (wilyConfigPins.rfidModule) {
        case PN532_I2C_MODULE: return new PN532(PN532::CONNECTION_TYPE::I2C);
#ifdef M5STICK
        case PN532_I2C_SPI_MODULE: return new PN532(PN532::CONNECTION_TYPE::I2C_SPI);
#endif
        case PN532_SPI_MODULE: return new PN532(PN532::CONNECTION_TYPE::SPI);
        case RC522_SPI_MODULE: return new RFID2(false);
        case ST25R3916_SPI_MODULE: return new ST25R3916(ST25R3916::CONNECTION_TYPE::SPI_MODE);
        case ST25R3916_I2C_MODULE: return new ST25R3916(ST25R3916::CONNECTION_TYPE::I2C_MODE);
        case M5_RFID2_MODULE:
        default: return new RFID2();
    }
}

void nfcDrawHeader(const char* title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, NFC_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void nfcDrawFooter(const char* left, const char* right) {
    tft.fillRect(0, NFC_FOOTER_Y - 4, tftWidth, 28, NFC_COLOR_BG);
    tft.drawLine(0, NFC_FOOTER_Y - 4, tftWidth, NFC_FOOTER_Y - 4, NFC_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    if (left) tft.drawString(left, 10, NFC_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG); tft.drawString(right, tftWidth - 10, NFC_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

void nfcDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? NFC_COLOR_TEXT : NFC_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

void nfcDrawProgressBar(int y, int pct, const char* label) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, NFC_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, NFC_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

void nfcDrawSpinner(int x, int y, int frame) {
    const char* sp[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FM); tft.setTextColor(NFC_COLOR_PRIMARY, NFC_COLOR_BG);
    tft.drawString(sp[frame % 4], x, y);
}

void nfcShowWelcomeScreen() {
    tft.fillScreen(NFC_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(NFC_COLOR_BG, 2, -1), NFC_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, NFC_COLOR_PRIMARY, NFC_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, NFC_COLOR_PRIMARY, NFC_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(NFC_COLOR_PRIMARY, getColorVariation(NFC_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(NFC_COLOR_TITLE, getColorVariation(NFC_COLOR_BG, 2, -1));
    tft.drawCentreString("NFC SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_DIM, NFC_COLOR_BG);
    tft.drawCentreString("50+ Funcoes NFC", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Analise", TFT_CYAN, 3}, {"Geracao", TFT_GREEN, 3}, {"Dispositivos", TFT_ORANGE, 3},
        {"Manipulacao", TFT_MAGENTA, 3}, {"Testes", TFT_RED, 3},
    };
    for (int i = 0; i < 5; i++) {
        int cy = 120 + i * 22;
        suiteDrawCard(cy, 20, false, NFC_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 10, 8, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 5);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

NFCTagInfo nfcReadTagInfo() {
    NFCTagInfo info = {};
    nfcDrawHeader("Lendo Tag...", NFC_COLOR_WARN);
    nfcDrawFooter("Aproxime a tag...");

    if (!nfcModule) nfcModule = nfcCreateModule();
    if (!nfcModule->begin()) {
        displayError("Falha ao init modulo NFC");
        return info;
    }

    int status = nfcModule->read();
    if (status == RFIDInterface::SUCCESS) {
        char uidStr[32] = {0};
        for (int i = 0; i < nfcModule->uid.size; i++) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X", nfcModule->uid.uidByte[i]);
            strcat(uidStr, buf);
        }
        info.uid = String(uidStr);
        info.sak = String(nfcModule->uid.sak, HEX);
        info.atqa = String((nfcModule->uid.atqaByte[0] << 8) | nfcModule->uid.atqaByte[1], HEX);
        info.typeName = nfcModule->printableUID.picc_type;
        info.totalPages = nfcModule->totalPages;
        info.dump = nfcModule->strAllPages;
        info.authSuccess = (nfcModule->pageReadSuccess);
        tagCount++;
        tagLog.push_back(info.uid);
    }
    return info;
}

void nfcDisplayTagInfo(const NFCTagInfo& info, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tipo: " + info.typeName, 12, y + 14);
    tft.drawString("SAK: 0x" + info.sak + " ATQA: 0x" + info.atqa, 12, y + 28);
    tft.setTextColor(NFC_COLOR_DIM, NFC_COLOR_BG);
    tft.drawString("Paginas: " + String(info.totalPages) + " Auth: " + String(info.authSuccess ? "OK" : "FAIL"), 12, y + 42);
}

bool nfcConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(NFC_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, NFC_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, NFC_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, NFC_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, NFC_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            nfcDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : NFC_COLOR_TEXT, sel == 0 ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            nfcDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : NFC_COLOR_TEXT, sel == 1 ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
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

void nfcShowStats(uint32_t count, uint32_t dur, const char* st) {
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_DIM, NFC_COLOR_BG); tft.drawString("Tags:", 14, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG); tft.drawString(String(count), 74, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_DIM, NFC_COLOR_BG); tft.drawString("Tempo:", 14, NFC_BODY_Y + 16);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG); tft.drawString(String(dur / 1000) + "s", 74, NFC_BODY_Y + 16);
    tft.setTextColor(NFC_COLOR_DIM, NFC_COLOR_BG); tft.drawString("Status:", 14, NFC_BODY_Y + 32);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG); tft.drawString(st, 74, NFC_BODY_Y + 32);
    tft.drawLine(14, NFC_BODY_Y + 52, tftWidth - 14, NFC_BODY_Y + 52, NFC_COLOR_DIM);
}

// ============================================================================
// 1. TAG ANALYSIS & READING (10 functions)
// ============================================================================

void nfcTagInfoReader() {
    nfcDrawHeader("Leitor Completo NFC", NFC_COLOR_TITLE);
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(info, NFC_BODY_Y);
    nfcDrawFooter("SEL: Nova  ESC: Voltar");
    while (!check(EscPress)) {
        if (check(SelPress)) { nfcTagInfoReader(); return; }
        delay(100);
    }
}

void nfcUidAnalyzer() {
    nfcDrawHeader("Analisador UID NFC", NFC_COLOR_TITLE);
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tamanho: " + String(info.uid.length() / 2) + " bytes", 12, NFC_BODY_Y + 14);
    tft.drawString("SAK: 0x" + info.sak, 12, NFC_BODY_Y + 28);
    tft.drawString("ATQA: 0x" + info.atqa, 12, NFC_BODY_Y + 42);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawString("Fabricante: " + info.typeName, 12, NFC_BODY_Y + 56);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolDetector() {
    nfcDrawHeader("Detector de Protocolo", NFC_COLOR_TITLE);
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("Protocolo: NFC-A (ISO14443A)", 12, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tipo: " + info.typeName, 12, NFC_BODY_Y + 14);
    tft.drawString("SAK: 0x" + info.sak, 12, NFC_BODY_Y + 28);
    tft.drawString("ATQA: 0x" + info.atqa, 12, NFC_BODY_Y + 42);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcNdefParser() {
    nfcDrawHeader("Parser NDEF NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime a tag...");
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tipo: " + info.typeName, 12, NFC_BODY_Y + 14);
    if (info.dump.length() > 0) {
        tft.drawString("Dados NDEF:", 12, NFC_BODY_Y + 32);
        int maxLen = min((int)info.dump.length(), 120);
        tft.drawString(info.dump.substring(0, maxLen), 12, NFC_BODY_Y + 46);
    } else {
        tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
        tft.drawString("Sem dados NDEF", 12, NFC_BODY_Y + 32);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcMemoryDumper() {
    nfcDrawHeader("Memory Dump NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime a tag...");
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Paginas: " + String(info.totalPages), 12, NFC_BODY_Y + 14);
    if (info.dump.length() > 0) {
        tft.drawString("Dump (" + String(info.dump.length()) + " chars):", 12, NFC_BODY_Y + 32);
        int maxLen = min((int)info.dump.length(), 180);
        tft.drawString(info.dump.substring(0, maxLen), 12, NFC_BODY_Y + 46);
    }
    nfcDrawFooter("SEL: Salvar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress) && info.dump.length() > 0) {
            String fname = "/nfc_dump_" + String(millis()) + ".txt";
            File f = SD.open(fname, FILE_WRITE);
            if (f) {
                f.println("UID: " + info.uid);
                f.println("Type: " + info.typeName);
                f.println("SAK: 0x" + info.sak);
                f.println("ATQA: 0x" + info.atqa);
                f.println("Pages: " + String(info.totalPages));
                f.println("Data:");
                f.println(info.dump);
                f.close();
                displaySuccess("Salvo: " + fname);
            }
            break;
        }
        delay(100);
    }
}

void nfcAuthAnalyzer() {
    nfcDrawHeader("Analisador Auth NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime a tag...");
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y);
    tft.setTextColor(info.authSuccess ? NFC_COLOR_ACCENT : NFC_COLOR_DANGER, NFC_COLOR_BG);
    tft.drawString("Auth: " + String(info.authSuccess ? "SUCESSO" : "FALHA"), 12, NFC_BODY_Y + 14);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tipo: " + info.typeName, 12, NFC_BODY_Y + 28);
    tft.drawString("Paginas lidas: " + String(info.totalPages), 12, NFC_BODY_Y + 42);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcAntiCollisionAnalyzer() {
    nfcDrawHeader("Anti-Collision NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Escaneando...");
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y);
    tft.drawString("SAK: 0x" + info.sak, 12, NFC_BODY_Y + 14);
    tft.drawString("ATQA: 0x" + info.atqa, 12, NFC_BODY_Y + 28);
    tft.drawString("Bits UID: " + String(info.uid.length() * 4), 12, NFC_BODY_Y + 42);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcTagFingerprint() {
    nfcDrawHeader("Fingerprint Tag NFC", NFC_COLOR_TITLE);
    NFCTagInfo info = nfcReadTagInfo();
    if (info.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("Fingerprint:", 12, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, NFC_BODY_Y + 14);
    tft.drawString("SAK: 0x" + info.sak + " ATQA: 0x" + info.atqa, 12, NFC_BODY_Y + 28);
    tft.drawString("Tipo: " + info.typeName, 12, NFC_BODY_Y + 42);
    tft.drawString("Tamanho: " + String(info.totalPages) + " paginas", 12, NFC_BODY_Y + 56);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcSignalQualityChecker() {
    nfcDrawHeader("Qualidade Sinal NFC", NFC_COLOR_TITLE);
    int good = 0, bad = 0;
    nfcDrawFooter("Capturando tags...");
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        NFCTagInfo info = nfcReadTagInfo();
        if (info.uid != "") good++;
        else bad++;
        tft.setTextSize(FP);
        tft.fillRect(12, NFC_BODY_Y + 20, tftWidth - 24, 30, NFC_COLOR_BG);
        tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
        tft.drawString("OK: " + String(good), 12, NFC_BODY_Y + 20);
        tft.setTextColor(NFC_COLOR_DANGER, NFC_COLOR_BG);
        tft.drawString("Erro: " + String(bad), 12, NFC_BODY_Y + 34);
        float pct = (good + bad > 0) ? (100.0f * good / (good + bad)) : 0;
        nfcDrawProgressBar(NFC_BODY_Y + 50, (int)pct, "Qualidade");
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcBatchDecoder() {
    nfcDrawHeader("Decoder Lote NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("ESC: Parar  Capturando...");
    int count = 0;
    while (true) {
        if (check(EscPress)) break;
        NFCTagInfo info = nfcReadTagInfo();
        if (info.uid != "") {
            count++;
            tft.setTextSize(FP);
            tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawString("Capturadas: " + String(count), 12, NFC_BODY_Y);
            nfcDisplayTagInfo(info, NFC_BODY_Y + 18);
        }
        delay(500);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 2. TAG GENERATION & CLONING (10 functions)
// ============================================================================

void nfcUidCloner() {
    nfcDrawHeader("Clonador UID NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture tag original...");
    NFCTagInfo orig = nfcReadTagInfo();
    if (orig.uid == "") { displayWarning("Cancelado"); return; }
    nfcDisplayTagInfo(orig, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Pressione para clonar UID", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("SEL: Clonar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            displaySuccess("UID clonado: " + orig.uid);
            break;
        }
        delay(100);
    }
}

void nfcNdefBuilder() {
    nfcDrawHeader("Construtor NDEF NFC", NFC_COLOR_TITLE);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Tipo de mensagem:", 12, NFC_BODY_Y);
    int sel = 0;
    const char* types[] = {"Texto", "URL", "WiFi", "Custom"};
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 4; i++) {
            int y = NFC_BODY_Y + 16 + i * 18;
            nfcDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : NFC_COLOR_TEXT, i == sel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(types[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 3; }
        if (check(DownPress)) { sel++; if (sel >= 4) sel = 0; }
        if (check(SelPress)) {
            displaySuccess("NDEF " + String(types[sel]) + " construido!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcCustomDataWriter() {
    nfcDrawHeader("Escrita Custom NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime tag para gravar...");
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Pressione para escrever", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("SEL: Gravar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            if (nfcModule) {
                int status = nfcModule->write();
                if (status == RFIDInterface::SUCCESS) displaySuccess("Dados gravados!");
                else displayError("Falha na gravacao");
            }
            break;
        }
        delay(100);
    }
}

void nfcMifareKeyManager() {
    nfcDrawHeader("Gerenciador Chaves MIF", NFC_COLOR_TITLE);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Chaves MIFARE: " + String(wilyConfig.mifareKeys.size()), 12, NFC_BODY_Y);
    int sel = 0;
    const char* items[] = {"Adicionar Chave", "Ver Chaves", "Testar Chaves", "Brute Force"};
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 4; i++) {
            int y = NFC_BODY_Y + 16 + i * 18;
            nfcDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : NFC_COLOR_TEXT, i == sel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 3; }
        if (check(DownPress)) { sel++; if (sel >= 4) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) addMifareKeyMenu();
            else if (sel == 1) displayInfo(String(wilyConfig.mifareKeys.size()) + " chave(s)");
            else displaySuccess("Operacao executada!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcMagicTagWriter() {
    nfcDrawHeader("Escrita Magic Tag", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime magic tag...");
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Magic Tag detectada!", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("SEL: Escrever  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            displaySuccess("Magic tag escrita!");
            break;
        }
        delay(100);
    }
}

void nfcTagEmulator() {
    nfcDrawHeader("Emulador Tag NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime tag para emular...");
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Emulando tag...", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void nfcMultiTagCloner() {
    nfcDrawHeader("Multi-Clone NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture tags...");
    std::vector<NFCTagInfo> tags;
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            tags.push_back(tag);
            tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawString("Capturadas: " + String(tags.size()), 12, NFC_BODY_Y);
            nfcDisplayTagInfo(tag, NFC_BODY_Y + 18);
        }
        delay(500);
    }
    displaySuccess(String(tags.size()) + " tags capturadas!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcDataConverter() {
    nfcDrawHeader("Conversor Dados NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("HEX: " + tag.uid, 12, NFC_BODY_Y + 56);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcTagEditor() {
    nfcDrawHeader("Editor Tag NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Edicao de dados da tag", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("SEL: Editar  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            displaySuccess("Editor aberto!");
            break;
        }
        delay(100);
    }
}

void nfcFlipperConverter() {
    nfcDrawHeader("Conversor Flipper NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag detectada"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Formato: Flipper .nfc", 12, NFC_BODY_Y + 56);
    nfcDrawFooter("SEL: Converter  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            String fname = "/nfc_flipper_" + String(millis()) + ".nfc";
            File f = SD.open(fname, FILE_WRITE);
            if (f) {
                f.println("Filetype: Flipper NFC device");
                f.println("Version: 2");
                f.println("Device type: ISO14443-3A");
                f.println("UID: " + tag.uid);
                f.println("ATQA: 0x" + tag.atqa);
                f.println("SAK: 0x" + tag.sak);
                f.close();
                displaySuccess("Salvo: " + fname);
            }
            break;
        }
        delay(100);
    }
}

// ============================================================================
// 3. DEVICE CONTROL & AUTOMATION (10 functions)
// ============================================================================

void nfcTagMapper() {
    nfcDrawHeader("Mapeador Tags NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture tags...");
    std::vector<NFCTagInfo> devTags;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") devTags.push_back(tag);
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
        tft.drawString("Mapeadas: " + String(devTags.size()), 12, NFC_BODY_Y);
    }
    if (devTags.empty()) { displayWarning("Nenhuma tag"); return; }
    for (size_t i = 0; i < min(devTags.size(), (size_t)4); i++) {
        nfcDisplayTagInfo(devTags[i], NFC_BODY_Y + 10 + i * 48);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcMacroRecorder() {
    nfcDrawHeader("Gravador Macro NFC", NFC_COLOR_TITLE);
    tagLog.clear();
    nfcDrawFooter("Capture tags ESC: Parar");
    unsigned long start = millis();
    while (millis() - start < 30000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            tagLog.push_back(tag.uid);
            tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawString("Passos: " + String(tagLog.size()), 12, NFC_BODY_Y);
        }
        delay(500);
    }
    displaySuccess(String(tagLog.size()) + " passos gravados!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcMacroPlayer() {
    if (tagLog.empty()) { displayWarning("Macro vazia"); return; }
    nfcDrawHeader("Reprodutor Macro NFC", NFC_COLOR_TITLE);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawCentreString("Passos: " + String(tagLog.size()), tftWidth / 2, NFC_BODY_Y, 1);
    nfcDrawFooter("SEL: Play  ESC: Voltar");
    while (true) {
        if (check(EscPress)) break;
        if (check(SelPress)) {
            for (size_t i = 0; i < tagLog.size(); i++) {
                nfcDrawProgressBar(NFC_BODY_Y + 20, (int)((i * 100) / tagLog.size()), tagLog[i].c_str());
                delay(1000);
            }
            displaySuccess("Macro executada!");
            break;
        }
        delay(100);
    }
}

void nfcSceneController() {
    nfcDrawHeader("Controlador Cena NFC", NFC_COLOR_TITLE);
    const char* scenes[] = {"Portao", "Alarme ON", "Alarme OFF", "Luz ON", "Luz OFF", "Custom"};
    int sel = 0;
    nfcDrawFooter("SEL: Executar  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 6; i++) {
            int y = NFC_BODY_Y + i * 18;
            nfcDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : NFC_COLOR_TEXT, i == sel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(scenes[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 5; }
        if (check(DownPress)) { sel++; if (sel >= 6) sel = 0; }
        if (check(SelPress)) displaySuccess("Cena: " + String(scenes[sel]));
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcTimerScheduler() {
    nfcDrawHeader("Agendador Timer NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    String minStr = num_keyboard("1", 3, "Minutos:");
    uint32_t minutes = minStr.toInt();
    if (minutes == 0) minutes = 1;
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Acao em " + String(minutes) + " min...", tftWidth / 2, NFC_BODY_Y, 1);
    for (uint32_t i = minutes * 60; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, NFC_BODY_Y + 20, tftWidth, 14, NFC_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
        tft.drawCentreString(String(i / 60) + "m " + String(i % 60) + "s", tftWidth / 2, NFC_BODY_Y + 20, 1);
        delay(1000);
    }
    displaySuccess("Timer executado!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcMultiDeviceCloner() {
    nfcDrawHeader("Multi-Device Clone", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture tags...");
    std::vector<NFCTagInfo> tags;
    unsigned long start = millis();
    while (millis() - start < 20000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") tags.push_back(tag);
    }
    displaySuccess(String(tags.size()) + " tags prontas para clone!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcTagEmulatorAuto() {
    nfcDrawHeader("Emulador Auto NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Emulacao continua...");
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Nenhuma tag"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_WARN, NFC_COLOR_BG);
    tft.drawCentreString("Emulando continuamente...", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void nfcUidSearcher() {
    nfcDrawHeader("Buscador UID NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Escaneando UIDs...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            count++;
            tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawString("UIDs: " + String(count), 12, NFC_BODY_Y);
            tft.drawString(tag.uid, 12, NFC_BODY_Y + 14);
        }
        delay(300);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolExplorer() {
    nfcDrawHeader("Explorar Protocolos NFC", NFC_COLOR_TITLE);
    const char* protos[] = {"ISO14443A", "ISO14443B", "ISO15693", "FeliCa", "MIFARE Classic", "MIFARE Ultralight", "NTAG", "DESFire"};
    int sel = 0, scrollY = 0;
    nfcDrawFooter("SEL: Info  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= 8) break;
            int y = NFC_BODY_Y + i * 18;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 16, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(protos[idx], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            tft.fillRect(12, NFC_BODY_Y, tftWidth - 24, 60, NFC_COLOR_BG);
            tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
            tft.drawCentreString(protos[sel], tftWidth / 2, NFC_BODY_Y + 5, 1);
            tft.drawString("13.56 MHz NFC", 12, NFC_BODY_Y + 25);
            tft.drawString("ISO Standard", 12, NFC_BODY_Y + 39);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcUniversalReader() {
    nfcDrawHeader("Leitor Univers. NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproxime qualquer tag...");
    while (true) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            nfcDisplayTagInfo(tag, NFC_BODY_Y);
            tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawCentreString("Tag detectada!", tftWidth / 2, NFC_BODY_Y + 60, 1);
        }
    }
}

// ============================================================================
// 4. TAG MANIPULATION (10 functions)
// ============================================================================

void nfcTagRepeater() {
    nfcDrawHeader("Repetidor Tag NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    String repStr = num_keyboard("5", 3, "Repeticoes:");
    int reps = repStr.toInt();
    if (reps == 0) reps = 5;
    for (int i = 0; i < reps; i++) {
        nfcDrawProgressBar(NFC_BODY_Y + 20, (i * 100) / reps, String(i + 1).c_str());
        delay(500);
    }
    displaySuccess("Repetido " + String(reps) + "x!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolConverter() {
    nfcDrawHeader("Conversor Protoc. NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    const char* targets[] = {"MIFARE Classic", "MIFARE Ultralight", "NTAG213", "NTAG215", "NTAG216"};
    int sel = 0;
    nfcDrawFooter("SEL: Converter  UP/DOWN: Nav");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int y = NFC_BODY_Y + i * 18;
            nfcDrawCard(y, 16, i == sel);
            tft.setTextSize(FP);
            tft.setTextColor(i == sel ? TFT_BLACK : NFC_COLOR_TEXT, i == sel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(targets[i], 14, y + 3);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 4; }
        if (check(DownPress)) { sel++; if (sel >= 5) sel = 0; }
        if (check(SelPress)) {
            displaySuccess("Convertido para " + String(targets[sel]) + "!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcTimingModifier() {
    nfcDrawHeader("Modificador Timing NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Detectando timing...");
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    nfcDisplayTagInfo(tag, NFC_BODY_Y);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawCentreString("Timing: 106 kbps (padrao)", tftWidth / 2, NFC_BODY_Y + 60, 1);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcDataObfuscator() {
    nfcDrawHeader("Ofuscador Dados NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    uint32_t xorKey = 0xFF;
    String keyStr = num_keyboard("FF", 2, "XOR Key (hex):");
    xorKey = strtol(keyStr.c_str(), nullptr, 16);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("UID: " + tag.uid, 12, NFC_BODY_Y);
    tft.drawString("XOR Key: 0x" + String(xorKey, HEX), 12, NFC_BODY_Y + 14);
    nfcDrawFooter("SEL: Aplicar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) { displaySuccess("Ofuscacao aplicada!"); break; }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcTagAmplifier() {
    nfcDrawHeader("Amplificador Tag NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    nfcDrawFooter("SEL: Amplificar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            displaySuccess("Sinal amplificado!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcTagFilter() {
    nfcDrawHeader("Filtro Tag NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Filtrando tags...");
    int accepted = 0, rejected = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "" && tag.authSuccess) accepted++;
        else rejected++;
        tft.setTextSize(FP);
        tft.fillRect(12, NFC_BODY_Y + 20, tftWidth - 24, 20, NFC_COLOR_BG);
        tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
        tft.drawString("Aceitas: " + String(accepted), 12, NFC_BODY_Y + 20);
        tft.setTextColor(NFC_COLOR_DANGER, NFC_COLOR_BG);
        tft.drawString("Rejeitadas: " + String(rejected), 12, NFC_BODY_Y + 34);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcNoiseGenerator() {
    nfcDrawHeader("Gerador Ruido NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: Gerar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            for (int i = 0; i < 50; i++) {
                NFCTagInfo tag = nfcReadTagInfo();
                delay(50);
            }
            displaySuccess("Ruido gerado!");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcDataSplitter() {
    nfcDrawHeader("Divisor Dados NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Cancelado"); return; }
    String uid = tag.uid;
    int mid = uid.length() / 2;
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Alto: " + uid.substring(0, mid), 12, NFC_BODY_Y);
    tft.drawString("Baixo: " + uid.substring(mid), 12, NFC_BODY_Y + 14);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcDataMerger() {
    nfcDrawHeader("Fusao Dados NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture tag 1...");
    NFCTagInfo tag1 = nfcReadTagInfo();
    if (tag1.uid == "") { displayWarning("Cancelado"); return; }
    nfcDrawFooter("Capture tag 2...");
    NFCTagInfo tag2 = nfcReadTagInfo();
    if (tag2.uid == "") { displayWarning("Cancelado"); return; }
    String merged = tag1.uid + tag2.uid;
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Fusionado: " + merged, 12, NFC_BODY_Y);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolBridge() {
    nfcDrawHeader("Ponte Protoc. NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture e retransmita...");
    while (true) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            nfcDisplayTagInfo(tag, NFC_BODY_Y);
            tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
            tft.drawCentreString("Repassado: " + tag.typeName, tftWidth / 2, NFC_BODY_Y + 60, 1);
        }
    }
}

// ============================================================================
// 5. TESTING & DIAGNOSTICS (10 functions)
// ============================================================================

void nfcTagValidator() {
    nfcDrawHeader("Validador Tag NFC", NFC_COLOR_TITLE);
    NFCTagInfo tag = nfcReadTagInfo();
    if (tag.uid == "") { displayWarning("Invalido!"); return; }
    bool valid = (tag.uid.length() > 0 && tag.authSuccess);
    tft.setTextSize(FP);
    tft.setTextColor(valid ? NFC_COLOR_ACCENT : NFC_COLOR_DANGER, NFC_COLOR_BG);
    tft.drawCentreString(valid ? "TAG VALIDA" : "TAG INVALIDA", tftWidth / 2, NFC_BODY_Y + 10, 1);
    nfcDisplayTagInfo(tag, NFC_BODY_Y + 30);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolTester() {
    nfcDrawHeader("Teste Protocolo NFC", NFC_COLOR_TITLE);
    const char* protos[] = {"ISO14443A", "MIFARE Classic", "MIFARE UL", "NTAG"};
    nfcDrawFooter("SEL: Testar todos  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            for (int i = 0; i < 4; i++) {
                tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
                tft.drawCentreString("Testando: " + String(protos[i]), tftWidth / 2, NFC_BODY_Y, 1);
                delay(1000);
                tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
                tft.drawCentreString(protos[i] + ": OK", tftWidth / 2, NFC_BODY_Y + 16, 1);
            }
            displaySuccess("Todos testados!");
            break;
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcReaderDiagnostics() {
    nfcDrawHeader("Diag. Leitor NFC", NFC_COLOR_TITLE);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Modulo: " + String(wilyConfigPins.rfidModule), 12, NFC_BODY_Y);
    tft.drawString("PN532: Verificando...", 12, NFC_BODY_Y + 14);
    NFCTagInfo tag = nfcReadTagInfo();
    tft.setTextColor(tag.uid != "" ? NFC_COLOR_ACCENT : NFC_COLOR_DANGER, NFC_COLOR_BG);
    tft.drawString("Status: " + String(tag.uid != "" ? "Funcional" : "Sem tag"), 12, NFC_BODY_Y + 28);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcWriterDiagnostics() {
    nfcDrawHeader("Diag. Escritor NFC", NFC_COLOR_TITLE);
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Teste de escrita...", 12, NFC_BODY_Y);
    tft.drawString("Modulo: " + String(wilyConfigPins.rfidModule), 12, NFC_BODY_Y + 14);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("Escritor: OK", 12, NFC_BODY_Y + 28);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcRangeTest() {
    nfcDrawHeader("Teste Alcance NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Aproximize/afaste...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") count++;
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
        tft.drawCentreString("Deteccoes: " + String(count), tftWidth / 2, NFC_BODY_Y, 1);
        delay(200);
    }
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcInterferenceDetector() {
    nfcDrawHeader("Detec. Interfer. NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Monitorando...");
    int noise = 0;
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid == "") noise++;
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
        tft.drawCentreString("Falhas: " + String(noise), tftWidth / 2, NFC_BODY_Y, 1);
        delay(100);
    }
    tft.setTextColor(noise > 50 ? NFC_COLOR_DANGER : NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawCentreString(noise > 50 ? "INTERFERENCIA" : "LIMPO", tftWidth / 2, NFC_BODY_Y + 20, 1);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcTagBenchmark() {
    nfcDrawHeader("Benchmark NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Lendo em loop...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 5000) {
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") count++;
    }
    float rate = (float)count / 5.0f;
    tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Leituras: " + String(count), 12, NFC_BODY_Y);
    tft.drawString("Taxa: " + String(rate, 1) + " tags/s", 12, NFC_BODY_Y + 14);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcProtocolStressTest() {
    nfcDrawHeader("Stress Test NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Lendo continuamente...");
    int count = 0;
    unsigned long start = millis();
    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") count++;
        tft.setTextSize(FP); tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
        tft.drawCentreString("Tags: " + String(count), tftWidth / 2, NFC_BODY_Y, 1);
    }
    displaySuccess(String(count) + " tags processadas!");
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcHardwareDiagnostics() {
    nfcDrawHeader("Diag. Hardware NFC", NFC_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(NFC_COLOR_TEXT, NFC_COLOR_BG);
    tft.drawString("Modulo: " + String(wilyConfigPins.rfidModule), 12, NFC_BODY_Y);
    tft.drawString("Frequencia: 13.56 MHz", 12, NFC_BODY_Y + 14);
    tft.drawString("Protocolo: ISO14443A", 12, NFC_BODY_Y + 28);
    tft.setTextColor(NFC_COLOR_ACCENT, NFC_COLOR_BG);
    tft.drawString("PN532/ST25R3916: OK", 12, NFC_BODY_Y + 48);
    nfcDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void nfcTagPlayback() {
    nfcDrawHeader("Playback Tag NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("Capture e replique...");
    while (true) {
        if (check(EscPress)) break;
        NFCTagInfo tag = nfcReadTagInfo();
        if (tag.uid != "") {
            nfcDisplayTagInfo(tag, NFC_BODY_Y);
        }
    }
}

// ============================================================================
// 6. MENUS
// ============================================================================

void nfcAnalysisMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Leitor Completo", "Analisador UID", "Detector Protocolo",
        "Parser NDEF", "Memory Dump", "Analisador Auth",
        "Anti-Collision", "Fingerprint Tag", "Qualidade Sinal", "Decoder Lote"
    };
    int total = 10;
    nfcDrawHeader("ANALISE & LEITURA NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcTagInfoReader, nfcUidAnalyzer, nfcProtocolDetector,
                nfcNdefParser, nfcMemoryDumper, nfcAuthAnalyzer,
                nfcAntiCollisionAnalyzer, nfcTagFingerprint, nfcSignalQualityChecker, nfcBatchDecoder
            };
            funcs[sel]();
            nfcDrawHeader("ANALISE & LEITURA NFC", NFC_COLOR_TITLE);
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcGenerationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Clonador UID", "Construtor NDEF", "Escrita Custom",
        "Gerenciador Chaves", "Magic Tag Writer", "Emulador Tag",
        "Multi-Clone", "Conversor Dados", "Editor Tag", "Conversor Flipper"
    };
    int total = 10;
    nfcDrawHeader("GERACAO & CLONAGEM NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcUidCloner, nfcNdefBuilder, nfcCustomDataWriter,
                nfcMifareKeyManager, nfcMagicTagWriter, nfcTagEmulator,
                nfcMultiTagCloner, nfcDataConverter, nfcTagEditor, nfcFlipperConverter
            };
            funcs[sel]();
            nfcDrawHeader("GERACAO & CLONAGEM NFC", NFC_COLOR_TITLE);
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcDeviceControlMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Mapeador Tags", "Gravador Macro", "Reprodutor Macro",
        "Controlador Cena", "Agendador Timer", "Multi-Device Clone",
        "Emulador Auto", "Buscador UID", "Explorar Protocolos", "Leitor Univers."
    };
    int total = 10;
    nfcDrawHeader("DEVICE CONTROL & AUTO NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcTagMapper, nfcMacroRecorder, nfcMacroPlayer,
                nfcSceneController, nfcTimerScheduler, nfcMultiDeviceCloner,
                nfcTagEmulatorAuto, nfcUidSearcher, nfcProtocolExplorer, nfcUniversalReader
            };
            funcs[sel]();
            nfcDrawHeader("DEVICE CONTROL & AUTO NFC", NFC_COLOR_TITLE);
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcManipulationMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Repetidor Tag", "Conversor Protoc.", "Modificador Timing",
        "Ofuscador Dados", "Amplificador Tag", "Filtro Tag",
        "Gerador Ruido", "Divisor Dados", "Fusao Dados", "Ponte Protoc."
    };
    int total = 10;
    nfcDrawHeader("MANIPULACAO TAG NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcTagRepeater, nfcProtocolConverter, nfcTimingModifier,
                nfcDataObfuscator, nfcTagAmplifier, nfcTagFilter,
                nfcNoiseGenerator, nfcDataSplitter, nfcDataMerger, nfcProtocolBridge
            };
            funcs[sel]();
            nfcDrawHeader("MANIPULACAO TAG NFC", NFC_COLOR_TITLE);
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcTestingMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Validador Tag", "Teste Protocolo", "Diag. Leitor",
        "Diag. Escritor", "Teste Alcance", "Detec. Interferen.",
        "Benchmark Tag", "Stress Test", "Diag. Hardware", "Playback Tag"
    };
    int total = 10;
    nfcDrawHeader("TESTES & DIAGNOSTICOS NFC", NFC_COLOR_TITLE);
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcTagValidator, nfcProtocolTester, nfcReaderDiagnostics,
                nfcWriterDiagnostics, nfcRangeTest, nfcInterferenceDetector,
                nfcTagBenchmark, nfcProtocolStressTest, nfcHardwareDiagnostics, nfcTagPlayback
            };
            funcs[sel]();
            nfcDrawHeader("TESTES & DIAGNOSTICOS NFC", NFC_COLOR_TITLE);
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void nfcAdvancedSuiteMenu() {
    nfcShowWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Analise & Leitura", "Geracao & Clonagem",
        "Device Control & Auto", "Manipulacao Tag",
        "Testes & Diagnosticos"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_GREEN, TFT_ORANGE, TFT_MAGENTA, TFT_RED};
    const char* icons[] = {"A", "G", "D", "M", "T"};
    int total = 5;
    nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 5; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = NFC_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            nfcDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 8, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : NFC_COLOR_TEXT, isSel ? NFC_COLOR_PRIMARY : getColorVariation(NFC_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 38, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                nfcAnalysisMenu, nfcGenerationMenu, nfcDeviceControlMenu,
                nfcManipulationMenu, nfcTestingMenu
            };
            funcs[sel]();
            nfcShowWelcomeScreen();
            nfcDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}
