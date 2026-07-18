#pragma once
#include <string>
#include <cstdlib>

using std::string;
typedef int gpio_num_t;

class String {
private:
    std::string str;
public:
    String() : str("") {}
    String(const char* s) : str(s ? s : "") {}
    String(std::string s) : str(s) {}
    String(int num) { str = std::to_string(num); }

    int lastIndexOf(char c) const {
        size_t pos = str.find_last_of(c);
        if (pos == std::string::npos) return -1;
        return static_cast<int>(pos);
    }

    String substring(int from, int to = -1) const {
        if (to == -1) return String(str.substr(from));
        return String(str.substr(from, to - from));
    }

    bool endsWith(const String& s) const {
        if (str.length() >= s.str.length()) {
            return (0 == str.compare(str.length() - s.str.length(), s.str.length(), s.str));
        } else {
            return false;
        }
    }

    bool startsWith(const String& s) const {
        if (str.length() >= s.str.length()) {
            return (0 == str.compare(0, s.str.length(), s.str));
        } else {
            return false;
        }
    }

    int length() const {
        return str.length();
    }

    String operator+(const String& s) const {
        return String(str + s.str);
    }
    String operator+(const char* s) const {
        return String(str + s);
    }
    String& operator+=(const String& s) {
        str += s.str;
        return *this;
    }
    String& operator+=(const char* s) {
        str += s;
        return *this;
    }
    bool operator==(const String& s) const {
        return str == s.str;
    }
    bool operator!=(const String& s) const {
        return str != s.str;
    }
    const char* c_str() const {
        return str.c_str();
    }

    friend std::ostream& operator<<(std::ostream& os, const String& s) {
        os << s.str;
        return os;
    }
};

inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs) + rhs.c_str());
}

// Mock for pinMode
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2

extern int pinMode_calls;
extern int last_pinMode_pin;
extern int last_pinMode_mode;

void pinMode(int pin, int mode);

class SerialMock {
public:
    void println(const String& s) {
        // std::cout << s << std::endl;
    }
    void println(const char* s) {
        // std::cout << s << std::endl;
    }
    void printf(const char* fmt, ...) {
        // Dummy
    }
};
extern SerialMock Serial;
