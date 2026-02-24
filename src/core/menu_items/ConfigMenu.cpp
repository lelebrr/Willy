#include "ConfigMenu.h"
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
            {"Tela & UI",     [this]() { displayUIMenu(); }},
#ifdef HAS_RGB_LED
            {"Config LED",    [this]() { ledMenu(); }      },
#endif
            {"Config Áudio",  [this]() { audioMenu(); }    },
            {"Config Sistema",[this]() { systemMenu(); }   },
            {"Energia",       [this]() { powerMenu(); }    },
        };
#if !defined(LITE_VERSION)
        if (!appStoreInstalled()) {
            localOptions.push_back({"Instalar App Store", []() { installAppStoreJS(); }});
        }
#endif

        if (bruceConfig.devMode) {
            localOptions.push_back({"Modo Dev", [this]() { devMenu(); }});
        }

        localOptions.push_back({"Sobre", showDeviceInfo});
        localOptions.push_back({"Menu Principal", []() {}});

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Configuracao");

        // Exit to Main Menu only if user pressed Back
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
            {"Brilho",      [this]() { setBrightnessMenu(); }               },
            {"Tempo Dim",   [this]() { setDimmerTimeMenu(); }               },
            {"Orientacao",  [this]() { lambdaHelper(gsetRotation, true)(); }},
            {"Cor UI",      [this]() { setUIColor(); }                      },
            {"Tema UI",     [this]() { setTheme(); }                        },
            {"Voltar",      []() {}                                         },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Display & UI");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
             }                                                                            },
            {"Efeito LED",
             [this]() {
                 beginLed();
                 setLedEffectConfig();
             }                                                                            },
            {"Brilho LED",
             [this]() {
                 beginLed();
                 setLedBrightnessConfig();
             }                                                                            },
            {String("Piscar LED: ") + (bruceConfig.ledBlinkEnabled ? "LIG" : "DES"),
             [this]() {
                 // Toggle LED blink setting
                 bruceConfig.ledBlinkEnabled = !bruceConfig.ledBlinkEnabled;
                 bruceConfig.saveFile();
             }                                                                            },
            {"Voltar",                                                               []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config LED");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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

            {String("Som: ") + (bruceConfig.soundEnabled ? "LIG" : "DES"),
                                                             [this]() {
                 // Toggle sound setting
                 bruceConfig.soundEnabled = !bruceConfig.soundEnabled;
                 bruceConfig.saveFile();
             }                                                                                                                                            },
#if defined(HAS_NS4168_SPKR)
            {"Volume Som",                                                  [this]() { setSoundVolume(); }},
#endif  // BUZZ_PIN || HAS_NS4168_SPKR
#endif  //  HAS_NS4168_SPKR
#endif  //  LITE_VERSION
            {"Voltar",                                                      []() {}                       },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config Áudio");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
            {String("InstaBoot: ") + (bruceConfig.instantBoot ? "LIG" : "DES"),
             [this]() {
                 // Toggle InstaBoot setting
                 bruceConfig.instantBoot = !bruceConfig.instantBoot;
                 bruceConfig.saveFile();
             }                                                                                                           },
            {String("WiFi no Inicio: ") + (bruceConfig.wifiAtStartup ? "LIG" : "DES"),
             [this]() {
                 // Toggle WiFi at startup setting
                 bruceConfig.wifiAtStartup = !bruceConfig.wifiAtStartup;
                 bruceConfig.saveFile();
             }                                                                                                           },
            {"App Inicial",                                                         [this]() { setStartupApp(); }        },
            {"Mostrar/Ocultar Apps",                                                [this]() { mainMenu.hideAppsMenu(); }},
            {"Relogio",                                                             [this]() { setClock(); }             },
            {"Avancado",                                                            [this]() { advancedMenu(); }         },
            {"Voltar",                                                              []() {}                              },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Config Sistema");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
            {"Alternar API BLE", [this]() { enableBLEAPI(); }       },
            {"BadUSB/BLE",       [this]() { setBadUSBBLEMenu(); }   },
#endif
            {"Credenciais Rede", [this]() { setNetworkCredsMenu(); }},
            {"Reset de Fabrica",
                                      []() {
                 // Confirmation dialog for destructive action
                 drawMainBorder(true);
                 int8_t choice = displayMessage(
                     "Tem certeza que deseja\nResetar de Fabrica?\nDados serao perdidos!",
                     "Nao",
                     nullptr,
                     "Sim",
                     TFT_RED
                 );

                 if (choice == 1) {
                     // User confirmed - perform factory reset
                     bruceConfigPins.factoryReset();
                     bruceConfig.factoryReset(); // Restarts ESP
                 }
                 // If cancelled, loop continues and menu rebuilds
             }                                                                             },
            {"Voltar",           []() {}                            },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Avancado");

        // Exit to System Config menu
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
            {"Deep Sleep",    goToDeepSleep          },
            {"Dormir",        setSleepMode           },
            {"Reiniciar",     []() { ESP.restart(); }},
            {"Desligar",
             []() {
                 // Confirmation dialog for power off
                 drawMainBorder(true);
                 int8_t choice = displayMessage("Desligar Dispositivo?", "Nao", nullptr, "Sim", TFT_RED);

                 if (choice == 1) { powerOff(); }
             }                                    },
            {"Voltar",        []() {}                },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Menu Energia");

        // Exit to Config menu
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
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
            {"I2C Finder",      [this]() { find_i2c_addresses(); }                      },
            {"Pinos CC1101",    [this]() { setSPIPinsMenu(bruceConfigPins.CC1101_bus); }},
            {"Pinos NRF24",     [this]() { setSPIPinsMenu(bruceConfigPins.NRF24_bus); } },
#if !defined(LITE_VERSION)
            {"Pinos LoRa",      [this]() { setSPIPinsMenu(bruceConfigPins.LoRa_bus); }  },
            {"Pinos W5500",     [this]() { setSPIPinsMenu(bruceConfigPins.W5500_bus); } },
#endif
            {"Pinos CartaoSD",  [this]() { setSPIPinsMenu(bruceConfigPins.SDCARD_bus); }},
            {"Pinos I2C",       [this]() { setI2CPinsMenu(bruceConfigPins.i2c_bus); }   },
            {"Pinos UART",      [this]() { setUARTPinsMenu(bruceConfigPins.uart_bus); } },
            {"Pinos GPS",       [this]() { setUARTPinsMenu(bruceConfigPins.gps_bus); }  },
            {"Serial USB",      [this]() { switchToUSBSerial(); }                       },
            {"Serial UART",     [this]() { switchToUARTSerial(); }                      },
            {"Desativar Modo Dev", [this]() { bruceConfig.setDevMode(false); }             },
            {"Voltar",          []() {}                                                 },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Modo Dev");

        // Check if "Disable DevMode" was pressed (second-to-last option)
        if (selected == localOptions.size() - 2) {
            returnToMenu = true; // Signal to exit all Config menus
            return;
        }

        // Exit to Config menu on Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds after each action
    }
}

/*********************************************************************
**  Function: switchToUSBSerial
**  Switch serial output to USB Serial
**********************************************************************/
void ConfigMenu::switchToUSBSerial() {
    USBserial.setSerialOutput(&Serial);
    Serial1.end();
}

/*********************************************************************
**  Function: switchToUARTSerial
**  Switch serial output to UART (handles pin conflicts)
**********************************************************************/
void ConfigMenu::switchToUARTSerial() {
    // Check and resolve SD card pin conflicts
    if (bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.uart_bus.tx)) {
        sdcardSPI.end();
    }

    // Check and resolve CC1101/NRF24 pin conflicts
    if (bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.uart_bus.tx) ||
        bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.uart_bus.tx)) {
        CC_NRF_SPI.end();
    }

    // Configure UART pins and switch serial output
    pinMode(bruceConfigPins.uart_bus.rx, INPUT);
    pinMode(bruceConfigPins.uart_bus.tx, OUTPUT);
    Serial1.begin(115200, SERIAL_8N1, bruceConfigPins.uart_bus.rx, bruceConfigPins.uart_bus.tx);
    USBserial.setSerialOutput(&Serial1);
}
/*********************************************************************
**  Function: drawIcon
**  Draw config gear icon
**********************************************************************/
void ConfigMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 9;

    // Draw 6 gear teeth segments
    for (int i = 0; i < 6; i++) {
        tft.drawArc(
            iconCenterX,
            iconCenterY,
            3.5 * radius,
            2 * radius,
            15 + 60 * i,
            45 + 60 * i,
            bruceConfig.priColor,
            bruceConfig.bgColor,
            true
        );
    }

    // Draw inner circle
    tft.drawArc(
        iconCenterX,
        iconCenterY,
        2.5 * radius,
        radius,
        0,
        360,
        bruceConfig.priColor,
        bruceConfig.bgColor,
        false
    );
}
