#ifndef ARDUINO_H
#define ARDUINO_H
#include <string>
#include <iostream>
#include <vector>

using std::string;
typedef int gpio_num_t;
typedef std::string String;

// Mock for pinMode
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2

extern int pinMode_calls;
extern int last_pinMode_pin;
extern int last_pinMode_mode;

void pinMode(int pin, int mode);

#endif
