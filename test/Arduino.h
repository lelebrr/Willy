#pragma once
#include <string>
#include <cstdlib>

class String {
public:
    std::string str;
    String() {}
    String(const char* s) : str(s ? s : "") {}
    String(const std::string& s) : str(s) {}
    int toInt() const { return std::atoi(str.c_str()); }
    const char* c_str() const { return str.c_str(); }
    bool operator==(const char* other) const { return str == other; }
    bool operator==(const String& other) const { return str == other.str; }
};
