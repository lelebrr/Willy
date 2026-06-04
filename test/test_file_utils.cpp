#include "Arduino.h"
#include "FS.h"
#include "../src/core/file_utils.h"

#include <iostream>
#include <cassert>

// Define variables for mocks
SerialMock Serial;
int pinMode_calls = 0;
int last_pinMode_pin = 0;
int last_pinMode_mode = 0;
void pinMode(int pin, int mode) {}

void test_create_new_file_normal() {
    FSMock fs_impl;
    FS* fs_ptr = &fs_impl;

    File f = createNewFile(fs_ptr, "/logs", "test.txt");

    assert(f.name == "/logs/test.txt");
    assert(fs_impl.mkdir_calls == 1);
    assert(fs_impl.last_mkdir_path == "/logs");
    assert(fs_impl.open_calls == 1);
    assert(fs_impl.last_open_path == "/logs/test.txt");
    assert(fs_impl.last_open_mode == FILE_WRITE);

    std::cout << "test_create_new_file_normal passed\n";
}

void test_create_new_file_trailing_slash() {
    FSMock fs_impl;
    FS* fs_ptr = &fs_impl;

    File f = createNewFile(fs_ptr, "/logs/", "test.txt");

    assert(f.name == "/logs/test.txt");
    assert(fs_impl.mkdir_calls == 1);
    assert(fs_impl.last_mkdir_path == "/logs");

    std::cout << "test_create_new_file_trailing_slash passed\n";
}

void test_create_new_file_existing_dir() {
    FSMock fs_impl;
    fs_impl.add_dir("/logs");
    FS* fs_ptr = &fs_impl;

    File f = createNewFile(fs_ptr, "/logs", "test.txt");

    assert(f.name == "/logs/test.txt");
    assert(fs_impl.mkdir_calls == 0);

    std::cout << "test_create_new_file_existing_dir passed\n";
}

void test_create_new_file_existing_file() {
    FSMock fs_impl;
    fs_impl.add_file("/logs/test.txt");
    FS* fs_ptr = &fs_impl;

    File f = createNewFile(fs_ptr, "/logs", "test.txt");

    assert(f.name == "/logs/test_1.txt");

    std::cout << "test_create_new_file_existing_file passed\n";
}

void test_create_new_file_multiple_existing_files() {
    FSMock fs_impl;
    fs_impl.add_file("/logs/test.txt");
    fs_impl.add_file("/logs/test_1.txt");
    fs_impl.add_file("/logs/test_2.txt");
    FS* fs_ptr = &fs_impl;

    File f = createNewFile(fs_ptr, "/logs", "test.txt");

    assert(f.name == "/logs/test_3.txt");

    std::cout << "test_create_new_file_multiple_existing_files passed\n";
}

int main() {
    std::cout << "Running file_utils tests...\n";
    test_create_new_file_normal();
    test_create_new_file_trailing_slash();
    test_create_new_file_existing_dir();
    test_create_new_file_existing_file();
    test_create_new_file_multiple_existing_files();
    std::cout << "All file_utils tests passed!\n";
    return 0;
}
