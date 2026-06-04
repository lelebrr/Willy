#include "Arduino.h"
#include <iostream>

int pinMode_calls = 0;
int last_pinMode_pin = -1;
int last_pinMode_mode = -1;
void pinMode(int pin, int mode) {
    pinMode_calls++;
    last_pinMode_pin = pin;
    last_pinMode_mode = mode;
}

// Mock for _setup_gpio
bool _setup_gpio_called = false;
void _setup_gpio() {
    _setup_gpio_called = true;
    // mock behavior: calling pinMode
    pinMode(5, OUTPUT);
}
void _post_setup_gpio() {}

// Mocks for globals in main.cpp
int IO_EXPANDER_ADDRESS = 0x58;
int Wire = 1;

class IOExpander {
public:
    int initialized_address = -1;
    int* initialized_wire = nullptr;
    void init(int addr, int* wire) {
        initialized_address = addr;
        initialized_wire = wire;
    }
};

IOExpander ioExpander;

struct SPIClass {
    int id;
};

class TFT {
public:
    SPIClass spi;
    SPIClass& getSPIinstance() {
        return spi;
    }
};

TFT tft;
SPIClass sdcardSPI;

struct ConfigPins {
    struct { gpio_num_t mosi; } CC1101_bus;
    struct { gpio_num_t mosi; } SDCARD_bus;
};

ConfigPins bruceConfigPins;

SPIClass* last_init_spi = (SPIClass*)-1;
void initCC1101once(SPIClass* spi) {
    last_init_spi = spi;
}

#ifndef TFT_MOSI
#define TFT_MOSI 15
#endif

// Include the extracted setup_gpio
#include "setup_gpio_impl.cpp"

void reset_mocks() {
    _setup_gpio_called = false;
    ioExpander.initialized_address = -1;
    ioExpander.initialized_wire = nullptr;
    last_init_spi = (SPIClass*)-1;
    pinMode_calls = 0;
    last_pinMode_pin = -1;
    last_pinMode_mode = -1;
}

void test_tft_mosi_match() {
    reset_mocks();
    bruceConfigPins.CC1101_bus.mosi = 15;
    bruceConfigPins.SDCARD_bus.mosi = 20;

    setup_gpio();

    if (!_setup_gpio_called) { std::cout << "FAIL: _setup_gpio not called" << std::endl; exit(1); }
    if (pinMode_calls != 1) { std::cout << "FAIL: pinMode not called by mock _setup_gpio" << std::endl; exit(1); }
    if (last_pinMode_pin != 5 || last_pinMode_mode != OUTPUT) { std::cout << "FAIL: pinMode called with wrong args" << std::endl; exit(1); }
    if (ioExpander.initialized_address != IO_EXPANDER_ADDRESS) { std::cout << "FAIL: ioExpander init address mismatch" << std::endl; exit(1); }
    if (last_init_spi != &tft.getSPIinstance()) { std::cout << "FAIL: initCC1101once not called with tft.getSPIinstance()" << std::endl; exit(1); }
    std::cout << "✅ test_tft_mosi_match PASS" << std::endl;
}

void test_sdcard_mosi_match() {
    reset_mocks();
    bruceConfigPins.CC1101_bus.mosi = 20;
    bruceConfigPins.SDCARD_bus.mosi = 20;

    setup_gpio();

    if (last_init_spi != &sdcardSPI) { std::cout << "FAIL: initCC1101once not called with sdcardSPI" << std::endl; exit(1); }
    std::cout << "✅ test_sdcard_mosi_match PASS" << std::endl;
}

void test_no_mosi_match() {
    reset_mocks();
    bruceConfigPins.CC1101_bus.mosi = 10;
    bruceConfigPins.SDCARD_bus.mosi = 20;

    setup_gpio();

    if (last_init_spi != NULL) { std::cout << "FAIL: initCC1101once not called with NULL" << std::endl; exit(1); }
    std::cout << "✅ test_no_mosi_match PASS" << std::endl;
}

int main() {
    std::cout << "🧪 Running setup_gpio tests..." << std::endl;
    test_tft_mosi_match();
    test_sdcard_mosi_match();
    test_no_mosi_match();
    std::cout << "✨ All setup_gpio tests passed!" << std::endl;
    return 0;
}
