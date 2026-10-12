#include "TagTechnology.h"
#include <sstream>
#include <iomanip>
#include <string.h>
#include <algorithm>

std::string TagTechnology::bytesToHexString(std::vector<uint8_t> *data, bool reverse)
{
    if (reverse) {
        std::reverse(data->begin(), data->end());
    }
    std::ostringstream hexStream;
    for (const auto &byte : *data)
    {
        hexStream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
    }
    return hexStream.str();
}

TagTechnology::Iso14aTagInfo TagTechnology::parseIso14aTag(uint8_t *data, uint8_t dataSize) {
    Iso14aTagInfo tagInfo;
    tagInfo.atqa = {data[2], data[3]};
    tagInfo.sak = data[4];
    tagInfo.uidSize = data[5];
    tagInfo.uid.assign(data + 6, data + 6 + tagInfo.uidSize);
    tagInfo.uid_hex = bytesToHexString(&tagInfo.uid);
    tagInfo.atqa_hex = bytesToHexString(&tagInfo.atqa);
    std::vector<uint8_t> sakVector = {tagInfo.sak};
    tagInfo.sak_hex = bytesToHexString(&sakVector);
    tagInfo.type = getIso14aTagType(tagInfo.sak);
    return tagInfo;
}

std::string TagTechnology::getIso14aTagType(uint8_t sak) {
    switch (sak) {
        case 0x09:
            return "MIFARE Classic Mini";
        case 0x08:
        case 0x88:
            return "MIFARE Classic 1K";
        case 0x18:
            return "MIFARE Classic 4K";
        case 0x00:
            return "MIFARE Ultralight";
        default:
            return "Unknown";
    }
}
