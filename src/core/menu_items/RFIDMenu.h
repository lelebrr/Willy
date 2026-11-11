#ifndef __RFID_MENU_H__
#define __RFID_MENU_H__

#include <MenuItemInterface.h>

class RFIDMenu : public MenuItemInterface {
public:
    RFIDMenu() : MenuItemInterface("RFID") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return willyConfig.theme.rfid; }
    String themePath() { return willyConfig.theme.paths.rfid; }

private:
    void configMenu(void);
};

#endif
