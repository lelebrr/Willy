#include "RFMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "modules/rf/record.h"
#include "modules/rf/rf_bruteforce.h"
#include "modules/rf/rf_jammer.h"
#include "modules/rf/rf_listen.h"
#include "modules/rf/rf_scan.h"
#include "modules/rf/rf_send.h"
#include "modules/rf/rf_spectrum.h"
#include "modules/rf/rf_waterfall.h"
#include "modules/rf/rf_advanced_atks.h"

void RFMenu::optionsMenu() {
    options = {
        {"Escanear/Copiar",       [=]() { RFScan(); }       },
#if !defined(LITE_VERSION)
        {"Gravar RAW",      rf_raw_record             }, // Pablo-Ortiz-Lopez
        {"SubGHz Personalizado",   sendCustomRF              },
#endif
        {"Espectro",        rf_spectrum               },
#if !defined(LITE_VERSION)
        {"Espectro RSSI",   rf_CC1101_rssi            }, // @Pirata
        {"Espectro OndaQ", rf_SquareWave             }, // @Pirata
        {"Espectrograma",      rf_waterfall              }, // dev_eclipse
#if (defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR) || defined(CYD)) && defined(RF_LISTEN_H)
        {"Ouvir",          rf_listen                 }, // dev_eclipse
#endif
        {"Força Bruta",      rf_bruteforce             },
        {"Garage Brute P2262", [=]() { PredictiveGarageBrute(); } },
        {"Stealth L.P. Beacon", [=]() { StealthLowPowerExfilBeacon(); } },
        {"Coord. Replay Seq", [=]() { MultiDeviceCoordinatedReplay(); } },
        {"Smart Fuzzer",      [=]() { ProtocolFuzzerSmart(); }          },
        {"Alarm Trigger",     [=]() { FalseAlarmTriggerLoop(); }        },
        {"TPMS Spoof",        [=]() { TPMSSpoofChaos(); }               },
        {"Rolling Learner",   [=]() { RollingCodeLearnerReplay(); }     },
        {"Spectrum Lock",     [=]() { SpectrumTargetLock(); }           },
        {"Sensor Drain Flood", [=]() { SensorBatteryDrainFlood(); } },
        {"Jammer Intermitente",
         [=]() {
             if (displayMessage("AVISO: Jamming pode ser\nilegal. Continuar?", "Não", nullptr, "Sim", TFT_RED) ==
                 2) {
                 RFJammer(0);
             }
         }},

#endif
        {"Jammer Contínuo",
         [=]() {
             if (displayMessage("AVISO: Jamming pode ser\nilegal. Continuar?", "Não", nullptr, "Sim", TFT_RED) ==
                 2) {
                 RFJammer(1);
             }
         }},
        {"Jammer Hopper",
         [=]() {
             if (displayMessage("AVISO: Jamming pode ser\nilegal. Continuar?", "Não", nullptr, "Sim", TFT_RED) ==
                 2) {
                 RFJammer(2);
             }
         }},

        {"Config",          [this]() { configMenu(); }},
    };
    addOptionToMainMenu();

    delay(200);
    String txt = "Radio Frequencia";
    if (bruceConfigPins.rfModule == CC1101_SPI_MODULE) txt += " (CC1101)"; // Indicates if CC1101 is connected
    else txt += " Tx: " + String(bruceConfigPins.rfTx) + " Rx: " + String(bruceConfigPins.rfRx);

    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
}

void RFMenu::configMenu() {
    options = {
        {"Pino RF TX", lambdaHelper(gsetRfTxPin, true)},
        {"Pino RF RX", lambdaHelper(gsetRfRxPin, true)},
        {"Modulo RF", setRFModuleMenu},
        {"Frequencia RF", setRFFreqMenu},
        {"Voltar", [this]() { optionsMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config RF");
}

void RFMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 7;
    int deltaRadius = scale * 10;
    int triangleSize = scale * 30;

    if (triangleSize % 2 != 0) triangleSize++;

    // Body
    tft.fillCircle(iconCenterX, iconCenterY - radius, radius, bruceConfig.priColor);
    tft.fillTriangle(
        iconCenterX,
        iconCenterY,
        iconCenterX - triangleSize / 2,
        iconCenterY + triangleSize,
        iconCenterX + triangleSize / 2,
        iconCenterY + triangleSize,
        bruceConfig.priColor
    );

    // Left Arcs
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius,
        2 * radius,
        40,
        140,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius + deltaRadius,
        2 * radius + deltaRadius,
        40,
        140,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius + 2 * deltaRadius,
        2 * radius + 2 * deltaRadius,
        40,
        140,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );

    // Right Arcs
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius,
        2 * radius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius + deltaRadius,
        2 * radius + deltaRadius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY - radius,
        2.5 * radius + 2 * deltaRadius,
        2 * radius + 2 * deltaRadius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
