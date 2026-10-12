#ifndef PN532_COMMAND_H
#define PN532_COMMAND_H

#include <cstdint>

namespace PN532Cmd {
static constexpr uint8_t Data_Preamble = 0x00;
static constexpr uint8_t Data_Start_Code_0 = 0x00;
static constexpr uint8_t Data_Start_Code_1 = 0xFF;
static constexpr uint8_t Data_Tif_Send = 0xD4;
static constexpr uint8_t Data_Tif_Receive = 0xD5;
static constexpr uint8_t Data_Postamble = 0x00;
static constexpr uint8_t Diagnose = 0x00;
static constexpr uint8_t GetFirmwareVersion = 0x02;
static constexpr uint8_t ReadRegister = 0x06;
static constexpr uint8_t WriteRegister = 0x08;
static constexpr uint8_t SAMConfiguration = 0x14;
static constexpr uint8_t PowerDown = 0x16;
static constexpr uint8_t InDataExchange = 0x40;
static constexpr uint8_t InCommunicateThru = 0x42;
static constexpr uint8_t InListPassiveTarget = 0x4A;
static constexpr uint8_t InDeselect = 0x44;
static constexpr uint8_t InRelease = 0x52;
static constexpr uint8_t InSelect = 0x54;
static constexpr uint8_t InAutoPoll = 0x60;
static constexpr uint8_t TgInitAsTarget = 0x8C;
static constexpr uint8_t TgGetData = 0x86;
static constexpr uint8_t TgSetData = 0x8E;
} // namespace PN532Cmd

#endif // PN532_COMMAND_H
