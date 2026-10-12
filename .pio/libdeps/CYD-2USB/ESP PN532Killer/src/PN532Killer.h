/**
 * @file PN532Killer.h
 * @brief Control PN532Killer board with UART
 * @version 0.1
 * @date 2025-03-31
 */

#ifndef PN532KILLER_H
#define PN532KILLER_H

#include "PN532Cmd.h"
#include "PN532KillerCmd.h"
#include "PN532KillerTagTechnology.h"
#include "PN532_UART.h"

class PN532Killer : public PN532_UART {
public:
    explicit PN532Killer(HardwareSerial &serial);
    ~PN532Killer();

    bool isPn532killer();
    bool switchReaderMode();
    bool switchEmulatorMifareSlot(uint8_t slot);
    bool switchEmulatorNtagSlot(uint8_t slot);
    bool switchEmulatorIso15693Slot(uint8_t slot);
    bool switchEmulatorEm4100Slot(uint8_t slot);
    bool setMfkey32v2Sniffing();
    bool setMfkey64Sniffing();
    bool setSnifferMode(uint8_t type);
    bool setSnifferSlotBlock0(const std::vector<uint8_t> &block0);
    bool setSnifferUid(const std::string &uid_hex);

    PN532KillerTagTechnology::Iso15693TagInfo hf15Scan();
    PN532KillerTagTechnology::Em4100TagInfo lfEm4100Scan();

    int8_t tagIndex = -1;

private:
    bool setWorkMode(uint8_t mode, uint8_t type, uint8_t index);
    PN532KillerTagTechnology tagTechnology = PN532KillerTagTechnology();

    bool uploadData(uint8_t type, uint8_t slot, uint16_t index, const std::vector<uint8_t> &datas);
    bool uploadDataDone(uint8_t type, uint8_t slot, const std::vector<uint8_t> &extra);
};

#endif
