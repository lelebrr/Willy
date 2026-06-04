#include <iostream>
#include <cassert>
#include "Arduino.h"

// --- Mocks ---

struct LittleFSMock {
    bool begin_result = true;
    int begin_calls = 0;
    int format_calls = 0;

    bool begin(bool formatOnFail = false) {
        begin_calls++;
        return begin_result;
    }

    void format() {
        format_calls++;
        begin_result = true; // Typically format makes the next begin successful
    }
};

LittleFSMock LittleFS;

bool setupSdCard_result = true;
int setupSdCard_calls = 0;

bool setupSdCard() {
    setupSdCard_calls++;
    return setupSdCard_result;
}

struct BruceConfigMock {
    int fromFile_calls = 0;
    bool last_checkFS = false;

    void fromFile(bool checkFS) {
        fromFile_calls++;
        last_checkFS = checkFS;
    }
};

BruceConfigMock bruceConfig;
BruceConfigMock bruceConfigPins;


// --- Include the extracted block ---
// We will extract just the section marked between BEGIN_STORAGE_TEST_EXTRACT and END_STORAGE_TEST_EXTRACT
#include "sd_functions_extracted.cpp"

// Mock for sdcardMounted and SD.exists/SD.mkdir
bool sdcardMounted = true;

struct SDClass {
    bool exists(String path) { return true; }
    void mkdir(String path) {}
};

SDClass SD;

#include "sd_functions_get_hierarchical_path_extracted.cpp"


// --- Tests ---

void reset_mocks() {
    sdcardMounted = true;
    LittleFS.begin_result = true;
    LittleFS.begin_calls = 0;
    LittleFS.format_calls = 0;

    setupSdCard_result = true;
    setupSdCard_calls = 0;

    bruceConfig.fromFile_calls = 0;
    bruceConfig.last_checkFS = false;

    bruceConfigPins.fromFile_calls = 0;
    bruceConfigPins.last_checkFS = false;
}

void test_littlefs_success_sdcard_success() {
    reset_mocks();

    LittleFS.begin_result = true;
    setupSdCard_result = true;

    begin_storage();

    assert(LittleFS.begin_calls == 1);
    assert(LittleFS.format_calls == 0);

    assert(setupSdCard_calls == 1);

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == true);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == true);

    std::cout << "test_littlefs_success_sdcard_success passed\n";
}

void test_littlefs_success_sdcard_fail() {
    reset_mocks();

    LittleFS.begin_result = true;
    setupSdCard_result = false;

    begin_storage();

    assert(LittleFS.begin_calls == 1);
    assert(LittleFS.format_calls == 0);

    assert(setupSdCard_calls == 1);

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == false);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == false);

    std::cout << "test_littlefs_success_sdcard_fail passed\n";
}

void test_littlefs_fail_format_success() {
    reset_mocks();

    LittleFS.begin_result = false;
    setupSdCard_result = true;

    begin_storage();

    assert(LittleFS.begin_calls == 2);
    assert(LittleFS.format_calls == 1);

    assert(setupSdCard_calls == 1);

    assert(bruceConfig.fromFile_calls == 1);
    assert(bruceConfig.last_checkFS == true);

    assert(bruceConfigPins.fromFile_calls == 1);
    assert(bruceConfigPins.last_checkFS == true);

    std::cout << "test_littlefs_fail_format_success passed\n";
}

void test_getHierarchicalPath_no_slash() {
    reset_mocks();
    String result = getHierarchicalPath("/baseDir");
    assert(result == "/baseDir");
    std::cout << "test_getHierarchicalPath_no_slash passed\n";
}

void test_getHierarchicalPath_with_slash() {
    reset_mocks();
    String result = getHierarchicalPath("/baseDir/");
    assert(result == "/baseDir");
    std::cout << "test_getHierarchicalPath_with_slash passed\n";
}

void test_getHierarchicalPath_empty() {
    reset_mocks();
    String result = getHierarchicalPath("");
    assert(result == "");
    std::cout << "test_getHierarchicalPath_empty passed\n";
}

void test_getHierarchicalPath_only_slash() {
    reset_mocks();
    String result = getHierarchicalPath("/");
    assert(result == "");
    std::cout << "test_getHierarchicalPath_only_slash passed\n";
}

void test_getHierarchicalPath_not_mounted() {
    reset_mocks();
    sdcardMounted = false;
    String result = getHierarchicalPath("/baseDir/");
    assert(result == "/baseDir");
    std::cout << "test_getHierarchicalPath_not_mounted passed\n";
}

int main() {
    std::cout << "Running storage tests...\n";

    test_littlefs_success_sdcard_success();
    test_littlefs_success_sdcard_fail();
    test_littlefs_fail_format_success();

    std::cout << "Running getHierarchicalPath tests...\n";
    test_getHierarchicalPath_no_slash();
    test_getHierarchicalPath_with_slash();
    test_getHierarchicalPath_empty();
    test_getHierarchicalPath_only_slash();
    test_getHierarchicalPath_not_mounted();

    std::cout << "All storage tests passed!\n";
    return 0;
}
