#ifndef PN532_UART_H
#define PN532_UART_H

#include "HardwareSerial.h"
#include "PN532Cmd.h"
#include "TagTechnology.h"
#include <vector>

class PN532_UART {
public:
    explicit PN532_UART(HardwareSerial &serial);
    virtual ~PN532_UART();

    std::vector<uint8_t>
    writeCommand(uint8_t cmd, const std::vector<uint8_t> &datas = {}, int time_out = 1000);
    std::vector<uint8_t> writeRawCommand(const std::vector<uint8_t> data, int time_out = 1000);
    bool setNormalMode();
    std::vector<uint8_t> getVersion();
    TagTechnology::Iso14aTagInfo hf14aScan();
    void close();

    // NDEF emulation methods
    std::vector<uint8_t> getData();
    std::vector<uint8_t> setData(const std::vector<uint8_t> &data);
    bool inRelease();
    std::vector<uint8_t> tgInitAsTarget(const std::vector<uint8_t> &data);

protected:
    HardwareSerial &serial;
    uint8_t dcs(uint8_t *data, size_t length);
    TagTechnology tagTechnology = TagTechnology();
};

#endif // PN532_UART_H
