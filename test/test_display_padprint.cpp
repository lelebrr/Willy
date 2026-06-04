#include <iostream>
#include <cassert>
#include "Arduino.h"

// --- Mocks ---

#define BORDER_PAD_X 10

struct TFTMock {
    int cursor_x = 0;
    int cursor_y = 0;
    String last_printed_string = "";
    const char* last_printed_chars = nullptr;
    char last_printed_char = 0;
    int last_printed_base = 0;
    long long last_printed_number = 0;
    double last_printed_double = 0;
    int last_printed_digits = 0;

    void setCursor(int x, int y) {
        cursor_x = x;
        cursor_y = y;
    }

    int getCursorX() {
        return cursor_x;
    }

    int getCursorY() {
        return cursor_y;
    }

    void print(const String& s) {
        last_printed_string = s;
    }

    void print(const char str[]) {
        last_printed_chars = str;
    }

    void print(char c) {
        last_printed_char = c;
    }

    void print(unsigned char b, int base) {
        last_printed_number = b;
        last_printed_base = base;
    }

    void print(int n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(unsigned int n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(long n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(unsigned long n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(long long n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(unsigned long long n, int base) {
        last_printed_number = n;
        last_printed_base = base;
    }

    void print(double n, int digits) {
        last_printed_double = n;
        last_printed_digits = digits;
    }
};

TFTMock tft;

// --- Include the extracted block ---
#include "display_padprint_extracted.cpp"

// --- Tests ---

void reset_mocks() {
    tft.cursor_x = 5;
    tft.cursor_y = 15;
    tft.last_printed_string = "";
    tft.last_printed_chars = nullptr;
    tft.last_printed_char = 0;
    tft.last_printed_base = 0;
    tft.last_printed_number = 0;
    tft.last_printed_double = 0;
    tft.last_printed_digits = 0;
}

void test_padprint_string() {
    reset_mocks();
    padprint(String("hello"), 2);
    assert(tft.cursor_x == 20); // 2 * BORDER_PAD_X
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_string == "hello");
    std::cout << "test_padprint_string passed\n";
}

void test_padprint_chars() {
    reset_mocks();
    padprint("world", 3);
    assert(tft.cursor_x == 30);
    assert(tft.cursor_y == 15);
    assert(std::string(tft.last_printed_chars) == "world");
    std::cout << "test_padprint_chars passed\n";
}

void test_padprint_char() {
    reset_mocks();
    padprint('A', 4);
    assert(tft.cursor_x == 40);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_char == 'A');
    std::cout << "test_padprint_char passed\n";
}

void test_padprint_unsigned_char() {
    reset_mocks();
    unsigned char b = 255;
    padprint(b, 16, 1);
    assert(tft.cursor_x == 10);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == 255);
    assert(tft.last_printed_base == 16);
    std::cout << "test_padprint_unsigned_char passed\n";
}

void test_padprint_int() {
    reset_mocks();
    padprint(42, 10, 5);
    assert(tft.cursor_x == 50);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == 42);
    assert(tft.last_printed_base == 10);
    std::cout << "test_padprint_int passed\n";
}

void test_padprint_unsigned_int() {
    reset_mocks();
    unsigned int n = 420;
    padprint(n, 16, 2);
    assert(tft.cursor_x == 20);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == 420);
    assert(tft.last_printed_base == 16);
    std::cout << "test_padprint_unsigned_int passed\n";
}

void test_padprint_long() {
    reset_mocks();
    long n = -100000;
    padprint(n, 10, 3);
    assert(tft.cursor_x == 30);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == -100000);
    assert(tft.last_printed_base == 10);
    std::cout << "test_padprint_long passed\n";
}

void test_padprint_unsigned_long() {
    reset_mocks();
    unsigned long n = 100000;
    padprint(n, 10, 4);
    assert(tft.cursor_x == 40);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == 100000);
    assert(tft.last_printed_base == 10);
    std::cout << "test_padprint_unsigned_long passed\n";
}

void test_padprint_long_long() {
    reset_mocks();
    long long n = -5000000000;
    padprint(n, 10, 1);
    assert(tft.cursor_x == 10);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == -5000000000);
    assert(tft.last_printed_base == 10);
    std::cout << "test_padprint_long_long passed\n";
}

void test_padprint_unsigned_long_long() {
    reset_mocks();
    unsigned long long n = 5000000000;
    padprint(n, 16, 2);
    assert(tft.cursor_x == 20);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_number == 5000000000);
    assert(tft.last_printed_base == 16);
    std::cout << "test_padprint_unsigned_long_long passed\n";
}

void test_padprint_double() {
    reset_mocks();
    padprint(3.14159, 4, 3);
    assert(tft.cursor_x == 30);
    assert(tft.cursor_y == 15);
    assert(tft.last_printed_double == 3.14159);
    assert(tft.last_printed_digits == 4);
    std::cout << "test_padprint_double passed\n";
}

int main() {
    std::cout << "Running padprint tests...\n";

    test_padprint_string();
    test_padprint_chars();
    test_padprint_char();
    test_padprint_unsigned_char();
    test_padprint_int();
    test_padprint_unsigned_int();
    test_padprint_long();
    test_padprint_unsigned_long();
    test_padprint_long_long();
    test_padprint_unsigned_long_long();
    test_padprint_double();

    std::cout << "All padprint tests passed!\n";
    return 0;
}
