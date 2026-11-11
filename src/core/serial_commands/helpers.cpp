#include "helpers.h"
#ifndef LITE_VERSION
#include <globals.h>

bool _setupPsramFs() {
  // https://github.com/tobozo/ESP32-PsRamFS/blob/main/examples/PSRamFS_Test/PSRamFS_Test.ino
  static bool psRamFSMounted = false;
  if (psRamFSMounted)
    return true; // avoid reinit

#ifdef BOARD_HAS_PSRAM
  // PSRamFS.setPartitionSize(ESP.getFreePsram() / 2); // use half of psram
  // PSRamFS.begin() requires specific ESP32-PSRamFS library which may not be available
  // Using standard LittleFS or SPIFFS instead when PSRamFS is not available
  serialDevice->println("PSRamFS not available, using standard filesystem");
  psRamFSMounted = false;
  return false;
#else
  // Use standard filesystem
  psRamFSMounted = false;
  return false;
#endif
}

char *_readFileFromSerial(size_t fileSizeChar) {
  char *buf = psramFound() ? (char *)ps_malloc(fileSizeChar + 1)
                           : (char *)malloc(fileSizeChar + 1);

  if (buf == NULL) {
    serialDevice->printf("Could not allocate %d\n", fileSizeChar);
    return NULL;
  }

  size_t bufSize = 0;
  buf[0] = '\0';

  unsigned long lastData = millis();

  String currLine = "";
  serialDevice->println("Reading input data from serial buffer until EOF");
  serialDevice->flush();
  while (true) {
    if (serialDevice->available()) {
      lastData = millis();
      int c = serialDevice->read();
      if (c == -1)
        continue;

      buf[bufSize++] = (char)c;
      buf[bufSize] = '\0';

      if (bufSize >= 3 && strcmp(buf + bufSize - 3, "EOF") == 0) {
        bufSize -= 3;
        buf[bufSize] = '\0';
        break;
      }

      if (bufSize >= fileSizeChar) {
        log_e("Input truncated!");
        break;
      }
    } else {
      if (millis() - lastData > 5000)
        break; // timeout
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
  buf[bufSize] = '\0';
  return buf;
}

// Função de sanitização global para entradas
bool sanitizeInput(const String &input, size_t maxLength) {
  if (input.length() > maxLength) {
    return false;
  }

  // Verificar caracteres permitidos
  for (size_t i = 0; i < input.length(); ++i) {
    char c = input[i];
    if (!isalnum(c) && c != ' ' && c != '-' && c != '_' && c != '.' &&
        c != '/' && c != '"' && c != '\r' && c != '\n' && c != ':' &&
        c != '@') {
      return false;
    }
  }

  // Verificar sequências perigosas
  if (input.indexOf("..") != -1 || input.indexOf("//") != -1) {
    return false;
  }

  return true;
}

// Sanitização específica para nomes de arquivo
String sanitizeFilename(const String &filename) {
  String sanitized = filename;
  sanitized.trim();

  // Remover caracteres perigosos
  sanitized.replace("..", "");
  sanitized.replace("/", "");
  sanitized.replace("\\", "");

  // Limitar comprimento
  if (sanitized.length() > 255) {
    sanitized = sanitized.substring(0, 255);
  }

  return sanitized;
}
#endif
