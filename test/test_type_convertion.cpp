#include <iostream>
#include <cassert>
#include "Arduino.h"

// Declare the function to test
String hexStrToBinStr(const String &hexStr);

void test_hexStrToBinStr_empty() {
    String result = hexStrToBinStr("");
    assert(result == "");
}

void test_hexStrToBinStr_single_char() {
    String result = hexStrToBinStr("A");
    assert(result == ""); // Needs 2 chars to form a byte
}

void test_hexStrToBinStr_valid_byte() {
    String result = hexStrToBinStr("0A");
    assert(result == "00001010");
}

void test_hexStrToBinStr_multiple_bytes() {
    String result = hexStrToBinStr("FF00");
    assert(result == "1111111100000000");
}

void test_hexStrToBinStr_with_spaces() {
    String result = hexStrToBinStr("FF 00");
    assert(result == "1111111100000000"); // Spaces are ignored
}

void test_hexStrToBinStr_mixed_case() {
    String result = hexStrToBinStr("aB");
    assert(result == "10101011");
}

void test_hexStrToBinStr_invalid_chars() {
    String result = hexStrToBinStr("0x0A");
    // '0' -> "0"
    // 'x' -> ignored
    // '0' -> "00" => outputs 00000000
    // 'A' -> "A" => not a full byte
    assert(result == "00000000");

    result = hexStrToBinStr("Z12Y3");
    // 'Z' -> ignored
    // '1' -> "1"
    // '2' -> "12" => 00010010
    // 'Y' -> ignored
    // '3' -> "3"
    assert(result == "00010010");
}

int main() {
    std::cout << "Running type_convertion tests...\n";
    test_hexStrToBinStr_empty();
    test_hexStrToBinStr_single_char();
    test_hexStrToBinStr_valid_byte();
    test_hexStrToBinStr_multiple_bytes();
    test_hexStrToBinStr_with_spaces();
    test_hexStrToBinStr_mixed_case();
    test_hexStrToBinStr_invalid_chars();
    std::cout << "All type_convertion tests passed!\n";
    return 0;
}
