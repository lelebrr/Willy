#ifndef __SERIAL_HELPERS_H__
#define __SERIAL_HELPERS_H__
#ifndef LITE_VERSION

#include <Arduino.h>

// Tamanho seguro para buffer de stack
#ifndef SAFE_STACK_BUFFER_SIZE
#define SAFE_STACK_BUFFER_SIZE 1024
#endif

bool _setupPsramFs();
char *_readFileFromSerial(size_t fileSizeChar = SAFE_STACK_BUFFER_SIZE);

// Função de sanitização global para entradas
bool sanitizeInput(const String &input, size_t maxLength = 1024);
String sanitizeFilename(const String &filename);

#endif
#endif
