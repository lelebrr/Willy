#ifndef __OTHERS_MENU_H__
#define __OTHERS_MENU_H__

#include "MenuItemInterface.h"

class OthersMenu : public MenuItemInterface {

public:
    OthersMenu() : MenuItemInterface("Outros") {}

    void micMenu();
    void badUsbHidMenu(); // New submenu for BadUSB & HID tools
    void optionsMenu(void);
    void drawIcon(float scale);

    bool hasTheme() { return willyConfig.theme.others; }
    String themePath() { return willyConfig.theme.paths.others; }
};

#endif
