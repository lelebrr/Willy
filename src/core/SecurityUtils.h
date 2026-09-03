#ifndef SECURITY_UTILS_H
#define SECURITY_UTILS_H

#include <Arduino.h>
#include <string>
#include <vector>


// Classe para validação de segurança
class SecurityValidator {
public:
  // Validação de SSID
  static bool validateSSID(const String &ssid);

  // Validação de senha WiFi
  static bool validateWiFiPassword(const String &password);

  // Validação de endereço MAC
  static bool validateMAC(const String &mac);

  // Validação de canal WiFi
  static bool validateWiFiChannel(uint8_t channel);

  // Validação de frequência RF
  static bool validateRFFrequency(float frequency);

  // Validação de código RFID
  static bool validateRFIDCode(const String &code);

  // Sanitização de string
  static String sanitizeString(const String &input);

  // Validação de tamanho de buffer
  static bool validateBufferSize(size_t size, size_t maxSize);
};

// Classe para rate limiting
class RateLimiter {
private:
  unsigned long lastAction;
  unsigned long minInterval;

public:
  RateLimiter(unsigned long intervalMs = 1000);

  // Verifica se ação é permitida
  bool allowAction();

  // Reseta o limiter
  void reset();
};

// Classe para logs de segurança
class SecurityLogger {
public:
  enum LogLevel { INFO, WARNING, ERROR, CRITICAL };

  static void log(LogLevel level, const String &module, const String &message);
  static void logSecurityEvent(const String &event, const String &details);
};

// Classe para verificação de integridade
class IntegrityChecker {
public:
  // Calcula hash SHA256 de arquivo
  static String calculateFileHash(const String &path);

  // Verifica integridade de arquivo
  static bool verifyFileIntegrity(const String &path,
                                  const String &expectedHash);
};

// Classe para proteção contra buffer overflow
class BufferProtector {
public:
  // Copia string com proteção
  static bool safeStringCopy(char *dest, size_t destSize, const char *src);

  // Copia memória com proteção
  static bool safeMemoryCopy(void *dest, size_t destSize, const void *src,
                             size_t srcSize);
};

#endif // SECURITY_UTILS_H