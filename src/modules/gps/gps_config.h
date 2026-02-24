/**
 * @file gps_config.h
 * @author Willy Firmware
 * @brief GPS Advanced Configuration for NEO-6M GY-GPS6MV2
 * @version 1.0
 *
 * Otimizações para o Módulo GPS (NEO-6M GY-GPS6MV2):
 * - Taxa de update: 1Hz (Padrão) a 10Hz (Avançado)
 * - Power Save Mode (PSM): Reduz consumo de 45mA para ~10-200uA
 * - Protocolo UBX (binário): Mais eficiente que NMEA
 * - Modo Estacionário e Filtro de Satélites
 * - Eco Mode vs Max Performance
 */

#ifndef __GPS_CONFIG_H__
#define __GPS_CONFIG_H__

#include <Arduino.h>
#include <globals.h>
#include <HardwareSerial.h>

// GPS Operation Modes
enum GPSMode {
    GPS_MODE_STANDARD = 0,      // Padrão: 1Hz, NMEA, sem otimizações
    GPS_MODE_ADVANCED = 1       // Avançado: Configurações otimizadas
};

// GPS Update Rate options (Hz)
enum GPSUpdateRate {
    GPS_RATE_1HZ = 1,           // Padrão: 1Hz (1000ms)
    GPS_RATE_2HZ = 2,           // 2Hz (500ms)
    GPS_RATE_5HZ = 5,           // 5Hz (200ms) - Recomendado para wardriving
    GPS_RATE_10HZ = 10          // 10Hz (100ms) - Pode instabilizar módulos baratos
};

// GPS Power Mode
enum GPSPowerMode {
    GPS_POWER_MAX_PERFORMANCE = 0,  // Máximo desempenho, maior consumo
    GPS_POWER_ECO = 1,               // Economia de energia
    GPS_POWER_SAVE = 2               // Power Save Mode (PSM) - Mínimo consumo
};

// GPS Protocol Mode
enum GPSProtocolMode {
    GPS_PROTOCOL_NMEA = 0,      // NMEA (texto) - Padrão
    GPS_PROTOCOL_UBX = 1,       // UBX (binário) - Mais eficiente
    GPS_PROTOCOL_BOTH = 2       // Ambos NMEA e UBX
};

// GPS Navigation Mode
enum GPSNavigationMode {
    GPS_NAV_PORTABLE = 0,       // Portátil (padrão)
    GPS_NAV_STATIONARY = 1,     // Estacionário - Melhor precisão
    GPS_NAV_PEDESTRIAN = 2,     // Pedestre
    GPS_NAV_AUTOMOTIVE = 3,     // Automotivo
    GPS_NAV_SEA = 4,            // Marítimo
    GPS_NAV_AIRBORNE = 5        // Aéreo
};

// GPS Dynamic Model for UBX-CFG-NAV5
enum GPSDynamicModel {
    GPS_DYN_PORTABLE = 0,
    GPS_DYN_STATIONARY = 2,
    GPS_DYN_PEDESTRIAN = 3,
    GPS_DYN_AUTOMOTIVE = 4,
    GPS_DYN_SEA = 5,
    GPS_DYN_AIRBORNE_1G = 6,
    GPS_DYN_AIRBORNE_2G = 7,
    GPS_DYN_AIRBORNE_4G = 8
};

/**
 * @brief GPS Advanced Configuration Class
 *
 * Gerencia todas as configurações avançadas do módulo GPS NEO-6M
 */
class GPSConfig {
public:
    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    GPSConfig();

    /////////////////////////////////////////////////////////////////////////////////////
    // Configuration Properties
    /////////////////////////////////////////////////////////////////////////////////////

    // Operation Mode
    GPSMode mode = GPS_MODE_STANDARD;

    // Update Rate (Hz)
    GPSUpdateRate updateRate = GPS_RATE_1HZ;

    // Power Mode
    GPSPowerMode powerMode = GPS_POWER_MAX_PERFORMANCE;

    // Protocol Mode
    GPSProtocolMode protocolMode = GPS_PROTOCOL_NMEA;

    // Navigation Mode
    GPSNavigationMode navMode = GPS_NAV_PORTABLE;

    // Dynamic Model
    GPSDynamicModel dynamicModel = GPS_DYN_PORTABLE;

    // Minimum satellite signal strength filter (dB-Hz)
    uint8_t minSatelliteSignal = 0;  // 0 = disabled, typical: 25-35 dB-Hz

    // Enable AssistNow (offline almanac data)
    bool assistNowEnabled = false;

    // Baudrate for advanced mode (higher speeds for higher update rates)
    uint32_t advancedBaudrate = 9600;

    /////////////////////////////////////////////////////////////////////////////////////
    // Configuration Methods
    /////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Apply configuration to GPS module via serial
     * @param serial HardwareSerial connected to GPS
     * @return true if successful
     */
    bool applyConfiguration(HardwareSerial &serial);

    /**
     * @brief Apply standard (default) configuration
     * @param serial HardwareSerial connected to GPS
     * @return true if successful
     */
    bool applyStandardConfig(HardwareSerial &serial);

    /**
     * @brief Apply advanced configuration with all optimizations
     * @param serial HardwareSerial connected to GPS
     * @return true if successful
     */
    bool applyAdvancedConfig(HardwareSerial &serial);

    /**
     * @brief Set update rate (1-10 Hz)
     * @param serial HardwareSerial connected to GPS
     * @param rateHz Update rate in Hz (1, 2, 5, or 10)
     * @return true if successful
     */
    bool setUpdateRate(HardwareSerial &serial, uint8_t rateHz);

    /**
     * @brief Enable Power Save Mode
     * @param serial HardwareSerial connected to GPS
     * @param enable true to enable, false to disable
     * @return true if successful
     */
    bool setPowerSaveMode(HardwareSerial &serial, bool enable);

    /**
     * @brief Set protocol mode (NMEA, UBX, or both)
     * @param serial HardwareSerial connected to GPS
     * @param mode Protocol mode
     * @return true if successful
     */
    bool setProtocolMode(HardwareSerial &serial, GPSProtocolMode mode);

    /**
     * @brief Set navigation/dynamic model
     * @param serial HardwareSerial connected to GPS
     * @param model Dynamic model
     * @return true if successful
     */
    bool setNavigationMode(HardwareSerial &serial, GPSDynamicModel model);

    /**
     * @brief Set minimum satellite signal filter
     * @param serial HardwareSerial connected to GPS
     * @param minSignalDb Minimum signal in dB-Hz (0-50, 0=disabled)
     * @return true if successful
     */
    bool setSatelliteFilter(HardwareSerial &serial, uint8_t minSignalDb);

    /**
     * @brief Change GPS baudrate
     * @param serial HardwareSerial connected to GPS
     * @param newBaudrate New baudrate (9600, 19200, 38400, 57600, 115200)
     * @return true if successful
     */
    bool changeBaudrate(HardwareSerial &serial, uint32_t newBaudrate);

    /**
     * @brief Save current configuration to GPS EEPROM
     * @param serial HardwareSerial connected to GPS
     * @return true if successful
     */
    bool saveToEEPROM(HardwareSerial &serial);

    /**
     * @brief Reset GPS to factory defaults
     * @param serial HardwareSerial connected to GPS
     * @return true if successful
     */
    bool factoryReset(HardwareSerial &serial);

    /////////////////////////////////////////////////////////////////////////////////////
    // Helper Methods
    /////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Get update rate description string
     * @param rate Update rate enum
     * @return Human readable string
     */
    static String getUpdateRateString(GPSUpdateRate rate);

    /**
     * @brief Get power mode description string
     * @param mode Power mode enum
     * @return Human readable string
     */
    static String getPowerModeString(GPSPowerMode mode);

    /**
     * @brief Get protocol mode description string
     * @param mode Protocol mode enum
     * @return Human readable string
     */
    static String getProtocolModeString(GPSProtocolMode mode);

    /**
     * @brief Get navigation mode description string
     * @param mode Navigation mode enum
     * @return Human readable string
     */
    static String getNavigationModeString(GPSNavigationMode mode);

private:
    /////////////////////////////////////////////////////////////////////////////////////
    // UBX Protocol Helpers
    /////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Calculate UBX checksum
     * @param data Pointer to data array
     * @param len Length of data
     * @param ck_a Output checksum A
     * @param ck_b Output checksum B
     */
    void calcUBXChecksum(uint8_t *data, uint16_t len, uint8_t &ck_a, uint8_t &ck_b);

    /**
     * @brief Send UBX command
     * @param serial HardwareSerial connected to GPS
     * @param class_id UBX class ID
     * @param msg_id UBX message ID
     * @param payload Payload data
     * @param payload_len Payload length
     * @return true if sent successfully
     */
    bool sendUBXCommand(HardwareSerial &serial, uint8_t class_id, uint8_t msg_id, uint8_t *payload, uint16_t payload_len);

    /**
     * @brief Send PMTK command (for MT3339 based GPS)
     * @param serial HardwareSerial connected to GPS
     * @param command PMTK command string (without $ and *)
     * @return true if sent successfully
     */
    bool sendPMTKCommand(HardwareSerial &serial, const String &command);

    /**
     * @brief Wait for UBX ACK
     * @param serial HardwareSerial connected to GPS
     * @param class_id Expected class ID
     * @param msg_id Expected message ID
     * @param timeout_ms Timeout in milliseconds
     * @return true if ACK received
     */
    bool waitForUBXAck(HardwareSerial &serial, uint8_t class_id, uint8_t msg_id, uint32_t timeout_ms = 1000);
};

// Global GPS Config Instance
extern GPSConfig gpsConfig;

#endif // __GPS_CONFIG_H__
