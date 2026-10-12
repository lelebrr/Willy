#ifndef TAGTECHNOLOGY_H
#define TAGTECHNOLOGY_H

#include <string>
#include <vector>

class TagTechnology {
public:
    virtual ~TagTechnology() = default;

    std::string bytesToHexString(std::vector<uint8_t> *data, bool reverse = false);

    typedef struct {
        std::vector<uint8_t> atqa;
        uint8_t sak;
        uint8_t uidSize;
        std::vector<uint8_t> uid;
        std::string uid_hex;
        std::string sak_hex;
        std::string atqa_hex;
        std::string type;
    } Iso14aTagInfo;

    Iso14aTagInfo parseIso14aTag(uint8_t *data, uint8_t dataSize);
    std::string getIso14aTagType(uint8_t sak);
};

#endif // TAGTECHNOLOGY_H
