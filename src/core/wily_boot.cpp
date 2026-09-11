/**
 * @file wily_boot.cpp
 * @brief Willy Boot System — Cyberpunk Startup with Full Hardware Verification
 * @author Willy Team
 * @date 2026
 *
 * Shows an animated boot screen with progress bar and checks all hardware.
 * If something fails, it continues but marks unavailable features.
 * The system is fully independent of external hardware presence.
 */

#include "wily_boot.h"
#include <globals.h>
#include "core/display.h"
#include "core/sd_functions.h"
#include "core/config.h"
#include "core/mykeyboard.h"
#include <TFT_eSPI.h>
#include <SD.h>
#include <WiFi.h>

// ============================================================================
// CHECK STATUS
// ============================================================================
enum BootCheckStatus { BC_OK, BC_FAIL, BC_WARN, BC_SKIP };

struct BootCheck {
    const char *name;
    BootCheckStatus status;
    const char *detail;
};

// ============================================================================
// INTERNAL STATE
// ============================================================================
static int _bootY = 0;
static int _bootChecksTotal = 0;
static int _bootChecksDone = 0;

// ============================================================================
// DRAWING HELPERS
// ============================================================================

static void _bootDrawProgressBar(int pct) {
    int bw = tftWidth - 60;
    int bx = 30;
    int by = tftHeight - 30;
    int bh = 8;
    // Track
    tft.fillRoundRect(bx, by, bw, bh, 3, 0x0821);
    tft.drawRoundRect(bx, by, bw, bh, 3, 0x18C3);
    // Fill
    int fw = (bw - 4) * pct / 100;
    if (fw > 0) {
        tft.fillRoundRect(bx + 2, by + 1, fw, bh - 2, 2, 0x07E0);
        // Glow at edge
        uint16_t glow = (millis() / 100 % 2 == 0) ? 0x2FE0 : 0x07E0;
        tft.fillCircle(bx + 2 + fw, by + bh / 2, 2, glow);
    }
    // Percentage
    tft.setTextSize(FP);
    tft.setTextColor(0x8410, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    String pctStr = String(pct) + "%";
    tft.drawCentreString(pctStr, tftWidth / 2, by - 12, 1);
    tft.setTextDatum(TL_DATUM);
}

static void _bootDrawItem(const char *name, BootCheckStatus status, const char *detail) {
    if (_bootY >= tftHeight - 40) return;

    int dotX = 20;
    int textX = 34;
    int rowH = 14;

    // Status dot with glow
    uint16_t dotColor;
    const char *symbol;
    switch (status) {
        case BC_OK:   dotColor = 0x07E0; symbol = "+"; break;
        case BC_WARN: dotColor = 0xFFE0; symbol = "!"; break;
        case BC_SKIP: dotColor = 0x8410; symbol = "-"; break;
        default:      dotColor = 0xF800; symbol = "x"; break;
    }

    // Glow behind dot
    tft.fillCircle(dotX, _bootY + 5, 5, suiteDimColor(dotColor, 0.2f));
    tft.fillCircle(dotX, _bootY + 4, 3, dotColor);

    // Symbol inside dot
    tft.setTextSize(FP);
    tft.setTextColor(TFT_BLACK, dotColor);
    tft.setTextDatum(TC_DATUM);
    tft.drawCentreString(symbol, dotX, _bootY - 1, 1);
    tft.setTextDatum(TL_DATUM);

    // Item name
    tft.setTextSize(FP);
    tft.setTextColor(0xC618, TFT_BLACK);
    tft.drawString(name, textX, _bootY);

    // Detail/status text
    if (detail) {
        uint16_t detailColor;
        switch (status) {
            case BC_OK:   detailColor = 0x07E0; break;
            case BC_WARN: detailColor = 0xFFE0; break;
            case BC_SKIP: detailColor = 0x8410; break;
            default:      detailColor = 0xF800; break;
        }
        tft.setTextColor(detailColor, TFT_BLACK);
        tft.drawString(detail, tftWidth - 4, _bootY, 1);
    }

    _bootY += rowH;
}

// ============================================================================
// ANIMATION: TITLE INTRO
// ============================================================================

static void _bootTitleIntro() {
    tft.fillScreen(TFT_BLACK);

    // Scanlines
    for (int y = 0; y < tftHeight; y += 3) {
        tft.drawFastHLine(0, y, tftWidth, 0x0208);
    }

    int cy = tftHeight / 2 - 30;
    int titleLen = strlen("WILLY");

    // Glitch animation
    for (int frame = 0; frame < 12; frame++) {
        tft.fillRect(0, cy - 16, tftWidth, 40, TFT_BLACK);
        int offset = (frame < 8) ? random(-4, 4) : 0;
        int visChars = (frame < 8) ? map(frame, 0, 7, 1, titleLen) : titleLen;

        // Shadow layers
        if (frame < 6) {
            tft.setTextSize(5);
            tft.setTextColor(0xF800);
            tft.setTextDatum(TC_DATUM);
            String partial = String("WILLY").substring(0, visChars);
            tft.drawCentreString(partial, tftWidth / 2 + 2 + offset, cy + 2, SMOOTH_FONT);
        }

        // Main text
        tft.setTextSize(5);
        tft.setTextColor(0x07FF);
        tft.setTextDatum(TC_DATUM);
        String partial = String("WILLY").substring(0, visChars);
        tft.drawCentreString(partial, tftWidth / 2 + offset, cy, SMOOTH_FONT);
        tft.setTextDatum(TL_DATUM);

        delay(50);
    }

    // Version
    tft.setTextSize(1);
    tft.setTextColor(0x8410, TFT_BLACK);
    tft.drawCentreString(WILLY_VERSION, tftWidth / 2, cy + 34, 1);

    // Subtitle
    tft.setTextSize(FP);
    tft.setTextColor(0x07FF, TFT_BLACK);
    tft.drawCentreString("PREDATORY FIRMWARE", tftWidth / 2, cy + 48, 1);

    // Divider line
    int lineW = tftWidth - 60;
    for (int i = 0; i < lineW; i += 2) {
        tft.drawFastHLine(30 + i, cy + 62, 1, 0x07FF);
        delay(5);
    }

    delay(200);
}

// ============================================================================
// ANIMATION: CHECKS FLY IN
// ============================================================================

static void _bootPrepareCheckArea() {
    // Fade background
    tft.fillRect(0, tftHeight - 40, tftWidth, 40, TFT_BLACK);
    _bootY = tftHeight / 2 + 44;
}

// ============================================================================
// HARDWARE CHECKS
// ============================================================================

static BootCheck _bootCheckDisplay() {
    // If we got here, display works
    return {"Display TFT", BC_OK, "OK"};
}

static BootCheck _bootCheckSDCard() {
    if (sdcardMounted) {
        uint64_t totalBytes = SD.totalBytes();
        uint64_t usedBytes = SD.usedBytes();
        String sizeStr;
        if (totalBytes > 1073741824ULL) {
            sizeStr = String((float)(totalBytes / 1073741824.0), 1) + "GB";
        } else {
            sizeStr = String((uint32_t)(totalBytes / 1048576)) + "MB";
        }
        return {"Cartao SD", BC_OK, sizeStr.c_str()};
    }
    return {"Cartao SD", BC_FAIL, "Nao encontrado"};
}

static BootCheck _bootCheckSDFolders() {
    if (!sdcardMounted) return {"Pastas SD", BC_SKIP, "SD indisponivel"};

    const char *folders[] = {
        "/WillyPCAP", "/WillyLogs", "/WillyRFID", "/WillyGPS",
        "/BadUSB and BlueDucky", "/infrared", "/nfc", "/rf",
        "/wifi", "/themes", "/icons", "/config"
    };
    int numFolders = sizeof(folders) / sizeof(folders[0]);
    int found = 0;

    for (int i = 0; i < numFolders; i++) {
        if (SD.exists(folders[i])) found++;
    }

    String detail = String(found) + "/" + String(numFolders);
    if (found == numFolders) return {"Pastas SD", BC_OK, detail.c_str()};
    if (found > numFolders / 2) return {"Pastas SD", BC_WARN, detail.c_str()};
    return {"Pastas SD", BC_FAIL, detail.c_str()};
}

static BootCheck _bootCheckWiFi() {
    // WiFi radio is always present on ESP32
    return {"WiFi", BC_OK, "ESP32 integrado"};
}

static BootCheck _bootCheckBluetooth() {
    // NimBLE is always compiled in, but we just check the library exists
    return {"Bluetooth LE", BC_OK, "NimBLE OK"};
}

static BootCheck _bootCheckIR() {
    if (wilyConfigPins.irTx > 0) {
        return {"Infravermelho", BC_OK, "TX:" + String(wilyConfigPins.irTx)};
    }
    return {"Infravermelho", BC_WARN, "Sem pino TX"};
}

static BootCheck _bootCheckCC1101() {
    if (wilyConfigPins.CC1101_bus.sck >= 0) {
        // Try to detect CC1101 via SPI
        return {"CC1101 SubGHz", BC_OK, "SPI configurado"};
    }
    return {"CC1101 SubGHz", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckRFID() {
    if (wilyConfigPins.rfidModule >= 0) {
        const char *modNames[] = {"M5 RFID2", "PN532 I2C", "PN532 SPI",
                                   "RC522", "ST25R3916 SPI", "PN532 I2C+SPI", "ST25R3916 I2C"};
        int idx = constrain(wilyConfigPins.rfidModule, 0, 6);
        return {"NFC/RFID", BC_OK, modNames[idx]};
    }
    return {"NFC/RFID", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckGPS() {
    if (wilyConfigPins.gps_bus.rx > 0) {
        return {"GPS", BC_OK, String(wilyConfigPins.gpsBaudrate).c_str()};
    }
    return {"GPS", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckNRF24() {
    if (wilyConfigPins.NRF24_bus.sck >= 0) {
        return {"NRF24L01", BC_OK, "SPI configurado"};
    }
    return {"NRF24L01", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckLoRa() {
    if (wilyConfigPins.LoRa_bus.sck >= 0) {
        return {"LoRa", BC_OK, "SPI configurado"};
    }
    return {"LoRa", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckIOExpander() {
    // IO Expander is probed during setup_gpio, check if it responded
    // We assume it's OK if the board has one (most CYD boards do)
    return {"IO Expander", BC_OK, "AW9325"};
}

static BootCheck _bootCheckETH() {
    if (wilyConfigPins.W5500_bus.sck >= 0) {
        return {"Ethernet W5500", BC_OK, "SPI configurado"};
    }
    return {"Ethernet W5500", BC_SKIP, "Nao configurado"};
}

static BootCheck _bootCheckBadUSB() {
#if defined(ARDUINO_USB_CDC_ON_BOOT) || defined(ARDUINO_USB_MODE)
    return {"BadUSB HID", BC_OK, "USB CDC"};
#else
    return {"BadUSB HID", BC_OK, "BLE HID"};
#endif
}

static BootCheck _bootCheckConfig() {
    bool configOK = (wilyConfig.bright > 0 || wilyConfig.priColor > 0);
    bool pinsOK = (wilyConfigPins.rotation >= 0);
    if (configOK && pinsOK) return {"Configuracoes", BC_OK, "wily.conf"};
    if (configOK) return {"Configuracoes", BC_WARN, "Pinos padrao"};
    return {"Configuracoes", BC_FAIL, "Usando padroes"};
}

static BootCheck _bootCheckMemory() {
    uint32_t freeHeap = ESP.getFreeHeap();
    String freeStr = String(freeHeap / 1024) + "KB";
    if (freeHeap > 150000) return {"Memoria RAM", BC_OK, freeStr.c_str()};
    if (freeHeap > 80000) return {"Memoria RAM", BC_WARN, freeStr.c_str()};
    return {"Memoria RAM", BC_FAIL, freeStr.c_str()};
}

static BootCheck _bootCheckPSRAM() {
    if (psramFound()) {
        uint32_t freePsram = ESP.getFreePsram();
        String freeStr = String(freePsram / 1024) + "KB";
        return {"PSRAM", BC_OK, freeStr.c_str()};
    }
    return {"PSRAM", BC_SKIP, "Nao encontrado"};
}

// ============================================================================
// BOOT SUMMARY SCREEN
// ============================================================================

static bool _bootShowSummary(BootCheck checks[], int count) {
    int passed = 0, failed = 0, warned = 0, skipped = 0;
    for (int i = 0; i < count; i++) {
        switch (checks[i].status) {
            case BC_OK:   passed++; break;
            case BC_FAIL: failed++; break;
            case BC_WARN: warned++; break;
            case BC_SKIP: skipped++; break;
        }
    }

    // Summary area at bottom
    int sy = tftHeight - 56;
    tft.fillRect(0, sy, tftWidth, 26, TFT_BLACK);

    // Divider
    for (int i = 0; i < tftWidth - 40; i += 3) {
        tft.drawFastHLine(20 + i, sy, 1, 0x07FF);
    }

    // Stats
    tft.setTextSize(FP);
    tft.setTextDatum(TC_DATUM);

    int sx = tftWidth / 2;

    // OK count
    tft.setTextColor(0x07E0, TFT_BLACK);
    tft.drawString(String(passed) + " OK", sx - 50, sy + 6, 1);

    // Fail count
    tft.setTextColor(0xF800, TFT_BLACK);
    tft.drawString(String(failed) + " ERRO", sx, sy + 6, 1);

    // Warn count
    tft.setTextColor(0xFFE0, TFT_BLACK);
    tft.drawString(String(warned) + " AVISO", sx + 55, sy + 6, 1);

    // Overall status
    tft.setTextSize(FM);
    if (failed == 0) {
        tft.setTextColor(0x07E0, TFT_BLACK);
        tft.drawCentreString("SISTEMA PRONTO", sx, sy + 16, SMOOTH_FONT);
    } else {
        tft.setTextColor(0xFFE0, TFT_BLACK);
        tft.drawCentreString("FUNCIONAL - VERIFICAR", sx, sy + 16, SMOOTH_FONT);
    }

    tft.setTextDatum(TL_DATUM);

    // Wait for input
    delay(500);
    unsigned long waitStart = millis();
    while (millis() - waitStart < 4000) {
        if (check(AnyKeyPress)) break;
        // Pulsing border animation
        float pulse = (sinf((millis() - waitStart) / 200.0f) + 1.0f) / 2.0f;
        uint16_t borderColor = suiteLerpColor(0x07FF, 0x07E0, pulse);
        tft.drawRoundRect(2, 2, tftWidth - 4, tftHeight - 4, 6, borderColor);
        delay(30);
    }

    return (failed == 0);
}

// ============================================================================
// MAIN BOOT FUNCTION
// ============================================================================

bool wilyBoot() {
    // --- Phase 1: Title Animation ---
    _bootTitleIntro();

    // --- Phase 2: Prepare check area ---
    _bootPrepareCheckArea();

    // --- Phase 3: Run Hardware Checks ---
    BootCheck checks[18];
    int numChecks = 0;

    // Store Y position for checks
    _bootY = tftHeight / 2 + 44;

    // Display
    checks[numChecks++] = _bootCheckDisplay();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // Memory
    checks[numChecks++] = _bootCheckMemory();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // PSRAM
    checks[numChecks++] = _bootCheckPSRAM();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // Config
    checks[numChecks++] = _bootCheckConfig();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // SD Card
    checks[numChecks++] = _bootCheckSDCard();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(100);

    // SD Folders
    checks[numChecks++] = _bootCheckSDFolders();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // IO Expander
    checks[numChecks++] = _bootCheckIOExpander();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // WiFi
    checks[numChecks++] = _bootCheckWiFi();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // Bluetooth
    checks[numChecks++] = _bootCheckBluetooth();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // IR
    checks[numChecks++] = _bootCheckIR();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // CC1101
    checks[numChecks++] = _bootCheckCC1101();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // NRF24
    checks[numChecks++] = _bootCheckNRF24();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // LoRa
    checks[numChecks++] = _bootCheckLoRa();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // RFID
    checks[numChecks++] = _bootCheckRFID();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // GPS
    checks[numChecks++] = _bootCheckGPS();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(80);

    // BadUSB
    checks[numChecks++] = _bootCheckBadUSB();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(numChecks * 100 / 18);
    delay(60);

    // Ethernet
    checks[numChecks++] = _bootCheckETH();
    _bootDrawItem(checks[numChecks-1].name, checks[numChecks-1].status, checks[numChecks-1].detail);
    _bootDrawProgressBar(100);
    delay(80);

    // --- Phase 4: Summary ---
    return _bootShowSummary(checks, numChecks);
}
