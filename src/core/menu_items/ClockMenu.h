#ifndef __CLOCK_MENU_H__
#define __CLOCK_MENU_H__

#include <MenuItemInterface.h>

class ClockMenu : public MenuItemInterface {
public:
    ClockMenu() : MenuItemInterface("Relógio") {}

    void optionsMenu(void);
    void showSubMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return willyConfig.theme.clock; }
    String themePath() { return willyConfig.theme.paths.clock; }
};

#endif
