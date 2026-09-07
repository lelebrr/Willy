#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

// Mocks
bool returnToMenu = false;

void drawMainBorderWithTitle(const char* title) {}
void padprintln(const char* text) {}
void padprint(const char* text) {}
void delay(int ms) {}

enum Key { EscPress, SelPress };

int check_call_count = 0;
bool check(Key key) {
    check_call_count++;
    return true; // immediately exit loop
}

struct TFT {
    std::string output;
    void print(const char* s) {
        output += s;
    }
    void printf(const char* format, int val) {
        char buf[64];
        snprintf(buf, sizeof(buf), format, val);
        output += buf;
    }
} tft;

struct BruceConfigPins {
    struct {
        int sda = 1;
        int scl = 2;
    } i2c_bus;
} bruceConfigPins;

struct WireMock {
    int active_address = -1;
    std::vector<int> existing_addresses;

    void begin(int sda, int scl) {}
    void beginTransmission(int address) {
        active_address = address;
    }
    int endTransmission() {
        for (int addr : existing_addresses) {
            if (addr == active_address) return 0;
        }
        return 2; // Not found
    }
} Wire;

#define FIRST_I2C_ADDRESS 0x01
#define LAST_I2C_ADDRESS 0x7F

// Include the extracted code
#include "i2c_finder_extracted.cpp"

void run_test(const char* name, bool (*test_func)()) {
    std::cout << "Running " << name << "... ";
    if (test_func()) {
        std::cout << "PASSED\n";
    } else {
        std::cout << "FAILED\n";
        exit(1);
    }
}

bool test_find_first_i2c_address_found() {
    Wire.existing_addresses = {0x3C};
    return find_first_i2c_address() == 0x3C;
}

bool test_find_first_i2c_address_not_found() {
    Wire.existing_addresses = {};
    return find_first_i2c_address() == 0;
}

bool test_check_i2c_address_found() {
    Wire.existing_addresses = {0x42};
    return check_i2c_address(0x42) == true;
}

bool test_check_i2c_address_not_found() {
    Wire.existing_addresses = {0x42};
    return check_i2c_address(0x43) == false;
}

bool test_find_i2c_addresses_output() {
    Wire.existing_addresses = {0x10, 0x20};
    tft.output = "";
    returnToMenu = false;
    check_call_count = 0;

    find_i2c_addresses();

    // Output should contain "0x10, 0x20"
    return tft.output == "0x10, 0x20";
}

int main() {
    run_test("test_find_first_i2c_address_found", test_find_first_i2c_address_found);
    run_test("test_find_first_i2c_address_not_found", test_find_first_i2c_address_not_found);
    run_test("test_check_i2c_address_found", test_check_i2c_address_found);
    run_test("test_check_i2c_address_not_found", test_check_i2c_address_not_found);
    run_test("test_find_i2c_addresses_output", test_find_i2c_addresses_output);

    std::cout << "All I2C finder tests passed!\n";
    return 0;
}
