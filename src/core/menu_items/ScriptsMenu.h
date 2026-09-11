#ifndef __SCRIPTS_MENU_H__
#define __SCRIPTS_MENU_H__

#include <MenuItemInterface.h>

class ScriptsMenu : public MenuItemInterface {
public:
    ScriptsMenu() : MenuItemInterface("Interpretador JS") {}

    void optionsMenu();
    void drawIcon(float scale);
    bool hasTheme() { return wilyConfig.theme.interpreter; }
    String themePath() { return wilyConfig.theme.paths.interpreter; }
};

#endif
