/**
 * @file ir_config.h
 * @author Willy Firmware
 * @brief IR Advanced Configuration for IR Transmitter/Receiver
 * @version 1.0
 * 
 * Otimizações para o Módulo IR:
 * - Frequência de Modulação: Ajustável de 36-40kHz (padrão 38kHz)
 * - Preamble Mais Longo: Melhora AGC lock para maior distância
 * - Duty Cycle Otimizado: 50% para potência sem superaquecer
 * - Filtro de Ruído: Ignora códigos curtos ou repetidos
 * - Modo Pulsado Otimizado: Transmissão mais estável
 */

#ifndef __IR_CONFIG_H__
#define __IR_CONFIG_H__

#include <Arduino.h>

// IR Operation Modes
enum IRMode {
    IR_MODE_STANDARD = 0,       // Padrão: 38kHz, configurações padrão
    IR_MODE_ADVANCED = 1        // Avançado: Configurações otimizadas
};

// IR Modulation Frequency options (kHz)
enum IRModulationFrequency {
    IR_FREQ_36KHZ = 36,         // 36kHz - Alguns dispositivos antigos
    IR_FREQ_38KHZ = 38,         // 38kHz - Padrão mais comum
    IR_FREQ_40KHZ = 40,         // 40kHz - Sony e outros
    IR_FREQ_CUSTOM = 0          // Customizado
};

// IR Duty Cycle options (percent)
enum IRDutyCycle {
    IR_DUTY_33_PERCENT = 33,    // 33% - Padrão (menor consumo)
    IR_DUTY_50_PERCENT = 50,    // 50% - Máxima potência (mais calor)
    IR_DUTY_CUSTOM = 0          // Customizado
};

// IR Preamble Mode
enum IRPreambleMode {
    IR_PREAMBLE_STANDARD = 0,   // Padrão - Funciona na maioria
    IR_PREAMBLE_LONG = 1,       // Longo - Melhor para distância
    IR_PREAMBLE_EXTRA_LONG = 2  // Extra longo - Máxima distância
};

// IR Noise Filter Mode
enum IRNoiseFilterMode {
    IR_FILTER_OFF = 0,          // Desativado
    IR_FILTER_LIGHT = 1,        // Leve - Filtra apenas ruído óbvio
    IR_FILTER_MEDIUM = 2,       // Médio - Balanceado
    IR_FILTER_AGGRESSIVE = 3    // Agressivo - Máxima filtragem
};

// IR Transmission Power Mode
enum IRPowerMode {
    IR_POWER_NORMAL = 0,        // Normal - Uso padrão
    IR_POWER_HIGH = 1,          // Alto - Maior distância
    IR_POWER_BOOST = 2          // Boost - Máxima distância (cuidado com LED)
};

/**
 * @brief IR Advanced Configuration Class
 * 
 * Gerencia todas as configurações avançadas do módulo IR
 */
class IRConfig {
public:
    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    IRConfig();
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Configuration Properties
    /////////////////////////////////////////////////////////////////////////////////////
    
    // Operation Mode
    IRMode mode = IR_MODE_STANDARD;
    
    // Modulation Frequency (kHz)
    IRModulationFrequency frequency = IR_FREQ_38KHZ;
    
    // Custom frequency in Hz (if frequency == IR_FREQ_CUSTOM)
    uint16_t customFrequencyHz = 38000;
    
    // Duty Cycle (percent)
    IRDutyCycle dutyCycle = IR_DUTY_33_PERCENT;
    
    // Custom duty cycle (if dutyCycle == IR_DUTY_CUSTOM)
    uint8_t customDutyCycle = 33;
    
    // Preamble Mode
    IRPreambleMode preambleMode = IR_PREAMBLE_STANDARD;
    
    // Noise Filter Mode
    IRNoiseFilterMode noiseFilter = IR_FILTER_OFF;
    
    // Minimum valid pulse width in microseconds (for noise filtering)
    uint16_t minPulseWidth = 100;
    
    // Power Mode
    IRPowerMode powerMode = IR_POWER_NORMAL;
    
    // Repeat count for commands (additional repeats)
    uint8_t repeatCount = 0;
    
    // Inter-command delay in milliseconds
    uint16_t interCommandDelay = 20;
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Configuration Methods
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Get current modulation frequency in Hz
     * @return Frequency in Hz
     */
    uint16_t getFrequencyHz() const;
    
    /**
     * @brief Get current duty cycle as float (0.0-1.0)
     * @return Duty cycle as float
     */
    float getDutyCycleFloat() const;
    
    /**
     * @brief Get preamble multiplier (1x, 2x, 3x)
     * @return Preamble multiplier
     */
    uint8_t getPreambleMultiplier() const;
    
    /**
     * @brief Get minimum signal quality threshold (0-100)
     * @return Quality threshold
     */
    uint8_t getMinSignalQuality() const;
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Helper Methods
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Get frequency description string
     * @param freq Frequency enum
     * @return Human readable string
     */
    static String getFrequencyString(IRModulationFrequency freq);
    
    /**
     * @brief Get duty cycle description string
     * @param duty Duty cycle enum
     * @return Human readable string
     */
    static String getDutyCycleString(IRDutyCycle duty);
    
    /**
     * @brief Get preamble mode description string
     * @param mode Preamble mode enum
     * @return Human readable string
     */
    static String getPreambleModeString(IRPreambleMode mode);
    
    /**
     * @brief Get noise filter description string
     * @param filter Noise filter enum
     * @return Human readable string
     */
    static String getNoiseFilterString(IRNoiseFilterMode filter);
    
    /**
     * @brief Get power mode description string
     * @param mode Power mode enum
     * @return Human readable string
     */
    static String getPowerModeString(IRPowerMode mode);
};

// Global IR Config Instance
extern IRConfig irConfig;

#endif // __IR_CONFIG_H__
