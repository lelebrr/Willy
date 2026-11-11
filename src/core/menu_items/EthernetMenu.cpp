#include "EthernetMenu.h"
#if !defined(LITE_VERSION)
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "modules/ethernet/ARPScanner.h"
#include "modules/ethernet/DHCPStarvation.h"
#include "modules/ethernet/EthernetHelper.h"
#include "modules/ethernet/MACFlooding.h"

void EthernetMenu::start_ethernet() {
    eth = new EthernetHelper();
    if (!eth->setup()) {
        displayError("W5500 not found");
        delete eth;
        eth = nullptr;
        return;
    }
    while (!eth->is_connected()) { delay(100); }
}

void EthernetMenu::optionsMenu() {
    options = {
        {"Escanear Hosts",
         [this]() {
             start_ethernet();
             if (eth != nullptr) {
                 run_arp_scanner();
                 eth->stop();
                 delete eth;
                 eth = nullptr;
             } else {
                 displayError("W5500 nao encontrado");
             }
         }},
        {"DHCP Starvation",
         [this]() {
             start_ethernet();
             if (eth != nullptr) {
                 DHCPStarvation();
                 eth->stop();
                 delete eth;
                 eth = nullptr;
             } else {
                 displayError("W5500 nao encontrado");
             }
         }},
        {"MAC Flooding",    [this]() {
             start_ethernet();
             if (eth != nullptr) {
                 MACFlooding();
                 eth->stop();
                 delete eth;
                 eth = nullptr;
             } else {
                 displayError("W5500 nao encontrado");
             }
         }}

    };
    addOptionToMainMenu();

    delay(200);

    loopOptions(options, MENU_TYPE_SUBMENU, "Ethernet");
}

void EthernetMenu::drawIcon(float scale) {
    clearIconArea();

    int iconW = scale * 30;
    int iconH = scale * 40;

    int Y = iconCenterY - 25;

    int smallerH = scale * 16;

    int starterX = iconCenterX - iconW; // X of the first side
    int finalX = iconCenterX + iconW;

    int lineWidth = 2;

    // Draw the main socket structure
    /*
    |-----|
    |     |
    |     |
    */
    tft.drawRect(starterX, Y, lineWidth, iconH, willyConfig.priColor);

    tft.drawRect(finalX, Y, lineWidth, iconH, willyConfig.priColor);

    tft.drawRect(starterX, Y, iconW * 2, lineWidth, willyConfig.priColor);

    // Draw the shorter side to close the first part of socket
    /*
    |-----|
    |     |
    |-   -|
    */
    tft.drawRect(starterX, Y + iconH, smallerH, lineWidth, willyConfig.priColor);

    tft.drawRect(finalX - smallerH + lineWidth, Y + iconH, smallerH, lineWidth, willyConfig.priColor);

    // Draw the final enclosure
    /*
    |------|
    |      |
    |-    -|
      |  |
    */
    tft.drawRect(starterX + smallerH, Y + iconH, lineWidth, smallerH, willyConfig.priColor);
    tft.drawRect(finalX - smallerH + lineWidth, Y + iconH, lineWidth, smallerH, willyConfig.priColor);

    // Draw the four cable pin at a distance of 15 pixel
    for (size_t i = 0; i < 4; i++) {
        tft.drawRect(starterX + 15 + (i * 15), Y, lineWidth, 16, willyConfig.priColor);
    }

    // Close the socket calculating width of this side removing from total width, the size of the smaller size
    tft.drawRect(
        starterX + smallerH,
        Y + iconH + smallerH,
        (iconW * 2) - (smallerH * 2) + 4,
        lineWidth,
        willyConfig.priColor
    );
}
#endif
