#include "main_menu.h"
#include "display.h"
#include "utils.h"
#include <globals.h>
#include "../ui/cyber_menu.h"

MainMenu::MainMenu() {
    _menuItems = {
        &wifiMenu,
        &bleMenu,
#if !defined(LITE_VERSION)
        &ethernetMenu,
#endif
        &rfMenu,
        &rfidMenu,
        &irMenu,
#if defined(FM_SI4713) && !defined(LITE_VERSION)
        &fmMenu,
#endif
        &fileMenu,
        &gpsMenu,
        &nrf24Menu,
#if !defined(LITE_VERSION)
#if !defined(DISABLE_INTERPRETER)
        &scriptsMenu,
#endif
        &loraMenu,
#endif
        &othersMenu,
        &clockMenu,
#if !defined(LITE_VERSION)
        &connectMenu,
#endif
        &configMenu,
    };

    // Add Cyber Menu experimental option
    options.push_back({
        "Cyber Menu (Exp)",
        []() {
            lv_obj_t *menu_scr = lv_obj_create(NULL);
            setup_cyber_menu(menu_scr);
            lv_scr_load_anim(menu_scr, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, true);
        },
        true
    });

    _totalItems = _menuItems.size();
}

MainMenu::~MainMenu() {}

void MainMenu::begin(void) {
    returnToMenu = false;
    options = {};

    std::vector<String> l = bruceConfig.disabledMenus;
    for (int i = 0; i < _totalItems; i++) {
        String itemName = _menuItems[i]->getName();
        if (find(l.begin(), l.end(), itemName) == l.end()) { // If menu item is not disabled
            options.push_back(
                {// selected lambda
                 _menuItems[i]->getName(),
                 [this, i]() { _menuItems[i]->optionsMenu(); },
                 false,                                  // selected = false
                 [](void *menuItem, bool shouldRender) { // render lambda
                     if (!shouldRender) return false;
                     drawMainBorder(false);

                     MenuItemInterface *obj = static_cast<MenuItemInterface *>(menuItem);
                     float scale = float((float)tftWidth / (float)240);
                     if (bruceConfigPins.rotation & 0b01) scale = float((float)tftHeight / (float)135);
                     obj->draw(scale);
#if defined(HAS_TOUCH)
                     TouchFooter();
#endif
                     return true;
                 },
                 _menuItems[i]
                }
            );
        }
    }
    _currentIndex = loopOptions(options, MENU_TYPE_MAIN, "Menu Principal", _currentIndex);
};

/*********************************************************************
**  Function: hideAppsMenu
**  Menu to Hide or show menus
**********************************************************************/

void MainMenu::hideAppsMenu() {
    auto items = this->getItems();
RESTART: // using gotos to avoid stackoverflow after many choices
    options.clear();
    for (auto item : items) {
        String label = item->getName();
        std::vector<String> l = bruceConfig.disabledMenus;
        bool enabled = find(l.begin(), l.end(), label) == l.end();
        options.push_back({label, [this, label]() { bruceConfig.addDisabledMenu(label); }, enabled});
    }
    options.push_back({"Mostrar Tudo", [=]() { bruceConfig.disabledMenus.clear(); }, true});
    addOptionToMainMenu();
    loopOptions(options);
    bruceConfig.saveFile();
    if (!returnToMenu) goto RESTART;
}
