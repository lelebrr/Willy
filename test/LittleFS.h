#pragma once
#include "Arduino.h"
#include <string>
#include <map>

class File {
    std::string content;
    size_t pos;
    bool valid;
public:
    File() : pos(0), valid(false) {}
    File(std::string c) : content(c), pos(0), valid(true) {}
    operator bool() const { return valid; }
    void close() { valid = false; }
    String readStringUntil(char terminator) {
        if (!valid || pos >= content.size()) return String("");
        size_t next = content.find(terminator, pos);
        if (next == std::string::npos) {
            std::string res = content.substr(pos);
            pos = content.size();
            return String(res);
        }
        std::string res = content.substr(pos, next - pos);
        pos = next + 1;
        return String(res);
    }
};

class LittleFSClass {
public:
    bool begin_result = true;
    std::map<std::string, std::string> files;

    bool begin() { return begin_result; }
    File open(const char* path, const char* mode) {
        if (files.find(path) != files.end()) {
            return File(files[path]);
        }
        return File();
    }
};

extern LittleFSClass LittleFS;
