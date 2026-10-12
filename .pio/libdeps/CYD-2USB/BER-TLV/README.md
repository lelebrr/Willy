# BER-TLV, Basic Encoding Rules - Tag Length Value parser and serializer
Simple BER-TLV parser and serializer library implemented in C++98. It follows BER-TLV rules from ISO 7816 - Annex D: Use of Basic Encoding Rules ASN.1.


## Supported functionalities
1. Create new TLV objects
2. Append existing TLV objects
3. Add primitive data objects
4. Add constructed data objects
5. Parse primitive data objects
6. Parse constructed data objects
7. Dump all data from TLV


## Usage
Add [BerTlv.cpp](https://github.com/huckor/BER-TLV/blob/master/src/BerTlv.cpp) and [BerTlv.h](https://github.com/huckor/BER-TLV/blob/master/src/BerTlv.h) files into your project and compile along with it.


### How to use parser
You can observe usage from file [tests.cpp](https://github.com/huckor/BER-TLV/blob/master/src/tests.cpp) or follow below examples.

1. Add BerTlv.cpp and BerTlv.h to your project.

2. Follow below example using hex string approach.
```cpp
//Create BerTlv object
BerTlv Tlv;

//Add your existing TLV
std::string HexTlv = "9f1a020764FF81050E9f1a0207649f02060000000010009f02060000000010005f2a020764";
Tlv.SetTlv(HexTlv);

//Get value of a tag
std::string Value;
if(Tlv.GetValue("9F02", &Value) == OK) {
    std::cout << Value;
}
```

3. Or follow below example using binary collection approach.
```cpp
//Create BerTlv object
BerTlv Tlv;

//Add your existing TLV
unsigned char RawTlv[] = {0x9f, 0x1a, 0x02, 0x07, 0x64, 0xFF, 0x81, 0x05, 0x0E, 0x9f, 0x1a, 0x02, 0x07, 0x64, 0x9f, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x9f, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5f, 0x2a, 0x02, 0x07, 0x64}; 
std::vector<unsigned char> BinTlv(RawTlv, RawTlv + sizeof(RawTlv));
Tlv.SetTlv(BinTlv);

//Get value of a tag
unsigned char RawTag[] = {0x5F, 0x2A};
std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
std::string Value;
if(Tlv.GetValue(BinTag, &Value) == OK) {
    std::cout << Value;
}
```


### How to use serializer
You can observe usage from file [tests.cpp](https://github.com/huckor/BER-TLV/blob/master/src/tests.cpp) or follow below examples.

1. Add BerTlv.cpp and BerTlv.h to your project.

2. Follow below example using hex string approach.
```cpp
//Create BerTlv object
BerTlv Tlv;

//Add primitive data object
if(Tlv.Add("9F40", "31323334353637AA") != OK) {
    std::cout << "Error!";
}

//Add constructed data object
BerTlv NestedTlv;
if(NestedTlv.Add("5F30", "31323334353637BB") != OK) {
    std::cout << "Error!";
}
if(NestedTlv.Add("9F1F", "313233343536372D") != OK) {
    std::cout << "Error!";
}
if(Tlv.Add("FF8105", NestedTlv.GetTlv()) != OK) {
    std::cout << "Error!";
}

//Get TLV
std::cout << Tlv.GetTlvAsHexString();
```

3. Or follow below example using binary collection approach.
```cpp
//Create BerTlv object
BerTlv Tlv;

//Add primitive data object
unsigned char RawTag[] = {0x9F, 0x40};
std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
unsigned char RawValue[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0xAA};
std::vector<unsigned char> BinValue(RawValue, RawValue + sizeof(RawValue));
if(Tlv->Add(BinTag, BinValue) != OK) {
    std::cout << "Error!";
}

//Add constructed data object
BerTlv NestedTlv;
if(AddBinTagsWithValues(&NestedTlv) != OK) {
    std::cout << "Error!";
}
unsigned char RawTag[] = {0xFF, 0x81, 0x05};
std::vector<unsigned char> BinTag(RawTag, RawTag + sizeof(RawTag));
unsigned char RawValue[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0xAA};
std::vector<unsigned char> BinValue(RawValue, RawValue + sizeof(RawValue));
if(Tlv->Add(BinTag,  NestedTlv.GetTlv()) != OK) {
    std::cout << "Error!";
}

std::vector<unsigned char> OutTlv = Tlv.GetTlv();
```


## How to Contribute

Contributions are highly encouraged and appreciated! If you have any ideas, suggestions, or bug fixes, please feel free to contribute. Here's how you can get started:

1. Fork this repository to your GitHub account.
2. Make the desired changes or add new features to your forked repository.
3. Write test cases for your changes.
4. Submit a pull request, explaining the changes you have made and why they are beneficial.
5. I will review your pull request as soon as possible and provide feedback if needed.
6. Once your changes are approved, they will be merged into the main repository.
7. Congratulations! You have successfully contributed to this project.

## Code Style and Guidelines

To maintain code consistency and readability, please adhere to the following guidelines:

- Write clean and self-explanatory code.
- Use meaningful variable and function names.
- Follow the established coding style and formatting.
- Add comments where necessary to improve code understanding.

## Issues and Bug Reports

If you come across any issues or bugs while using this project, please report them using the GitHub issue tracker. Include a detailed description of the problem, steps to reproduce it, and any relevant information that might help in resolving the issue.

## Support

If you need any assistance or have questions regarding this project, feel free to reach out to me. I'll do my best to help you.

## Spread the Word

If you find this project helpful or interesting, consider giving it a star on GitHub. Also, sharing it with others who might benefit from it would be greatly appreciated.

Thank you for your interest in this project.