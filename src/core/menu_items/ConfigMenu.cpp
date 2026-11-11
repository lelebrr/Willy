#include "ConfigMenu.h"
#include <esp_pm.h>

#include "core/display.h"
#include "core/i2c_finder.h"
#include "core/main_menu.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#ifdef HAS_RGB_LED
#include "core/led_control.h"
#endif

/*********************************************************************
**  Function: optionsMenu
**  Main Config menu entry point
**********************************************************************/
void ConfigMenu::optionsMenu() {
  returnToMenu = false;
  while (true) {
    // Check if we need to exit to Main Menu (e.g., DevMode disabled)
    if (returnToMenu) {
      returnToMenu = false; // Reset flag
      return;
    }

    std::vector<Option> localOptions = {
        {"Tela & UI", [this]() { displayUIMenu(); }},
#ifdef HAS_RGB_LED
        {"Config LED", [this]() { ledMenu(); }},
#endif
        {"Config Áudio", [this]() { audioMenu(); }},
        {"Config Joystick", [this]() { joystickMenu(); }},
        {"Config Sistema", [this]() { systemMenu(); }},
        {"Energia", [this]() { powerMenu(); }},
    };
#if !defined(LITE_VERSION)
    if (!appStoreInstalled()) {
      localOptions.push_back(
          {"Instalar App Store", []() { installAppStoreJS(); }});
    }
#endif

    if (willyConfig.devMode) {
      localOptions.push_back({"Modo Dev", [this]() { devMenu(); }});
    }

    localOptions.push_back({"Sobre", showDeviceInfo});
    localOptions.push_back({"Menu Principal", []() {}});

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Configuracao");

    // Exit to Main Menu only if user pressed Back
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Otherwise rebuild Config menu after submenu returns
  }
}

/*********************************************************************
**  Function: displayUIMenu
**  Display & UI configuration submenu with auto-rebuild
**********************************************************************/
void ConfigMenu::displayUIMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {"Brilho", [this]() { setBrightnessMenu(); }},
        {"Tempo Dim", [this]() { setDimmerTimeMenu(); }},
        {"Orientacao", [this]() { lambdaHelper(gsetRotation, true)(); }},
        {"Cor UI", [this]() { setUIColor(); }},
        {"Tema UI", [this]() { setTheme(); }},
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Display & UI");

    // Exit only if user pressed Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Otherwise loop continues and menu rebuilds
  }
}

/*********************************************************************
**  Function: ledMenu
**  LED configuration submenu with auto-rebuild for toggles
**********************************************************************/
#ifdef HAS_RGB_LED
void ConfigMenu::ledMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {"Cor LED",
         [this]() {
           beginLed();
           setLedColorConfig();
         }},
        {"Efeito LED",
         [this]() {
           beginLed();
           setLedEffectConfig();
         }},
        {"Brilho LED",
         [this]() {
           beginLed();
           setLedBrightnessConfig();
         }},
        {String("Piscar LED: ") + (willyConfig.ledBlinkEnabled ? "LIG" : "DES"),
         [this]() {
           // Toggle LED blink setting
           willyConfig.ledBlinkEnabled = !willyConfig.ledBlinkEnabled;
           willyConfig.saveFile();
         }},
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config LED");

    // Exit only if user pressed Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds to update toggle label
  }
}
#endif
/*********************************************************************
**  Function: audioMenu
**  Audio configuration submenu with auto-rebuild for toggles
**********************************************************************/
void ConfigMenu::audioMenu() {
  while (true) {
    std::vector<Option> localOptions = {
#if !defined(LITE_VERSION)
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR) || defined(CYD)

        {String("Som: ") + (willyConfig.soundEnabled ? "LIG" : "DES"),
         [this]() {
           // Toggle sound setting
           willyConfig.soundEnabled = !willyConfig.soundEnabled;
           willyConfig.saveFile();
         }},
#if defined(HAS_NS4168_SPKR)
        {"Volume Som", [this]() { setSoundVolume(); }},
#endif // BUZZ_PIN || HAS_NS4168_SPKR
#endif //  HAS_NS4168_SPKR
#endif //  LITE_VERSION
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config Áudio");

    // Exit only if user pressed Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds to update toggle label
  }
}

/*********************************************************************
**  Function: systemMenu
**  System configuration submenu with auto-rebuild for toggles
**********************************************************************/
void ConfigMenu::systemMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {String("InstaBoot: ") + (willyConfig.instantBoot ? "LIG" : "DES"),
         [this]() {
           // Toggle InstaBoot setting
           willyConfig.instantBoot = !willyConfig.instantBoot;
           willyConfig.saveFile();
         }},
        {String("WiFi no Inicio: ") +
             (willyConfig.wifiAtStartup ? "LIG" : "DES"),
         [this]() {
           // Toggle WiFi at startup setting
           willyConfig.wifiAtStartup = !willyConfig.wifiAtStartup;
           willyConfig.saveFile();
         }},
        {"App Inicial", [this]() { setStartupApp(); }},
        {"Mostrar/Ocultar Apps", [this]() { mainMenu.hideAppsMenu(); }},
        {"Relogio", [this]() { setClock(); }},
        {"Avancado", [this]() { advancedMenu(); }},
        {"Voltar", []() {}},
    };

    int selected =
        loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config Sistema");

    // Exit only if user pressed Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds to update toggle labels
  }
}

/*********************************************************************
**  Function: advancedMenu
**  Advanced settings submenu (nested under System Config)
**********************************************************************/
void ConfigMenu::advancedMenu() {
  while (true) {
    std::vector<Option> localOptions = {
#if !defined(LITE_VERSION)
        {"Alternar API BLE", [this]() { enableBLEAPI(); }},
        {"BadUSB/BLE", [this]() { setBadUSBBLEMenu(); }},
#endif
        {"Credenciais Rede", [this]() { setNetworkCredsMenu(); }},
        {"Reset de Fabrica",
         []() {
           // Confirmation dialog for destructive action
           drawMainBorder(true);
           int8_t choice = displayMessage("Tem certeza que deseja\nResetar de "
                                          "Fabrica?\nDados serao perdidos!",
                                          "Nao", nullptr, "Sim", TFT_RED);

           if (choice == 1) {
             // User confirmed - perform factory reset
             willyConfigPins.factoryReset();
             willyConfig.factoryReset(); // Restarts ESP
           }
           // If cancelled, loop continues and menu rebuilds
         }},
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Avancado");

    // Exit to System Config menu
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds after each action
  }
}
/*********************************************************************
**  Function: powerMenu
**  Power management submenu with auto-rebuild
**********************************************************************/
void ConfigMenu::powerMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {"Deep Sleep", goToDeepSleep},
        {"Dormir", setSleepMode},
        {"Reiniciar", []() { ESP.restart(); }},
        {"Desligar",
         []() {
           // Confirmation dialog for power off
           drawMainBorder(true);
           int8_t choice = displayMessage("Desligar Dispositivo?", "Nao",
                                          nullptr, "Sim", TFT_RED);

           if (choice == 1) {
             powerOff();
           }
         }},
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Menu Energia");

    // Exit to Config menu
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds after each action
  }
}

/*********************************************************************
**  Function: devMenu
**  Developer mode menu for advanced hardware configuration
**********************************************************************/
void ConfigMenu::devMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {"I2C Finder", [this]() { find_i2c_addresses(); }},
        {"Pinos CC1101",
         [this]() { setSPIPinsMenu(willyConfigPins.CC1101_bus); }},
        {"Pinos NRF24",
         [this]() { setSPIPinsMenu(willyConfigPins.NRF24_bus); }},
#if !defined(LITE_VERSION)
        {"Pinos LoRa", [this]() { setSPIPinsMenu(willyConfigPins.LoRa_bus); }},
        {"Pinos W5500",
         [this]() { setSPIPinsMenu(willyConfigPins.W5500_bus); }},
#endif
        {"Pinos CartaoSD",
         [this]() { setSPIPinsMenu(willyConfigPins.SDCARD_bus); }},
        {"Pinos I2C", [this]() { setI2CPinsMenu(willyConfigPins.i2c_bus); }},
        {"Pinos UART", [this]() { setUARTPinsMenu(willyConfigPins.uart_bus); }},
        {"Pinos GPS", [this]() { setUARTPinsMenu(willyConfigPins.gps_bus); }},
        {"Serial USB", [this]() { switchToUSBSerial(); }},
        {"Serial UART", [this]() { switchToUARTSerial(); }},
        {"Desativar Modo Dev", [this]() { willyConfig.setDevMode(false); }},
        {"Voltar", []() {}},
    };

    int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Modo Dev");

    // Check if "Disable DevMode" was pressed (second-to-last option)
    if ((size_t)selected == localOptions.size() - 2) {
      returnToMenu = true; // Signal to exit all Config menus
      return;
    }

    // Exit to Config menu on Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds after each action
  }
}

/*********************************************************************
**  Function: switchToUSBSerial
**  Switch serial output to USB Serial
**********************************************************************/
void ConfigMenu::switchToUSBSerial() {
  if (usbSerial.getSerialOutput() == &Serial)
    return;
  usbSerial.setSerialOutput(&Serial);
  Serial1.end();
}

/*********************************************************************
**  Function: switchToUARTSerial
**  Switch serial output to UART (handles pin conflicts)
**  Ensures robust transition between serial interfaces
**********************************************************************/
void ConfigMenu::switchToUARTSerial() {
  if (usbSerial.getSerialOutput() == &Serial1)
    return;

  // Check and resolve SD card pin conflicts
  if (willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.uart_bus.rx) ||
      willyConfigPins.SDCARD_bus.checkConflict(willyConfigPins.uart_bus.tx)) {
    if (sdcardMounted) {
      sdcardSPI.end();
      sdcardMounted = false;
    }
  }

  // Check and resolve CC1101/NRF24 pin conflicts
  if (willyConfigPins.CC1101_bus.checkConflict(willyConfigPins.uart_bus.rx) ||
      willyConfigPins.CC1101_bus.checkConflict(willyConfigPins.uart_bus.tx) ||
      willyConfigPins.NRF24_bus.checkConflict(willyConfigPins.uart_bus.rx) ||
      willyConfigPins.NRF24_bus.checkConflict(willyConfigPins.uart_bus.tx)) {
    CC_NRF_SPI.end();
  }

  // Configure UART pins and switch serial output
  pinMode(willyConfigPins.uart_bus.rx, INPUT);
  pinMode(willyConfigPins.uart_bus.tx, OUTPUT);
  Serial1.begin(115200, SERIAL_8N1, willyConfigPins.uart_bus.rx,
                willyConfigPins.uart_bus.tx);
  usbSerial.setSerialOutput(&Serial1);
}

/*********************************************************************
**  Function: drawIcon
**  Draw config gear icon
**********************************************************************/
void ConfigMenu::drawIcon(float scale) {
  clearIconArea();
  int radius = scale * 9;

  const int toothCount = 8;
  const float angleStep = 360.0f / toothCount;
  const float toothWidth = angleStep * 0.5f;
  const float startOffset = (angleStep - toothWidth) / 2.0f;

  for (int i = 0; i < toothCount; i++) {
    float startAngle = startOffset + i * angleStep;
    tft.drawArc(iconCenterX, iconCenterY, 3.5 * radius, 2 * radius, startAngle,
                startAngle + toothWidth, willyConfig.priColor,
                willyConfig.bgColor, true);
  }

  // Draw inner circle
  tft.drawArc(iconCenterX, iconCenterY, 2.5 * radius, radius, 0, 360,
              willyConfig.priColor, willyConfig.bgColor, false);
}

/*********************************************************************
**  Function: joystickMenu
**  Joystick configuration and calibration menu
**********************************************************************/
void ConfigMenu::joystickMenu() {
  while (true) {
    std::vector<Option> localOptions = {
        {"Testar Joystick", [this]() { testJoystick(); }},
        {"Calibrar Joystick", [this]() { calibrateJoystick(); }},
        {"Status Detecção", [this]() { showJoystickStatus(); }},
        {"Voltar", []() {}},
    };

    int selected =
        loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config Joystick");

    // Exit only if user pressed Back or ESC
    if (selected == -1 || (size_t)selected == localOptions.size() - 1) {
      return;
    }
    // Menu rebuilds after each action
  }
}

/*********************************************************************
**  Function: testJoystick
**  Test joystick readings in real-time
**********************************************************************/
void ConfigMenu::testJoystick() {
  tft.fillScreen(willyConfig.bgColor);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.setTextSize(FM);
  tft.drawCentreString("Teste do Joystick", tftWidth / 2, 10, 1);
  tft.setTextSize(FP);
  tft.drawCentreString("Pressione ESC para sair", tftWidth / 2, 30, 1);

  tft.setTextSize(FM);
  tft.drawString("X: ", 10, 60);
  tft.drawString("Y: ", 10, 80);
  tft.drawString("BTN: ", 10, 100);

  bool exitTest = false;
  uint32_t lastUpdate = 0;

  while (!exitTest) {
    if (millis() - lastUpdate > 100) { // Update every 100ms
      int x = analogRead(JOY_X_PIN);
      int y = analogRead(JOY_Y_PIN);
      bool btn = digitalRead(JOY_BTN_PIN);

      // Clear previous values
      tft.fillRect(50, 60, 100, 60, willyConfig.bgColor);

      // Draw new values
      tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
      tft.drawString(String(x), 50, 60);
      tft.drawString(String(y), 50, 80);
      tft.drawString(btn ? "HIGH" : "LOW", 70, 100);

      // Show direction indicators
      tft.fillRect(10, 120, tftWidth - 20, 80, willyConfig.bgColor);
      if (x < 1500)
        tft.drawString("ESQUERDA", 10, 120);
      else if (x > 3500)
        tft.drawString("DIREITA", 10, 120);

      if (y < 1500)
        tft.drawString("CIMA", 10, 140);
      else if (y > 3500)
        tft.drawString("BAIXO", 10, 140);

      if (btn == LOW)
        tft.drawString("PRESSIONADO", 10, 160);

      lastUpdate = millis();
    }

    // Check for ESC press
    if (check(EscPress)) {
      exitTest = true;
    }

    delay(10);
  }

  tft.fillScreen(willyConfig.bgColor);
}

/*********************************************************************
**  Function: calibrateJoystick
**  Calibrate joystick center and deadzone
**********************************************************************/
void ConfigMenu::calibrateJoystick() {
  tft.fillScreen(willyConfig.bgColor);
  tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
  tft.setTextSize(FM);
  tft.drawCentreString("Calibração do Joystick", tftWidth / 2, 10, 1);
  tft.setTextSize(FP);
  tft.drawCentreString("Mantenha o joystick no centro", tftWidth / 2, 30, 1);
  tft.drawCentreString("Pressione o botão quando pronto", tftWidth / 2, 50, 1);

  // Wait for button press
  while (digitalRead(JOY_BTN_PIN) == HIGH) {
    delay(10);
  }

  // Read center values
  int centerX = 0, centerY = 0;
  const int samples = 10;
  for (int i = 0; i < samples; i++) {
    centerX += analogRead(JOY_X_PIN);
    centerY += analogRead(JOY_Y_PIN);
    delay(50);
  }
  centerX /= samples;
  centerY /= samples;

  tft.fillScreen(willyConfig.bgColor);
  tft.drawCentreString("Centro calibrado!", tftWidth / 2, 10, 1);
  tft.drawString("Centro X: " + String(centerX), 10, 40);
  tft.drawString("Centro Y: " + String(centerY), 10, 60);
  tft.drawCentreString("Pressione OK", tftWidth / 2, 100, 1);

  // Wait for button release and press again
  while (digitalRead(JOY_BTN_PIN) == LOW)
    delay(10);
  while (digitalRead(JOY_BTN_PIN) == HIGH)
    delay(10);

  tft.fillScreen(willyConfig.bgColor);
}

/*********************************************************************
**  Function: showJoystickStatus
**  Show joystick detection status
**********************************************************************/
void ConfigMenu::showJoystickStatus() {
  String status = "Status do Joystick:\n\n";

  // Check detection flags (from interface.cpp)
  extern bool getJoystickDetected();
  extern bool getJoystickButtonDetected();
  bool joystickDetected = getJoystickDetected();
  bool joystickButtonDetected = getJoystickButtonDetected();

  status +=
      "Joystick detectado: " + String(joystickDetected ? "SIM" : "NAO") + "\n";
  status +=
      "Botão detectado: " + String(joystickButtonDetected ? "SIM" : "NAO") +
      "\n\n";

  if (joystickDetected) {
    int x = analogRead(JOY_X_PIN);
    int y = analogRead(JOY_Y_PIN);
    bool btn = digitalRead(JOY_BTN_PIN);

    status += "Leituras atuais:\n";
    status += "X: " + String(x) + "\n";
    status += "Y: " + String(y) + "\n";
    status += "BTN: " + String(btn ? "HIGH" : "LOW") + "\n";
  } else {
    status += "Joystick não detectado.\nVerifique conexões.";
  }

  displayMessage(status.c_str(), "OK", nullptr, nullptr, willyConfig.priColor);
}
