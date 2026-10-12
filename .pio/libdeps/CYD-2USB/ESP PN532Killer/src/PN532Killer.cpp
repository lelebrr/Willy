/**
 * @file PN532Killer.cpp
 * @brief Control PN532Killer via UART
 * @version 0.1
 * @date 2025-03-31
 */

#include "PN532Killer.h"

PN532Killer::PN532Killer(HardwareSerial &serial) : PN532_UART(serial) {}

PN532Killer::~PN532Killer() {}

bool PN532Killer::setWorkMode(uint8_t mode, uint8_t type, uint8_t index) {
    std::vector<uint8_t> response = writeCommand(PN532KillerCmd::SetWorkMode, {mode, type, index});
    return response.size() > 0;
}

bool PN532Killer::isPn532killer() {
    std::vector<uint8_t> response = writeCommand(PN532KillerCmd::CheckPn532Killer, {});
    return response.size() > 2;
}

bool PN532Killer::switchReaderMode() {
    return setWorkMode(PN532KillerCmd::WorkMode::Reader, PN532KillerCmd::TagType::MFC1K, 0x00);
}

bool PN532Killer::switchEmulatorMifareSlot(uint8_t slot) {
    return setWorkMode(PN532KillerCmd::WorkMode::Emulator, PN532KillerCmd::TagType::MFC1K, slot);
}

bool PN532Killer::switchEmulatorNtagSlot(uint8_t slot) {
    return setWorkMode(PN532KillerCmd::WorkMode::Emulator, PN532KillerCmd::TagType::NTAG, slot);
}

bool PN532Killer::switchEmulatorIso15693Slot(uint8_t slot) {
    return setWorkMode(PN532KillerCmd::WorkMode::Emulator, PN532KillerCmd::TagType::ISO15693, slot);
}

bool PN532Killer::switchEmulatorEm4100Slot(uint8_t slot) {
    return setWorkMode(PN532KillerCmd::WorkMode::Emulator, PN532KillerCmd::TagType::EM4100, slot);
}

// deprecated, use setSnifferMode instead
bool PN532Killer::setMfkey32v2Sniffing() {
    return setWorkMode(PN532KillerCmd::WorkMode::Sniffer, PN532KillerCmd::TagType::MFC1K, 0x00);
}

// deprecated, use setSnifferMode instead
bool PN532Killer::setMfkey64Sniffing() {
    return setWorkMode(PN532KillerCmd::WorkMode::Sniffer, PN532KillerCmd::TagType::MFC1K, 0x01);
}

bool PN532Killer::setSnifferMode(uint8_t type) {
    return setWorkMode(PN532KillerCmd::WorkMode::Sniffer, PN532KillerCmd::TagType::MFC1K, type);
}

bool PN532Killer::uploadData(uint8_t type, uint8_t slot, uint16_t index, const std::vector<uint8_t> &datas) {
    std::vector<uint8_t> payload;
    payload.push_back(type);
    payload.push_back(slot);
    payload.push_back((index >> 8) & 0xFF);
    payload.push_back(index & 0xFF);
    if (!datas.empty()) { payload.insert(payload.end(), datas.begin(), datas.end()); }
    std::vector<uint8_t> resp = writeCommand(PN532KillerCmd::SetEmulatorData, payload);
    return !resp.empty();
}

bool PN532Killer::uploadDataDone(uint8_t type, uint8_t slot, const std::vector<uint8_t> &extra) {
    std::vector<uint8_t> payload;
    payload.push_back(type);
    payload.push_back(slot);
    payload.push_back(0xFF);
    payload.push_back(0xFF);
    if (extra.empty()) {
        payload.insert(payload.end(), 16, 0x00);
    } else {
        payload.insert(payload.end(), extra.begin(), extra.end());
    }
    std::vector<uint8_t> resp = writeCommand(PN532KillerCmd::SetEmulatorData, payload);
    return !resp.empty();
}

bool PN532Killer::setSnifferSlotBlock0(const std::vector<uint8_t> &block0) {
    if (block0.size() != 16) { return false; }
    return uploadData(PN532KillerCmd::TagType::MFC1K, 0x11, 0x0000, block0) &&
           uploadDataDone(PN532KillerCmd::TagType::MFC1K, 0x11, {});
}

bool PN532Killer::setSnifferUid(const std::string &uid_hex) {
    if (uid_hex.length() != 8 && uid_hex.length() != 14) { return false; }
    std::vector<uint8_t> uid;
    for (size_t i = 0; i < uid_hex.length(); i += 2) {
        uint8_t byte = std::stoi(uid_hex.substr(i, 2), nullptr, 16);
        uid.push_back(byte);
    }
    if (uid.size() != 4 && uid.size() != 7) { return false; }

    std::vector<uint8_t> block0(16, 0x00);
    std::copy(uid.begin(), uid.end(), block0.begin());

    if (uid.size() == 4) {
        uint8_t bcc = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
        block0[4] = bcc;
        block0[5] = 0x08; // SAK
        block0[6] = 0x04;
        block0[7] = 0x00; // ATQA
    } else if (uid.size() == 7) {
        std::copy(uid.begin(), uid.end(), block0.begin());
        block0[7] = 0x08;
        block0[8] = 0x44;
        block0[9] = 0x00;
    }

    return setSnifferSlotBlock0(block0);
}

PN532KillerTagTechnology::Iso15693TagInfo PN532Killer::hf15Scan() {
    std::vector<uint8_t> result = writeCommand(PN532KillerCmd::InListPassiveTarget, {0x01, 0x05}, 1000);
    if (result.size() < 9) { return PN532KillerTagTechnology::Iso15693TagInfo(); }

    return tagTechnology.parseIso15693Tag(result.data(), result.size());
}

PN532KillerTagTechnology::Em4100TagInfo PN532Killer::lfEm4100Scan() {
    std::vector<uint8_t> result = writeCommand(PN532KillerCmd::InListPassiveTarget, {0x01, 0x06}, 1000);
    if (result.size() < 6) { return PN532KillerTagTechnology::Em4100TagInfo(); }
    return tagTechnology.parseEm4100Tag(result.data(), result.size());
}
