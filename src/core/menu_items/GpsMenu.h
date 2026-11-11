#ifndef __GPS_MENU_H__
#define __GPS_MENU_H__

#include <MenuItemInterface.h>

class GpsMenu : public MenuItemInterface {
public:
    GpsMenu() : MenuItemInterface("GPS") {}

    void optionsMenu(void);
    void wardrivingMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return willyConfig.theme.gps; }
    String themePath() { return willyConfig.theme.paths.gps; }

private:
    void configMenu(void);
    void modeMenu(void);
    void advancedConfigMenu(void);
    void updateRateMenu(void);
    void powerModeMenu(void);
    void protocolMenu(void);
    void navigationMenu(void);
    void satelliteFilterMenu(void);
    void applyAdvancedConfig(void);
};

#endif
