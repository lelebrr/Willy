#include <iostream>
#include <cassert>
#include <string>
#include <cstdint>
#include "Arduino.h"

// --- Mocks ---

#define HIGH 1
#define OUTPUT 1
#define GPIO_NUM_NC -1
#define TFT_MOSI -1

typedef int gpio_num_t;

int pinMode_calls = 0;
void pinMode(int pin, int mode) {
    pinMode_calls++;
}

int digitalWrite_calls = 0;
void digitalWrite(int pin, int val) {
    digitalWrite_calls++;
}

void delay(int ms) {}

struct SPIClassMock {
    int begin_calls = 0;
    void begin(int sck, int miso, int mosi, int cs) {
        begin_calls++;
    }
    void end() {}
};

SPIClassMock sdcardSPI;
SPIClassMock SPI;

#define FILE_READ 0
#define FILE_WRITE 1

struct File {
    bool is_valid = false;
    int _size = 0;
    int _read_calls = 0;
    int _write_calls = 0;

    operator bool() const { return is_valid; }
    int size() { return _size; }
    size_t read(uint8_t* buf, size_t size) {
        _read_calls++;
        if (_read_calls == 1) return 10;
        return 0; // Return bytes read once, then 0
    }
    size_t write(const uint8_t* buf, size_t size) {
        _write_calls++;
        return size;
    }
    void close() {}
};

struct FS {
    bool open_result = true;
    int open_calls = 0;
    File open(String path, int mode = FILE_READ) {
        open_calls++;
        File f;
        f.is_valid = open_result;
        f._size = 10; // Some default size
        return f;
    }
}; // Dummy base class or just dummy class to satisfy types

struct SDClassMock : public FS {
    bool begin_result = true;
    int begin_calls = 0;
    int mkdir_calls = 0;

    bool begin(const char* mountpoint, bool formatOnFail) {
        begin_calls++;
        return begin_result;
    }

    bool begin(int cs) {
        begin_calls++;
        return begin_result;
    }

    bool begin(int cs, SPIClassMock& spi, int freq=0) {
        begin_calls++;
        return begin_result;
    }

    void mkdir(const char* path) {
        mkdir_calls++;
    }
};

SDClassMock SD;

// SerialMock and Serial are defined in Arduino.h
SerialMock Serial;

struct BusPin {
    int sck = 1;
    int miso = 2;
    int mosi = 3;
    int cs = 4;
};

struct BruceConfigPinsMock {
    BusPin SDCARD_bus;
    BusPin CC1101_bus;
    BusPin NRF24_bus;
    BusPin W5500_bus;

    int fromFile_calls = 0;
    bool last_checkFS = false;

    void fromFile(bool checkFS) {
        fromFile_calls++;
        last_checkFS = checkFS;
    }
};

BruceConfigPinsMock bruceConfigPins;

bool sdcardMounted = false;

struct FSMock : public FS {
    bool begin_result = true;
    int begin_calls = 0;
    int format_calls = 0;

    int totalBytes_val = 8192;
    int usedBytes_val = 0;

    bool begin(bool formatOnFail = false) {
        begin_calls++;
        return begin_result;
    }

    void format() {
        format_calls++;
        begin_result = true; // Typically format makes the next begin successful
    }

    int totalBytes() { return totalBytes_val; }
    int usedBytes() { return usedBytes_val; }
};

FSMock LittleFS;

int displayError_calls = 0;
void displayError(const char* msg, bool flag) {
    displayError_calls++;
}

struct TFTMock {
    void drawArc(int, int, int, int, int, int, uint32_t, uint32_t, bool) {}
};
TFTMock tft;

uint32_t ALCOLOR = 0;
int tftWidth = 320;
int tftHeight = 240;

struct BruceConfigMock {
    int fromFile_calls = 0;
    bool last_checkFS = false;
    uint32_t bgColor = 0;

    void fromFile(bool checkFS) {
        fromFile_calls++;
        last_checkFS = checkFS;
    }
};

BruceConfigMock bruceConfig;


// --- Extracted blocks ---
#include "sd_setup_extracted.cpp"
#include "sd_fs_extracted.cpp"

bool setupSdCard_result = true;
int setupSdCard_calls = 0;
// We must hide setupSdCard() from the storage test if we want to mock it.
// The `begin_storage` in `sd_functions_extracted.cpp` uses `setupSdCard()`.
// Since we have `#include "sd_setup_extracted.cpp"` which defines `setupSdCard()`,
// we cannot redefine it. Let's rely on the real `setupSdCard` for `begin_storage`,
// or avoid including `sd_setup_extracted.cpp` before `sd_functions_extracted.cpp`.
// It's better to just use the actual `setupSdCard` function and set the mock parameters!
// Oh wait, `sd_functions_extracted.cpp` (which is `begin_storage`) calls `setupSdCard()`.

#include "sd_functions_extracted.cpp"
#include "sd_copy_to_fs_extracted.cpp"

// --- Tests ---

void reset_mocks() {
    pinMode_calls = 0;
    digitalWrite_calls = 0;
    sdcardSPI.begin_calls = 0;
    SD.begin_result = true;
    SD.begin_calls = 0;
    SD.mkdir_calls = 0;
    sdcardMounted = false;

    bruceConfigPins.SDCARD_bus.sck = 1;
    bruceConfigPins.SDCARD_bus.cs = 4;
    bruceConfigPins.CC1101_bus.cs = -1;
    bruceConfigPins.NRF24_bus.cs = -1;
    bruceConfigPins.W5500_bus.cs = -1;

    LittleFS.begin_result = true;
    LittleFS.begin_calls = 0;
    LittleFS.format_calls = 0;
    LittleFS.totalBytes_val = 8192;
    LittleFS.usedBytes_val = 0;

    bruceConfig.fromFile_calls = 0;
    bruceConfig.last_checkFS = false;

    bruceConfigPins.fromFile_calls = 0;
    bruceConfigPins.last_checkFS = false;

    displayError_calls = 0;
}

void test_copyToFs_sdcard_mount_fails() {
    reset_mocks();
    sdcardMounted = false;
    bruceConfigPins.SDCARD_bus.sck = -1; // Force setupSdCard to fail

    bool result = copyToFs(SD, LittleFS, "/test.txt", false);

    assert(result == false);
    assert(sdcardMounted == false);
    std::cout << "test_copyToFs_sdcard_mount_fails passed\n";
}

void test_copyToFs_success() {
    reset_mocks();
    sdcardMounted = true;
    LittleFS.begin_result = true;
    SD.open_result = true;
    LittleFS.open_result = true;
    LittleFS.totalBytes_val = 10000;
    LittleFS.usedBytes_val = 0;

    bool result = copyToFs(SD, LittleFS, "/test.txt", false);

    assert(result == true);
    std::cout << "test_copyToFs_success passed\n";
}

// Storage extraction tests

void test_littlefs_success_sdcard_success() {
    reset_mocks();

    LittleFS.begin_result = true;
    // setupSdCard will succeed naturally because default pin states are good and SD mock succeeds

    begin_storage();

    assert(LittleFS.begin_calls == 1);
    assert(LittleFS.format_calls == 0);

    assert(SD.begin_calls == 1);

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == true);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == true);

    std::cout << "test_littlefs_success_sdcard_success passed\n";
}

void test_littlefs_success_sdcard_fail() {
    reset_mocks();

    LittleFS.begin_result = true;
    // setupSdCard will fail if sck is -1
    bruceConfigPins.SDCARD_bus.sck = -1;

    begin_storage();

    assert(LittleFS.begin_calls == 1);
    assert(LittleFS.format_calls == 0);

    assert(SD.begin_calls == 0); // Didn't even try to mount

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == false);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == false);

    std::cout << "test_littlefs_success_sdcard_fail passed\n";
}

void test_littlefs_fail_format_success() {
    reset_mocks();

    LittleFS.begin_result = false;
    // setupSdCard will succeed

    begin_storage();

    assert(LittleFS.begin_calls == 2);
    assert(LittleFS.format_calls == 1);

    assert(SD.begin_calls == 1);

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == true);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == true);

    std::cout << "test_littlefs_fail_format_success passed\n";
}

// SD Functions extraction tests

void test_setupSdCard_sck_negative() {
    reset_mocks();
    bruceConfigPins.SDCARD_bus.sck = -1;

    bool result = setupSdCard();

    assert(result == false);
    assert(sdcardMounted == false);
    assert(SD.begin_calls == 0);
    std::cout << "test_setupSdCard_sck_negative passed\n";
}

void test_setupSdCard_already_mounted() {
    reset_mocks();
    sdcardMounted = true;

    bool result = setupSdCard();

    assert(result == true);
    assert(SD.begin_calls == 0);
    std::cout << "test_setupSdCard_already_mounted passed\n";
}

void test_setupSdCard_success() {
    reset_mocks();

    bool result = setupSdCard();

    assert(result == true);
    assert(sdcardMounted == true);
    assert(SD.begin_calls == 1);
    assert(sdcardSPI.begin_calls == 1);
    assert(SD.mkdir_calls == 4);
    assert(pinMode_calls > 0);
    assert(digitalWrite_calls > 0);
    std::cout << "test_setupSdCard_success passed\n";
}

void test_setupSdCard_fail_first_fallback() {
    reset_mocks();

    SD.begin_result = false;

    bool result = setupSdCard();

    assert(result == false);
    assert(sdcardMounted == false);
    assert(SD.begin_calls == 3); // Tries 3 frequencies
    std::cout << "test_setupSdCard_fail passed\n";
}

void test_checkLittleFsSize_full() {
    LittleFS.totalBytes_val = 8192;
    LittleFS.usedBytes_val = 8192 - 4000; // < 4096 remaining

    bool result = checkLittleFsSize();
    assert(result == false);

    bool resultNM = checkLittleFsSizeNM();
    assert(resultNM == false);

    std::cout << "test_checkLittleFsSize_full passed\n";
}

void test_checkLittleFsSize_ok() {
    LittleFS.totalBytes_val = 8192;
    LittleFS.usedBytes_val = 8192 - 5000; // > 4096 remaining

    bool result = checkLittleFsSize();
    assert(result == true);

    bool resultNM = checkLittleFsSizeNM();
    assert(resultNM == true);

    std::cout << "test_checkLittleFsSize_ok passed\n";
}

void test_getFsStorage() {
    FS* fs_ptr = nullptr;

    // Case 1: SD card mounted
    sdcardMounted = true;
    LittleFS.usedBytes_val = 0; // Plenty of space
    bool res = getFsStorage(fs_ptr);
    assert(res == true);
    assert(fs_ptr == &SD);

    // Case 2: SD card not mounted, LittleFS has space
    sdcardMounted = false;
    LittleFS.totalBytes_val = 8192;
    LittleFS.usedBytes_val = 8192 - 5000; // > 4096
    res = getFsStorage(fs_ptr);
    assert(res == true);
    assert(fs_ptr == &LittleFS);

    // Case 3: SD card not mounted, LittleFS full
    sdcardMounted = false;
    LittleFS.totalBytes_val = 8192;
    LittleFS.usedBytes_val = 8192 - 4000; // < 4096
    res = getFsStorage(fs_ptr);
    assert(res == false);

    std::cout << "test_getFsStorage passed\n";
}


int main() {
    std::cout << "Running storage core tests...\n";
    test_littlefs_success_sdcard_success();
    test_littlefs_success_sdcard_fail();
    test_littlefs_fail_format_success();

    std::cout << "Running setupSdCard tests...\n";
    test_setupSdCard_sck_negative();
    test_setupSdCard_already_mounted();
    test_setupSdCard_success();
    test_setupSdCard_fail_first_fallback();

    std::cout << "Running LittleFS size tests...\n";
    test_checkLittleFsSize_full();
    test_checkLittleFsSize_ok();

    std::cout << "Running getFsStorage tests...\n";
    test_getFsStorage();

    std::cout << "Running copyToFs tests...\n";
    test_copyToFs_sdcard_mount_fails();
    test_copyToFs_success();

    std::cout << "All tests passed!\n";
    return 0;
}
