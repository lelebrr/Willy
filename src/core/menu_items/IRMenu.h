#ifndef __IR_MENU_H__
#define __IR_MENU_H__

#include <MenuItemInterface.h>

class IRMenu : public MenuItemInterface {
public:
    IRMenu() : MenuItemInterface("IR") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return willyConfig.theme.ir; }
    String themePath() { return willyConfig.theme.paths.ir; }

private:
    void configMenu(void);
    void modeMenu(void);
    void advancedConfigMenu(void);
    void frequencyMenu(void);
    void dutyCycleMenu(void);
    void preambleMenu(void);
    void noiseFilterMenu(void);
    void powerModeMenu(void);
};

#endif
