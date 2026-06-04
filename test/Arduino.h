#ifndef ARDUINO_H
#define ARDUINO_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>

class String {
public:
    std::string str;
    String() {}
    String(const char* c) : str(c) {}
    String(const std::string& s) : str(s) {}
    int toInt() const { return std::stoi(str); }
    const char* c_str() const { return str.c_str(); }
    bool operator==(const char* other) const { return str == other; }
};

class File {
public:
    std::string path;
    std::string* content_ptr = nullptr;
    bool is_open = false;
    std::stringstream* ss = nullptr;

    File() : content_ptr(nullptr), is_open(false), ss(nullptr) {}
    File(std::string p, std::string* content) : path(p), content_ptr(content), is_open(true), ss(new std::stringstream()) {}

    // Copy constructor to handle pointers
    File(const File& other) : path(other.path), content_ptr(other.content_ptr), is_open(other.is_open) {
        if (other.ss) {
            ss = new std::stringstream(other.ss->str());
            ss->seekg(other.ss->tellg());
            ss->seekp(other.ss->tellp());
        } else {
            ss = nullptr;
        }
    }

    File& operator=(const File& other) {
        if (this != &other) {
            path = other.path;
            content_ptr = other.content_ptr;
            is_open = other.is_open;
            if (ss) delete ss;
            if (other.ss) {
                ss = new std::stringstream(other.ss->str());
                ss->seekg(other.ss->tellg());
                ss->seekp(other.ss->tellp());
            } else {
                ss = nullptr;
            }
        }
        return *this;
    }

    ~File() {
        if (ss) delete ss;
    }

    operator bool() const { return is_open; }

    String readStringUntil(char terminator) {
        std::string result;
        if (ss) {
            std::getline(*ss, result, terminator);
        }
        return String(result);
    }

    void println(int val) {
        if (content_ptr) *content_ptr += std::to_string(val) + "\n";
    }

    void println(const char* val) {
        if (content_ptr) *content_ptr += std::string(val) + "\n";
    }

    template<typename... Args>
    void printf(const char* format, Args... args) {
        if (content_ptr) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), format, args...);
            *content_ptr += buffer;
        }
    }

    void close() {
        is_open = false;
    }
};

class LittleFSClass {
public:
    std::string file_content = "";
    bool begin_result = true;

    bool begin() { return begin_result; }

    File open(const char* path, const char* mode) {
        if (std::string(mode) == "w") {
            file_content = "";
            return File(path, &file_content);
        } else if (std::string(mode) == "r") {
            File f(path, &file_content);
            f.ss->str(file_content);
            f.ss->clear();
            return f;
        }
        return File();
    }
};

extern LittleFSClass LittleFS;

#endif
