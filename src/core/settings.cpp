#include "settings.h"
#include "core/led_control.h"
#include "core/wifi/wifi_common.h"
#include "current_year.h"
#include "display.h"
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
#include "modules/bjs_interpreter/interpreter.h"
#endif
#include "modules/ble_api/ble_api.hpp"
#include "modules/others/qrcode_menu.h"
#include "modules/rf/rf_utils.h" // for initRfModule
#include "mykeyboard.h"
#include "powerSave.h"
#include "sd_functions.h"
#include "settingsColor.h"
#include "utils.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <globals.h>

int currentScreenBrightness = -1;

// This function comes from interface.h
void _setBrightness(uint8_t brightval) {}

/*********************************************************************
**  Function: setBrightness
**  set brightness value
**********************************************************************/
void setBrightness(uint8_t brightval, bool save) {
  if (willyConfig.bright > 100)
    willyConfig.setBright(100);
  _setBrightness(brightval);
  delay(10);

  currentScreenBrightness = brightval;
  if (save) {
    willyConfig.setBright(brightval);
  }
}

/*********************************************************************
**  Function: getBrightness
**  get brightness value
**********************************************************************/
void getBrightness() {
  if (willyConfig.bright > 100) {
    willyConfig.setBright(100);
    _setBrightness(willyConfig.bright);
    delay(10);
    setBrightness(100);
  }

  _setBrightness(willyConfig.bright);
  delay(10);

  currentScreenBrightness = willyConfig.bright;
}

/*********************************************************************
**  Function: gsetRotation
**  get/set rotation value
**********************************************************************/
int gsetRotation(bool set) {
  int getRot = willyConfigPins.rotation;
  int result = ROTATION;
  int mask = ROTATION > 1 ? -2 : 2;

  options = {
      {"Padrao", [&]() { result = ROTATION; }},
      {"Paisagem (180)", [&]() { result = ROTATION + mask; }},
#if TFT_WIDTH >= 170 && TFT_HEIGHT >= 240
      {"Retrato (+90)", [&]() { result = ROTATION > 0 ? ROTATION - 1 : 3; }},
      {"Retrato (-90)", [&]() { result = ROTATION == 3 ? 0 : ROTATION + 1; }},

#endif
  };
  addOptionToMainMenu();
  if (set)
    loopOptions(options);
  else
    result = getRot;

  if (result > 3 || result < 0) {
    result = ROTATION;
    set = true;
  }
  if (set) {
    willyConfigPins.setRotation(result);
    ((tft_display *)&tft)->setRotation(result);
    ((tft_display *)&tft)->setRotation(result); // must repeat
  }
  returnToMenu = true;

  if (result & 0b01) { // if 1 or 3
    tftWidth = TFT_HEIGHT;
    tftHeight = TFT_WIDTH;
  } else { // if 2 or 0
    tftWidth = TFT_WIDTH;
    tftHeight = TFT_HEIGHT;
  }
  return result;
}

/*********************************************************************
**  Function: setBrightnessMenu
**  Handles Menu to set brightness
**********************************************************************/
void setBrightnessMenu() {
  int idx = 0;
  if (willyConfig.bright == 100)
    idx = 0;
  else if (willyConfig.bright == 75)
    idx = 1;
  else if (willyConfig.bright == 50)
    idx = 2;
  else if (willyConfig.bright == 25)
    idx = 3;
  else if (willyConfig.bright == 1)
    idx = 4;

  options = {
      {"100%", [=]() { setBrightness((uint8_t)100); },
       willyConfig.bright == 100,
       [](void *pointer, bool shouldRender) {
         setBrightness((uint8_t)100, false);
         return false;
       }},
      {"75 %", [=]() { setBrightness((uint8_t)75); }, willyConfig.bright == 75,
       [](void *pointer, bool shouldRender) {
         setBrightness((uint8_t)75, false);
         return false;
       }},
      {"50 %", [=]() { setBrightness((uint8_t)50); }, willyConfig.bright == 50,
       [](void *pointer, bool shouldRender) {
         setBrightness((uint8_t)50, false);
         return false;
       }},
      {"25 %", [=]() { setBrightness((uint8_t)25); }, willyConfig.bright == 25,
       [](void *pointer, bool shouldRender) {
         setBrightness((uint8_t)25, false);
         return false;
       }},
      {" 1 %", [=]() { setBrightness((uint8_t)1); }, willyConfig.bright == 1,
       [](void *pointer, bool shouldRender) {
         setBrightness((uint8_t)1, false);
         return false;
       }}};
  addOptionToMainMenu(); // this one bugs the brightness selection
  loopOptions(options, MENU_TYPE_REGULAR, "", idx);
  setBrightness(willyConfig.bright, false);
}

/*********************************************************************
**  Function: setSleepMode
**  Turn screen off and reduces cpu clock
**********************************************************************/
void setSleepMode() {
  sleepModeOn();
  while (1) {
    if (check(AnyKeyPress)) {
      sleepModeOff();
      returnToMenu = true;
      break;
    }
  }
}

/*********************************************************************
**  Function: setDimmerTimeMenu
**  Handles Menu to set dimmer time
**********************************************************************/
void setDimmerTimeMenu() {
  int idx = 0;
  if (willyConfig.dimmerSet == 10)
    idx = 0;
  else if (willyConfig.dimmerSet == 20)
    idx = 1;
  else if (willyConfig.dimmerSet == 30)
    idx = 2;
  else if (willyConfig.dimmerSet == 60)
    idx = 3;
  else if (willyConfig.dimmerSet == 0)
    idx = 4;
  options = {
      {"10s", [=]() { willyConfig.setDimmer(10); },
       willyConfig.dimmerSet == 10},
      {"20s", [=]() { willyConfig.setDimmer(20); },
       willyConfig.dimmerSet == 20},
      {"30s", [=]() { willyConfig.setDimmer(30); },
       willyConfig.dimmerSet == 30},
      {"60s", [=]() { willyConfig.setDimmer(60); },
       willyConfig.dimmerSet == 60},
      {"Desativado", [=]() { willyConfig.setDimmer(0); },
       willyConfig.dimmerSet == 0},
  };
  loopOptions(options, idx);
}

/*********************************************************************
**  Function: setUIColor
**  Set and store main UI color
**********************************************************************/
void setUIColor() {

  while (1) {
    options.clear();
    int idx = UI_COLOR_COUNT;
    int i = 0;
    for (const auto &mapping : UI_COLORS) {
      if (willyConfig.priColor == mapping.priColor &&
          willyConfig.secColor == mapping.secColor &&
          willyConfig.bgColor == mapping.bgColor) {
        idx = i;
      }

      options.emplace_back(
          mapping.name,
          [=, &mapping]() {
            uint16_t secColor = mapping.secColor;
            uint16_t bgColor = mapping.bgColor;
            willyConfig.setUiColor(mapping.priColor, &secColor, &bgColor);
          },
          idx == i);
      ++i;
    }

    options.push_back(
        {"Cor Customizada",
         [=]() {
           uint16_t oldPriColor = willyConfig.priColor;
           uint16_t oldSecColor = willyConfig.secColor;
           uint16_t oldBgColor = willyConfig.bgColor;

           if (setCustomUIColorMenu()) {
             willyConfig.setUiColor(willyConfig.priColor, &willyConfig.secColor,
                                    &willyConfig.bgColor);
           } else {
             willyConfig.priColor = oldPriColor;
             willyConfig.secColor = oldSecColor;
             willyConfig.bgColor = oldBgColor;
           }
           tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
         },
         idx == UI_COLOR_COUNT});

    options.push_back(
        {"Inverter Cores",
         [=]() {
           willyConfig.setColorInverted(!willyConfig.colorInverted);
           ((tft_display *)&tft)->invertDisplay(willyConfig.colorInverted);
         },
         willyConfig.colorInverted > 0});

    addOptionToMainMenu();

    int selectedOption = loopOptions(options, idx);
    if (selectedOption == -1 ||
        static_cast<size_t>(selectedOption) == options.size() - 1)
      return;
  }
}

uint16_t alterOneColorChannel565(uint16_t color, int newR, int newG, int newB) {
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  if (newR != 256)
    r = newR & 0x1F;
  if (newG != 256)
    g = newG & 0x3F;
  if (newB != 256)
    b = newB & 0x1F;

  return (r << 11) | (g << 5) | b;
}

bool setCustomUIColorMenu() {
  while (1) {
    options = {{"Primaria", [=]() { setCustomUIColorChoiceMenu(1); }},
               {"Secundaria", [=]() { setCustomUIColorChoiceMenu(2); }},
               {"Fundo", [=]() { setCustomUIColorChoiceMenu(3); }},
               {"Salvar", [=]() {}},
               {"Cancelar", [=]() {}}};

    int selectedOption = loopOptions(options);
    if (selectedOption == -1 || (size_t)selectedOption == options.size() - 1) {
      return false;
    } else if (selectedOption == 3) {
      return true;
    }
  }
}

void setCustomUIColorChoiceMenu(int colorType) {
  while (1) {
    options = {
        {"Canal Vermelho", [=]() { setCustomUIColorSettingMenuR(colorType); }},
        {"Canal Verde", [=]() { setCustomUIColorSettingMenuG(colorType); }},
        {"Canal Azul", [=]() { setCustomUIColorSettingMenuB(colorType); }},
        {"Voltar", [=]() {}}};

    int selectedOption = loopOptions(options);
    if (selectedOption == -1 ||
        static_cast<size_t>(selectedOption) == options.size() - 1)
      return;
  }
}

void setCustomUIColorSettingMenuR(int colorType) {
  setCustomUIColorSettingMenu(colorType, 1, [](uint16_t baseColor, int i) {
    return alterOneColorChannel565(baseColor, i, 256, 256);
  });
}

void setCustomUIColorSettingMenuG(int colorType) {
  setCustomUIColorSettingMenu(colorType, 2, [](uint16_t baseColor, int i) {
    return alterOneColorChannel565(baseColor, 256, i, 256);
  });
}

void setCustomUIColorSettingMenuB(int colorType) {
  setCustomUIColorSettingMenu(colorType, 3, [](uint16_t baseColor, int i) {
    return alterOneColorChannel565(baseColor, 256, 256, i);
  });
}

constexpr const char *colorTypes[] = {
    "Fundo",     // 0
    "Primaria",  // 1
    "Secundaria" // 2
};

constexpr const char *rgbNames[] = {
    "Azul",     // 0
    "Vermelho", // 1
    "Verde"     // 2
};

void setCustomUIColorSettingMenu(
    int colorType, int rgb,
    std::function<uint16_t(uint16_t, int)> colorGenerator) {
  uint16_t color = (colorType == 1)   ? willyConfig.priColor
                   : (colorType == 2) ? willyConfig.secColor
                                      : willyConfig.bgColor;

  options.clear();

  static auto hoverFunctionPriColor = [](void *pointer,
                                         bool shouldRender) -> bool {
    uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
    // Serial.printf("Setting primary color to: %04X\n", colorToSet);
    willyConfig.priColor = colorToSet;
    return false;
  };
  static auto hoverFunctionSecColor = [](void *pointer,
                                         bool shouldRender) -> bool {
    uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
    // Serial.printf("Setting secondary color to: %04X\n", colorToSet);
    willyConfig.secColor = colorToSet;
    return false;
  };

  static auto hoverFunctionBgColor = [](void *pointer,
                                        bool shouldRender) -> bool {
    uint16_t colorToSet = *static_cast<uint16_t *>(pointer);
    // Serial.printf("Setting bg color to: %04X\n", colorToSet);
    willyConfig.bgColor = colorToSet;
    tft.fillScreen(willyConfig.bgColor);
    return false;
  };

  static uint16_t colorStorage[32];
  int selectedIndex = 0;
  int i = 0;
  int index = 0;

  if (rgb == 1) {
    selectedIndex = (color >> 11) & 0x1F;
  } else if (rgb == 2) {
    selectedIndex = ((color >> 5) & 0x3F);
  } else {
    selectedIndex = color & 0x1F;
  }

  while (i <= (rgb == 2 ? 63 : 31)) {
    if (i == 0 || (rgb == 2 && (i + 1) % 2 == 0) || (rgb != 2)) {
      uint16_t updatedColor = colorGenerator(color, i);
      colorStorage[index] = updatedColor;

      options.emplace_back(
          String(i),
          [colorType, updatedColor]() {
            if (colorType == 1)
              willyConfig.priColor = updatedColor;
            else if (colorType == 2)
              willyConfig.secColor = updatedColor;
            else
              willyConfig.bgColor = updatedColor;
          },
          selectedIndex == i,
          (colorType == 1 ? hoverFunctionPriColor
                          : (colorType == 2 ? hoverFunctionSecColor
                                            : hoverFunctionBgColor)),
          &colorStorage[index]);
      ++index;
    }
    ++i;
  }

  addOptionToMainMenu();

  int selectedOption = loopOptions(
      options, MENU_TYPE_SUBMENU,
      (String(colorType == 1 ? "Primary"
                             : (colorType == 2 ? "Secondary" : "Background")) +
       " - " + (rgb == 1 ? "Red" : (rgb == 2 ? "Green" : "Blue")))
          .c_str(),
      (rgb != 2) ? selectedIndex
                 : (selectedIndex > 0 ? (selectedIndex + 1) / 2 : 0));
  if (selectedOption == -1 || (size_t)selectedOption == options.size() - 1) {
    if (colorType == 1) {
      willyConfig.priColor = color;
    } else if (colorType == 2) {
      willyConfig.secColor = color;
    } else {
      willyConfig.bgColor = color;
    }
    return;
  }
}

/*********************************************************************
**  Function: setSoundConfig - 01/2026 - Refactored "ConfigMenu" (this function
* manteined for retrocompatibility)
**  Enable or disable sound
**********************************************************************/
void setSoundConfig() {
  options = {
      {"Som Desligado", [=]() { willyConfig.setSoundEnabled(0); },
       willyConfig.soundEnabled == 0},
      {"Som Ligado", [=]() { willyConfig.setSoundEnabled(1); },
       willyConfig.soundEnabled == 1},
  };
  loopOptions(options, willyConfig.soundEnabled);
}

/*********************************************************************
**  Function: setSoundVolume
**  Set sound volume
**********************************************************************/
void setSoundVolume() {
  options = {
      {"10%", [=]() { willyConfig.setSoundVolume(10); },
       willyConfig.soundVolume == 10},
      {"20%", [=]() { willyConfig.setSoundVolume(20); },
       willyConfig.soundVolume == 20},
      {"30%", [=]() { willyConfig.setSoundVolume(30); },
       willyConfig.soundVolume == 30},
      {"40%", [=]() { willyConfig.setSoundVolume(40); },
       willyConfig.soundVolume == 40},
      {"50%", [=]() { willyConfig.setSoundVolume(50); },
       willyConfig.soundVolume == 50},
      {"60%", [=]() { willyConfig.setSoundVolume(60); },
       willyConfig.soundVolume == 60},
      {"70%", [=]() { willyConfig.setSoundVolume(70); },
       willyConfig.soundVolume == 70},
      {"80%", [=]() { willyConfig.setSoundVolume(80); },
       willyConfig.soundVolume == 80},
      {"90%", [=]() { willyConfig.setSoundVolume(90); },
       willyConfig.soundVolume == 90},
      {"100%", [=]() { willyConfig.setSoundVolume(100); },
       willyConfig.soundVolume == 100},
  };
  loopOptions(options, willyConfig.soundVolume);
}

#ifdef HAS_RGB_LED
/*********************************************************************
**  Function: setLedBlinkConfig - 01/2026 - Refactored "ConfigMenu" (this
* function manteined for retrocompatibility)
**  Enable or disable led blink
**********************************************************************/
void setLedBlinkConfig() {
  options = {
      {"Piscar LED Off", [=]() { willyConfig.setLedBlinkEnabled(0); },
       willyConfig.ledBlinkEnabled == 0},
      {"Piscar LED On", [=]() { willyConfig.setLedBlinkEnabled(1); },
       willyConfig.ledBlinkEnabled == 1},
  };
  loopOptions(options, willyConfig.ledBlinkEnabled);
}
#endif

/*********************************************************************
**  Function: setWifiStartupConfig
**  Enable or disable wifi connection at startup
**********************************************************************/
void setWifiStartupConfig() {
  options = {
      {"Desativar", [=]() { willyConfig.setWifiAtStartup(0); },
       willyConfig.wifiAtStartup == 0},
      {"Ativar", [=]() { willyConfig.setWifiAtStartup(1); },
       willyConfig.wifiAtStartup == 1},
  };
  loopOptions(options, willyConfig.wifiAtStartup);
}

/*********************************************************************
**  Function: addEvilWifiMenu
**  Handles Menu to add evil wifi names into config list
**********************************************************************/
void addEvilWifiMenu() {
  String apName = keyboard("", 30, "Evil Portal SSID");
  if (apName != "\x1B")
    willyConfig.addEvilWifiName(apName);
}

/*********************************************************************
**  Function: removeEvilWifiMenu
**  Handles Menu to remove evil wifi names from config list
**********************************************************************/
void removeEvilWifiMenu() {
  options = {};

  for (const auto &wifi_name : willyConfig.evilWifiNames) {
    options.push_back({wifi_name.c_str(), [wifi_name]() {
                         willyConfig.removeEvilWifiName(wifi_name);
                       }});
  }

  options.push_back({"Cancelar", [=]() { backToMenu(); }});

  loopOptions(options);
}

/*********************************************************************
**  Function: setEvilEndpointCreds
**  Handles menu for changing the endpoint to access captured creds
**********************************************************************/
void setEvilEndpointCreds() {
  String userInput = keyboard(willyConfig.evilPortalEndpoints.getCredsEndpoint,
                              30, "Evil creds endpoint");
  if (userInput != "\x1B")
    willyConfig.setEvilEndpointCreds(userInput);
}

/*********************************************************************
**  Function: setEvilEndpointSsid
**  Handles menu for changing the endpoint to change evilSsid
**********************************************************************/
void setEvilEndpointSsid() {
  String userInput = keyboard(willyConfig.evilPortalEndpoints.setSsidEndpoint,
                              30, "Evil creds endpoint");
  if (userInput != "\x1B")
    willyConfig.setEvilEndpointSsid(userInput);
}

/*********************************************************************
**  Function: setEvilAllowGetCredentials
**  Handles menu for toggling access to the credential list endpoint
**********************************************************************/

void setEvilAllowGetCreds() {
  options = {
      {"Bloquear", [=]() { willyConfig.setEvilAllowGetCreds(false); },
       willyConfig.evilPortalEndpoints.allowGetCreds == false},
      {"Permitir", [=]() { willyConfig.setEvilAllowGetCreds(true); },
       willyConfig.evilPortalEndpoints.allowGetCreds == true},
  };
  loopOptions(options, willyConfig.evilPortalEndpoints.allowGetCreds);
}

/*********************************************************************
**  Function: setEvilAllowGetCredentials
**  Handles menu for toggling access to the change SSID endpoint
**********************************************************************/

void setEvilAllowSetSsid() {
  options = {
      {"Bloquear", [=]() { willyConfig.setEvilAllowSetSsid(false); },
       willyConfig.evilPortalEndpoints.allowSetSsid == false},
      {"Permitir", [=]() { willyConfig.setEvilAllowSetSsid(true); },
       willyConfig.evilPortalEndpoints.allowSetSsid == true},
  };
  loopOptions(options, willyConfig.evilPortalEndpoints.allowSetSsid);
}

/*********************************************************************
**  Function: setEvilAllowEndpointDisplay
**  Handles menu for toggling the display of the Evil Portal endpoints
**********************************************************************/

void setEvilAllowEndpointDisplay() {
  options = {
      {"Bloquear", [=]() { willyConfig.setEvilAllowEndpointDisplay(false); },
       willyConfig.evilPortalEndpoints.showEndpoints == false},
      {"Permitir", [=]() { willyConfig.setEvilAllowEndpointDisplay(true); },
       willyConfig.evilPortalEndpoints.showEndpoints == true},
  };
  loopOptions(options, willyConfig.evilPortalEndpoints.showEndpoints);
}

/*********************************************************************
** Function: setEvilPasswordMode
** Handles menu for setting the evil portal password mode
***********************************************************************/
void setEvilPasswordMode() {
  options = {
      {"Save 'password'",
       [=]() { willyConfig.setEvilPasswordMode(FULL_PASSWORD); },
       willyConfig.evilPortalPasswordMode == FULL_PASSWORD},
      {"Save 'p******d'",
       [=]() { willyConfig.setEvilPasswordMode(FIRST_LAST_CHAR); },
       willyConfig.evilPortalPasswordMode == FIRST_LAST_CHAR},
      {"Save '*hidden*'",
       [=]() { willyConfig.setEvilPasswordMode(HIDE_PASSWORD); },
       willyConfig.evilPortalPasswordMode == HIDE_PASSWORD},
      {"Save length", [=]() { willyConfig.setEvilPasswordMode(SAVE_LENGTH); },
       willyConfig.evilPortalPasswordMode == SAVE_LENGTH},
  };
  loopOptions(options, willyConfig.evilPortalPasswordMode);
}

/*********************************************************************
**  Function: setRFModuleMenu
**  Handles Menu to set the RF module in use
**********************************************************************/
void setRFModuleMenu() {
  int result = 0;
  int idx = 0;
  uint8_t pins_setup = 0;
  if (willyConfigPins.rfModule == M5_RF_MODULE)
    idx = 0;
  else if (willyConfigPins.rfModule == CC1101_SPI_MODULE) {
    idx = 1;
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
    if (willyConfigPins.CC1101_bus.mosi == GPIO_NUM_26)
      idx = 2;
#endif
  }

  options = {
      {"M5 RF433T/R", [&]() { result = M5_RF_MODULE; }},
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
      {"CC1101 (Legado)", [&pins_setup]() { pins_setup = 1; }},
      {"CC1101 (SPI Comp.)", [&pins_setup]() { pins_setup = 2; }},
#else
      {"CC1101", [&]() { result = CC1101_SPI_MODULE; }},
#endif
      /* WIP:
       * #ifdef USE_CC1101_VIA_PCA9554
       * {"CC1101+PCA9554",  [&]() { result = 2; }},
       * #endif
       */
  };
  loopOptions(options, idx);
  if (result == CC1101_SPI_MODULE || pins_setup > 0) {
    // This setting is meant to StickCPlus and StickCPlus2 to setup the ports
    // from RF Menu
    if (pins_setup == 1) {
      result = CC1101_SPI_MODULE;
      willyConfigPins.setCC1101Pins(
          {(gpio_num_t)CC1101_SCK_PIN, (gpio_num_t)CC1101_MISO_PIN,
           (gpio_num_t)CC1101_MOSI_PIN, (gpio_num_t)CC1101_SS_PIN,
           (gpio_num_t)CC1101_GDO0_PIN, GPIO_NUM_NC});
      willyConfigPins.setNrf24Pins(
          {(gpio_num_t)CC1101_SCK_PIN, (gpio_num_t)CC1101_MISO_PIN,
           (gpio_num_t)CC1101_MOSI_PIN, (gpio_num_t)CC1101_SS_PIN,
           (gpio_num_t)CC1101_GDO0_PIN, GPIO_NUM_NC});
    } else if (pins_setup == 2) {
#if CONFIG_SOC_GPIO_OUT_RANGE_MAX > 30
      result = CC1101_SPI_MODULE;
      willyConfigPins.setCC1101Pins(
          {(gpio_num_t)SDCARD_SCK, (gpio_num_t)SDCARD_MISO,
           (gpio_num_t)SDCARD_MOSI, GPIO_NUM_33, GPIO_NUM_32, GPIO_NUM_NC});
      willyConfigPins.setNrf24Pins(
          {(gpio_num_t)SDCARD_SCK, (gpio_num_t)SDCARD_MISO,
           (gpio_num_t)SDCARD_MOSI, GPIO_NUM_33, GPIO_NUM_32, GPIO_NUM_NC});
#endif
    }
    if (initRfModule()) {
      willyConfigPins.setRfModule(CC1101_SPI_MODULE);
      deinitRfModule();
      if (pins_setup == 1)
        CC_NRF_SPI.end();
      return;
    }
    // else display an error
    displayError("CC1101 nao encontrado", true);
    if (pins_setup == 1)
      qrcode_display("https://github.com/lelebrr/Willy_ESP_s3/blob/main/media/"
                     "connections/cc1101_stick.jpg");
    if (pins_setup == 2)
      qrcode_display("https://github.com/lelebrr/Willy_ESP_s3/blob/main/media/"
                     "connections/cc1101_stick_SDCard.jpg");
    while (!check(AnyKeyPress))
      vTaskDelay(50 / portTICK_PERIOD_MS);
  }
  // fallback to "M5 RF433T/R" on errors
  willyConfigPins.setRfModule(M5_RF_MODULE);
}

/*********************************************************************
**  Function: setRFFreqMenu
**  Handles Menu to set the default frequency for the RF module
**********************************************************************/
void setRFFreqMenu() {
  float result = 433.92;
  String freq_str =
      num_keyboard(String(willyConfigPins.rfFreq), 10, "Frequencia Padrao:");
  if (freq_str == "\x1B")
    return;
  if (freq_str.length() > 1) {
    result = freq_str.toFloat(); // returns 0 if not valid
    // Check valid freq according to current module
    bool valid = false;
    int module = willyConfigPins.rfModule;
    if (module == CC1101_SPI_MODULE && result >= 300 && result <= 928)
      valid = true;
    else if (module == M5_RF_MODULE && result >= 2400 && result <= 2525)
      valid = true;
    else if (result >= 280 && result <= 928)
      valid = true; // fallback
    if (valid) {
      willyConfigPins.setRfFreq(result);
      return;
    }
  }
  // else
  displayError("Frequencia Invalida");
  willyConfigPins.setRfFreq(433.92); // reset to default
  delay(1000);
}

/*********************************************************************
**  Function: setRFIDModuleMenu
**  Handles Menu to set the RFID module in use
**********************************************************************/
void setRFIDModuleMenu() {
  options = {
      {"M5 RFID2", [=]() { willyConfigPins.setRfidModule(M5_RFID2_MODULE); },
       willyConfigPins.rfidModule == M5_RFID2_MODULE},
#ifdef M5STICK
      {"PN532 I2C G33",
       [=]() { willyConfigPins.setRfidModule(PN532_I2C_MODULE); },
       willyConfigPins.rfidModule == PN532_I2C_MODULE},
      {"PN532 I2C G36",
       [=]() { willyConfigPins.setRfidModule(PN532_I2C_SPI_MODULE); },
       willyConfigPins.rfidModule == PN532_I2C_SPI_MODULE},
#else
      {"PN532 em I2C",
       [=]() { willyConfigPins.setRfidModule(PN532_I2C_MODULE); },
       willyConfigPins.rfidModule == PN532_I2C_MODULE},
#endif
      {"PN532 em SPI",
       [=]() { willyConfigPins.setRfidModule(PN532_SPI_MODULE); },
       willyConfigPins.rfidModule == PN532_SPI_MODULE},
      {"RC522 em SPI",
       [=]() { willyConfigPins.setRfidModule(RC522_SPI_MODULE); },
       willyConfigPins.rfidModule == RC522_SPI_MODULE},
  };
  loopOptions(options, willyConfigPins.rfidModule);
}

/*********************************************************************
**  Function: addMifareKeyMenu
**  Handles Menu to add MIFARE keys into config list
**********************************************************************/
void addMifareKeyMenu() {
  String key = keyboard("", 12, "Chave MIFARE");
  if (key != "\x1B" && !key.isEmpty()) {
    if (key.length() == 12) {
      bool isHex = true;
      for (char c : key) {
        if (!isxdigit(c)) {
          isHex = false;
          break;
        }
      }
      if (isHex) {
        willyConfig.addMifareKey(key);
        displaySuccess("Chave adicionada");
      } else {
        displayError("Apenas caracteres HEX", true);
      }
    } else {
      displayError("Deve ter 12 digitos", true);
    }
  }
}

/*********************************************************************
**  Function: setClock
**  Handles Menu to set timezone to NTP
**********************************************************************/
const char *ntpServer = "pool.ntp.org";
WiFiUDP *ntpUDP = nullptr;
NTPClient *timeClient = nullptr;

void setClock() {
#if defined(HAS_RTC)
  RTC_TimeTypeDef TimeStruct;
#if defined(HAS_RTC_BM8563)
  _rtc.GetBm8563Time();
#endif
#if defined(HAS_RTC_PCF85063A)
  _rtc.GetPcf85063Time();
#endif
#endif
  if (!timeClient) {
    ntpUDP = new WiFiUDP();
    timeClient = new NTPClient(*ntpUDP, ntpServer, 0, 0);
  }

  options = {
      {"Via NTP Definir Fuso",
       [&]() { willyConfig.setAutomaticTimeUpdateViaNTP(true); }},
      {"Definir Hora Manualmente",
       [&]() { willyConfig.setAutomaticTimeUpdateViaNTP(false); }},
      {("Horario Verao " + String(willyConfig.dst ? "On" : "Off")).c_str(),
       [&]() {
         willyConfig.setDST(!willyConfig.dst);
         updateClockTimezone();
         returnToMenu = true;
       }},
      {(willyConfig.clock24hr ? "Formato 24H" : "Formato 12H"), [&]() {
         willyConfig.setClock24Hr(!willyConfig.clock24hr);
         returnToMenu = true;
       }}};

  addOptionToMainMenu();
  loopOptions(options);

  if (returnToMenu)
    return;

  if (willyConfig.automaticTimeUpdateViaNTP) {
    if (!wifiConnected)
      wifiConnectMenu();

    options.clear();

#ifndef LITE_VERSION

    struct TimezoneMapping {
      const char *name;
      float offset;
    };

    constexpr TimezoneMapping timezoneMappings[] = {
        {"UTC-12 (Baker Island, Howland Island)", -12},
        {"UTC-11 (Niue, Pago Pago)", -11},
        {"UTC-10 (Honolulu, Papeete)", -10},
        {"UTC-9 (Anchorage, Gambell)", -9},
        {"UTC-9.5 (Marquesas Islands)", -9.5},
        {"UTC-8 (Los Angeles, Vancouver, Tijuana)", -8},
        {"UTC-7 (Denver, Phoenix, Edmonton)", -7},
        {"UTC-6 (Mexico City, Chicago, Tegucigalpa)", -6},
        {"UTC-5 (New York, Toronto, Lima)", -5},
        {"UTC-4 (Caracas, Santiago, La Paz)", -4},
        {"UTC-3 (Brasilia, Sao Paulo, Montevideo)", -3},
        {"UTC-2 (South Georgia, Mid-Atlantic)", -2},
        {"UTC-1 (Azores, Cape Verde)", -1},
        {"UTC+0 (London, Lisbon, Casablanca)", 0},
        {"UTC+0.5 (Tehran)", 0.5},
        {"UTC+1 (Berlin, Paris, Rome)", 1},
        {"UTC+2 (Cairo, Athens, Johannesburg)", 2},
        {"UTC+3 (Moscow, Riyadh, Nairobi)", 3},
        {"UTC+3.5 (Tehran)", 3.5},
        {"UTC+4 (Dubai, Baku, Muscat)", 4},
        {"UTC+4.5 (Kabul)", 4.5},
        {"UTC+5 (Islamabad, Karachi, Tashkent)", 5},
        {"UTC+5.5 (New Delhi, Mumbai, Colombo)", 5.5},
        {"UTC+5.75 (Kathmandu)", 5.75},
        {"UTC+6 (Dhaka, Almaty, Omsk)", 6},
        {"UTC+6.5 (Yangon, Cocos Islands)", 6.5},
        {"UTC+7 (Bangkok, Jakarta, Hanoi)", 7},
        {"UTC+8 (Beijing, Singapore, Perth)", 8},
        {"UTC+8.75 (Eucla)", 8.75},
        {"UTC+9 (Tokyo, Seoul, Pyongyang)", 9},
        {"UTC+9.5 (Adelaide, Darwin)", 9.5},
        {"UTC+10 (Sydney, Melbourne, Vladivostok)", 10},
        {"UTC+10.5 (Lord Howe Island)", 10.5},
        {"UTC+11 (Solomon Islands, Nouméa)", 11},
        {"UTC+12 (Auckland, Fiji, Kamchatka)", 12},
        {"UTC+12.75 (Chatham Islands)", 12.75},
        {"UTC+13 (Tonga, Phoenix Islands)", 13},
        {"UTC+14 (Kiritimati)", 14}};

    int idx = 0;
    int i = 0;
    for (const auto &mapping : timezoneMappings) {
      if (willyConfig.tmz == mapping.offset) {
        idx = i;
      }

      options.emplace_back(
          mapping.name, [=, &mapping]() { willyConfig.setTmz(mapping.offset); },
          idx == i);
      ++i;
    }

#else
    constexpr float timezoneOffsets[] = {
        -12, -11, -10,  -9.5, -9,  -8,  -7,   -6,  -5, -4,    -3,   -2, -1,
        0,   0.5, 1,    2,    3,   3.5, 4,    4.5, 5,  5.5,   5.75, 6,  6.5,
        7,   8,   8.75, 9,    9.5, 10,  10.5, 11,  12, 12.75, 13,   14};

    int idx = 0;
    int i = 0;
    for (const auto &offset : timezoneOffsets) {
      if (willyConfig.tmz == offset)
        idx = i;

      options.emplace_back(
          ("UTC" + String(offset >= 0 ? "+" : "") + String(offset)).c_str(),
          [=]() { willyConfig.setTmz(offset); }, willyConfig.tmz == offset);
      ++i;
    }

#endif

    addOptionToMainMenu();

    loopOptions(options, idx);

    updateClockTimezone();

  } else {
    int hr, mn, am;
    options = {};
    for (int i = 0; i < 12; i++) {
      String tmp = String(i < 10 ? "0" : "") + String(i);
      options.push_back({tmp.c_str(), [&]() { delay(1); }});
    }

    hr = loopOptions(options, MENU_TYPE_SUBMENU, "Set Hour");
    options.clear();

    for (uint8_t i = 0; i < 60; i++) {
      String tmp = String(i < 10 ? "0" : "") + String(i);
      options.push_back({tmp.c_str(), [&]() { delay(1); }});
    }

    mn = loopOptions(options, MENU_TYPE_SUBMENU, "Set Minute");
    options.clear();

    options = {
        {"AM", [&]() { am = 0; }},
        {"PM", [&]() { am = 12; }},
    };

    loopOptions(options);

#if defined(HAS_RTC)
    TimeStruct.Hours = hr + am;
    TimeStruct.Minutes = mn;
    TimeStruct.Seconds = 0;
    _rtc.SetTime(&TimeStruct);
    _rtc.GetTime(&_time);
    _rtc.GetDate(&_date);

    struct tm timeinfo = {};
    timeinfo.tm_sec = _time.Seconds;
    timeinfo.tm_min = _time.Minutes;
    timeinfo.tm_hour = _time.Hours;
    timeinfo.tm_mday = _date.Date;
    timeinfo.tm_mon = _date.Month > 0 ? _date.Month - 1 : 0;
    timeinfo.tm_year = _date.Year >= 1900 ? _date.Year - 1900 : 0;
    time_t epoch = mktime(&timeinfo);
    struct timeval tv = {.tv_sec = epoch};
    settimeofday(&tv, nullptr);
#else
    rtc.setTime(0, mn, hr + am, 20, 06,
                CURRENT_YEAR); // send me a gift, @Pirata!
    struct tm t = rtc.getTimeStruct();
    time_t epoch = mktime(&t);
    struct timeval tv = {.tv_sec = epoch};
    settimeofday(&tv, nullptr);
#endif
    clock_set = true;
  }
}

void runClockLoop(bool showMenuHint) {
  unsigned long tmp = 0; // Changed int to unsigned long
  unsigned long hintStartTime = millis();
  bool hintVisible = showMenuHint;

#if defined(HAS_RTC)
#if defined(HAS_RTC_BM8563)
  _rtc.GetBm8563Time();
#endif
#if defined(HAS_RTC_PCF85063A)
  _rtc.GetPcf85063Time();
#endif
  _rtc.GetTime(&_time);
#endif

  // Delay due to SelPress() detected on run
  tft.fillScreen(willyConfig.bgColor);
  delay(300);

  for (;;) {
    if (millis() - tmp > 1000) {
#if defined(HAS_RTC)
      updateTimeStr(_rtc.getTimeStruct());
#else
      updateTimeStr(rtc.getTimeStruct());
#endif
      Serial.print("Current time: ");
      Serial.println(timeStr);
      tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
      tft.drawRect(BORDER_PAD_X, BORDER_PAD_X, tftWidth - 2 * BORDER_PAD_X,
                   tftHeight - 2 * BORDER_PAD_X, willyConfig.priColor);
      uint8_t f_size = 4;
      for (uint8_t i = 4; i > 0; i--) {
        if (i * LW * strlen(timeStr) <
            static_cast<size_t>(tftWidth - BORDER_PAD_X * 2)) {
          f_size = i;
          break;
        }
      }
      tft.setTextSize(f_size);
      tft.drawCentreString(timeStr, tftWidth / 2, tftHeight / 2 - 13, 1);

      // "OK to show menu" hint management
      if (hintVisible && (millis() - hintStartTime < 5000)) {
        tft.setTextSize(1);
        tft.drawCentreString("OK to show menu", tftWidth / 2,
                             tftHeight / 2 + 25, 1);
      } else if (hintVisible && (millis() - hintStartTime >= 5000)) {
        // Clear hint after 5 seconds
        tft.fillRect(BORDER_PAD_X + 1, tftHeight / 2 + 20,
                     tftWidth - 2 * BORDER_PAD_X - 2, 20, willyConfig.bgColor);
        hintVisible = false;
      }
      tmp = millis();
    }

    // Checks to exit the loop
    if (check(SelPress)) {
      tft.fillScreen(willyConfig.bgColor);
      if (showMenuHint) {
        // Exits the loop to return to the caller (ClockMenu)
        break;
      } else {
        // Original behavior
        returnToMenu = true;
        break;
      }
    }

    if (check(EscPress)) {
      tft.fillScreen(willyConfig.bgColor);
      returnToMenu = true;
      break;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/*********************************************************************
**  Function: gsetIrTxPin
**  get or set IR Tx Pin
**********************************************************************/
int gsetIrTxPin(bool set) {
  int result = willyConfigPins.irTx;

  if (result > 50)
    willyConfigPins.setIrTxPin(TXLED);
  if (set) {
    options.clear();
    std::vector<std::pair<const char *, int>> pins;
    pins = {IR_TX_PINS};
    int idx = 100;
    int j = 0;
    for (auto pin : pins) {
      if (pin.second == willyConfigPins.irTx && idx == 100)
        idx = j;
      j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i = pin.second;
      if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI &&
          i != TFT_BL && i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI &&
          i != SDCARD_MISO)
#endif
        options.push_back({pin.first,
                           [=]() { willyConfigPins.setIrTxPin(pin.second); },
                           pin.second == willyConfigPins.irTx});
    }

    loopOptions(options, idx);
    options.clear();

    Serial.println("Saved pin: " + String(willyConfigPins.irTx));
  }

  returnToMenu = true;
  return willyConfigPins.irTx;
}

void setIrTxRepeats() {
  uint8_t chRpts = 0; // Chosen Repeats

  options = {
      {"None", [&]() { chRpts = 0; }},
      {"5  (+ 1 initial)", [&]() { chRpts = 5; }},
      {"10 (+ 1 initial)", [&]() { chRpts = 10; }},
      {"Custom",
       [&]() {
         // up to 99 repeats
         String rpt = num_keyboard(String(willyConfigPins.irTxRepeats), 2,
                                   "Nbr of Repeats (+ 1 initial)");
         chRpts = static_cast<uint8_t>(rpt.toInt());
       }},
  };
  addOptionToMainMenu();

  loopOptions(options);

  if (returnToMenu)
    return;

  willyConfigPins.setIrTxRepeats(chRpts);
}
/*********************************************************************
**  Function: gsetIrRxPin
**  get or set IR Rx Pin
**********************************************************************/
int gsetIrRxPin(bool set) {
  int result = willyConfigPins.irRx;

  if (result > 45)
    willyConfigPins.setIrRxPin(GROVE_SCL);
  if (set) {
    options.clear();
    std::vector<std::pair<const char *, int>> pins;
    pins = {IR_RX_PINS};
    int idx = -1;
    int j = 0;
    for (auto pin : pins) {
      if (pin.second == willyConfigPins.irRx && idx < 0)
        idx = j;
      j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i = pin.second;
      if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI &&
          i != TFT_BL && i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI &&
          i != SDCARD_MISO)
#endif
        options.push_back({pin.first,
                           [=]() { willyConfigPins.setIrRxPin(pin.second); },
                           pin.second == willyConfigPins.irRx});
    }

    loopOptions(options);
  }

  returnToMenu = true;
  return willyConfigPins.irRx;
}

/*********************************************************************
**  Function: gsetRfTxPin
**  get or set RF Tx Pin
**********************************************************************/
int gsetRfTxPin(bool set) {
  int result = willyConfigPins.rfTx;

  if (result > 45)
    willyConfigPins.setRfTxPin(GROVE_SDA);
  if (set) {
    options.clear();
    std::vector<std::pair<const char *, int>> pins;
    pins = {RF_TX_PINS};
    int idx = -1;
    int j = 0;
    for (auto pin : pins) {
      if (pin.second == willyConfigPins.rfTx && idx < 0)
        idx = j;
      j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i = pin.second;
      if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI &&
          i != TFT_BL && i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI &&
          i != SDCARD_MISO)
#endif
        options.push_back({pin.first,
                           [=]() { willyConfigPins.setRfTxPin(pin.second); },
                           pin.second == willyConfigPins.rfTx});
    }

    loopOptions(options);
    options.clear();
  }

  returnToMenu = true;
  return willyConfigPins.rfTx;
}

/*********************************************************************
**  Function: gsetRfRxPin
**  get or set FR Rx Pin
**********************************************************************/
int gsetRfRxPin(bool set) {
  int result = willyConfigPins.rfRx;

  if (result > 36)
    willyConfigPins.setRfRxPin(GROVE_SCL);
  if (set) {
    options.clear();
    std::vector<std::pair<const char *, int>> pins;
    pins = {RF_RX_PINS};
    int idx = -1;
    int j = 0;
    for (auto pin : pins) {
      if (pin.second == willyConfigPins.rfRx && idx < 0)
        idx = j;
      j++;
#ifdef ALLOW_ALL_GPIO_FOR_IR_RF
      int i = pin.second;
      if (i != TFT_CS && i != TFT_RST && i != TFT_SCLK && i != TFT_MOSI &&
          i != TFT_BL && i != TOUCH_CS && i != SDCARD_CS && i != SDCARD_MOSI &&
          i != SDCARD_MISO)
#endif
        options.push_back({pin.first,
                           [=]() { willyConfigPins.setRfRxPin(pin.second); },
                           pin.second == willyConfigPins.rfRx});
    }

    loopOptions(options);
    options.clear();
  }

  returnToMenu = true;
  return willyConfigPins.rfRx;
}

/*********************************************************************
**  Function: setStartupApp
**  Handles Menu to set startup app
**********************************************************************/
void setStartupApp() {
  int idx = 0;
  if (willyConfig.startupApp == "")
    idx = 0;

  options = {{"None", [=]() { willyConfig.setStartupApp(""); },
              willyConfig.startupApp == ""}};

  int index = 0;
  for (String appName : startupApp.getAppNames()) {
    index++;
    if (willyConfig.startupApp == appName)
      idx = index;

    options.push_back({appName.c_str(), [=]() {
                         willyConfig.setStartupApp(appName);
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
                         if (appName == "JS Interpreter") {
                           options = getScriptsOptionsList("", true);
                           loopOptions(options, MENU_TYPE_SUBMENU,
                                       "Startup Script");
                         }
#endif
                       }});
  }

  loopOptions(options, idx);
  options.clear();
}

/*********************************************************************
**  Function: setGpsBaudrateMenu
**  Handles Menu to set the baudrate for the GPS module
**********************************************************************/
void setGpsBaudrateMenu() {
  options = {
      {"9600 bps", [=]() { willyConfigPins.setGpsBaudrate(9600); },
       willyConfigPins.gpsBaudrate == 9600},
      {"19200 bps", [=]() { willyConfigPins.setGpsBaudrate(19200); },
       willyConfigPins.gpsBaudrate == 19200},
      {"38400 bps", [=]() { willyConfigPins.setGpsBaudrate(38400); },
       willyConfigPins.gpsBaudrate == 38400},
      {"57600 bps", [=]() { willyConfigPins.setGpsBaudrate(57600); },
       willyConfigPins.gpsBaudrate == 57600},
      {"115200 bps", [=]() { willyConfigPins.setGpsBaudrate(115200); },
       willyConfigPins.gpsBaudrate == 115200},
  };

  loopOptions(options, willyConfigPins.gpsBaudrate);
}

/*********************************************************************
**  Function: setWifiApSsidMenu
**  Handles Menu to set the WiFi AP SSID
**********************************************************************/
void setWifiApSsidMenu() {
  const bool isDefault = willyConfig.wifiAp.ssid == "WillyNet";

  options = {
      {"Default (WillyNet)",
       [=]() {
         willyConfig.setWifiApCreds("WillyNet", willyConfig.wifiAp.pwd);
       },
       isDefault},
      {"Custom",
       [=]() {
         String newSsid =
             keyboard(willyConfig.wifiAp.ssid, 32, "WiFi AP SSID:");
         if (newSsid != "\x1B") {
           if (!newSsid.isEmpty())
             willyConfig.setWifiApCreds(newSsid, willyConfig.wifiAp.pwd);
           else
             displayError("SSID cannot be empty", true);
         }
       },
       !isDefault},
  };
  addOptionToMainMenu();

  loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApPasswordMenu
**  Handles Menu to set the WiFi AP Password
**********************************************************************/
void setWifiApPasswordMenu() {
  const bool isDefault = willyConfig.wifiAp.pwd == "WillyNet";

  options = {
      {"Default (WillyNet)",
       [=]() {
         willyConfig.setWifiApCreds(willyConfig.wifiAp.ssid, "WillyNet");
       },
       isDefault},
      {"Custom",
       [=]() {
         String newPassword =
             keyboard(willyConfig.wifiAp.pwd, 32, "WiFi AP Password:", true);
         if (newPassword != "\x1B") {
           if (!newPassword.isEmpty())
             willyConfig.setWifiApCreds(willyConfig.wifiAp.ssid, newPassword);
           else
             displayError("Password cannot be empty", true);
         }
       },
       !isDefault},
  };
  addOptionToMainMenu();

  loopOptions(options, isDefault ? 0 : 1);
}

/*********************************************************************
**  Function: setWifiApCredsMenu
**  Handles Menu to configure WiFi AP Credentials
**********************************************************************/
void setWifiApCredsMenu() {
  options = {
      {"SSID", setWifiApSsidMenu},
      {"Password", setWifiApPasswordMenu},
  };
  addOptionToMainMenu();

  loopOptions(options);
}

/*********************************************************************
**  Function: setNetworkCredsMenu
**  Main Menu for setting Network credentials (BLE & WiFi)
**********************************************************************/
void setNetworkCredsMenu() {
  options = {{"WiFi AP Creds", setWifiApCredsMenu}};
  addOptionToMainMenu();

  loopOptions(options);
}

/*********************************************************************
**  Function: setBadUSBBLEMenu
**  Main Menu for setting Bad USB/BLE options
**********************************************************************/
void setBadUSBBLEMenu() {
  options = {
      {"Keyboard Layout", setBadUSBBLEKeyboardLayoutMenu},
      {"Key Delay", setBadUSBBLEKeyDelayMenu},
      {"Show Output", setBadUSBBLEShowOutputMenu},
  };
  addOptionToMainMenu();

  loopOptions(options);
}

/*********************************************************************
**  Function: setBadUSBBLEKeyboardLayoutMenu
**  Main Menu for setting Bad USB/BLE Keyboard Layout
**********************************************************************/
void setBadUSBBLEKeyboardLayoutMenu() {
  uint8_t opt = willyConfig.badUSBBLEKeyboardLayout;

  options.clear();
  options = {
      {"US International", [&]() { opt = 0; }},
      {"Danish", [&]() { opt = 1; }},
      {"English (UK)", [&]() { opt = 2; }},
      {"French (AZERTY)", [&]() { opt = 3; }},
      {"German", [&]() { opt = 4; }},
      {"Hungarian", [&]() { opt = 5; }},
      {"Italian", [&]() { opt = 6; }},
      {"Polish", [&]() { opt = 7; }},
      {"Portuguese (Brazil)", [&]() { opt = 8; }},
      {"Portuguese (Portugal)", [&]() { opt = 9; }},
      {"Slovenian", [&]() { opt = 10; }},
      {"Spanish", [&]() { opt = 11; }},
      {"Swedish", [&]() { opt = 12; }},
      {"Turkish", [&]() { opt = 13; }},
  };
  addOptionToMainMenu();

  loopOptions(options, opt);

  if (opt != willyConfig.badUSBBLEKeyboardLayout) {
    willyConfig.setBadUSBBLEKeyboardLayout(opt);
  }
}

/*********************************************************************
**  Function: setBadUSBBLEKeyDelayMenu
**  Main Menu for setting Bad USB/BLE Keyboard Key Delay
**********************************************************************/
void setBadUSBBLEKeyDelayMenu() {
  String delayStr =
      num_keyboard(String(willyConfig.badUSBBLEKeyDelay), 3, "Key Delay (ms):");
  if (delayStr != "\x1B") {
    uint16_t delayVal = static_cast<uint16_t>(delayStr.toInt());
    if (delayVal <= 500) {
      willyConfig.setBadUSBBLEKeyDelay(delayVal);
    } else if (delayVal != 0) {
      displayError("Invalid key delay value (0 to 500)", true);
    }
  }
}

/*********************************************************************
**  Function: setBadUSBBLEShowOutputMenu
**  Main Menu for setting Bad USB/BLE Show Output
**********************************************************************/
void setBadUSBBLEShowOutputMenu() {
  options.clear();
  options = {
      {"Enable", [&]() { willyConfig.setBadUSBBLEShowOutput(true); }},
      {"Disable", [&]() { willyConfig.setBadUSBBLEShowOutput(false); }},
  };
  addOptionToMainMenu();

  loopOptions(options, willyConfig.badUSBBLEShowOutput ? 0 : 1);
}

/*********************************************************************
**  Function: setMacAddressMenu - @IncursioHack
**  Handles Menu to configure WiFi MAC Address
**********************************************************************/
void setMacAddressMenu() {
  String currentMAC = willyConfig.wifiMAC;
  if (currentMAC == "")
    currentMAC = WiFi.macAddress();

  options.clear();
  options = {{"Default MAC (" + WiFi.macAddress() + ")",
              [&]() { willyConfig.setWifiMAC(""); }, willyConfig.wifiMAC == ""},
             {"Set Custom MAC",
              [&]() {
                String newMAC =
                    keyboard(willyConfig.wifiMAC, 17, "XX:YY:ZZ:AA:BB:CC");
                if (newMAC == "\x1B")
                  return;
                if (newMAC.length() == 17) {
                  willyConfig.setWifiMAC(newMAC);
                } else {
                  displayError("Invalid MAC format");
                }
              },
              willyConfig.wifiMAC != ""},
             {"Random MAC", [&]() {
                uint8_t randomMac[6];
                for (int i = 0; i < 6; i++)
                  randomMac[i] = random(0x00, 0xFF);
                char buf[18];
                sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", randomMac[0],
                        randomMac[1], randomMac[2], randomMac[3], randomMac[4],
                        randomMac[5]);
                willyConfig.setWifiMAC(String(buf));
              }}};

  addOptionToMainMenu();
  loopOptions(options, MENU_TYPE_REGULAR, ("Current: " + currentMAC).c_str());
}

/*********************************************************************
**  Function: setSPIPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setSPIPinsMenu(WillyConfigPins::SPIPins &value) {
  uint8_t opt = 0;
  bool changed = false;
  WillyConfigPins::SPIPins points = value;

RELOAD:
  options = {
      {String("SCK =" + String(points.sck)).c_str(), [&]() { opt = 1; }},
      {String("MISO=" + String(points.miso)).c_str(), [&]() { opt = 2; }},
      {String("MOSI=" + String(points.mosi)).c_str(), [&]() { opt = 3; }},
      {String("CS  =" + String(points.cs)).c_str(), [&]() { opt = 4; }},
      {String("CE/GDO0=" + String(points.io0)).c_str(), [&]() { opt = 5; }},
      {String("NC/GDO2=" + String(points.io2)).c_str(), [&]() { opt = 6; }},
      {"Save Config", [&]() { opt = 7; }, changed},
      {"Main Menu", [&]() { opt = 0; }},
  };

  loopOptions(options);
  if (opt == 0)
    return;
  else if (opt == 7) {
    if (changed) {
      value = points;
      willyConfigPins.setSpiPins(value);
    }
  } else {
    options = {};
    gpio_num_t sel = GPIO_NUM_NC;
    int index = 0;
    if (opt == 1)
      index = points.sck + 1;
    else if (opt == 2)
      index = points.miso + 1;
    else if (opt == 3)
      index = points.mosi + 1;
    else if (opt == 4)
      index = points.cs + 1;
    else if (opt == 5)
      index = points.io0 + 1;
    else if (opt == 6)
      index = points.io2 + 1;
    for (int8_t i = -1; i <= GPIO_NUM_MAX; i++) {
      String tmp = String(i);
      options.push_back({tmp.c_str(), [i, &sel]() { sel = (gpio_num_t)i; }});
    }
    loopOptions(options, index);
    options.clear();
    if (opt == 1)
      points.sck = sel;
    else if (opt == 2)
      points.miso = sel;
    else if (opt == 3)
      points.mosi = sel;
    else if (opt == 4)
      points.cs = sel;
    else if (opt == 5)
      points.io0 = sel;
    else if (opt == 6)
      points.io2 = sel;
    changed = true;
    goto RELOAD;
  }
}

/*********************************************************************
**  Function: setUARTPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setUARTPinsMenu(WillyConfigPins::UARTPins &value) {
  uint8_t opt = 0;
  bool changed = false;
  WillyConfigPins::UARTPins points = value;

RELOAD:
  options = {
      {String("RX = " + String(points.rx)).c_str(), [&]() { opt = 1; }},
      {String("TX = " + String(points.tx)).c_str(), [&]() { opt = 2; }},
      {"Save Config", [&]() { opt = 7; }, changed},
      {"Main Menu", [&]() { opt = 0; }},
  };

  loopOptions(options);
  if (opt == 0)
    return;
  else if (opt == 7) {
    if (changed) {
      value = points;
      willyConfigPins.setUARTPins(value);
    }
  } else {
    options = {};
    gpio_num_t sel = GPIO_NUM_NC;
    int index = 0;
    if (opt == 1)
      index = points.rx + 1;
    else if (opt == 2)
      index = points.tx + 1;
    for (int8_t i = -1; i <= GPIO_NUM_MAX; i++) {
      String tmp = String(i);
      options.push_back({tmp.c_str(), [i, &sel]() { sel = (gpio_num_t)i; }});
    }
    loopOptions(options, index);
    options.clear();
    if (opt == 1)
      points.rx = sel;
    else if (opt == 2)
      points.tx = sel;
    changed = true;
    goto RELOAD;
  }
}

/*********************************************************************
**  Function: setI2CPins
**  Main Menu to manually set SPI Pins
**********************************************************************/
void setI2CPinsMenu(WillyConfigPins::I2CPins &value) {
  uint8_t opt = 0;
  bool changed = false;
  WillyConfigPins::I2CPins points = value;

RELOAD:
  options = {
      {String("SDA = " + String(points.sda)).c_str(), [&]() { opt = 1; }},
      {String("SCL = " + String(points.scl)).c_str(), [&]() { opt = 2; }},
      {"Save Config", [&]() { opt = 7; }, changed},
      {"Main Menu", [&]() { opt = 0; }},
  };

  loopOptions(options);
  if (opt == 0)
    return;
  else if (opt == 7) {
    if (changed) {
      value = points;
      willyConfigPins.setI2CPins(value);
    }
  } else {
    options = {};
    gpio_num_t sel = GPIO_NUM_NC;
    int index = 0;
    if (opt == 1)
      index = points.sda + 1;
    else if (opt == 2)
      index = points.scl + 1;
    for (int8_t i = -1; i <= GPIO_NUM_MAX; i++) {
      String tmp = String(i);
      options.push_back({tmp.c_str(), [i, &sel]() { sel = (gpio_num_t)i; }});
    }
    loopOptions(options, index);
    options.clear();
    if (opt == 1)
      points.sda = sel;
    else if (opt == 2)
      points.scl = sel;
    changed = true;
    goto RELOAD;
  }
}

/*********************************************************************
**  Function: setTheme
**  Menu to change Theme
**********************************************************************/
void setTheme() {
  FS *fs = &LittleFS;
  options = {{"Little FS", [&]() { fs = &LittleFS; }},
             {"Default",
              [&]() {
                willyConfig.removeTheme();
                willyConfig.themePath = "";
                willyConfig.theme.fs = 0;
                willyConfig.secColor = DEFAULT_SECCOLOR;
                willyConfig.bgColor = TFT_BLACK;
                willyConfig.setUiColor(DEFAULT_PRICOLOR);
#ifdef HAS_RGB_LED
                willyConfig.ledBright = 50;
                willyConfig.ledColor = 0x960064;
                willyConfig.ledEffect = 0;
                willyConfig.ledEffectSpeed = 5;
                willyConfig.ledEffectDirection = 1;
                ledSetup();
#endif
                willyConfig.saveFile();
                fs = nullptr;
              }},
             {"Main Menu", [&]() { fs = nullptr; }}};
  if (setupSdCard()) {
    options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});
  }
  loopOptions(options);
  if (fs == nullptr)
    return;

  String filepath = loopSD(*fs, true, "JSON");
  if (willyConfig.openThemeFile(fs, filepath, true)) {
    willyConfig.themePath = filepath;
    if (fs == &LittleFS)
      willyConfig.theme.fs = 1;
    else if (fs == &SD)
      willyConfig.theme.fs = 2;
    else
      willyConfig.theme.fs = 0;

    willyConfig.saveFile();
  }
}
#if !defined(LITE_VERSION)
BLE_API bleApi;
static bool ble_api_enabled = false;

void enableBLEAPI() {
  if (!ble_api_enabled) {
    // displayWarning("BLE API require huge amount of RAM.");
    // displayWarning("Some features may stop working.");
    Serial.println(ESP.getFreeHeap());
    bleApi.setup();
    Serial.println(ESP.getFreeHeap());
  } else {
    bleApi.end();
  }

  ble_api_enabled = !ble_api_enabled;
}

bool appStoreInstalled() {
  FS *fs;
  if (!getFsStorage(fs)) {
    log_i("Fail getting filesystem");
    return false;
  }

  return fs->exists("/WillyJS/Tools/App Store.js");
}

#include <HTTPClient.h>
void installAppStoreJS() {

  if (WiFi.status() != WL_CONNECTED) {
    wifiConnectMenu(WIFI_STA);
  }
  if (WiFi.status() != WL_CONNECTED) {
    displayWarning("WiFi not connected", true);
    return;
  }

  FS *fs;
  if (!getFsStorage(fs)) {
    log_i("Fail getting filesystem");
    return;
  }

  if (!fs->exists("/WillyJS")) {
    if (!fs->mkdir("/WillyJS")) {
      displayWarning("Failed to create /WillyJS directory", true);
      return;
    }
  }

  if (!fs->exists("/WillyJS/Tools")) {
    if (!fs->mkdir("/WillyJS/Tools")) {
      displayWarning("Failed to create /WillyJS/Tools directory", true);
      return;
    }
  }

  HTTPClient http;
  http.begin("https://raw.githubusercontent.com/WillyDevices/App-Store/refs/"
             "heads/main/minified/App%20Store.js");
  int httpCode = http.GET();
  if (httpCode != 200) {
    http.end();
    displayWarning("Failed to download App Store", true);
    return;
  }

  File file = fs->open("/WillyJS/Tools/App Store.js", FILE_WRITE);
  if (!file) {
    displayWarning("Failed to save App Store", true);
    return;
  }
  file.print(http.getString());
  http.end();
  file.close();

  displaySuccess("App Store installed", true);
  displaySuccess("Goto JS Interpreter -> Tools -> App Store", true);
}
#endif
