#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdint>

// Mocking Arduino String
class String {
public:
    std::string s;

    String() {}
    String(const char* str) : s(str) {}
    String(const std::string& str) : s(str) {}
    String(char c) : s(1, c) {}

    size_t length() const { return s.length(); }
    const char* c_str() const { return s.c_str(); }
    char& operator[](size_t index) { return s[index]; }
    const char& operator[](size_t index) const { return s[index]; }

    String& operator+=(const String& rhs) {
        s += rhs.s;
        return *this;
    }

    String& operator+=(const char* rhs) {
        s += rhs;
        return *this;
    }

    void trim() {
        if (s.empty()) return;
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            s.clear();
            return;
        }
        size_t last = s.find_last_not_of(" \t\r\n");
        s = s.substr(first, last - first + 1);
    }

    void reserve(size_t n) {
        s.reserve(n);
    }

    bool startsWith(const String& prefix) const {
        return s.substr(0, prefix.length()) == prefix.s;
    }

    bool endsWith(const String& suffix) const {
        if (s.length() < suffix.length()) return false;
        return s.substr(s.length() - suffix.length()) == suffix.s;
    }

    String substring(size_t from, size_t to = -1) const {
        if (to == (size_t)-1) return String(s.substr(from));
        return String(s.substr(from, to - from));
    }

    bool operator==(const String& other) const {
        return s == other.s;
    }

    bool operator!=(const String& other) const {
        return s != other.s;
    }

    friend std::ostream& operator<<(std::ostream& os, const String& str) {
        return os << str.s;
    }
};

String operator+(const String& lhs, const String& rhs) {
    return String(lhs.s + rhs.s);
}

String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs) + rhs.s);
}

String operator+(const String& lhs, const char* rhs) {
    return String(lhs.s + std::string(rhs));
}

// Mock esp_rom_md5
typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
} md5_context_t;

void esp_rom_md5_init(md5_context_t *context) {}
void esp_rom_md5_update(md5_context_t *context, const uint8_t *input, uint32_t inputLen) {}
void esp_rom_md5_final(uint8_t digest[16], md5_context_t *context) {
    // mock md5 sum just returns a constant sequence for test
    for (int i = 0; i < 16; i++) {
        digest[i] = i;
    }
}

// Extract testing blocks
#include "passwords_extracted.cpp"

void test_encryptString_empty() {
    String plaintext = "";
    String password = "password";
    String result = encryptString(plaintext, password);

    assert(result.startsWith("Filetype: Bruce Encrypted File\n"));
    assert(result.endsWith("Data: \n"));
    std::cout << "✅ test_encryptString_empty passed" << std::endl;
}

void test_encryptString_basic() {
    String plaintext = "hello"; // 'h' ^ 0, 'e' ^ 1, 'l' ^ 2, 'l' ^ 3, 'o' ^ 4
    // hex ascii: h(104/68) ^ 0 = 68, e(101/65) ^ 1 = 64, l(108/6C) ^ 2 = 6E, l(108/6C) ^ 3 = 6F, o(111/6F) ^ 4 = 6B
    String password = "pass";
    String result = encryptString(plaintext, password);

    assert(result.startsWith("Filetype: Bruce Encrypted File\n"));
    // check Data line
    size_t dataPos = result.s.find("Data: ");
    assert(dataPos != std::string::npos);
    std::string dataStr = result.s.substr(dataPos + 6);
    // Trim newline
    if (!dataStr.empty() && dataStr.back() == '\n') dataStr.pop_back();

    // Expected: 68 64 6E 6F 6B
    assert(dataStr == "68 64 6E 6F 6B");

    std::cout << "✅ test_encryptString_basic passed" << std::endl;
}

void test_encryptString_special_chars() {
    // Testing characters that result in < 16 byte (to test the 0-padding `if (c < 16)`)
    // md5Hash[0] = 0, so if plaintext is 0x0A ('\n'), it will result in 0x0A.
    String plaintext(std::string(1, '\n'));
    String password = "pass";
    String result = encryptString(plaintext, password);

    size_t dataPos = result.s.find("Data: ");
    std::string dataStr = result.s.substr(dataPos + 6);
    if (!dataStr.empty() && dataStr.back() == '\n') dataStr.pop_back();

    assert(dataStr == "0A");

    std::cout << "✅ test_encryptString_special_chars passed" << std::endl;
}

int main() {
    std::cout << "🧪 Running passwords tests..." << std::endl;
    test_encryptString_empty();
    test_encryptString_basic();
    test_encryptString_special_chars();
    std::cout << "✨ All passwords tests passed!" << std::endl;
    return 0;
}
