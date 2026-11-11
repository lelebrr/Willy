/**
 * @file ir_config.cpp
 * @author Willy Firmware
 * @brief IR Advanced Configuration Implementation
 * @version 1.0
 */

#include "ir_config.h"

// Global instance
IRConfig irConfig;

IRConfig::IRConfig() {
    // Default to standard mode
    mode = IR_MODE_STANDARD;
}

/////////////////////////////////////////////////////////////////////////////////////
// Configuration Methods
/////////////////////////////////////////////////////////////////////////////////////

uint16_t IRConfig::getFrequencyHz() const {
    if (frequency == IR_FREQ_CUSTOM) {
        return customFrequencyHz;
    }
    return (uint16_t)frequency * 1000;
}

float IRConfig::getDutyCycleFloat() const {
    if (dutyCycle == IR_DUTY_CUSTOM) {
        return customDutyCycle / 100.0f;
    }
    return (float)dutyCycle / 100.0f;
}

uint8_t IRConfig::getPreambleMultiplier() const {
    switch (preambleMode) {
        case IR_PREAMBLE_STANDARD:    return 1;
        case IR_PREAMBLE_LONG:        return 2;
        case IR_PREAMBLE_EXTRA_LONG:  return 3;
        default:                      return 1;
    }
}

uint8_t IRConfig::getMinSignalQuality() const {
    switch (noiseFilter) {
        case IR_FILTER_OFF:        return 0;
        case IR_FILTER_LIGHT:      return 25;
        case IR_FILTER_MEDIUM:     return 50;
        case IR_FILTER_AGGRESSIVE: return 75;
        default:                   return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Helper Methods
/////////////////////////////////////////////////////////////////////////////////////

String IRConfig::getFrequencyString(IRModulationFrequency freq) {
    switch (freq) {
        case IR_FREQ_36KHZ:  return "36 kHz";
        case IR_FREQ_38KHZ:  return "38 kHz (Padrao)";
        case IR_FREQ_40KHZ:  return "40 kHz";
        case IR_FREQ_CUSTOM: return "Customizado";
        default:             return "Desconhecido";
    }
}

String IRConfig::getDutyCycleString(IRDutyCycle duty) {
    switch (duty) {
        case IR_DUTY_33_PERCENT: return "33% (Padrao)";
        case IR_DUTY_50_PERCENT: return "50% (Max Potencia)";
        case IR_DUTY_CUSTOM:     return "Customizado";
        default:                 return "Desconhecido";
    }
}

String IRConfig::getPreambleModeString(IRPreambleMode mode) {
    switch (mode) {
        case IR_PREAMBLE_STANDARD:    return "Padrao";
        case IR_PREAMBLE_LONG:        return "Longo (+distancia)";
        case IR_PREAMBLE_EXTRA_LONG:  return "Extra Longo";
        default:                      return "Desconhecido";
    }
}

String IRConfig::getNoiseFilterString(IRNoiseFilterMode filter) {
    switch (filter) {
        case IR_FILTER_OFF:        return "Desativado";
        case IR_FILTER_LIGHT:      return "Leve";
        case IR_FILTER_MEDIUM:     return "Medio";
        case IR_FILTER_AGGRESSIVE: return "Agressivo";
        default:                   return "Desconhecido";
    }
}

String IRConfig::getPowerModeString(IRPowerMode mode) {
    switch (mode) {
        case IR_POWER_NORMAL: return "Normal";
        case IR_POWER_HIGH:   return "Alto (+alcance)";
        case IR_POWER_BOOST:  return "Boost (Max)";
        default:              return "Desconhecido";
    }
}
