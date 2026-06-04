#ifndef FS_H
#define FS_H

#include "Arduino.h"
#include <set>
#include <string>

#define FILE_WRITE 1

struct File {
    String name;
    File() : name("") {}
    File(String n) : name(n) {}
};

struct FSMock {
    std::set<std::string> existing_files;
    std::set<std::string> existing_dirs;

    int mkdir_calls = 0;
    String last_mkdir_path;

    int open_calls = 0;
    String last_open_path;
    int last_open_mode = 0;

    bool exists(String path) {
        return existing_files.count(path.c_str()) > 0 || existing_dirs.count(path.c_str()) > 0;
    }

    void mkdir(String path) {
        mkdir_calls++;
        last_mkdir_path = path;
        existing_dirs.insert(path.c_str());
    }

    File open(String path, int mode) {
        open_calls++;
        last_open_path = path;
        last_open_mode = mode;
        existing_files.insert(path.c_str());
        return File(path);
    }

    void add_file(String path) {
        existing_files.insert(path.c_str());
    }

    void add_dir(String path) {
        existing_dirs.insert(path.c_str());
    }
};

typedef FSMock FS;

#endif
