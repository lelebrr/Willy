#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <Arduino.h>

// Declaração global do dispositivo serial para comandos
extern HardwareSerial *serialDevice;

// Declaração global do estado do SD card
extern bool sdcardMounted;

#endif