#ifndef __FILE_MENU_H__
#define __FILE_MENU_H__

#include <MenuItemInterface.h>

class FileMenu : public MenuItemInterface {
public:
    FileMenu() : MenuItemInterface("Arquivos") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return wilyConfig.theme.files; }
    String themePath() { return wilyConfig.theme.paths.files; }
};

#endif
