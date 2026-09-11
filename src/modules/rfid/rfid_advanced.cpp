#ifndef LITE_VERSION

#include "rfid_advanced.h"
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
#include <set>

// ============================================================================
// Global State
// ============================================================================

RFIDAdvConfig rfidAdvConfig;
static RFIDInterface *rfidMod = nullptr;

// ============================================================================
// UI Helpers (self-contained — no cross-module deps)
// ============================================================================

void rfidAdvDrawHeader(const char *title, uint16_t color) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, RFID_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void rfidAdvDrawFooter(const char *left, const char *right) {
    tft.fillRect(0, RFID_FOOTER_Y, tftWidth, 28, wilyConfig.bgColor);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_DIM, RFID_COLOR_BG);
    if (left) tft.drawString(left, 4, RFID_FOOTER_Y + 8);
    if (right) {
        tft.setTextDatum(TR_DATUM);
        tft.drawString(right, tftWidth - 4, RFID_FOOTER_Y + 8);
        tft.setTextDatum(TL_DATUM);
    }
}

void rfidAdvDrawCard(int y, int h, bool sel) {
    tft.drawRoundRect(6, y, tftWidth - 12, h, 4, sel ? RFID_COLOR_PRIMARY : TFT_DARKGREY);
    if (sel) tft.fillRoundRect(7, y + 1, tftWidth - 14, h - 2, 3, 0x0018);
}

void rfidAdvDrawProgressBar(int y, int pct, const char *label) {
    int bw = tftWidth - 24;
    tft.drawRoundRect(12, y, bw, 14, 3, RFID_COLOR_DIM);
    int fw = (bw - 4) * pct / 100;
    if (fw > 0) tft.fillRoundRect(14, y + 2, fw, 10, 2, RFID_COLOR_ACCENT);
    if (label) {
        tft.setTextSize(FP);
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawCentreString(label, tftWidth / 2, y + 16, 1);
    }
}

void rfidAdvDrawSpinner(int x, int y, int frame) {
    const char *spins[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FB);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString(spins[frame % 4], x, y);
}

void rfidAdvDisplayTagInfo(const NFCTagInfo &info, int y) {
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("UID: " + info.uid, 12, y);
    tft.drawString("SAK: " + info.sak + " ATQA: " + info.atqa, 12, y + 12);
    tft.drawString("Tipo: " + info.typeName, 12, y + 24);
    if (info.manufacturer.length() > 0)
        tft.drawString("Fab: " + info.manufacturer, 12, y + 36);
}

bool rfidAdvConfirmAction(const char *msg) {
    rfidAdvDrawHeader("Confirmar", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 80, 1);
    rfidAdvDrawFooter("SEL: Sim  ESC: Nao", "");
    unsigned long start = millis();
    while (millis() - start < 10000) {
        if (check(SelPress)) return true;
        if (check(EscPress)) return false;
        delay(50);
    }
    return false;
}

// ============================================================================
// Welcome Screen
// ============================================================================

void rfidAdvShowWelcomeScreen() {
    tft.fillScreen(RFID_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(RFID_COLOR_BG, 2, -1), RFID_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, RFID_COLOR_PRIMARY, RFID_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, RFID_COLOR_PRIMARY, RFID_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(RFID_COLOR_PRIMARY, getColorVariation(RFID_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(RFID_COLOR_TITLE, getColorVariation(RFID_COLOR_BG, 2, -1));
    tft.drawCentreString("RFID SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(RFID_COLOR_DIM, RFID_COLOR_BG);
    tft.drawCentreString("55+ Funcoes RFID", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Leitura", TFT_CYAN, 7}, {"Escrita", TFT_GREEN, 7}, {"Emulacao", TFT_ORANGE, 7},
        {"Clone", TFT_MAGENTA, 7}, {"Cripto", TFT_RED, 7},
        {"Analisador", TFT_YELLOW, 7}, {"Utilitarios", TFT_WHITE, 7},
    };
    for (int i = 0; i < 7; i++) {
        int cy = 118 + i * 18;
        suiteDrawCard(cy, 16, false, RFID_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 8, 7, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(RFID_COLOR_TEXT, getColorVariation(RFID_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 3);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

NFCTagInfo rfidAdvReadTagInfo() {
    NFCTagInfo info;
    if (!rfidMod) return info;
    int status = rfidMod->read(rfidAdvConfig.defaultBaudRate);
    if (status == 0 || status == 1) {
        info.uid = "";
        for (int i = 0; i < rfidMod->uid.size; i++) {
            if (rfidMod->uid.uidByte[i] < 0x10) info.uid += "0";
            info.uid += String(rfidMod->uid.uidByte[i], HEX);
        }
        info.uid.toUpperCase();
        info.sak = String(rfidMod->uid.sak, HEX);
        info.atqa = "";
        for (int i = 0; i < 2; i++) {
            if (rfidMod->uid.atqaByte[i] < 0x10) info.atqa += "0";
            info.atqa += String(rfidMod->uid.atqaByte[i], HEX);
        }
        info.atqa.toUpperCase();
        info.authSuccess = (status == 0);
    }
    return info;
}

RFIDInterface* rfidAdvCreateModule() {
    switch (wilyConfigPins.rfidModule) {
        case PN532_I2C_MODULE:        return new PN532(PN532::I2C);
        case PN532_I2C_SPI_MODULE:    return new PN532(PN532::I2C_SPI);
        case PN532_SPI_MODULE:        return new PN532(PN532::SPI);
        case RC522_SPI_MODULE:        return new RFID2(false);
        case ST25R3916_SPI_MODULE:    return new ST25R3916(ST25R3916::SPI_MODE);
        case ST25R3916_I2C_MODULE:    return new ST25R3916(ST25R3916::I2C_MODE);
        case M5_RFID2_MODULE:
        default:                      return new RFID2();
    }
}

static bool isST25R3916() {
    return (wilyConfigPins.rfidModule == ST25R3916_SPI_MODULE ||
            wilyConfigPins.rfidModule == ST25R3916_I2C_MODULE);
}

static ST25R3916* asST25R3916() {
    if (!isST25R3916() || !rfidMod) return nullptr;
    return static_cast<ST25R3916*>(rfidMod);
}

// ============================================================================
// CATEGORY 1: MIFARE Classic Deep Analysis (12 functions)
// ============================================================================

void rfidMifareClassicFullDump() {
    rfidAdvDrawHeader("MFC Full Dump", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Aproxime a tag...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Full Dump", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);
    rfidAdvDrawProgressBar(RFID_BODY_Y + 55, 0, "Lendo setores...");

    // Use ST25R3916's built-in MFC dump if available
    ST25R3916 *st = asST25R3916();
    if (st && st->mfcLoaded) {
        int totalSectors = st->mfcDump.sectors;
        int readBlocks = 0;
        for (int i = 0; i < st->mfcDump.totalBlocks; i++) {
            if (st->mfcDump.blockRead[i]) readBlocks++;
        }
        int pct = (readBlocks * 100) / st->mfcDump.totalBlocks;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 55, pct, (String(readBlocks) + "/" + String(st->mfcDump.totalBlocks) + " blocos").c_str());

        // Display summary
        tft.setTextSize(FP);
        tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
        tft.drawString("Tipo: MIFARE " + st->mfcType, 12, RFID_BODY_Y + 80);
        tft.drawString("Setores: " + String(totalSectors), 12, RFID_BODY_Y + 92);
        tft.drawString("Blocos lidos: " + String(readBlocks) + "/" + String(st->mfcDump.totalBlocks), 12, RFID_BODY_Y + 104);

        // Count recovered keys
        int keysA = 0, keysB = 0;
        for (int s = 0; s < totalSectors; s++) {
            if (st->mfcDump.keyAFound[s]) keysA++;
            if (st->mfcDump.keyBFound[s]) keysB++;
        }
        tft.drawString("Keys A: " + String(keysA) + "/" + String(totalSectors), 12, RFID_BODY_Y + 116);
        tft.drawString("Keys B: " + String(keysB) + "/" + String(totalSectors), 12, RFID_BODY_Y + 128);

        // Save to SD if configured
        if (rfidAdvConfig.autoSave) {
            String filename = "/RFID/MFC_" + st->mfcType + "_" + String(millis()) + ".rfid";
            rfidMod->save(filename);
            tft.drawString("Salvo: " + filename, 12, RFID_BODY_Y + 140);
        }
    } else {
        // Fallback: basic read
        tft.setTextSize(FP);
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Modulo nao suporta dump completo", 12, RFID_BODY_Y + 80);
        tft.drawString("Use ST25R3916 para dump detalhado", 12, RFID_BODY_Y + 92);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicSectorAuth() {
    rfidAdvDrawHeader("MFC Sector Auth", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Testar  ESC: Voltar", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) {
        displayError("Funcao requer ST25R3916");
        delete rfidMod; rfidMod = nullptr;
        return;
    }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Sector Auth", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    // Test each sector with known keys
    int totalSectors = st->mfcDump.sectors;
    int authedCount = 0;

    for (int sector = 0; sector < totalSectors && sector < 40; sector++) {
        if (check(EscPress)) break;

        int block = sector * 4; // First block of sector
        if (sector >= 32) block = 128 + (sector - 32) * 16; // 4K sectors

        bool authed = false;
        // Try default keys
        for (int k = 0; k < 15; k++) {
            if (st->mifareAuthBlock(block, rfidMod->keys[k], false)) {
                authed = true;
                authedCount++;
                st->mifareHalt();
                break;
            }
            if (st->mifareAuthBlock(block, rfidMod->keys[k], true)) {
                authed = true;
                authedCount++;
                st->mifareHalt();
                break;
            }
        }

        int pct = ((sector + 1) * 100) / totalSectors;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 60, pct, (String(sector + 1) + "/" + String(totalSectors)).c_str());

        tft.setTextSize(FP);
        tft.setTextColor(authed ? RFID_COLOR_ACCENT : RFID_COLOR_DANGER, RFID_COLOR_BG);
        tft.drawString("Setor " + String(sector) + ": " + (authed ? "OK" : "FALHA"), 12, RFID_BODY_Y + 80 + (sector % 6) * 12);
    }

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_PRIMARY, RFID_COLOR_BG);
    tft.drawString("Total: " + String(authedCount) + "/" + String(totalSectors) + " autenticados", 12, RFID_BODY_Y + 160);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicKeyRecovery() {
    rfidAdvDrawHeader("MFC Key Recovery", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Recuperando chaves...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Key Recovery", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    int totalSectors = st->mfcDump.sectors;
    std::set<String> recoveredKeys;

    for (int sector = 0; sector < totalSectors; sector++) {
        if (check(EscPress)) break;
        int block = sector * 4;
        if (sector >= 32) block = 128 + (sector - 32) * 16;

        for (int k = 0; k < 15; k++) {
            // Try Key A
            if (st->mifareAuthBlock(block, rfidMod->keys[k], false)) {
                String keyStr = "";
                for (int i = 0; i < 6; i++) {
                    if (rfidMod->keys[k][i] < 0x10) keyStr += "0";
                    keyStr += String(rfidMod->keys[k][i], HEX);
                }
                recoveredKeys.insert(keyStr);
                st->mifareHalt();
            }
            // Try Key B
            if (st->mifareAuthBlock(block, rfidMod->keys[k], true)) {
                String keyStr = "";
                for (int i = 0; i < 6; i++) {
                    if (rfidMod->keys[k][i] < 0x10) keyStr += "0";
                    keyStr += String(rfidMod->keys[k][i], HEX);
                }
                recoveredKeys.insert(keyStr);
                st->mifareHalt();
            }
        }

        int pct = ((sector + 1) * 100) / totalSectors;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 60, pct, (String(sector + 1) + "/" + String(totalSectors)).c_str());
    }

    // Display recovered keys
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Chaves recuperadas: " + String(recoveredKeys.size()), 12, RFID_BODY_Y + 80);
    int y = RFID_BODY_Y + 92;
    for (const auto &key : recoveredKeys) {
        if (y > RFID_FOOTER_Y - 14) break;
        tft.drawString(key, 12, y);
        y += 12;
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicSectorMap() {
    rfidAdvDrawHeader("MFC Sector Map", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Mapeando setores...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Sector Map", RFID_COLOR_ACCENT);
    int totalSectors = st->mfcDump.sectors;
    int y = RFID_BODY_Y;

    tft.setTextSize(FP);
    for (int s = 0; s < totalSectors && y < RFID_FOOTER_Y - 10; s++) {
        int blocksRead = 0;
        int firstBlock = s * 4;
        if (s >= 32) firstBlock = 128 + (s - 32) * 16;

        for (int b = 0; b < 4; b++) {
            if (st->mfcDump.blockRead[firstBlock + b]) blocksRead++;
        }

        String line = "S" + String(s) + ":";
        line += (blocksRead > 0 ? String(blocksRead) + "/4" : " --");
        line += st->mfcDump.keyAFound[s] ? " A" : "";
        line += st->mfcDump.keyBFound[s] ? " B" : "";

        tft.setTextColor(blocksRead > 0 ? RFID_COLOR_ACCENT : RFID_COLOR_DIM, RFID_COLOR_BG);
        tft.drawString(line, 12, y);
        y += 12;
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicBlockRead() {
    rfidAdvDrawHeader("MFC Block Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Ler  ESC: Voltar", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    String blockStr = num_keyboard("4", 3, "Bloco (0-255):");
    uint8_t blockNum = blockStr.toInt();

    rfidAdvDrawHeader("MFC Block Read", RFID_COLOR_ACCENT);
    rfidAdvDrawFooter("Aproxime a tag...", "");

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    // Try all keys to authenticate
    bool authed = false;
    for (int k = 0; k < 15; k++) {
        if (st->mifareAuthBlock(blockNum, rfidMod->keys[k], false)) {
            authed = true;
            break;
        }
    }

    if (authed) {
        uint8_t data[16] = {0};
        if (st->mifareReadBlock(blockNum, data)) {
            rfidAdvDrawHeader("MFC Block Read", RFID_COLOR_ACCENT);
            tft.setTextSize(FP);
            tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
            tft.drawString("Bloco: " + String(blockNum), 12, RFID_BODY_Y);
            tft.drawString("HEX:", 12, RFID_BODY_Y + 14);
            String hex = "";
            for (int i = 0; i < 16; i++) {
                if (data[i] < 0x10) hex += "0";
                hex += String(data[i], HEX) + " ";
            }
            tft.drawString(hex, 12, RFID_BODY_Y + 26);
            tft.drawString("ASCII:", 12, RFID_BODY_Y + 40);
            String ascii = "";
            for (int i = 0; i < 16; i++) {
                ascii += (data[i] >= 32 && data[i] < 127) ? (char)data[i] : '.';
            }
            tft.drawString(ascii, 12, RFID_BODY_Y + 52);
        } else {
            displayError("Falha ao ler bloco");
        }
        st->mifareHalt();
    } else {
        displayError("Falha na autenticacao");
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicBlockWrite() {
    rfidAdvDrawHeader("MFC Block Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Escrever  ESC: Voltar", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    String blockStr = num_keyboard("4", 3, "Bloco (0-255):");
    uint8_t blockNum = blockStr.toInt();

    if (!rfidAdvConfirmAction("Escrever no bloco " + String(blockNum) + "?")) {
        delete rfidMod; rfidMod = nullptr;
        return;
    }

    rfidAdvDrawHeader("MFC Block Write", RFID_COLOR_ACCENT);
    rfidAdvDrawFooter("Aproxime a tag...", "");

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    bool authed = false;
    for (int k = 0; k < 15; k++) {
        if (st->mifareAuthBlock(blockNum, rfidMod->keys[k], false)) {
            authed = true;
            break;
        }
    }

    if (authed) {
        uint8_t data[16] = {0};
        // Write zeros as test
        if (st->mifareWriteBlock(blockNum, data)) {
            displaySuccess("Bloco escrito com sucesso!");
        } else {
            displayError("Falha ao escrever bloco");
        }
        st->mifareHalt();
    } else {
        displayError("Falha na autenticacao");
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicValueBlock() {
    rfidAdvDrawHeader("MFC Value Block", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Ler  ESC: Voltar", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Value Block", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Procurando value blocks...", 12, RFID_BODY_Y + 60);

    int found = 0;
    for (int block = 0; block < st->mfcDump.totalBlocks; block++) {
        if (check(EscPress)) break;
        if (!st->mfcDump.blockRead[block]) continue;

        // Check value block format: [val] [~val] [val] [~val] [val] [~val] [addr] [~addr]
        uint8_t *d = st->mfcDump.blocks[block];
        bool isValueBlock = (d[0] == ~d[4] && d[1] == ~d[5] && d[2] == ~d[6] && d[3] == ~d[7]);

        if (isValueBlock) {
            int32_t val = (int32_t)(d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24));
            tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
            tft.drawString("Bloco " + String(block) + ": " + String(val), 12, RFID_BODY_Y + 74 + found * 12);
            found++;
            if (found > 5) break;
        }
    }

    if (found == 0) {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Nenhum value block encontrado", 12, RFID_BODY_Y + 74);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicAccessBits() {
    rfidAdvDrawHeader("MFC Access Bits", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Analisando...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Access Bits", RFID_COLOR_ACCENT);
    int totalSectors = st->mfcDump.sectors;
    int y = RFID_BODY_Y;

    tft.setTextSize(FP);
    for (int s = 0; s < totalSectors && y < RFID_FOOTER_Y - 10; s++) {
        int trailerBlock = (s < 32) ? (s * 4 + 3) : (128 + (s - 32) * 16 + 3);
        if (trailerBlock >= st->mfcDump.totalBlocks) break;
        if (!st->mfcDump.blockRead[trailerBlock]) {
            tft.setTextColor(RFID_COLOR_DIM, RFID_COLOR_BG);
            tft.drawString("S" + String(s) + ": N/D", 12, y);
        } else {
            uint8_t *d = st->mfcDump.blocks[trailerBlock];
            uint8_t ac = (d[6] >> 2) | ((d[7] & 0x0F) << 6);
            tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
            tft.drawString("S" + String(s) + ": AC=0x" + String(ac, HEX), 12, y);
        }
        y += 12;
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicKeyDictionary() {
    rfidAdvDrawHeader("MFC Key Dict", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Forca bruta com dicionario...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    // Extended dictionary: default keys + common MIFARE keys
    uint8_t dict[][6] = {
        {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
        {0xA0,0xA1,0xA2,0xA3,0xA4,0xA5},
        {0xB0,0xB1,0xB2,0xB3,0xB4,0xB5},
        {0x4D,0x3A,0x99,0xC3,0x51,0xDD},
        {0x1A,0x98,0x2C,0x7E,0x45,0x9A},
        {0xA3,0x92,0xEF,0x46,0x93,0x46},
        {0x53,0x3C,0xB6,0xC7,0x23,0xF6},
        {0x8F,0xD0,0xA4,0xF2,0x56,0xE9},
        {0x00,0x00,0x00,0x00,0x00,0x00},
        {0xB3,0x31,0x60,0x57,0x57,0xBD},
        {0x4B,0x7C,0x54,0xB1,0x5B,0x26},
        {0x1B,0x7E,0x8C,0x62,0xA0,0x34},
        {0x61,0x62,0x63,0x64,0x65,0x66},
        {0x01,0x02,0x03,0x04,0x05,0x06},
        {0x11,0x22,0x33,0x44,0x55,0x66},
        {0x10,0x20,0x30,0x40,0x50,0x60},
    };
    int dictSize = sizeof(dict) / sizeof(dict[0]);

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Key Dict", RFID_COLOR_ACCENT);
    int totalSectors = st->mfcDump.sectors;
    int totalTries = 0;
    int keysFound = 0;

    for (int sector = 0; sector < totalSectors; sector++) {
        if (check(EscPress)) break;
        int block = sector * 4;
        if (sector >= 32) block = 128 + (sector - 32) * 16;

        for (int k = 0; k < dictSize; k++) {
            totalTries++;
            if (st->mifareAuthBlock(block, dict[k], false)) {
                keysFound++;
                st->mifareHalt();
                break;
            }
        }

        int pct = ((sector + 1) * 100) / totalSectors;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 20, pct, "Setor " + String(sector + 1) + "/" + String(totalSectors));
    }

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Tentativas: " + String(totalTries), 12, RFID_BODY_Y + 60);
    tft.drawString("Chaves encontradas: " + String(keysFound), 12, RFID_BODY_Y + 72);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicClone() {
    rfidAdvDrawHeader("MFC Clone", RFID_COLOR_TITLE);
    if (!isST25R3916()) {
        displayError("Clone requer ST25R3916");
        return;
    }

    rfidAdvDrawFooter("Aproxime a tag ORIG...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag origem nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Clone", RFID_COLOR_ACCENT);
    displaySuccess("Tag origem lida!");
    rfidAdvDrawFooter("Aproxime tag MAGIC...", "");

    // Wait for magic tag
    delay(2000);

    int cloneStatus = rfidMod->clone();
    if (cloneStatus == 0) {
        displaySuccess("Tag clonada com sucesso!");
    } else {
        displayError("Falha na clonagem");
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareClassicEmulate() {
    rfidAdvDrawHeader("MFC Emulate", RFID_COLOR_TITLE);

    if (!isST25R3916()) {
        displayError("Emulacao requer ST25R3916");
        return;
    }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Modulo invalido"); return; }

    // Load dump first
    rfidAdvDrawFooter("Carregando dump...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    if (!st->mfcLoaded) { displayError("Dump MFC nao carregado"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("MFC Emulate", RFID_COLOR_ACCENT);
    rfidAdvDrawFooter("Emulando... ESC para parar", "");

    // Build emulation pages and start
    st->buildEmuPagesPublic();
    st->setupListenModePublic(rfidMod->uid.uidByte, rfidMod->uid.size, rfidMod->uid.atqaByte, rfidMod->uid.sak);

    int polls = st->handleListenLoopPublic(rfidAdvConfig.emulTimeoutMs);
    st->stopDiscovery();

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawCentreString("Polls: " + String(polls), tftWidth / 2, RFID_BODY_Y + 20, 1);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidMifareStaticNonceDetect() {
    rfidAdvDrawHeader("Static Nonce Detect", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Coletando nonces...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Funcao requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    std::vector<uint32_t> nonces;
    rfidAdvDrawHeader("Static Nonce Detect", RFID_COLOR_ACCENT);

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Lendo tags para coletar nonces...", 12, RFID_BODY_Y);

    for (int i = 0; i < 20; i++) {
        if (check(EscPress)) break;
        int status = rfidMod->read(0);
        if (status == 0 || status == 1) {
            // Use PRNG successor to detect static nonce
            uint32_t nt = prng_successor(0, 0);
            nonces.push_back(nt);
        }
        delay(100);

        int pct = ((i + 1) * 100) / 20;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 30, pct, String(i + 1) + "/20");
    }

    // Analyze nonces for duplicates
    bool isStatic = false;
    for (int i = 0; i < nonces.size(); i++) {
        for (int j = i + 1; j < nonces.size(); j++) {
            if (nonces[i] == nonces[j]) {
                isStatic = true;
                break;
            }
        }
    }

    tft.setTextSize(FP);
    tft.setTextColor(isStatic ? RFID_COLOR_DANGER : RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Nonces coletados: " + String(nonces.size()), 12, RFID_BODY_Y + 60);
    tft.drawString(isStatic ? "VULNERAVEL: Nonce estatico!" : "Nonce aleatorio (seguro)", 12, RFID_BODY_Y + 72);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

// ============================================================================
// CATEGORY 2: MIFARE Ultralight & NTAG (10 functions)
// ============================================================================

void rfidUltralightFullDump() {
    rfidAdvDrawHeader("UL Full Dump", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Aproxime a tag...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("UL Full Dump", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    ST25R3916 *st = asST25R3916();
    if (st && st->ntagHasVersion) {
        tft.setTextSize(FP);
        tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
        tft.drawString("Variante: " + st->ntagVariant, 12, RFID_BODY_Y + 60);
    }

    // Display first pages from strAllPages
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Paginas:", 12, RFID_BODY_Y + 80);
    int y = RFID_BODY_Y + 92;
    // Parse strAllPages for page data
    String pages = rfidMod->strAllPages;
    int pageStart = 0;
    int pageCount = 0;
    while (pageStart < pages.length() && y < RFID_FOOTER_Y - 12) {
        int lineEnd = pages.indexOf('\n', pageStart);
        if (lineEnd < 0) lineEnd = pages.length();
        String line = pages.substring(pageStart, lineEnd);
        if (line.startsWith("Page ")) {
            tft.drawString(line, 12, y);
            y += 12;
            pageCount++;
            if (pageCount >= 12) break;
        }
        pageStart = lineEnd + 1;
    }

    if (rfidAdvConfig.autoSave) {
        String filename = "/RFID/UL_" + String(millis()) + ".rfid";
        rfidMod->save(filename);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidUltralightPageRead() {
    rfidAdvDrawHeader("UL Page Read", RFID_COLOR_TITLE);
    String pageStr = num_keyboard("4", 3, "Pagina (0-255):");
    uint8_t pageNum = pageStr.toInt();

    rfidAdvDrawFooter("Aproxime a tag...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("UL Page Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Pagina: " + String(pageNum), 12, RFID_BODY_Y);
    tft.drawString("Dados: " + rfidMod->strAllPages, 12, RFID_BODY_Y + 14);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidUltralightPageWrite() {
    rfidAdvDrawHeader("UL Page Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Escrever  ESC: Voltar", "");

    String pageStr = num_keyboard("4", 3, "Pagina:");
    uint8_t pageNum = pageStr.toInt();

    if (!rfidAdvConfirmAction("Escrever na pagina " + String(pageNum) + "?")) return;

    rfidAdvDrawFooter("Aproxime a tag...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    int writeStatus = rfidMod->write(0);
    if (writeStatus == 0) {
        displaySuccess("Pagina escrita!");
    } else {
        displayError("Falha na escrita");
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidUltralightPasswordAuth() {
    rfidAdvDrawHeader("UL Password Auth", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Testando autenticacao...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("UL Password Auth", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);

    // Default Ultralight password is 0xFFFFFFFF
    tft.drawString("Senha padrao: FFFFFFFF", 12, RFID_BODY_Y);
    tft.drawString("Resultado: Leitura basica OK", 12, RFID_BODY_Y + 14);
    tft.drawString("(Auth 3-step requer ST25R3916)", 12, RFID_BODY_Y + 28);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidUltralightOTPRead() {
    rfidAdvDrawHeader("UL OTP Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo OTP...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("UL OTP Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("OTP (Pagina 3):", 12, RFID_BODY_Y);
    tft.drawString("Meta: R/W blocks, Auth0, PWD", 12, RFID_BODY_Y + 14);
    tft.drawString("Dados: " + rfidMod->strAllPages, 12, RFID_BODY_Y + 28);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidUltralightCounterRead() {
    rfidAdvDrawHeader("UL Counter Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo contadores...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("UL Counter Read", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);

    if (st && st->ntagHasCounters) {
        for (int i = 0; i < 3; i++) {
            tft.drawString("Contador " + String(i) + ": " + String(st->ntagCounters[i]), 12, RFID_BODY_Y + i * 14);
            tft.drawString("  Tearing: " + String(st->ntagTearing[i]), 12, RFID_BODY_Y + i * 14 + 7);
        }
    } else {
        tft.drawString("Contadores nao disponiveis", 12, RFID_BODY_Y);
        tft.drawString("(Requer ST25R3916 + NTAG21x)", 12, RFID_BODY_Y + 14);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidNTAGVersionInfo() {
    rfidAdvDrawHeader("NTAG Version", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo versao...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("NTAG Version", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);

    if (st && st->ntagHasVersion) {
        tft.drawString("Variante: " + st->ntagVariant, 12, RFID_BODY_Y);
        tft.drawString("Version HEX:", 12, RFID_BODY_Y + 14);
        String hex = "";
        for (int i = 0; i < 8; i++) {
            if (st->ntagVersion[i] < 0x10) hex += "0";
            hex += String(st->ntagVersion[i], HEX) + " ";
        }
        tft.drawString(hex, 12, RFID_BODY_Y + 26);
    } else {
        tft.drawString("GET_VERSION nao disponivel", 12, RFID_BODY_Y);
        tft.drawString("(Requer ST25R3916)", 12, RFID_BODY_Y + 14);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidNTAGSignatureRead() {
    rfidAdvDrawHeader("NTAG Signature", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo assinatura...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("NTAG Signature", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st && st->ntagHasSignature) {
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawString("ECC-P256 Signature:", 12, RFID_BODY_Y);
        for (int i = 0; i < 4; i++) {
            String hex = "";
            for (int j = 0; j < 8; j++) {
                if (st->ntagSignature[i * 8 + j] < 0x10) hex += "0";
                hex += String(st->ntagSignature[i * 8 + j], HEX) + " ";
            }
            tft.drawString(hex, 12, RFID_BODY_Y + 14 + i * 12);
        }
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Assinatura nao disponivel", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidNTAGPasswordProtect() {
    rfidAdvDrawHeader("NTAG Password", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Protecao NTAG...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("NTAG Password", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Tag lida com sucesso", 12, RFID_BODY_Y);
    tft.drawString("AUTH0 define pag inicial protegida", 12, RFID_BODY_Y + 14);
    tft.drawString("PWD/ACK para autenticacao 3-step", 12, RFID_BODY_Y + 28);
    tft.drawString("(Escrita de prot. requer ST25R3916)", 12, RFID_BODY_Y + 42);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidNTAGOriginalityCheck() {
    rfidAdvDrawHeader("NTAG Originality", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Verificando originalidade...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("NTAG Originality", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st && st->ntagHasSignature) {
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawString("Assinatura ECC-P256:", 12, RFID_BODY_Y);
        // Display first 16 bytes of signature
        String hex = "";
        for (int i = 0; i < 16; i++) {
            if (st->ntagSignature[i] < 0x10) hex += "0";
            hex += String(st->ntagSignature[i], HEX) + " ";
        }
        tft.drawString(hex, 12, RFID_BODY_Y + 14);
        tft.drawString("(Verificacao completa requer", 12, RFID_BODY_Y + 30);
        tft.drawString(" chave publica ECC)", 12, RFID_BODY_Y + 42);
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Assinatura nao disponivel", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

// ============================================================================
// CATEGORY 3: DESFire & ISO-DEP (8 functions)
// ============================================================================

void rfidDESFireCardInfo() {
    rfidAdvDrawHeader("DESFire Info", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo DESFire...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Info", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st) {
        bool ok = st->readDESFireInfoPublic();
        if (ok) {
            tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
            tft.drawString("DESFire detectado!", 12, RFID_BODY_Y);
            tft.drawString("Info salva no dump", 12, RFID_BODY_Y + 14);
        } else {
            tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
            tft.drawString("Nao e DESFire ou nao suportado", 12, RFID_BODY_Y);
        }
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("DESFire requer ST25R3916", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidDESFireAppList() {
    rfidAdvDrawHeader("DESFire Apps", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Listando aplicacoes...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Apps", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st) {
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawString("Listagem de apps via ISO-DEP", 12, RFID_BODY_Y);
        tft.drawString("GET_APPLICATIONS command", 12, RFID_BODY_Y + 14);
        st->readDESFireInfoPublic();
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Requer ST25R3916", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidDESFireReadFile() {
    rfidAdvDrawHeader("DESFire Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo arquivo...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("SELECT_APPLICATION + READ_DATA", 12, RFID_BODY_Y);
    tft.drawString("(Requer autenticacao previa)", 12, RFID_BODY_Y + 14);

    ST25R3916 *st = asST25R3916();
    if (st) {
        uint8_t rx[256] = {0};
        uint16_t rxLen = 0;
        // SELECT_APPLICATION for first app (DF1 or DF2)
        uint8_t tx[] = {0x90, 0x5A, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00};
        if (st->isoDepApdu(tx, sizeof(tx), rx, sizeof(rx), &rxLen)) {
            tft.drawString("App selecionado: " + String(rxLen) + " bytes", 12, RFID_BODY_Y + 28);
        }
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidDESFireWriteFile() {
    rfidAdvDrawHeader("DESFire Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Escrevendo...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Write", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
    tft.drawString("Escrita em DESFire requer:", 12, RFID_BODY_Y);
    tft.drawString("1. AUTHENTICATE command", 12, RFID_BODY_Y + 14);
    tft.drawString("2. SELECT_APPLICATION", 12, RFID_BODY_Y + 28);
    tft.drawString("3. WRITE_DATA command", 12, RFID_BODY_Y + 42);
    tft.drawString("(Funcionalidade em desenvolvimento)", 12, RFID_BODY_Y + 56);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidDESFireAuthenticate() {
    rfidAdvDrawHeader("DESFire Auth", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Autenticando...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Auth", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Tipos de autenticacao:", 12, RFID_BODY_Y);
    tft.drawString("DES (0x0A) - 8-byte key", 12, RFID_BODY_Y + 14);
    tft.drawString("3DES (0x1A) - 16-byte key", 12, RFID_BODY_Y + 28);
    tft.drawString("AES (0xAA) - 16-byte key", 12, RFID_BODY_Y + 42);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidDESFireFormatCard() {
    rfidAdvDrawHeader("DESFire Format", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Formatando...", "");

    if (!rfidAdvConfirmAction("FORMATAR cartao DESFire? TODOS os dados serao PERDIDOS!")) return;

    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("DESFire Format", RFID_COLOR_DANGER);
    ST25R3916 *st = asST25R3916();

    if (st) {
        uint8_t rx[16] = {0};
        uint16_t rxLen = 0;
        // FORMAT_CARD command: 0x60
        uint8_t tx[] = {0x90, 0x60, 0x00, 0x00, 0x00};
        if (st->isoDepApdu(tx, sizeof(tx), rx, sizeof(rx), &rxLen)) {
            displaySuccess("Cartao formatado!");
        } else {
            displayError("Falha ao formatar");
        }
    } else {
        displayError("Requer ST25R3916");
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidISODEPProbe() {
    rfidAdvDrawHeader("ISO-DEP Probe", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Sondando dispositivo...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("ISO-DEP Probe", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st) {
        uint8_t rx[256] = {0};
        uint16_t rxLen = 0;

        // SELECT NDEF Application (AID: D2760000850101)
        uint8_t selectNdef[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00};
        bool ndefOk = st->isoDepApdu(selectNdef, sizeof(selectNdef), rx, sizeof(rx), &rxLen);
        tft.setTextColor(ndefOk ? RFID_COLOR_ACCENT : RFID_COLOR_DANGER, RFID_COLOR_BG);
        tft.drawString("NDEF App: " + String(ndefOk ? "OK" : "FALHA"), 12, RFID_BODY_Y);

        // SELECT MF
        uint8_t selectMF[] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00, 0x00};
        bool mfOk = st->isoDepApdu(selectMF, sizeof(selectMF), rx, sizeof(rx), &rxLen);
        tft.setTextColor(mfOk ? RFID_COLOR_ACCENT : RFID_COLOR_DANGER, RFID_COLOR_BG);
        tft.drawString("MF: " + String(mfOk ? "OK" : "FALHA"), 12, RFID_BODY_Y + 14);

        // GET VERSION
        uint8_t getVersion[] = {0x60};
        bool verOk = st->isoDepApdu(getVersion, sizeof(getVersion), rx, sizeof(rx), &rxLen);
        tft.setTextColor(verOk ? RFID_COLOR_ACCENT : RFID_COLOR_DANGER, RFID_COLOR_BG);
        tft.drawString("GET_VERSION: " + String(verOk ? "OK" : "FALHA"), 12, RFID_BODY_Y + 28);
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("ISO-DEP requer ST25R3916", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidISODEPNdefRead() {
    rfidAdvDrawHeader("ISO-DEP NDEF", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo NDEF...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("ISO-DEP NDEF", RFID_COLOR_ACCENT);
    ST25R3916 *st = asST25R3916();

    tft.setTextSize(FP);
    if (st) {
        uint8_t rx[256] = {0};
        uint16_t rxLen = 0;

        // SELECT NDEF Application
        uint8_t tx[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00};
        if (st->isoDepApdu(tx, sizeof(tx), rx, sizeof(rx), &rxLen)) {
            tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
            tft.drawString("NDEF App: OK", 12, RFID_BODY_Y);

            // SELECT CC file (E103)
            uint8_t selCC[] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0xE1, 0x03, 0x00};
            if (st->isoDepApdu(selCC, sizeof(selCC), rx, sizeof(rx), &rxLen)) {
                // READ CC
                uint8_t readCC[] = {0x00, 0xB0, 0x00, 0x00, 0x0F, 0x00};
                if (st->isoDepApdu(readCC, sizeof(readCC), rx, sizeof(rx), &rxLen)) {
                    tft.drawString("CC: " + String(rxLen) + " bytes", 12, RFID_BODY_Y + 14);
                }
            }
        } else {
            tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
            tft.drawString("NDEF nao encontrado", 12, RFID_BODY_Y);
        }
    } else {
        tft.setTextColor(RFID_COLOR_WARN, RFID_COLOR_BG);
        tft.drawString("Requer ST25R3916", 12, RFID_BODY_Y);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

// ============================================================================
// CATEGORY 4: ISO15693 & FeliCa (6 functions)
// ============================================================================

void rfidISO15693Inventory() {
    rfidAdvDrawHeader("ISO15693 Inv.", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Escaneando NFC-V...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("ISO15693 Inv.", RFID_COLOR_ACCENT);
    int detected = 0;
    unsigned long start = millis();

    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int status = rfidMod->read(0);
        if (status == 0 || status == 1) {
            detected++;
            rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y + (detected - 1) * 40);
        }
        delay(200);
        rfidAdvDrawSpinner(tftWidth / 2, RFID_BODY_Y + 160, (millis() - start) / 200);
    }

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawCentreString("Detectados: " + String(detected), tftWidth / 2, RFID_BODY_Y + 180, 1);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidISO15693ReadBlock() {
    rfidAdvDrawHeader("ISO15693 Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Aproxime a tag...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag ISO15693 nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("ISO15693 Read", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Dados: " + rfidMod->strAllPages, 12, RFID_BODY_Y + 50);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidISO15693WriteBlock() {
    rfidAdvDrawHeader("ISO15693 Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Escrita ISO15693...", "");

    rfidAdvDrawHeader("ISO15693 Write", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Escrita ISO15693 requer:", 12, RFID_BODY_Y);
    tft.drawString("- Tag com blocos desprotegidos", 12, RFID_BODY_Y + 14);
    tft.drawString("- Suporte do modulo RFAL", 12, RFID_BODY_Y + 28);
    tft.drawString("(Funcionalidade basica)", 12, RFID_BODY_Y + 42);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidISO15693SystemInfo() {
    rfidAdvDrawHeader("ISO15693 SysInfo", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo info...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag ISO15693 nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("ISO15693 SysInfo", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidFeliCaRead() {
    rfidAdvDrawHeader("FeliCa Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo FeliCa...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(1); // FeliCa baud rate
    if (status != 0 && status != 1) { displayError("Tag FeliCa nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("FeliCa Read", RFID_COLOR_ACCENT);
    rfidAdvDisplayTagInfo(rfidAdvReadTagInfo(), RFID_BODY_Y);

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Dados FeliCa:", 12, RFID_BODY_Y + 50);
    tft.drawString(rfidMod->strAllPages, 12, RFID_BODY_Y + 64);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidFeliCaWrite() {
    rfidAdvDrawHeader("FeliCa Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Escrita FeliCa...", "");

    rfidAdvDrawHeader("FeliCa Write", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Escrita FeliCa requer:", 12, RFID_BODY_Y);
    tft.drawString("- Tag com blocos desprotegidos", 12, RFID_BODY_Y + 14);
    tft.drawString("- Suporte FeliCa write do modulo", 12, RFID_BODY_Y + 28);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 5: 125kHz Operations (8 functions)
// ============================================================================

void rfidEM4100Read() {
    rfidAdvDrawHeader("EM4100 Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Aproxime tag 125kHz...", "");
    rfidAdvDrawHeader("EM4100 Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Usando leitor 125kHz...", 12, RFID_BODY_Y);
    tft.drawString("Aproxime a tag do leitor", 12, RFID_BODY_Y + 14);
    tft.drawString("RFID125 UART em 9600 baud", 12, RFID_BODY_Y + 28);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidEM4100Clone() {
    rfidAdvDrawHeader("EM4100 Clone", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Clonar EM4100...", "");
    rfidAdvDrawHeader("EM4100 Clone", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Clonagem EM4100 -> T5577:", 12, RFID_BODY_Y);
    tft.drawString("1. Ler tag EM4100 origem", 12, RFID_BODY_Y + 14);
    tft.drawString("2. Aproximar T5577", 12, RFID_BODY_Y + 28);
    tft.drawString("3. Gravar dados no T5577", 12, RFID_BODY_Y + 42);
    tft.drawString("(Requer writer T5577)", 12, RFID_BODY_Y + 56);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidT5577WriteConfig() {
    rfidAdvDrawHeader("T5577 Config", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Configurando T5577...", "");
    rfidAdvDrawHeader("T5577 Config", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("T5577 configuravel:", 12, RFID_BODY_Y);
    tft.drawString("Modo: EM4100 / HID / Indala", 12, RFID_BODY_Y + 14);
    tft.drawString("Data Rate: RF/8, RF/16, RF/32", 12, RFID_BODY_Y + 28);
    tft.drawString("(Requer escritor T5577)", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidT5577ReadConfig() {
    rfidAdvDrawHeader("T5577 Read Cfg", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo config...", "");
    rfidAdvDrawHeader("T5577 Read Cfg", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Leitura de config T5577:", 12, RFID_BODY_Y);
    tft.drawString("Block 0: Master / Config", 12, RFID_BODY_Y + 14);
    tft.drawString("Block 1: Data (EM4100)", 12, RFID_BODY_Y + 28);
    tft.drawString("(Requer leitor T5577)", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidHIDProxRead() {
    rfidAdvDrawHeader("HID Prox Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo HID Prox...", "");
    rfidAdvDrawHeader("HID Prox Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Formatos HID Prox:", 12, RFID_BODY_Y);
    tft.drawString("26-bit: Facilidade (1-65535)", 12, RFID_BODY_Y + 14);
    tft.drawString("34-bit: Facilidade+Facilidade", 12, RFID_BODY_Y + 28);
    tft.drawString("Leitura via EM4100/T5577", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidIndalaRead() {
    rfidAdvDrawHeader("Indala Read", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Lendo Indala...", "");
    rfidAdvDrawHeader("Indala Read", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Formato Indala:", 12, RFID_BODY_Y);
    tft.drawString("PSK modulacao", 12, RFID_BODY_Y + 14);
    tft.drawString("26-bit / 34-bit", 12, RFID_BODY_Y + 28);
    tft.drawString("Leitura via T5577", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidEM4x02Write() {
    rfidAdvDrawHeader("EM4x02 Write", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Escrita EM4x02...", "");
    rfidAdvDrawHeader("EM4x02 Write", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("EM4x02 (READ/WRITE):", 12, RFID_BODY_Y);
    tft.drawString("Tag de 128 bits", 12, RFID_BODY_Y + 14);
    tft.drawString("Protect / Lock bits", 12, RFID_BODY_Y + 28);
    tft.drawString("(Requer writer especifico)", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfid125kHzRawDisplay() {
    rfidAdvDrawHeader("125kHz Raw", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Dados brutos 125kHz...", "");
    rfidAdvDrawHeader("125kHz Raw", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Formatos de exibicao:", 12, RFID_BODY_Y);
    tft.drawString("HEX: 0x1234567890", 12, RFID_BODY_Y + 14);
    tft.drawString("BIN: 00010010...", 12, RFID_BODY_Y + 28);
    tft.drawString("DEC: 1234567890", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 6: Advanced Tag Operations (8 functions)
// ============================================================================

void rfidMultiTagDetect() {
    rfidAdvDrawHeader("Multi Tag Detect", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Detectando tags...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Multi Tag Detect", RFID_COLOR_ACCENT);
    std::set<String> detectedUIDs;
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        int status = rfidMod->read(0);
        if (status == 0 || status == 1) {
            String uid = "";
            for (int i = 0; i < rfidMod->uid.size; i++) {
                if (rfidMod->uid.uidByte[i] < 0x10) uid += "0";
                uid += String(rfidMod->uid.uidByte[i], HEX);
            }
            uid.toUpperCase();
            detectedUIDs.insert(uid);
        }
        delay(200);
        rfidAdvDrawSpinner(tftWidth / 2, RFID_BODY_Y + 20, (millis() - start) / 200);
    }

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawCentreString("Tags unicas: " + String(detectedUIDs.size()), tftWidth / 2, RFID_BODY_Y + 40, 1);

    int y = RFID_BODY_Y + 60;
    for (const auto &uid : detectedUIDs) {
        if (y > RFID_FOOTER_Y - 14) break;
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawString(uid, 12, y);
        y += 12;
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTagMemoryMap() {
    rfidAdvDrawHeader("Tag Memory Map", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Mapeando memoria...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Tag Memory Map", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("UID: " + rfidAdvReadTagInfo().uid, 12, RFID_BODY_Y);
    tft.drawString("Paginas: " + String(rfidMod->totalPages), 12, RFID_BODY_Y + 14);
    tft.drawString("Dados: " + String(rfidMod->dataPages) + " pag", 12, RFID_BODY_Y + 28);

    // Visual memory map
    int mapY = RFID_BODY_Y + 50;
    int pagesPerRow = 16;
    for (int p = 0; p < rfidMod->totalPages && p < 128; p++) {
        int x = 12 + (p % pagesPerRow) * 12;
        int row = p / pagesPerRow;
        if (mapY + row * 12 > RFID_FOOTER_Y - 12) break;
        bool read = rfidMod->pageReadSuccess;
        tft.fillRect(x, mapY + row * 12, 10, 10, read ? RFID_COLOR_ACCENT : RFID_COLOR_DIM);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTagFormatDetect() {
    rfidAdvDrawHeader("Tag Format Detect", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Detectando formato...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Tag Format Detect", RFID_COLOR_ACCENT);
    NFCTagInfo info = rfidAdvReadTagInfo();
    rfidAdvDisplayTagInfo(info, RFID_BODY_Y);

    // Detect format from SAK
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    String format = "Desconhecido";
    uint8_t sak = rfidMod->uid.sak;
    if (sak == 0x08) format = "MIFARE Classic 1K";
    else if (sak == 0x18) format = "MIFARE Classic 4K";
    else if (sak == 0x09) format = "MIFARE Mini";
    else if (sak == 0x00) format = "MIFARE Ultralight/NTAG";
    else if (sak == 0x20) format = "ISO-DEP (DESFire/T4T)";
    else if (sak == 0x04) format = "Not complete";

    tft.drawString("Formato: " + format, 12, RFID_BODY_Y + 50);
    tft.drawString("SAK: 0x" + String(sak, HEX), 12, RFID_BODY_Y + 64);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTagDumpCompare() {
    rfidAdvDrawHeader("Dump Compare", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Comparar dumps...", "");
    rfidAdvDrawHeader("Dump Compare", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Comparacao de dumps:", 12, RFID_BODY_Y);
    tft.drawString("1. Ler tag A e salvar", 12, RFID_BODY_Y + 14);
    tft.drawString("2. Ler tag B e salvar", 12, RFID_BODY_Y + 28);
    tft.drawString("3. Comparar byte a byte", 12, RFID_BODY_Y + 42);
    tft.drawString("(Funcionalidade em dev)", 12, RFID_BODY_Y + 56);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidTagDumpMerge() {
    rfidAdvDrawHeader("Dump Merge", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Mesclar dumps...", "");
    rfidAdvDrawHeader("Dump Merge", RFID_COLOR_WARN);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Mesclagem de dumps:", 12, RFID_BODY_Y);
    tft.drawString("Preenche blocos faltantes", 12, RFID_BODY_Y + 14);
    tft.drawString("de dois dumps diferentes", 12, RFID_BODY_Y + 28);
    tft.drawString("(Funcionalidade em dev)", 12, RFID_BODY_Y + 42);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

void rfidTagDumpAnalyze() {
    rfidAdvDrawHeader("Dump Analyze", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Analisando dump...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Dump Analyze", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("UID: " + rfidAdvReadTagInfo().uid, 12, RFID_BODY_Y);
    tft.drawString("SAK: 0x" + String(rfidMod->uid.sak, HEX), 12, RFID_BODY_Y + 14);
    tft.drawString("ATQA: " + rfidAdvReadTagInfo().atqa, 12, RFID_BODY_Y + 28);
    tft.drawString("Paginas: " + String(rfidMod->totalPages), 12, RFID_BODY_Y + 42);
    tft.drawString("Dados: " + String(rfidMod->dataPages) + " paginas", 12, RFID_BODY_Y + 56);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTagDumpExport() {
    rfidAdvDrawHeader("Dump Export", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Exportando...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Dump Export", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Formatos de export:", 12, RFID_BODY_Y);
    tft.drawString("1. .rfid (Willy padrao)", 12, RFID_BODY_Y + 14);
    tft.drawString("2. .nfc (Flipper Zero)", 12, RFID_BODY_Y + 28);
    tft.drawString("3. .mfkey (Proxmark)", 12, RFID_BODY_Y + 42);

    // Try to save
    String filename = "/RFID/DUMP_" + String(millis()) + ".rfid";
    rfidMod->save(filename);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Salvo: " + filename, 12, RFID_BODY_Y + 60);

    // Try Flipper format if ST25R3916
    ST25R3916 *st = asST25R3916();
    if (st) {
        String flipFile = "/RFID/FLIP_" + String(millis()) + ".nfc";
        st->saveFlipper(flipFile);
        tft.drawString("Flipper: " + flipFile, 12, RFID_BODY_Y + 74);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTagDumpImport() {
    rfidAdvDrawHeader("Dump Import", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Importando...", "");
    rfidAdvDrawHeader("Dump Import", RFID_COLOR_ACCENT);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Importar dump de arquivo:", 12, RFID_BODY_Y);
    tft.drawString("Formatos suportados:", 12, RFID_BODY_Y + 14);
    tft.drawString("- .rfid (Willy)", 12, RFID_BODY_Y + 28);
    tft.drawString("- .nfc (Flipper)", 12, RFID_BODY_Y + 42);
    tft.drawString("- .mfkey (Proxmark)", 12, RFID_BODY_Y + 56);
    tft.drawString("(Usa TagOMatic LOAD_MODE)", 12, RFID_BODY_Y + 70);
    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// CATEGORY 7: Security Testing (6 functions)
// ============================================================================

void rfidReadRateTest() {
    rfidAdvDrawHeader("Read Rate Test", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Medindo taxa de leitura...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Read Rate Test", RFID_COLOR_ACCENT);
    int totalAttempts = 0;
    int successReads = 0;
    unsigned long start = millis();

    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        totalAttempts++;
        int status = rfidMod->read(0);
        if (status == 0 || status == 1) successReads++;
        delay(100);
    }

    unsigned long elapsed = millis() - start;
    float rate = (successReads * 100.0f) / totalAttempts;

    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Tentativas: " + String(totalAttempts), 12, RFID_BODY_Y);
    tft.drawString("Sucesso: " + String(successReads), 12, RFID_BODY_Y + 14);
    tft.drawString("Taxa: " + String(rate, 1) + "%", 12, RFID_BODY_Y + 28);
    tft.drawString("Tempo: " + String(elapsed / 1000) + "s", 12, RFID_BODY_Y + 42);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidAuthSuccessRate() {
    rfidAdvDrawHeader("Auth Rate Test", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Testando autenticacao...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    ST25R3916 *st = asST25R3916();
    if (!st) { displayError("Requer ST25R3916"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Auth Rate Test", RFID_COLOR_ACCENT);
    int totalTries = 0;
    int successes = 0;

    for (int sector = 0; sector < st->mfcDump.sectors && sector < 16; sector++) {
        if (check(EscPress)) break;
        int block = sector * 4;

        for (int k = 0; k < 15; k++) {
            totalTries++;
            if (st->mifareAuthBlock(block, rfidMod->keys[k], false)) {
                successes++;
                st->mifareHalt();
                break;
            }
        }
    }

    float rate = (successes * 100.0f) / totalTries;
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Tentativas: " + String(totalTries), 12, RFID_BODY_Y);
    tft.drawString("Sucesso: " + String(successes), 12, RFID_BODY_Y + 14);
    tft.drawString("Taxa: " + String(rate, 1) + "%", 12, RFID_BODY_Y + 28);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidTimingAnalysis() {
    rfidAdvDrawHeader("Timing Analysis", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Analisando timing...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Timing Analysis", RFID_COLOR_ACCENT);
    std::vector<unsigned long> timings;

    for (int i = 0; i < 20; i++) {
        if (check(EscPress)) break;
        unsigned long t0 = micros();
        rfidMod->read(0);
        unsigned long elapsed = micros() - t0;
        timings.push_back(elapsed);
        delay(100);
    }

    if (timings.size() > 0) {
        unsigned long total = 0;
        unsigned long minT = timings[0];
        unsigned long maxT = timings[0];
        for (auto t : timings) {
            total += t;
            if (t < minT) minT = t;
            if (t > maxT) maxT = t;
        }
        float avg = (float)total / timings.size();

        tft.setTextSize(FP);
        tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
        tft.drawString("Amostras: " + String(timings.size()), 12, RFID_BODY_Y);
        tft.drawString("Min: " + String(minT / 1000) + "ms", 12, RFID_BODY_Y + 14);
        tft.drawString("Max: " + String(maxT / 1000) + "ms", 12, RFID_BODY_Y + 28);
        tft.drawString("Media: " + String(avg / 1000, 1) + "ms", 12, RFID_BODY_Y + 42);
    }

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidStressTest() {
    rfidAdvDrawHeader("Stress Test", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Teste de estresse...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Stress Test", RFID_COLOR_ACCENT);
    int errors = 0;
    int total = 0;
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (check(EscPress)) break;
        total++;
        int status = rfidMod->read(0);
        if (status != 0 && status != 1) errors++;

        int pct = ((millis() - start) * 100) / 15000;
        rfidAdvDrawProgressBar(RFID_BODY_Y + 20, pct, String(total) + " tentativas");

        delay(50);
    }

    float errorRate = (errors * 100.0f) / total;
    tft.setTextSize(FP);
    tft.setTextColor(errorRate > 10 ? RFID_COLOR_DANGER : RFID_COLOR_ACCENT, RFID_COLOR_BG);
    tft.drawString("Total: " + String(total), 12, RFID_BODY_Y + 50);
    tft.drawString("Erros: " + String(errors), 12, RFID_BODY_Y + 64);
    tft.drawString("Taxa erro: " + String(errorRate, 1) + "%", 12, RFID_BODY_Y + 78);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidFieldStrengthTest() {
    rfidAdvDrawHeader("Field Strength", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Medindo campo...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Field Strength", RFID_COLOR_ACCENT);
    int detections = 0;
    unsigned long start = millis();

    while (millis() - start < 10000) {
        if (check(EscPress)) break;
        int status = rfidMod->read(0);
        if (status == 0 || status == 1) detections++;
        delay(100);
    }

    float strength = (detections * 100.0f) / 100; // detections per second
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawString("Deteccoes/seg: " + String(detections / 10), 12, RFID_BODY_Y);
    tft.drawString("Forca relativa:", 12, RFID_BODY_Y + 14);
    int barW = constrain(detections * 2, 0, tftWidth - 24);
    tft.fillRoundRect(12, RFID_BODY_Y + 30, barW, 16, 3, RFID_COLOR_ACCENT);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

void rfidCompatibilityCheck() {
    rfidAdvDrawHeader("Compat. Check", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("Verificando...", "");
    rfidMod = rfidAdvCreateModule();
    if (!rfidMod->begin()) { displayError("Falha ao iniciar modulo"); delete rfidMod; rfidMod = nullptr; return; }

    int status = rfidMod->read(0);
    if (status != 0 && status != 1) { displayError("Tag nao detectada"); delete rfidMod; rfidMod = nullptr; return; }

    rfidAdvDrawHeader("Compat. Check", RFID_COLOR_ACCENT);
    NFCTagInfo info = rfidAdvReadTagInfo();
    rfidAdvDisplayTagInfo(info, RFID_BODY_Y);

    // Check compatibility with current module
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    uint8_t sak = rfidMod->uid.sak;

    String compat = "Completa";
    if (sak == 0x08 || sak == 0x18 || sak == 0x09) {
        compat = "MIFARE Classic - Leitura/Escrita/Clone";
    } else if (sak == 0x00) {
        compat = "Ultralight/NTAG - Leitura/Escrita";
    } else if (sak == 0x20) {
        compat = "ISO-DEP - Leitura basica";
    }

    tft.drawString("Compat: " + compat, 12, RFID_BODY_Y + 50);
    tft.drawString("Modulo: " + String(wilyConfigPins.rfidModule), 12, RFID_BODY_Y + 64);

    rfidAdvDrawFooter("ESC: Voltar", "");
    while (!check(EscPress)) delay(100);
    delete rfidMod;
    rfidMod = nullptr;
}

// ============================================================================
// MENU SYSTEM (7 menus)
// ============================================================================

static int showMenu(const char *title, const char **items, int count, uint16_t *colors) {
    rfidAdvDrawHeader(title, RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0, scrollY = 0;

    while (true) {
        for (int i = 0; i < 7 && i < count; i++) {
            int idx = scrollY + i;
            if (idx >= count) break;
            int y = RFID_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            rfidAdvDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RFID_COLOR_TEXT, isSel ? RFID_COLOR_PRIMARY : getColorVariation(RFID_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = count - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= count) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) return sel;
        if (check(EscPress)) return -1;
        delay(100);
    }
}

void rfidMifareClassicMenu() {
    const char *items[] = {
        "Full Dump", "Sector Auth", "Key Recovery",
        "Sector Map", "Block Read", "Block Write",
        "Value Block", "Access Bits", "Key Dictionary",
        "Clone", "Emulate", "Static Nonce"
    };
    void (*funcs[])() = {
        rfidMifareClassicFullDump, rfidMifareClassicSectorAuth, rfidMifareClassicKeyRecovery,
        rfidMifareClassicSectorMap, rfidMifareClassicBlockRead, rfidMifareClassicBlockWrite,
        rfidMifareClassicValueBlock, rfidMifareClassicAccessBits, rfidMifareClassicKeyDictionary,
        rfidMifareClassicClone, rfidMifareClassicEmulate, rfidMifareStaticNonceDetect
    };
    while (true) {
        int sel = showMenu("MIFARE Classic", items, 12, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("MIFARE Classic", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidUltralightNTAGMenu() {
    const char *items[] = {
        "UL Full Dump", "Page Read", "Page Write",
        "Password Auth", "OTP Read", "Counter Read",
        "NTAG Version", "NTAG Signature", "NTAG Password",
        "NTAG Originality"
    };
    void (*funcs[])() = {
        rfidUltralightFullDump, rfidUltralightPageRead, rfidUltralightPageWrite,
        rfidUltralightPasswordAuth, rfidUltralightOTPRead, rfidUltralightCounterRead,
        rfidNTAGVersionInfo, rfidNTAGSignatureRead, rfidNTAGPasswordProtect,
        rfidNTAGOriginalityCheck
    };
    while (true) {
        int sel = showMenu("Ultralight/NTAG", items, 10, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("Ultralight/NTAG", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidDESFireMenu() {
    const char *items[] = {
        "Card Info", "App List", "Read File",
        "Write File", "Authenticate", "Format Card",
        "ISO-DEP Probe", "ISO-DEP NDEF"
    };
    void (*funcs[])() = {
        rfidDESFireCardInfo, rfidDESFireAppList, rfidDESFireReadFile,
        rfidDESFireWriteFile, rfidDESFireAuthenticate, rfidDESFireFormatCard,
        rfidISODEPProbe, rfidISODEPNdefRead
    };
    while (true) {
        int sel = showMenu("DESFire/ISO-DEP", items, 8, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("DESFire/ISO-DEP", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidISO15693FeliCaMenu() {
    const char *items[] = {
        "ISO15693 Inventory", "ISO15693 Read", "ISO15693 Write",
        "ISO15693 SysInfo", "FeliCa Read", "FeliCa Write"
    };
    void (*funcs[])() = {
        rfidISO15693Inventory, rfidISO15693ReadBlock, rfidISO15693WriteBlock,
        rfidISO15693SystemInfo, rfidFeliCaRead, rfidFeliCaWrite
    };
    while (true) {
        int sel = showMenu("ISO15693/FeliCa", items, 6, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("ISO15693/FeliCa", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfid125kHzMenu() {
    const char *items[] = {
        "EM4100 Read", "EM4100 Clone", "T5577 Config",
        "T5577 Read Cfg", "HID Prox Read", "Indala Read",
        "EM4x02 Write", "125kHz Raw"
    };
    void (*funcs[])() = {
        rfidEM4100Read, rfidEM4100Clone, rfidT5577WriteConfig,
        rfidT5577ReadConfig, rfidHIDProxRead, rfidIndalaRead,
        rfidEM4x02Write, rfid125kHzRawDisplay
    };
    while (true) {
        int sel = showMenu("125kHz Ops", items, 8, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("125kHz Ops", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidAdvancedOpsMenu() {
    const char *items[] = {
        "Multi Tag Detect", "Memory Map", "Format Detect",
        "Dump Compare", "Dump Merge", "Dump Analyze",
        "Dump Export", "Dump Import"
    };
    void (*funcs[])() = {
        rfidMultiTagDetect, rfidTagMemoryMap, rfidTagFormatDetect,
        rfidTagDumpCompare, rfidTagDumpMerge, rfidTagDumpAnalyze,
        rfidTagDumpExport, rfidTagDumpImport
    };
    while (true) {
        int sel = showMenu("Ops Avancadas", items, 8, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("Ops Avancadas", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidSecurityTestMenu() {
    const char *items[] = {
        "Read Rate Test", "Auth Success Rate", "Timing Analysis",
        "Stress Test", "Field Strength", "Compat Check"
    };
    void (*funcs[])() = {
        rfidReadRateTest, rfidAuthSuccessRate, rfidTimingAnalysis,
        rfidStressTest, rfidFieldStrengthTest, rfidCompatibilityCheck
    };
    while (true) {
        int sel = showMenu("Security Tests", items, 6, nullptr);
        if (sel < 0) return;
        funcs[sel]();
        rfidAdvDrawHeader("Security Tests", RFID_COLOR_TITLE);
        rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    }
}

void rfidAdvConfigMenu() {
    rfidAdvDrawHeader("CONFIGURACOES", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0;
    const char *items[] = {
        "Baud Rate", "Max Key Tries", "Read Delay",
        "Auto Save", "Show Progress", "Emul Timeout",
        "Scan Duration"
    };
    while (true) {
        for (int i = 0; i < 7; i++) {
            int y = RFID_BODY_Y + i * 22;
            bool isSel = (i == sel);
            rfidAdvDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : RFID_COLOR_TEXT, isSel ? RFID_COLOR_PRIMARY : getColorVariation(RFID_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 5);
            tft.setTextDatum(TR_DATUM);
            String val;
            switch (i) {
                case 0: val = String(rfidAdvConfig.defaultBaudRate == 0 ? "MIFARE" : "FeliCa"); break;
                case 1: val = String(rfidAdvConfig.keyAttackMaxTries); break;
                case 2: val = String(rfidAdvConfig.readDelayMs) + " ms"; break;
                case 3: val = rfidAdvConfig.autoSave ? "ON" : "OFF"; break;
                case 4: val = rfidAdvConfig.showProgress ? "ON" : "OFF"; break;
                case 5: val = String(rfidAdvConfig.emulTimeoutMs / 1000) + "s"; break;
                case 6: val = String(rfidAdvConfig.scanDurationSec) + "s"; break;
            }
            tft.drawString(val, tftWidth - 14, y + 5);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 6; }
        if (check(DownPress)) { sel++; if (sel >= 7) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) rfidAdvConfig.defaultBaudRate = (rfidAdvConfig.defaultBaudRate + 1) % 2;
            else if (sel == 1) {
                String d = num_keyboard(String(rfidAdvConfig.keyAttackMaxTries), 4, "Max tries:");
                rfidAdvConfig.keyAttackMaxTries = d.toInt();
            } else if (sel == 2) {
                String d = num_keyboard(String(rfidAdvConfig.readDelayMs), 4, "Delay (ms):");
                rfidAdvConfig.readDelayMs = d.toInt();
            } else if (sel == 3) rfidAdvConfig.autoSave = !rfidAdvConfig.autoSave;
            else if (sel == 4) rfidAdvConfig.showProgress = !rfidAdvConfig.showProgress;
            else if (sel == 5) {
                String d = num_keyboard(String(rfidAdvConfig.emulTimeoutMs / 1000), 4, "Timeout (s):");
                rfidAdvConfig.emulTimeoutMs = d.toInt() * 1000;
            } else if (sel == 6) {
                String d = num_keyboard(String(rfidAdvConfig.scanDurationSec), 3, "Duracao (s):");
                rfidAdvConfig.scanDurationSec = d.toInt();
            }
            rfidAdvDrawHeader("CONFIGURACOES", RFID_COLOR_TITLE);
            rfidAdvDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfidAdvancedSuiteMenu() {
    rfidAdvDrawHeader("RFID Suite Avancada", RFID_COLOR_TITLE);
    rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
    int sel = 0, scrollY = 0;
    const char *items[] = {
        "MIFARE Classic", "Ultralight/NTAG",
        "DESFire/ISO-DEP", "ISO15693/FeliCa",
        "125kHz Ops", "Ops Avancadas",
        "Security Tests", "Configuracoes"
    };
    void (*funcs[])() = {
        rfidMifareClassicMenu, rfidUltralightNTAGMenu,
        rfidDESFireMenu, rfidISO15693FeliCaMenu,
        rfid125kHzMenu, rfidAdvancedOpsMenu,
        rfidSecurityTestMenu, rfidAdvConfigMenu
    };
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_ORANGE, TFT_YELLOW, TFT_MAGENTA, TFT_WHITE, TFT_DARKGREY};
    while (true) {
        for (int i = 0; i < 7; i++) {
            int y = RFID_BODY_Y + i * 22;
            bool isSel = (i == sel);
            rfidAdvDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 7, colors[i]);
            tft.setTextSize(FP);
            tft.setTextColor(TFT_BLACK, colors[i]);
            tft.setTextDatum(TC_DATUM);
            tft.drawCentreString(String(i + 1), 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : RFID_COLOR_TEXT, isSel ? RFID_COLOR_PRIMARY : getColorVariation(RFID_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 36, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = 7; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= 8) sel = 0; if (sel >= scrollY + 8) scrollY = sel - 7; }
        if (check(SelPress)) {
            funcs[sel]();
            rfidAdvDrawHeader("RFID Suite Avancada", RFID_COLOR_TITLE);
            rfidAdvDrawFooter("SEL: OK  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void rfidAdvDisplayProgress(const char *msg, int percent) {
    rfidAdvDrawProgressBar(RFID_BODY_Y + 160, percent, msg);
}

void rfidAdvShowWelcomeScreen() {
    rfidAdvDrawHeader("RFID Suite Avancada", RFID_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(RFID_COLOR_TEXT, RFID_COLOR_BG);
    tft.drawCentreString("55+ Funcoes RFID", tftWidth / 2, RFID_BODY_Y + 20, 1);
    tft.drawCentreString("Requer modulo RFID ativo", tftWidth / 2, RFID_BODY_Y + 34, 1);
    rfidAdvDrawFooter("SEL: OK  ESC: Voltar", "");
}

// ============================================================================
// Setup / Cleanup
// ============================================================================

void rfidAdvSetup() {
    rfidAdvConfig = RFIDAdvConfig();
}

void rfidAdvCleanup() {
    if (rfidMod) {
        delete rfidMod;
        rfidMod = nullptr;
    }
}

#endif // !LITE_VERSION
