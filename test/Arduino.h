#ifndef ARDUINO_H
#define ARDUINO_H
#include <string>
#include <iostream>
#include <vector>

using std::string;
typedef int gpio_num_t;

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}

    bool endsWith(const String& suffix) const {
        if (length() >= suffix.length()) {
            return compare(length() - suffix.length(), suffix.length(), suffix) == 0;
        }
        return false;
    }

    String substring(size_t from, size_t to = std::string::npos) const {
        if (to == std::string::npos || to > length()) {
            to = length();
        }
        if (from >= to) return "";
        return substr(from, to - from);
    }
};

// Mock for pinMode
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2

extern int pinMode_calls;
extern int last_pinMode_pin;
extern int last_pinMode_mode;

void pinMode(int pin, int mode);

#endif
