#include "FMMenu.h"
#include "core/display.h"
#include "core/utils.h"
#include "modules/fm/fm.h"

void FMMenu::optionsMenu() {
    // Show legal warning
    drawMainBorder(true);
    int8_t choice = displayMessage(
        "AVISO LEGAL\nTransmitir em FM sem\nlicenca pode ser ilegal.\nContinuar?",
        "Voltar",
        nullptr,
        "Continuar",
        TFT_YELLOW
    );

    if (choice != 1) return;

    options = {
#if !defined(LITE_VERSION) and defined(FM_SI4713)
        // #if defined(FM_SI4713)
        {"Broadcast Padrao", lambdaHelper(fm_live_run, false)},
        {"Broadcast Rsvd", lambdaHelper(fm_live_run, true)},
        {"Parar Broadcast", fm_stop},
        {"Espectro FM", fm_spectrum},
        {"Hijack TA", fm_ta_run},
#endif
    };
    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "FM");
}


void FMMenu::drawIcon(float scale) {
    clearIconArea();
    int iconW = iconAreaW * 0.75 * scale;
    int iconH = iconAreaH * 0.5 * scale;

    if (iconW % 2 != 0) iconW++;
    if (iconH % 2 != 0) iconH++;


    int caseH = 5 * iconH / 6;
    int caseX = iconCenterX - iconW / 2;
    int caseY = iconCenterY - iconH / 3;

    int btnY = (2 * caseY + caseH + iconH / 10 + iconH / 3) / 2;
    int potX = (2 * caseX + iconW + iconW / 10 + iconW / 2) / 2;
    int potY = caseY + caseH / 3 + iconH / 10;

    // Case
    tft.drawRoundRect(caseX, caseY, iconW, caseH, iconW / 10, bruceConfig.priColor);

    // Potentiometer
    tft.fillCircle(potX, potY, iconH / 7, bruceConfig.priColor);

    // Screen
    tft.drawRect(caseX + iconW / 10, caseY + iconH / 10, iconW / 2, iconH / 3, bruceConfig.priColor);

    // Antenna
    tft.drawLine(
        caseX + iconW / 10, caseY, caseX + iconW / 10 + iconH / 3, caseY - iconH / 6, bruceConfig.priColor
    );
    tft.fillCircle(caseX + iconW / 10 + iconH / 3, caseY - iconH / 6, iconH / 30, bruceConfig.priColor);

    // Buttons
    tft.fillCircle(caseX + iconW / 10 + iconH / 8, btnY, iconH / 12, bruceConfig.priColor);
    tft.fillCircle(caseX + iconW / 10 + iconW / 2 - iconH / 8, btnY, iconH / 12, bruceConfig.priColor);
}
