#ifndef PN532_KILLER_COMMAND_H
#define PN532_KILLER_COMMAND_H

#include "PN532Cmd.h"
#include <cstdint>

namespace PN532KillerCmd {
using namespace PN532Cmd;

static constexpr uint8_t GetEmulatorData = 0x1C;
static constexpr uint8_t SetEmulatorData = 0x1E;
static constexpr uint8_t CheckPn532Killer = 0xAA;
static constexpr uint8_t SetWorkMode = 0xAC;
static constexpr uint8_t GetSnifferLog = 0x20;
static constexpr uint8_t ClearSnifferLog = 0x22;

enum WorkMode { Reader = 0x01, Emulator = 0x02, Sniffer = 0x03 };
enum TagType { MFC1K = 0x01, NTAG = 0X02, ISO15693 = 0x03, EM4100 = 0x04 };
enum ReaderProtocol { HF_ISO14443A = 0x01, HF_ISO15693 = 0x02, LF_EM4100 = 0x03 };
enum SnifferType { MFKey32v2 = 0x00, MFKey64 = 0x01 };
} // namespace PN532KillerCmd

#endif // PN532_KILLER_COMMAND_H
