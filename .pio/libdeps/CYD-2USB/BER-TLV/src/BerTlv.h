#ifndef BerTlv_h
#define BerTlv_h

#define OK                       1
#define FAILED                  -1
#define TAG_ERROR               -2
#define VALUE_ERROR             -3
#define TAG_IS_EMPTY            -4
#define VALUE_IS_EMPTY          -5
#define TLV_SIZE_ERROR          -6

#include <vector>
#include <string>

class BerTlv
{
public:
    BerTlv() { _TlvStruct = std::vector<unsigned char>(0); }
    ~BerTlv() { _TlvStruct.clear(); }
    void Clear() { _TlvStruct.clear(); }
    std::vector<unsigned char> GetTlv() { return _TlvStruct; }
    std::string GetTlvAsHexString() { return BinToAscii(_TlvStruct); }
    void SetTlv(std::vector<unsigned char> TlvStruct) { _TlvStruct = TlvStruct; }
    void SetTlv(std::string TlvStruct) { _TlvStruct = AsciiToBin(TlvStruct); }
    short Add(std::string Tag, std::string Value);
    short Add(std::string Tag, std::vector<unsigned char> Value);
    short Add(std::vector<unsigned char> Tag, std::vector<unsigned char> Value);
    short GetValue(std::string Tag, std::string *ValueOfTag, bool CheckNestedTag = true);
    short GetValue(std::string Tag, std::vector<unsigned char> *ValueOfTag, bool CheckNestedTags = true);
    short GetValue(std::vector<unsigned char> Tag, std::string *ValueOfTag, bool CheckNestedTags = true);
    short GetValue(std::vector<unsigned char> Tag, std::vector<unsigned char> *ValueOfTag, bool CheckNestedTags = true);
    short DumpAllTagsAndValues(std::string *Output, bool ParseNestedTags);
    
private:
    std::vector<unsigned char> _TlvStruct;
    
    std::vector<unsigned char> CalcSizeOfValue(std::vector<unsigned char> Value);
    short GetSizeOfValue(size_t StartPosition, int *SizeOfValue);
    short GetTagLength(size_t StartPosition);
    short DumpTlvInsideTag(size_t StartPosition, int Length, std::string *Output);
    bool IsTagNested(size_t StartPosition, short TagSize);
    short GetValueFromTlv(size_t StartPosition, int Length, std::vector<unsigned char> Tag, std::vector<unsigned char> *Output);
    std::vector<unsigned char> AsciiToBin(std::string Ascii);
    std::string BinToAscii(std::vector<unsigned char> BinData, bool comma = false);
    std::string BinToAscii(unsigned char BinData);
    std::string BinToAscii(unsigned char *BinData, int Length, bool comma);
};

#endif /* BerTlv_h */
