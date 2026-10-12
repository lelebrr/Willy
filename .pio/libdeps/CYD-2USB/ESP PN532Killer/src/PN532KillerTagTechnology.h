#ifndef PN532KILLERTAGTECHNOLOGY_H
#define PN532KILLERTAGTECHNOLOGY_H

#include <string>
#include <vector>
#include "TagTechnology.h"

class PN532KillerTagTechnology : public TagTechnology {
public:
    typedef struct {
        std::vector<uint8_t> uid;
        std::string afi;
        std::string dsf;
        std::string uid_hex;
    } Iso15693TagInfo;
    Iso15693TagInfo parseIso15693Tag(uint8_t *data, uint8_t dataSize);

    typedef struct {
        std::vector<uint8_t> uid;
        std::string uid_hex;
    } Em4100TagInfo;
    Em4100TagInfo parseEm4100Tag(uint8_t *data, uint8_t dataSize);
};

#endif // PN532KILLERTAGTECHNOLOGY_H
