#include "GpsMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "modules/gps/gps_tracker.h"
#include "modules/gps/wardriving.h"
#include "modules/gps/gps_config.h"
#include <math.h>

void GpsMenu::optionsMenu() {
    options = {
        {"Wardriving",  [this]() { wardrivingMenu(); }},
#if !defined(LITE_VERSION)
        {"Rastreador GPS", [=]() { GPSTracker(); }       },
#endif
        {"Config",      [this]() { configMenu(); }    },
    };
    addOptionToMainMenu();

    String txt = "GPS (" + String(bruceConfigPins.gpsBaudrate) + " bps)";
    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
}

void GpsMenu::wardrivingMenu() {
    options = {
        {"Escanear Redes WiFi", []() {
            Wardriving(true, false);
        }},
        {"Escanear Dispositivos Bluetooth",   []() {
            Wardriving(false, true);
        }},
        {"Escanear Ambos",          []() {
            Wardriving(true, true);
        }},

        {"Voltar",               [this]() { optionsMenu(); }      },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Wardriving");
}
void GpsMenu::configMenu() {
    options = {
        {"Modo de Operacao", [this]() { modeMenu(); }                      },
        {"Baudrate",         setGpsBaudrateMenu                           },
        {"Pinos GPS",        [this]() { setUARTPinsMenu(bruceConfigPins.gps_bus); }},
        {"Voltar",           [this]() { optionsMenu(); }                    },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config GPS");
}

void GpsMenu::modeMenu() {
    int idx = (gpsConfig.mode == GPS_MODE_STANDARD) ? 0 : 1;

    options = {
        {"Padrao (1Hz, NMEA)",  [this]() {
            gpsConfig.mode = GPS_MODE_STANDARD;
            gpsConfig.updateRate = GPS_RATE_1HZ;
            gpsConfig.powerMode = GPS_POWER_MAX_PERFORMANCE;
            gpsConfig.protocolMode = GPS_PROTOCOL_NMEA;
            displaySuccess("Modo Padrao selecionado");
        }, gpsConfig.mode == GPS_MODE_STANDARD},
        {"Avancado",            [this]() { advancedConfigMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Modo GPS", idx);
}

void GpsMenu::advancedConfigMenu() {
    options = {
        {"Taxa de Update",     [this]() { updateRateMenu(); }              },
        {"Modo de Energia",    [this]() { powerModeMenu(); }               },
        {"Protocolo",          [this]() { protocolMenu(); }                },
        {"Navegacao",          [this]() { navigationMenu(); }              },
        {"Filtro Satelite",    [this]() { satelliteFilterMenu(); }         },
        {"Aplicar Config",     [this]() { applyAdvancedConfig(); }         },
        {"Voltar",             [this]() { modeMenu(); }                    },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config Avancada");
}

void GpsMenu::updateRateMenu() {
    int idx = 0;
    if (gpsConfig.updateRate == GPS_RATE_1HZ) idx = 0;
    else if (gpsConfig.updateRate == GPS_RATE_2HZ) idx = 1;
    else if (gpsConfig.updateRate == GPS_RATE_5HZ) idx = 2;
    else if (gpsConfig.updateRate == GPS_RATE_10HZ) idx = 3;

    options = {
        {"1 Hz (Padrao)",      [=]() { gpsConfig.updateRate = GPS_RATE_1HZ; }, gpsConfig.updateRate == GPS_RATE_1HZ},
        {"2 Hz",               [=]() { gpsConfig.updateRate = GPS_RATE_2HZ; }, gpsConfig.updateRate == GPS_RATE_2HZ},
        {"5 Hz (Wardriving)",  [=]() { gpsConfig.updateRate = GPS_RATE_5HZ; }, gpsConfig.updateRate == GPS_RATE_5HZ},
        {"10 Hz (Instavel)",   [=]() { gpsConfig.updateRate = GPS_RATE_10HZ; }, gpsConfig.updateRate == GPS_RATE_10HZ},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Taxa de Update", idx);
}

void GpsMenu::powerModeMenu() {
    int idx = (int)gpsConfig.powerMode;

    options = {
        {"Max Desempenho",     [=]() { gpsConfig.powerMode = GPS_POWER_MAX_PERFORMANCE; }, gpsConfig.powerMode == GPS_POWER_MAX_PERFORMANCE},
        {"Eco (Economia)",     [=]() { gpsConfig.powerMode = GPS_POWER_ECO; }, gpsConfig.powerMode == GPS_POWER_ECO},
        {"Power Save (Min)",   [=]() { gpsConfig.powerMode = GPS_POWER_SAVE; }, gpsConfig.powerMode == GPS_POWER_SAVE},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Modo Energia", idx);
}

void GpsMenu::protocolMenu() {
    int idx = (int)gpsConfig.protocolMode;

    options = {
        {"NMEA (Texto)",       [=]() { gpsConfig.protocolMode = GPS_PROTOCOL_NMEA; }, gpsConfig.protocolMode == GPS_PROTOCOL_NMEA},
        {"UBX (Binario)",      [=]() { gpsConfig.protocolMode = GPS_PROTOCOL_UBX; }, gpsConfig.protocolMode == GPS_PROTOCOL_UBX},
        {"NMEA + UBX",         [=]() { gpsConfig.protocolMode = GPS_PROTOCOL_BOTH; }, gpsConfig.protocolMode == GPS_PROTOCOL_BOTH},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Protocolo", idx);
}

void GpsMenu::navigationMenu() {
    int idx = 0;
    switch (gpsConfig.navMode) {
        case GPS_NAV_PORTABLE: idx = 0; break;
        case GPS_NAV_STATIONARY: idx = 1; break;
        case GPS_NAV_PEDESTRIAN: idx = 2; break;
        case GPS_NAV_AUTOMOTIVE: idx = 3; break;
        case GPS_NAV_SEA: idx = 4; break;
        case GPS_NAV_AIRBORNE: idx = 5; break;
        default: idx = 0;
    }

    options = {
        {"Portatil",           [=]() { gpsConfig.navMode = GPS_NAV_PORTABLE; gpsConfig.dynamicModel = GPS_DYN_PORTABLE; }, gpsConfig.navMode == GPS_NAV_PORTABLE},
        {"Estacionario",       [=]() { gpsConfig.navMode = GPS_NAV_STATIONARY; gpsConfig.dynamicModel = GPS_DYN_STATIONARY; }, gpsConfig.navMode == GPS_NAV_STATIONARY},
        {"Pedestre",           [=]() { gpsConfig.navMode = GPS_NAV_PEDESTRIAN; gpsConfig.dynamicModel = GPS_DYN_PEDESTRIAN; }, gpsConfig.navMode == GPS_NAV_PEDESTRIAN},
        {"Automotivo",         [=]() { gpsConfig.navMode = GPS_NAV_AUTOMOTIVE; gpsConfig.dynamicModel = GPS_DYN_AUTOMOTIVE; }, gpsConfig.navMode == GPS_NAV_AUTOMOTIVE},
        {"Maritimo",           [=]() { gpsConfig.navMode = GPS_NAV_SEA; gpsConfig.dynamicModel = GPS_DYN_SEA; }, gpsConfig.navMode == GPS_NAV_SEA},
        {"Aereo",              [=]() { gpsConfig.navMode = GPS_NAV_AIRBORNE; gpsConfig.dynamicModel = GPS_DYN_AIRBORNE_1G; }, gpsConfig.navMode == GPS_NAV_AIRBORNE},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Navegacao", idx);
}

void GpsMenu::satelliteFilterMenu() {
    int idx = 0;
    if (gpsConfig.minSatelliteSignal == 0) idx = 0;
    else if (gpsConfig.minSatelliteSignal == 25) idx = 1;
    else if (gpsConfig.minSatelliteSignal == 30) idx = 2;
    else if (gpsConfig.minSatelliteSignal == 35) idx = 3;
    else if (gpsConfig.minSatelliteSignal == 40) idx = 4;

    options = {
        {"Desativado",         [=]() { gpsConfig.minSatelliteSignal = 0; }, gpsConfig.minSatelliteSignal == 0},
        {"25 dB-Hz",           [=]() { gpsConfig.minSatelliteSignal = 25; }, gpsConfig.minSatelliteSignal == 25},
        {"30 dB-Hz",           [=]() { gpsConfig.minSatelliteSignal = 30; }, gpsConfig.minSatelliteSignal == 30},
        {"35 dB-Hz (Recom.)",  [=]() { gpsConfig.minSatelliteSignal = 35; }, gpsConfig.minSatelliteSignal == 35},
        {"40 dB-Hz",           [=]() { gpsConfig.minSatelliteSignal = 40; }, gpsConfig.minSatelliteSignal == 40},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Filtro Satelite", idx);
}

void GpsMenu::applyAdvancedConfig() {
    drawMainBorderWithTitle("Aplicando Config");
    padprintln("");
    padprintln("Configurando GPS...");
    padprintln("");
    padprintln("Taxa: " + GPSConfig::getUpdateRateString(gpsConfig.updateRate));
    padprintln("Energia: " + GPSConfig::getPowerModeString(gpsConfig.powerMode));
    padprintln("Nav: " + GPSConfig::getNavigationModeString(gpsConfig.navMode));
    padprintln("");

    // Initialize serial for configuration
    HardwareSerial gpsSerial(2);
    gpsSerial.begin(bruceConfigPins.gpsBaudrate, SERIAL_8N1, bruceConfigPins.gps_bus.rx, bruceConfigPins.gps_bus.tx);

    delay(500);

    // Set mode to advanced
    gpsConfig.mode = GPS_MODE_ADVANCED;

    // Apply configuration
    bool success = gpsConfig.applyConfiguration(gpsSerial);

    gpsSerial.end();

    if (success) {
        displaySuccess("Configuracao aplicada!");
    } else {
        displayWarning("Algumas config falharam", true);
    }

    delay(1500);
    advancedConfigMenu();
}

void GpsMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 18;
    if (radius % 2 != 0) radius++;

    int tangentX = sqrt(radius * radius - (radius / 2 * radius / 2));
    int32_t tangentY = radius / 2;

    tft.fillCircle(iconCenterX, iconCenterY - radius / 2, radius, bruceConfig.priColor);
    tft.fillTriangle(
        iconCenterX - tangentX,
        iconCenterY - radius / 2 + tangentY,
        iconCenterX + tangentX,
        iconCenterY - radius / 2 + tangentY,
        iconCenterX,
        iconCenterY + 1.5 * radius,
        bruceConfig.priColor
    );
    tft.fillCircle(iconCenterX, iconCenterY - radius / 2, radius / 2, bruceConfig.bgColor);

    tft.drawEllipse(iconCenterX, iconCenterY + 1.5 * radius, 1.5 * radius, radius / 2, bruceConfig.priColor);
}
