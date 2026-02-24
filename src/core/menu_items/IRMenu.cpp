#include "IRMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/mykeyboard.h"
#include "modules/ir/TV-B-Gone.h"
#include "modules/ir/custom_ir.h"
#include "modules/ir/ir_jammer.h"
#include "modules/ir/ir_read.h"
#include "modules/ir/advanced_ir_atks.h"
#include "modules/ir/ir_config.h"

void IRMenu::optionsMenu() {
#if defined(ARDUINO_M5STICK_S3)
    bool prevPower = M5.Power.getExtOutput();
    M5.Power.setExtOutput(true); // ENABLE 5V OUTPUT
#endif
    options = {
        {"TV-B-Gone", StartTvBGone              },
        {"IR Custom", otherIRcodes              },
        {"Ler IR",   [=]() { IrRead(); }       },
#if !defined(LITE_VERSION)
        {"Jammer IR", startIrJammer             }, // Simple frequency-adjustable jammer
        {"Ataques Avancados", advancedIRAtksMenu},
#endif
        {"Config",    [this]() { configMenu(); }},
    };
    addOptionToMainMenu();

    String txt = "Infravermelho";
    txt += " Tx: " + String(bruceConfigPins.irTx) + " Rx: " + String(bruceConfigPins.irRx) +
           " Rpts: " + String(bruceConfigPins.irTxRepeats);
    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
#if defined(ARDUINO_M5STICK_S3)
    M5.Power.setExtOutput(prevPower);
#endif
}

void IRMenu::configMenu() {
    options = {
        {"Modo de Operacao", [this]() { modeMenu(); }                     },
        {"Pino IR TX",       lambdaHelper(gsetIrTxPin, true)              },
        {"Pino IR RX",       lambdaHelper(gsetIrRxPin, true)              },
        {"Repeticoes IR TX", setIrTxRepeats                               },
        {"Voltar",           [this]() { optionsMenu(); }                  },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config IR");
}

void IRMenu::modeMenu() {
    int idx = (irConfig.mode == IR_MODE_STANDARD) ? 0 : 1;

    options = {
        {"Padrao (38kHz)",  [=, this]() {
            irConfig.mode = IR_MODE_STANDARD;
            irConfig.frequency = IR_FREQ_38KHZ;
            irConfig.dutyCycle = IR_DUTY_33_PERCENT;
            irConfig.preambleMode = IR_PREAMBLE_STANDARD;
            irConfig.noiseFilter = IR_FILTER_OFF;
            irConfig.powerMode = IR_POWER_NORMAL;
            displaySuccess("Modo Padrao selecionado");
        }, irConfig.mode == IR_MODE_STANDARD},
        {"Avancado",        [=, this]() { advancedConfigMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Modo IR", idx);
}

void IRMenu::advancedConfigMenu() {
    options = {
        {"Frequencia",        [this]() { frequencyMenu(); }               },
        {"Duty Cycle",        [this]() { dutyCycleMenu(); }               },
        {"Preamble",          [this]() { preambleMenu(); }                },
        {"Filtro Ruido",      [this]() { noiseFilterMenu(); }             },
        {"Modo Potencia",     [this]() { powerModeMenu(); }               },
        {"Voltar",            [this]() { modeMenu(); }                    },
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Config Avancada");
}

void IRMenu::frequencyMenu() {
    int idx = 0;
    if (irConfig.frequency == IR_FREQ_36KHZ) idx = 0;
    else if (irConfig.frequency == IR_FREQ_38KHZ) idx = 1;
    else if (irConfig.frequency == IR_FREQ_40KHZ) idx = 2;
    else if (irConfig.frequency == IR_FREQ_CUSTOM) idx = 3;

    options = {
        {"36 kHz",            [=]() { irConfig.frequency = IR_FREQ_36KHZ; }, irConfig.frequency == IR_FREQ_36KHZ},
        {"38 kHz (Padrao)",   [=]() { irConfig.frequency = IR_FREQ_38KHZ; }, irConfig.frequency == IR_FREQ_38KHZ},
        {"40 kHz",            [=]() { irConfig.frequency = IR_FREQ_40KHZ; }, irConfig.frequency == IR_FREQ_40KHZ},
        {"Customizado",       [=]() {
            String freqStr = num_keyboard(String(irConfig.customFrequencyHz), 6, "Frequencia (Hz):");
            if (freqStr != "\x1B" && freqStr.length() > 0) {
                irConfig.customFrequencyHz = freqStr.toInt();
                irConfig.frequency = IR_FREQ_CUSTOM;
            }
        }, irConfig.frequency == IR_FREQ_CUSTOM},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Frequencia Modulacao", idx);
}

void IRMenu::dutyCycleMenu() {
    int idx = 0;
    if (irConfig.dutyCycle == IR_DUTY_33_PERCENT) idx = 0;
    else if (irConfig.dutyCycle == IR_DUTY_50_PERCENT) idx = 1;
    else if (irConfig.dutyCycle == IR_DUTY_CUSTOM) idx = 2;

    options = {
        {"33% (Padrao)",      [=]() { irConfig.dutyCycle = IR_DUTY_33_PERCENT; }, irConfig.dutyCycle == IR_DUTY_33_PERCENT},
        {"50% (Max Pot)",     [=]() { irConfig.dutyCycle = IR_DUTY_50_PERCENT; }, irConfig.dutyCycle == IR_DUTY_50_PERCENT},
        {"Customizado",       [=]() {
            String dutyStr = num_keyboard(String(irConfig.customDutyCycle), 3, "Duty Cycle (%):");
            if (dutyStr != "\x1B" && dutyStr.length() > 0) {
                int duty = dutyStr.toInt();
                if (duty >= 10 && duty <= 90) {
                    irConfig.customDutyCycle = duty;
                    irConfig.dutyCycle = IR_DUTY_CUSTOM;
                } else {
                    displayError("Valor deve ser 10-90%", true);
                }
            }
        }, irConfig.dutyCycle == IR_DUTY_CUSTOM},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Duty Cycle", idx);
}

void IRMenu::preambleMenu() {
    int idx = (int)irConfig.preambleMode;

    options = {
        {"Padrao",            [=]() { irConfig.preambleMode = IR_PREAMBLE_STANDARD; }, irConfig.preambleMode == IR_PREAMBLE_STANDARD},
        {"Longo (+dist)",     [=]() { irConfig.preambleMode = IR_PREAMBLE_LONG; }, irConfig.preambleMode == IR_PREAMBLE_LONG},
        {"Extra Longo",       [=]() { irConfig.preambleMode = IR_PREAMBLE_EXTRA_LONG; }, irConfig.preambleMode == IR_PREAMBLE_EXTRA_LONG},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Preamble", idx);
}

void IRMenu::noiseFilterMenu() {
    int idx = (int)irConfig.noiseFilter;

    options = {
        {"Desativado",        [=]() { irConfig.noiseFilter = IR_FILTER_OFF; }, irConfig.noiseFilter == IR_FILTER_OFF},
        {"Leve",              [=]() { irConfig.noiseFilter = IR_FILTER_LIGHT; }, irConfig.noiseFilter == IR_FILTER_LIGHT},
        {"Medio",             [=]() { irConfig.noiseFilter = IR_FILTER_MEDIUM; }, irConfig.noiseFilter == IR_FILTER_MEDIUM},
        {"Agressivo",         [=]() { irConfig.noiseFilter = IR_FILTER_AGGRESSIVE; }, irConfig.noiseFilter == IR_FILTER_AGGRESSIVE},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Filtro Ruido", idx);
}

void IRMenu::powerModeMenu() {
    int idx = (int)irConfig.powerMode;

    options = {
        {"Normal",            [=]() { irConfig.powerMode = IR_POWER_NORMAL; }, irConfig.powerMode == IR_POWER_NORMAL},
        {"Alto (+alcance)",   [=]() { irConfig.powerMode = IR_POWER_HIGH; }, irConfig.powerMode == IR_POWER_HIGH},
        {"Boost (Max)",       [=]() {
            displayWarning("Cuidado: pode superaquecer LED", true);
            irConfig.powerMode = IR_POWER_BOOST;
        }, irConfig.powerMode == IR_POWER_BOOST},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "Modo Potencia", idx);
}

void IRMenu::drawIcon(float scale) {
    clearIconArea();
    int iconSize = scale * 60;
    int radius = scale * 7;
    int deltaRadius = scale * 10;

    if (iconSize % 2 != 0) iconSize++;

    tft.fillRect(
        iconCenterX - iconSize / 2, iconCenterY - iconSize / 2, iconSize / 6, iconSize, bruceConfig.priColor
    );
    tft.fillRect(
        iconCenterX - iconSize / 3,
        iconCenterY - iconSize / 3,
        iconSize / 6,
        2 * iconSize / 3,
        bruceConfig.priColor
    );

    tft.drawCircle(iconCenterX - iconSize / 6, iconCenterY, radius, bruceConfig.priColor);

    tft.drawArc(
        iconCenterX - iconSize / 6,
        iconCenterY,
        2.5 * radius,
        2 * radius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 6,
        iconCenterY,
        2.5 * radius + deltaRadius,
        2 * radius + deltaRadius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 6,
        iconCenterY,
        2.5 * radius + 2 * deltaRadius,
        2 * radius + 2 * deltaRadius,
        220,
        320,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
