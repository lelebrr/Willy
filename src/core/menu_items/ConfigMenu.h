#ifndef __CONFIG_MENU_H__
#define __CONFIG_MENU_H__

#include <MenuItemInterface.h>

class ConfigMenu : public MenuItemInterface {
public:
  ConfigMenu() : MenuItemInterface("Configuração") {}

  void optionsMenu(void);
  void drawIcon(float scale);
  bool hasTheme() { return willyConfig.theme.config; }
  String themePath() { return willyConfig.theme.paths.config; }

private:
  // Submenus
  void displayUIMenu(void);
  void ledMenu(void);
  void audioMenu(void);
  void systemMenu(void);
  void advancedMenu(void);
  void powerMenu(void);
  void devMenu(void);
  void joystickMenu(void);

  // Joystick functions
  void testJoystick(void);
  void calibrateJoystick(void);
  void showJoystickStatus(void);

  // Helper methods for complex operations
  void switchToUSBSerial(void);
  void switchToUARTSerial(void);
};

#endif
