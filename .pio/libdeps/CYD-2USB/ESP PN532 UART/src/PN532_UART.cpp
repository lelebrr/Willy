/**
 * @file PN532_UART.cpp
 * @brief Control PN532 via UART
 * @version 0.1
 * @date 2025-04-03
 */

#include "PN532_UART.h"

PN532_UART::PN532_UART(HardwareSerial &serial) : serial(serial) {}

PN532_UART::~PN532_UART() {}

uint8_t PN532_UART::dcs(uint8_t *data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) { checksum += data[i]; }
    return (0x00 - checksum) & 0xFF;
}

std::vector<uint8_t> PN532_UART::writeCommand(uint8_t cmd, const std::vector<uint8_t> &datas, int time_out) {
    uint8_t *data = const_cast<uint8_t *>(datas.data());
    size_t length = datas.size();
    std::vector<uint8_t> commands;
    commands.push_back(PN532Cmd::Data_Tif_Send);
    commands.push_back(cmd);
    commands.insert(commands.end(), data, data + length);

    std::vector<uint8_t> frame;
    frame.push_back(PN532Cmd::Data_Preamble);
    frame.push_back(PN532Cmd::Data_Start_Code_0);
    frame.push_back(PN532Cmd::Data_Start_Code_1);

    uint8_t len = commands.size();
    uint8_t length_check_sum = (0x00 - len) & 0xFF;
    frame.push_back(len);
    frame.push_back(length_check_sum);
    frame.insert(frame.end(), commands.begin(), commands.end());

    uint8_t dcs_value = dcs(commands.data(), commands.size());
    frame.push_back(dcs_value);
    frame.push_back(PN532Cmd::Data_Postamble);

    Serial.print("PN532 <-");
    for (int i = 0; i < frame.size(); i++) {
        Serial.print(frame[i] < 0x10 ? " 0" : " ");
        Serial.print(frame[i], HEX);
    }
    Serial.println();

    serial.write(frame.data(), frame.size());
    serial.flush();

    uint8_t response[256];
    unsigned long startTime = millis();
    int resLength = 0;

    while (millis() - startTime < time_out) {
        resLength = serial.readBytes(response, sizeof(response));
        if (resLength >= 6) { break; }
        delay(10);
    }

    if (resLength < 6) {
        Serial.println("PN532 -> Timeout or invalid response length");
        return {};
    }

    std::vector<uint8_t> responseData(response, response + resLength);
    Serial.print("PN532 ->");
    for (int i = 0; i < resLength; i++) {
        Serial.print(response[i] < 0x10 ? " 0" : " ");
        Serial.print(response[i], HEX);
    }
    Serial.println();

    // Scan for the actual response frame (00 00 FF [LEN] [LCS] ...) dynamically,
    // skipping leading zeros and ACK frames. PN532Killer may send extra 0x00 bytes
    // before the real response, so fixed-offset parsing is not reliable.
    int frame_start = -1;
    for (int i = 0; i + 4 < resLength; i++) {
        if (response[i] == 0x00 && response[i + 1] == 0x00 && response[i + 2] == 0xFF) {
            uint8_t len = response[i + 3];
            uint8_t lcs = response[i + 4];
            // Valid response frame: LEN != 0 and (LEN + LCS) == 0x00
            if (len != 0 && ((len + lcs) & 0xFF) == 0) {
                frame_start = i;
                break;
            }
        }
    }
    if (frame_start < 0) {
        Serial.println("PN532 -> Length checksum error");
        return {};
    }
    int base = frame_start;
    uint8_t res_len = response[base + 3];
    if (base + 5 + res_len + 1 > resLength) {
        Serial.println("PN532 -> Response too short");
        return {};
    }
    uint8_t res_dcs_value = response[base + 5 + res_len];
    if (dcs(response + base + 5, res_len) != res_dcs_value) {
        Serial.println("PN532 -> Data checksum error");
        return {};
    }

    // Extract the data payload (TFI at base+5, CMD at base+6, data from base+7)
    std::vector<uint8_t> payload(response + base + 7, response + base + 7 + res_len);
    Serial.print("Payload(");
    Serial.print(payload.size());
    Serial.print("):");
    for (size_t i = 0; i < payload.size(); i++) {
        Serial.print(payload[i] < 0x10 ? " 0" : " ");
        Serial.print(payload[i], HEX);
    }
    Serial.println();

    return payload;
}

std::vector<uint8_t> PN532_UART::writeRawCommand(const std::vector<uint8_t> data, int time_out) {
    serial.write(data.data(), data.size());
    serial.flush();
    uint8_t response[256];
    unsigned long startTime = millis();
    int resLength = serial.readBytes(response, sizeof(response));
    std::vector<uint8_t> responseData(response, response + resLength);
    return responseData;
}

bool PN532_UART::setNormalMode() {
    std::vector<uint8_t> wakeUpCommand = {
        0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    serial.write(wakeUpCommand.data(), wakeUpCommand.size());
    std::vector<uint8_t> response = writeCommand(PN532Cmd::SAMConfiguration, {0x01});
    delay(10);
    return response.size() > 0;
}

std::vector<uint8_t> PN532_UART::getVersion() { return writeCommand(PN532Cmd::GetFirmwareVersion); }

TagTechnology::Iso14aTagInfo PN532_UART::hf14aScan() {
    std::vector<uint8_t> result = writeCommand(PN532Cmd::InListPassiveTarget, {0x01, 0x00}, 1000);
    if (result.size() < 6) { return TagTechnology::Iso14aTagInfo(); }
    return tagTechnology.parseIso14aTag(result.data(), result.size());
}

void PN532_UART::close() { serial.end(); }

// NDEF emulation methods
std::vector<uint8_t> PN532_UART::getData() { return writeCommand(PN532Cmd::TgGetData); }

std::vector<uint8_t> PN532_UART::setData(const std::vector<uint8_t> &data) {
    return writeCommand(PN532Cmd::TgSetData, data);
}

bool PN532_UART::inRelease() {
    std::vector<uint8_t> response = writeCommand(PN532Cmd::InRelease, {0x00});
    return response.size() > 0;
}

std::vector<uint8_t> PN532_UART::tgInitAsTarget(const std::vector<uint8_t> &data) {
    return writeCommand(PN532Cmd::TgInitAsTarget, data);
}
