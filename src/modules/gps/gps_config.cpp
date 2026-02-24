/**
 * @file gps_config.cpp
 * @author Willy Firmware
 * @brief GPS Advanced Configuration Implementation for NEO-6M GY-GPS6MV2
 * @version 1.0
 */

#include "gps_config.h"
#include "core/display.h"
#include <globals.h>

// Global instance
GPSConfig gpsConfig;

GPSConfig::GPSConfig() {
    // Default to standard mode
    mode = GPS_MODE_STANDARD;
}

/////////////////////////////////////////////////////////////////////////////////////
// Configuration Apply Methods
/////////////////////////////////////////////////////////////////////////////////////

bool GPSConfig::applyConfiguration(HardwareSerial &serial) {
    if (mode == GPS_MODE_STANDARD) {
        return applyStandardConfig(serial);
    } else {
        return applyAdvancedConfig(serial);
    }
}

bool GPSConfig::applyStandardConfig(HardwareSerial &serial) {
    // Standard configuration: 1Hz, NMEA, no optimizations
    // Most GPS modules come with this as default, so we just ensure it

    // Send PMTK command to set 1Hz update rate
    // $PMTK220,1000*1F = 1000ms interval (1Hz)
    sendPMTKCommand(serial, "PMTK220,1000");

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Reset NMEA output
    sendPMTKCommand(serial, "PMTK314,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Enable standard NMEA sentences: RMC, VTG, GGA, GSA, GSV
    sendPMTKCommand(serial, "PMTK314,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
    vTaskDelay(100 / portTICK_PERIOD_MS);

    return true;
}

bool GPSConfig::applyAdvancedConfig(HardwareSerial &serial) {
    bool success = true;

    // Step 1: Set update rate
    if (!setUpdateRate(serial, updateRate)) {
        displayWarning("Falha ao definir taxa de atualizacao", true);
        success = false;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Step 2: Set navigation mode
    if (!setNavigationMode(serial, dynamicModel)) {
        displayWarning("Falha ao definir modo navegacao", true);
        success = false;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Step 3: Set power mode
    if (powerMode == GPS_POWER_SAVE) {
        if (!setPowerSaveMode(serial, true)) {
            displayWarning("Falha ao definir Power Save", true);
            success = false;
        }
    } else if (powerMode == GPS_POWER_ECO) {
        // Eco mode via UBX-CFG-PM2
        uint8_t payload[] = {
            0x01, 0x06,  // version, mode
            0x00, 0x10,  // reserved, update period
            0x00, 0x10,  // search period
            0x00, 0x00,  // grid offset
            0x40, 0x42, 0x0F, 0x00,  // on-time, min acquisition
            0x00, 0x00, 0x00, 0x00   // reserved
        };
        sendUBXCommand(serial, 0x06, 0x3B, payload, sizeof(payload));
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Step 4: Set satellite filter
    if (minSatelliteSignal > 0) {
        if (!setSatelliteFilter(serial, minSatelliteSignal)) {
            displayWarning("Falha ao definir filtro satelite", true);
            success = false;
        }
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Step 5: Change baudrate if needed for higher update rates
    if (updateRate >= GPS_RATE_5HZ && advancedBaudrate > 9600) {
        if (!changeBaudrate(serial, advancedBaudrate)) {
            displayWarning("Falha ao mudar baudrate", true);
            success = false;
        }
        // Need to reinitialize serial at new baudrate (caller responsibility)
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Save to EEPROM to persist settings
    saveToEEPROM(serial);

    return success;
}

/////////////////////////////////////////////////////////////////////////////////////
// Individual Configuration Methods
/////////////////////////////////////////////////////////////////////////////////////

bool GPSConfig::setUpdateRate(HardwareSerial &serial, uint8_t rateHz) {
    uint16_t intervalMs = 1000 / rateHz;

    // Method 1: PMTK command (works with MT3339 and many clones)
    String cmd = "PMTK220," + String(intervalMs);
    sendPMTKCommand(serial, cmd);

    // Method 2: UBX-CFG-RATE command for u-blox modules
    // Payload: measure rate (ms), nav rate, time ref
    uint8_t payload[] = {
        (uint8_t)(intervalMs & 0xFF),      // measurement rate LSB
        (uint8_t)((intervalMs >> 8) & 0xFF), // measurement rate MSB
        0x01,  // navigation rate (1 measurement per nav solution)
        0x00,  // navigation rate MSB
        0x01,  // time reference: UTC time
        0x00   // time reference MSB
    };

    sendUBXCommand(serial, 0x06, 0x08, payload, sizeof(payload));

    return waitForUBXAck(serial, 0x06, 0x08, 500);
}

bool GPSConfig::setPowerSaveMode(HardwareSerial &serial, bool enable) {
    // UBX-CFG-PM2: Power Save Mode configuration
    if (enable) {
        // Enable Power Save Mode with cyclic tracking
        uint8_t payload[] = {
            0x01,        // version
            0x08,        // mode: Power Save Mode with cyclic tracking
            0x00, 0x10,  // update period: 1 second
            0x00, 0x10,  // search period: 1 second
            0x00, 0x00,  // grid offset
            0x40, 0x42, 0x0F, 0x00,  // on-time, min acquisition time
            0x00, 0x00, 0x00, 0x00   // reserved
        };
        sendUBXCommand(serial, 0x06, 0x3B, payload, sizeof(payload));
    } else {
        // Disable Power Save Mode (Max Performance)
        uint8_t payload[] = {
            0x01,        // version
            0x00,        // mode: Max Performance
            0x00, 0x10,  // update period
            0x00, 0x10,  // search period
            0x00, 0x00,  // grid offset
            0x00, 0x00, 0x00, 0x00,  // on-time, min acquisition
            0x00, 0x00, 0x00, 0x00   // reserved
        };
        sendUBXCommand(serial, 0x06, 0x3B, payload, sizeof(payload));
    }

    return waitForUBXAck(serial, 0x06, 0x3B, 500);
}

bool GPSConfig::setProtocolMode(HardwareSerial &serial, GPSProtocolMode mode) {
    // UBX-CFG-PRT: Port configuration for protocol
    // Port ID 1 = UART1
    uint8_t payload[20] = {0};
    payload[0] = 0x01;  // Port ID: UART1

    // TX ready pin
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x00;

    // Mode: 8N1
    payload[4] = 0xD0;
    payload[5] = 0x08;
    payload[6] = 0x00;
    payload[7] = 0x00;

    // Baudrate (keep current)
    payload[8] = 0x80;
    payload[9] = 0x25;
    payload[10] = 0x00;
    payload[11] = 0x00;  // 9600

    // inProtoMask (input protocols)
    payload[12] = 0x07;  // UBX + NMEA + RTCM
    payload[13] = 0x00;

    // outProtoMask (output protocols)
    switch (mode) {
        case GPS_PROTOCOL_NMEA:
            payload[14] = 0x02;  // NMEA only
            break;
        case GPS_PROTOCOL_UBX:
            payload[14] = 0x01;  // UBX only
            break;
        case GPS_PROTOCOL_BOTH:
            payload[14] = 0x03;  // UBX + NMEA
            break;
    }
    payload[15] = 0x00;

    // flags
    payload[16] = 0x00;
    payload[17] = 0x00;
    payload[18] = 0x00;
    payload[19] = 0x00;

    sendUBXCommand(serial, 0x06, 0x00, payload, sizeof(payload));

    return waitForUBXAck(serial, 0x06, 0x00, 500);
}

bool GPSConfig::setNavigationMode(HardwareSerial &serial, GPSDynamicModel model) {
    // UBX-CFG-NAV5: Navigation Engine Settings
    uint8_t payload[36] = {0};

    // Mask: set dyn model
    payload[0] = 0x01;  // LSB of mask
    payload[1] = 0x00;  // MSB of mask

    // Dynamic model
    payload[2] = (uint8_t)model;

    // Auto-fixed mode
    payload[3] = 0x03;

    // Min SVs for fix
    payload[4] = 0x03;

    // Send the command
    sendUBXCommand(serial, 0x06, 0x24, payload, sizeof(payload));

    return waitForUBXAck(serial, 0x06, 0x24, 500);
}

bool GPSConfig::setSatelliteFilter(HardwareSerial &serial, uint8_t minSignalDb) {
    // UBX-CFG-NAVX5: Navigation Engine Expert Settings
    uint8_t payload[40] = {0};

    // Mask
    payload[0] = 0x00;
    payload[1] = 0x04;  // Bit 2: minCno

    // Version
    payload[2] = 0x00;
    payload[3] = 0x02;

    // minCno (minimum satellite signal strength)
    payload[4] = minSignalDb;

    // Rest of payload is reserved/zeros

    sendUBXCommand(serial, 0x06, 0x23, payload, sizeof(payload));

    return waitForUBXAck(serial, 0x06, 0x23, 500);
}

bool GPSConfig::changeBaudrate(HardwareSerial &serial, uint32_t newBaudrate) {
    // UBX-CFG-PRT: Port configuration for baudrate
    uint8_t payload[20] = {0};
    payload[0] = 0x01;  // Port ID: UART1

    // TX ready pin
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x00;

    // Mode: 8N1
    payload[4] = 0xD0;
    payload[5] = 0x08;
    payload[6] = 0x00;
    payload[7] = 0x00;

    // Baudrate
    payload[8] = (uint8_t)(newBaudrate & 0xFF);
    payload[9] = (uint8_t)((newBaudrate >> 8) & 0xFF);
    payload[10] = (uint8_t)((newBaudrate >> 16) & 0xFF);
    payload[11] = (uint8_t)((newBaudrate >> 24) & 0xFF);

    // inProtoMask
    payload[12] = 0x07;
    payload[13] = 0x00;

    // outProtoMask
    payload[14] = 0x03;  // UBX + NMEA
    payload[15] = 0x00;

    // flags
    payload[16] = 0x00;
    payload[17] = 0x00;
    payload[18] = 0x00;
    payload[19] = 0x00;

    sendUBXCommand(serial, 0x06, 0x00, payload, sizeof(payload));

    // Wait a bit for the command to be processed
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Update our stored baudrate
    advancedBaudrate = newBaudrate;

    return true;
}

bool GPSConfig::saveToEEPROM(HardwareSerial &serial) {
    // UBX-CFG-CFG: Save current configuration
    uint8_t payload[13] = {0};

    // Clear mask: don't clear anything
    payload[0] = 0x00;
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x00;

    // Save mask: save all
    payload[4] = 0x1F;
    payload[5] = 0x1F;
    payload[6] = 0x00;
    payload[7] = 0x00;

    // Load mask: don't load
    payload[8] = 0x00;
    payload[9] = 0x00;
    payload[10] = 0x00;
    payload[11] = 0x00;

    // Device mask: all devices
    payload[12] = 0xFF;

    sendUBXCommand(serial, 0x06, 0x09, payload, sizeof(payload));

    return waitForUBXAck(serial, 0x06, 0x09, 1000);
}

bool GPSConfig::factoryReset(HardwareSerial &serial) {
    // UBX-CFG-CFG: Factory reset
    uint8_t payload[13] = {0};

    // Clear mask: clear all
    payload[0] = 0xFF;
    payload[1] = 0xFF;
    payload[2] = 0x00;
    payload[3] = 0x00;

    // Save mask: don't save
    payload[4] = 0x00;
    payload[5] = 0x00;
    payload[6] = 0x00;
    payload[7] = 0x00;

    // Load mask: load all defaults
    payload[8] = 0xFF;
    payload[9] = 0xFF;
    payload[10] = 0x00;
    payload[11] = 0x00;

    // Device mask: all devices
    payload[12] = 0xFF;

    sendUBXCommand(serial, 0x06, 0x09, payload, sizeof(payload));

    vTaskDelay(500 / portTICK_PERIOD_MS);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Helper Methods
/////////////////////////////////////////////////////////////////////////////////////

String GPSConfig::getUpdateRateString(GPSUpdateRate rate) {
    switch (rate) {
        case GPS_RATE_1HZ:  return "1 Hz (Padrao)";
        case GPS_RATE_2HZ:  return "2 Hz";
        case GPS_RATE_5HZ:  return "5 Hz (Wardriving)";
        case GPS_RATE_10HZ: return "10 Hz (Instavel)";
        default:            return "Desconhecido";
    }
}

String GPSConfig::getPowerModeString(GPSPowerMode mode) {
    switch (mode) {
        case GPS_POWER_MAX_PERFORMANCE: return "Max Desempenho";
        case GPS_POWER_ECO:              return "Eco (Economia)";
        case GPS_POWER_SAVE:             return "Power Save (Minimo)";
        default:                         return "Desconhecido";
    }
}

String GPSConfig::getProtocolModeString(GPSProtocolMode mode) {
    switch (mode) {
        case GPS_PROTOCOL_NMEA: return "NMEA (Texto)";
        case GPS_PROTOCOL_UBX:  return "UBX (Binario)";
        case GPS_PROTOCOL_BOTH: return "NMEA + UBX";
        default:                return "Desconhecido";
    }
}

String GPSConfig::getNavigationModeString(GPSNavigationMode mode) {
    switch (mode) {
        case GPS_NAV_PORTABLE:    return "Portatil";
        case GPS_NAV_STATIONARY:  return "Estacionario";
        case GPS_NAV_PEDESTRIAN:  return "Pedestre";
        case GPS_NAV_AUTOMOTIVE:  return "Automotivo";
        case GPS_NAV_SEA:         return "Maritimo";
        case GPS_NAV_AIRBORNE:    return "Aereo";
        default:                  return "Desconhecido";
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// UBX Protocol Helpers
/////////////////////////////////////////////////////////////////////////////////////

void GPSConfig::calcUBXChecksum(uint8_t *data, uint16_t len, uint8_t &ck_a, uint8_t &ck_b) {
    ck_a = 0;
    ck_b = 0;

    for (uint16_t i = 0; i < len; i++) {
        ck_a += data[i];
        ck_b += ck_a;
    }
}

bool GPSConfig::sendUBXCommand(HardwareSerial &serial, uint8_t class_id, uint8_t msg_id, uint8_t *payload, uint16_t payload_len) {
    // Build UBX frame: sync1, sync2, class, id, length, payload, checksum
    uint16_t frame_len = 8 + payload_len;
    uint8_t *frame = new uint8_t[frame_len];

    frame[0] = 0xB5;  // Sync char 1
    frame[1] = 0x62;  // Sync char 2
    frame[2] = class_id;
    frame[3] = msg_id;
    frame[4] = payload_len & 0xFF;
    frame[5] = (payload_len >> 8) & 0xFF;

    // Copy payload
    if (payload_len > 0 && payload != nullptr) {
        memcpy(&frame[6], payload, payload_len);
    }

    // Calculate checksum over class, id, length, and payload
    uint8_t ck_a, ck_b;
    calcUBXChecksum(&frame[2], 4 + payload_len, ck_a, ck_b);

    frame[6 + payload_len] = ck_a;
    frame[7 + payload_len] = ck_b;

    // Send the frame
    serial.write(frame, frame_len);
    serial.flush();

    delete[] frame;

    return true;
}

bool GPSConfig::sendPMTKCommand(HardwareSerial &serial, const String &command) {
    // Build PMTK command with checksum
    // Format: $PMTKxxx*checksum\r\n

    // Calculate NMEA checksum (XOR of all characters between $ and *)
    uint8_t checksum = 0;
    for (size_t i = 0; i < command.length(); i++) {
        checksum ^= (uint8_t)command[i];
    }

    String fullCommand = "$" + command + "*" + String(checksum, HEX);
    fullCommand.toUpperCase();
    fullCommand += "\r\n";

    serial.print(fullCommand);
    serial.flush();

    return true;
}

bool GPSConfig::waitForUBXAck(HardwareSerial &serial, uint8_t class_id, uint8_t msg_id, uint32_t timeout_ms) {
    uint32_t startTime = millis();

    // Buffer for ACK message
    uint8_t buffer[10];
    uint8_t idx = 0;
    bool inMessage = false;

    while (millis() - startTime < timeout_ms) {
        if (serial.available() > 0) {
            uint8_t byte = serial.read();

            if (!inMessage) {
                if (byte == 0xB5) {
                    idx = 0;
                    buffer[idx++] = byte;
                    inMessage = true;
                }
            } else {
                if (idx < sizeof(buffer)) {
                    buffer[idx++] = byte;
                }

                // Check for ACK-NAK or ACK-ACK message
                // ACK-ACK: B5 62 05 01 ...
                // ACK-NAK: B5 62 05 00 ...
                if (idx >= 10) {
                    if (buffer[0] == 0xB5 && buffer[1] == 0x62 &&
                        buffer[2] == 0x05) {  // ACK class

                        if (buffer[3] == 0x01 &&  // ACK-ACK
                            buffer[6] == class_id &&
                            buffer[7] == msg_id) {
                            return true;  // ACK received
                        } else if (buffer[3] == 0x00 &&  // ACK-NAK
                                   buffer[6] == class_id &&
                                   buffer[7] == msg_id) {
                            return false;  // NAK received
                        }
                    }

                    // Reset for next message
                    inMessage = false;
                    idx = 0;
                }
            }
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    return false;  // Timeout
}
