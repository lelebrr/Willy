#include "ClockMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "modules/others/timer.h"
#include "core/wifi/wifi_common.h"
#include "core/utils.h"


void ClockMenu::optionsMenu() {
    while (!returnToMenu) {
        runClockLoop(true);

        // If ESC is pressed on the watch, it exits
        if (returnToMenu) break;

        // OK pressed, show submenu
        showSubMenu();

        // If "Exit" is pressed in the submenu, it exits
        if (returnToMenu) break;
    }
}

void ClockMenu::showSubMenu() {
    options = {
        {"Timer",           [=]() { Timer(); }              },
        {"Sincronizar NTP",  [=]() {
            if (!wifiConnected) {
                if (!wifiConnectMenu()) return;
            }
            displayInfo("Sincronizando...");
            updateClockTimezone();
            displayInfo("OK!", true);
        }},
        {"Voltar ao Relogio", [=]() {}                        },
        {"Sair",            [=]() { returnToMenu = true; }  }
    };

    delay(200);
    loopOptions(options, MENU_TYPE_SUBMENU, "Relogio");
}


void ClockMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 30;
    int pointerSize = scale * 15;

    // Case
    tft.drawArc(
        iconCenterX, iconCenterY, 1.1 * radius, radius, 0, 360, bruceConfig.priColor, bruceConfig.bgColor
    );

    // Pivot center
    tft.fillCircle(iconCenterX, iconCenterY, radius / 10, bruceConfig.priColor);

    // Hours & minutes
    tft.drawLine(
        iconCenterX,
        iconCenterY,
        iconCenterX - 2 * pointerSize / 3,
        iconCenterY - 2 * pointerSize / 3,
        bruceConfig.priColor
    );
    tft.drawLine(
        iconCenterX, iconCenterY, iconCenterX + pointerSize, iconCenterY - pointerSize, bruceConfig.priColor
    );
}
