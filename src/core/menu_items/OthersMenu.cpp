#include "OthersMenu.h"

#include "core/display.h"
#include "core/utils.h"
#include "core/config.h"
#include "core/settings.h"
#include "modules/badusb_ble/ducky_typer.h"
#include "modules/bjs_interpreter/interpreter.h"
#include "modules/others/clicker.h"
#include "modules/others/ibutton.h"
#include "modules/others/mic.h"
#include "modules/others/qrcode_menu.h"
#include "modules/others/tururururu.h"
#include "modules/others/u2f.h"
#ifdef USB_as_HID
#include "modules/badusb_ble/advanced_usb_attacks.h"
#endif
// Removed: #include "modules/others/timer.h"

void OthersMenu::optionsMenu() {
    options = {
        {"Códigos QR",   qrcode_menu                  },
        {"Orca",    shark_setup                  },
        {"Sobre o Willy", []() {
             drawMainBorderWithTitle("Sobre");
             padprintln("");
             padprintln("Willy " + String(BRUCE_VERSION));
             padprintln(ESP.getChipModel());
             padprintln("Flash: " + String(ESP.getFlashChipSize() / 1048576) + "MB");
         }},
#if defined(JOY_X_PIN) && defined(JOY_Y_PIN)
        {"Testar Joystick", []() {
#if defined(JOY_BTN_PIN)
             pinMode(JOY_BTN_PIN, INPUT_PULLUP);
#endif
             analogReadResolution(12);
             drawMainBorderWithTitle("Joystick");
             while (!check(EscPress)) {
                 int x = analogRead(JOY_X_PIN);
                 int y = analogRead(JOY_Y_PIN);
#if defined(JOY_BTN_PIN)
                 int btn = digitalRead(JOY_BTN_PIN);
#else
                 int btn = HIGH;
#endif
                 tft.fillRect(10, 40, tftWidth - 20, 60, bruceConfig.bgColor);
                 tft.setCursor(10, 40);
                 tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                 tft.setTextSize(FM);
                 tft.println("X: " + String(x));
                 tft.println("Y: " + String(y));
                 tft.println("BTN: " + String(btn == LOW ? "ON" : "OFF"));
                 vTaskDelay(100 / portTICK_PERIOD_MS);
             }
             returnToMenu = true;
         }},
#endif

#if defined(MIC_SPM1423) || defined(MIC_INMP441)
        {"Microfone",    [this]() { micMenu(); }      }, //@deveclipse
#endif

// New consolidated BadUSB & HID submenu
#if !defined(LITE_VERSION) || defined(USB_as_HID)
        {"BadUSB & HID", [this]() { badUsbHidMenu(); }},
#endif

#ifndef LITE_VERSION
        {"iButton",      setup_ibutton                },
#endif

        // Timer removed - moved to another "Clock"
    };

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Outros");
}

void OthersMenu::badUsbHidMenu() {
    options = {
#ifndef LITE_VERSION
        {"BadUSB",       [=]() { ducky_setup(hid_usb, false); }   },
        {"Teclado USB",  [=]() { ducky_keyboard(hid_usb, false); }},
        {"Layout Teclado", [=]() { setBadUSBBLEMenu(); }         },
#endif

#ifdef USB_as_HID
        {"Clicker USB",  clicker_setup                            },
        {"U2F USB",      u2f_setup                                },
        {"Atks Avancados", advancedUsbAtksMenu                    },
#endif

        {"Voltar",       [this]() { optionsMenu(); }              },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "BadUSB & HID");
}

void OthersMenu::micMenu() {
    options = {
#if defined(MIC_SPM1423) || defined(MIC_INMP441)
        {"Espectro", mic_test                   },
        {"Gravar",   mic_record_app             },
#endif
        {"Voltar",   [this]() { optionsMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Microfone");
}

void OthersMenu::drawIcon(float scale) {
    clearIconArea();

    // Dynamic radius calculation based on scale for responsive rendering
    int radius = scale * 7;

    // Center circle
    tft.fillCircle(iconCenterX, iconCenterY, radius, bruceConfig.priColor);

    // Concentric arcs - dynamically scaled for different screen sizes
    tft.drawArc(
        iconCenterX, iconCenterY, 2.5 * radius, 2 * radius, 0, 340, bruceConfig.priColor, bruceConfig.bgColor
    );

    tft.drawArc(
        iconCenterX, iconCenterY, 3.5 * radius, 3 * radius, 20, 360, bruceConfig.priColor, bruceConfig.bgColor
    );

    tft.drawArc(
        iconCenterX, iconCenterY, 4.5 * radius, 4 * radius, 0, 200, bruceConfig.priColor, bruceConfig.bgColor
    );

    tft.drawArc(
        iconCenterX,
        iconCenterY,
        4.5 * radius,
        4 * radius,
        240,
        360,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
