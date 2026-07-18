#include "Arduino.h"
#include "../src/core/net_utils.h"
#include <iostream>
#include <cassert>
#include <cstring>

void test_stringToMAC_standard() {
    uint8_t mac[6] = {0};
    stringToMAC("12:34:56:78:9A:BC", mac);
    assert(mac[0] == 0x12);
    assert(mac[1] == 0x34);
    assert(mac[2] == 0x56);
    assert(mac[3] == 0x78);
    assert(mac[4] == 0x9A);
    assert(mac[5] == 0xBC);
    std::cout << "test_stringToMAC_standard passed\n";
}

void test_stringToMAC_lowercase() {
    uint8_t mac[6] = {0};
    stringToMAC("aa:bb:cc:dd:ee:ff", mac);
    assert(mac[0] == 0xAA);
    assert(mac[1] == 0xBB);
    assert(mac[2] == 0xCC);
    assert(mac[3] == 0xDD);
    assert(mac[4] == 0xEE);
    assert(mac[5] == 0xFF);
    std::cout << "test_stringToMAC_lowercase passed\n";
}

void test_stringToMAC_single_digits() {
    uint8_t mac[6] = {0};
    stringToMAC("1:2:3:4:5:6", mac);
    assert(mac[0] == 0x01);
    assert(mac[1] == 0x02);
    assert(mac[2] == 0x03);
    assert(mac[3] == 0x04);
    assert(mac[4] == 0x05);
    assert(mac[5] == 0x06);
    std::cout << "test_stringToMAC_single_digits passed\n";
}

void test_stringToMAC_dash_delimiter() {
    uint8_t mac[6] = {0};
    stringToMAC("12-34-56-78-9A-BC", mac);
    assert(mac[0] == 0x12);
    assert(mac[1] == 0x34);
    assert(mac[2] == 0x56);
    assert(mac[3] == 0x78);
    assert(mac[4] == 0x9A);
    assert(mac[5] == 0xBC);
    std::cout << "test_stringToMAC_dash_delimiter passed\n";
}

int main() {
    std::cout << "Running net_utils tests...\n";
    test_stringToMAC_standard();
    test_stringToMAC_lowercase();
    test_stringToMAC_single_digits();
    test_stringToMAC_dash_delimiter();
    std::cout << "All net_utils tests passed!\n";
    return 0;
}
