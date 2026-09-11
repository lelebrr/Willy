#ifndef __CONNECT_MENU_H__
#define __CONNECT_MENU_H__

#include <MenuItemInterface.h>

class ConnectMenu : public MenuItemInterface {
public:
    ConnectMenu() : MenuItemInterface("Conexão") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return wilyConfig.theme.connect; }
    String themePath() { return wilyConfig.theme.paths.connect; }
};

#endif
