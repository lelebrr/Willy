#include "RFIDMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "modules/rfid/PN532KillerTools.h"
#include "modules/rfid/amiibo.h"
#include "modules/rfid/chameleon.h"
#include "modules/rfid/pn532ble.h"
#include "modules/rfid/rfid125.h"
#include "modules/rfid/srix_tool.h" //added for srix Tool
#include "modules/rfid/tag_o_matic.h"
#include "modules/rfid/nfc_advanced_atks.h"

#ifndef LITE_VERSION
#include "modules/rfid/emv_reader.hpp"
#endif
void RFIDMenu::optionsMenu() {
    options = {
#if !defined(REMOVE_RFID_HW_INTERFACE)  // Remove Hardware interface menu due to lack of external GPIO
        {"Ler tag",    [=]() { TagOMatic(); }                          },
#ifndef LITE_VERSION
        {"Ler EMV",    [=]() { EMVReader(); }                          },
        {"Ler 125kHz", [=]() { RFID125(); }                            },
#endif
        {"Escanear tags",   [=]() { TagOMatic(TagOMatic::SCAN_MODE); }      },
        {"Carregar arq",   [=]() { TagOMatic(TagOMatic::LOAD_MODE); }      },
        {"Apagar dados",  [=]() { TagOMatic(TagOMatic::ERASE_MODE); }     },
        {"Escrever NDEF",  [=]() { TagOMatic(TagOMatic::WRITE_NDEF_MODE); }},
#endif
#ifndef LITE_VERSION
        {"Amiibolink",  [=]() { Amiibo(); }                             },
#endif
        {"Chameleon",   [=]() { Chameleon(); }                          },
        {"NFC Relay MITM",[=]() { NFCRelayMITM(); }                     },
        {"APDU Fuzzer",   [=]() { APDUCommandFuzzer(); }                },
        {"UID Collision", [=]() { DynamicUIDCollision(); }              },
        {"NDEF Exfil",    [=]() { NDEFStealthExfil(); }                 },
        {"MIFARE Rainbow", [=]() { MifareRainbowTable(); }               },
        {"Live Injection", [=]() { LiveDataInjectionEmulation(); }      },
        {"Chameleon Ultra", [=]() { MultiSlotChameleonUltra(); }         },
        {"Skimmer Hunter",  [=]() { RFIDSkimmerHunterPro(); }           },
        {"UID Obfuscator",  [=]() { UIDObfuscationRandomizer(); }        },
        {"NFC Worm",        [=]() { NFCWormPropagator(); }               },
        {"MSRP Sniffer",    [=]() { MobilePaymentSnifferMSRP(); }        },
        {"Mifare Brute",    [=]() { MifareMasterKeyBrute(); }            },
#ifndef LITE_VERSION
        {"PN532 BLE",   [=]() { Pn532ble(); }                           },
#if !defined(REMOVE_RFID_HW_INTERFACE)  // Remove Hardware interface menu due to lack of external GPIO
        {"PN532 UART",  [=]() { PN532KillerTools(); }                   },
#endif
#endif
        {"Config",      [this]() { configMenu(); }                      },
    };

#if !defined(REMOVE_RFID_HW_INTERFACE)
#ifndef LITE_VERSION
    if (bruceConfigPins.rfidModule == PN532_I2C_MODULE) {
        // Added SRIX Menu only if PN is set to i2c mode
        options.insert(options.begin() + 3, {"Ferramenta SRIX", [=]() { PN532_SRIX(); }});
    }
#endif
#endif

    addOptionToMainMenu();

    vTaskDelay(pdMS_TO_TICKS(200));

    String txt = "RFID";
    if (bruceConfigPins.rfidModule == M5_RFID2_MODULE) txt += " (RFID2)";
#ifdef M5STICK
    else if (bruceConfigPins.rfidModule == PN532_I2C_MODULE) txt += " (PN532-G33)";
    else if (bruceConfigPins.rfidModule == PN532_I2C_SPI_MODULE) txt += " (PN532-G36)";
#else
    else if (bruceConfigPins.rfidModule == PN532_I2C_MODULE) txt += " (PN532-I2C)";
#endif
    else if (bruceConfigPins.rfidModule == PN532_SPI_MODULE) txt += " (PN532-SPI)";
    else if (bruceConfigPins.rfidModule == RC522_SPI_MODULE) txt += " (RC522-SPI)";
    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
}

void RFIDMenu::configMenu() {
    options = {
#if !defined(REMOVE_RFID_HW_INTERFACE)  // Remove Hardware interface menu due to lack of external GPIO
        {"Modulo RFID", setRFIDModuleMenu          },
#endif
        {"Add Chave MIF", addMifareKeyMenu           },
        {"Voltar",        [this]() { optionsMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config RFID");
}

void RFIDMenu::drawIcon(float scale) {
    clearIconArea();
    int iconSize = scale * 70;
    int iconRadius = scale * 7;
    int deltaRadius = scale * 10;

    if (iconSize % 2 != 0) iconSize++;

    tft.drawRoundRect(
        iconCenterX - iconSize / 2,
        iconCenterY - iconSize / 2,
        iconSize,
        iconSize,
        iconRadius,
        bruceConfig.priColor
    );
    tft.fillRect(iconCenterX - iconSize / 2, iconCenterY, iconSize / 2, iconSize / 2, bruceConfig.bgColor);

    tft.drawCircle(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        iconRadius,
        bruceConfig.priColor
    );

    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius,
        2 * iconRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius + deltaRadius,
        2 * iconRadius + deltaRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius + 2 * deltaRadius,
        2 * iconRadius + 2 * deltaRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
