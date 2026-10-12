#include <iostream>
#include <ostream>
#include "BerTlv.h"

bool AddTagsWithValues(BerTlv *Tlv) {
    if(Tlv->Add("9F40", "31323334353637AA") != OK) {
        return false;
    }
    if(Tlv->Add("5F30", "31323334353637BB") != OK) {
        return false;
    }
    if(Tlv->Add("9F1F", "313233343536372D") != OK) {
        return false;
    }
    if(Tlv->Add("81", "313233343536FF37") != OK) {
        return false;
    }
    if(Tlv->Add("5A", "31323334A2353637") != OK) {
        return false;
    }
    if(Tlv->Add("DF8126", "3132333F24353637") != OK) {
        return false;
    }
    return true;
}

bool AddTagsWithNestedValues(BerTlv *Tlv) {
    BerTlv NestedTlv;
    if(AddTagsWithValues(&NestedTlv) != OK) {
        return false;
    }
    if(Tlv->Add("FF8105", NestedTlv.GetTlv()) != OK) {
        return false;
    }
    if(Tlv->Add("FF818205", NestedTlv.GetTlv()) != OK) {
        return false;
    }
    if(Tlv->Add("FF8107", NestedTlv.GetTlv()) != OK) {
        return false;
    }
    return true;
}

bool AddBinTagsWithValues(BerTlv *Tlv) {
    unsigned char RawTag[] = {0x9F, 0x40};
    std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
    unsigned char RawValue[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0xAA};
    std::vector<unsigned char> BinValue(RawValue, RawValue + sizeof(RawValue));
    if(Tlv->Add(BinTag, BinValue) != OK) {
        return false;
    }
    return true;
}

bool AddBinTagsWithNestedValues(BerTlv *Tlv) {
    BerTlv NestedTlv;
    if(AddBinTagsWithValues(&NestedTlv) != OK) {
        return false;
    }
    unsigned char RawTag[] = {0xFF, 0x81, 0x05};
    std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
    unsigned char RawValue[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0xAA};
    std::vector<unsigned char> BinValue(RawValue, RawValue + sizeof(RawValue));
    if(Tlv->Add(BinTag,  NestedTlv.GetTlv()) != OK) {
        return false;
    }
    return true;
}

int main(int argc, const char * argv[]) {
    BerTlv Tlv;
    std::string ValueHexString;
    std::vector<unsigned char> Value;
    std::string Tag, HexTlv;
    std::string Output;
    short TestNo = 0;

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Create TLV - add tags and values" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(AddTagsWithValues(&Tlv)) {
        std::cout << "TLV: " << Tlv.GetTlvAsHexString() << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "9F40";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "81";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Dump all tags and values from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.DumpAllTagsAndValues(&Output, true) == OK) {
        std::cout << "Dumped TLV:" << std::endl;
        std::cout << Output << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Add tags with nested values to existing TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(AddTagsWithNestedValues(&Tlv)) {
        std::cout << "TLV: " << Tlv.GetTlvAsHexString() << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "FF818205";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Dump all tags and values and nested values from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.DumpAllTagsAndValues(&Output, true) == OK) {
        std::cout << "Dumped TLV:" << std::endl;
        std::cout << Output << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;
    
    //Clear TLV collection
    Tlv.Clear();

    HexTlv = "9f1a020764FF81050E9f1a0207649f02060000000010009f02060000000010005f2a0207649a031612209c0100950500000000009f37040095055b820220009f2608ab07b02018d87dc89f2701409f100706010a039000009f360200069f6604320040009f03060000000000009f34031f00009f45009f4c0200065a0847617390010100105f3401045f2403221231500b564953412043524544495456009b0200005f201741445654205156534443205445535420434152442030345f28009f07009f0d009f0e009f0f009f51009f3901079f33030068c01f1901089f42008407a00000000310109f5d009f6c0210009f74009f12104361727465204465204372656469746f5f2d009f0607a00000000310105f25009f21031714228f01929f1e082d3433332d313930";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Parse existing TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    Tlv.SetTlv(HexTlv);
    std::cout << "TLV to parse: " << Tlv.GetTlvAsHexString() << std::endl;
    if(Tlv.DumpAllTagsAndValues(&Output, true) == OK) {
        std::cout << "Dumped TLV:" << std::endl;
        std::cout << Output << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "9F02";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "9F34";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    Tag = "FF8105";
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(Tag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    //Clear TLV collection
    Tlv.Clear();

    HexTlv = ValueHexString;
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Parse nested TLV from tag: " << Tag << std::endl;
    std::cout << "*********************************************************" << std::endl;
    Tlv.SetTlv(HexTlv);
    std::cout << "TLV to parse: " << Tlv.GetTlvAsHexString() << std::endl;
    if(Tlv.DumpAllTagsAndValues(&Output, true) == OK) {
        std::cout << "Dumped TLV:" << std::endl;
        std::cout << Output << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    //Clear TLV collection
    Tlv.Clear();

    unsigned char RawTlv[] = {0x9f, 0x1a, 0x02, 0x07, 0x64, 0xFF, 0x81, 0x05, 0x0E, 0x9f, 0x1a, 0x02, 0x07, 0x64, 0x9f, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x9f, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5f, 0x2a, 0x02, 0x07, 0x64}; 
    std::vector<unsigned char> BinTlv(RawTlv, RawTlv + sizeof(RawTlv));
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Parse existing binary TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    Tlv.SetTlv(BinTlv);
    std::cout << "TLV to parse: " << Tlv.GetTlvAsHexString() << std::endl;
    if(Tlv.DumpAllTagsAndValues(&Output, true) == OK) {
        std::cout << "Dumped TLV:" << std::endl;
        std::cout << Output << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    unsigned char RawTag[] = {0x5F, 0x2A};
    std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Retrieve value of binary tag " << Tag << " from TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(Tlv.GetValue(BinTag, &ValueHexString) == OK) {
        std::cout << "Value of tag " << Tag << " is: " << ValueHexString << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    //Clear TLV collection
    Tlv.Clear();

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Create TLV - add binary tags and values" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(AddBinTagsWithValues(&Tlv)) {
        std::cout << "TLV: " << Tlv.GetTlvAsHexString() << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "*********************************************************" << std::endl;
    std::cout << "Test " << ++TestNo << ": Add binary tags with nested values to existing TLV" << std::endl;
    std::cout << "*********************************************************" << std::endl;
    if(AddBinTagsWithNestedValues(&Tlv)) {
        std::cout << "TLV: " << Tlv.GetTlvAsHexString() << std::endl;
        std::cout << "Test " << TestNo << " - PASSED" << std::endl;
    } else {
        std::cout << "Test " << TestNo << " - !!!FAILED!!!" << std::endl;
    }
    std::cout << std::endl;
        
    return 0;
}
