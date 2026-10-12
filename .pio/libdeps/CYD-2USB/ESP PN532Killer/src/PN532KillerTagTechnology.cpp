#include "PN532KillerTagTechnology.h"
#include <sstream>
#include <iomanip>
#include <string.h>
#include <algorithm>

PN532KillerTagTechnology::Iso15693TagInfo PN532KillerTagTechnology::parseIso15693Tag(uint8_t *data, uint8_t dataSize) {
    Iso15693TagInfo tagInfo;
    tagInfo.uid.assign(data + 2, data + 10);
    tagInfo.uid_hex = TagTechnology::bytesToHexString(&tagInfo.uid, true);
    return tagInfo;
}

PN532KillerTagTechnology::Em4100TagInfo PN532KillerTagTechnology::parseEm4100Tag(uint8_t *data, uint8_t dataSize) {
    Em4100TagInfo tagInfo;
    tagInfo.uid.assign(data + 2, data + 7);
    tagInfo.uid_hex = TagTechnology::bytesToHexString(&tagInfo.uid);
    return tagInfo;
}
