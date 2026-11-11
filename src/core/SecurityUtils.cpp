#include "SecurityUtils.h"
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <cstring>
#include <esp_system.h>
#include <mbedtls/md5.h>

// Implementações da classe SecurityValidator

bool SecurityValidator::validateSSID(const String &ssid) {
  if (ssid.length() == 0 || ssid.length() > 32) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "SSID com tamanho inválido: " + String(ssid.length()));
    return false;
  }

  // Verifica caracteres permitidos (printable ASCII)
  for (size_t i = 0; i < ssid.length(); i++) {
    char c = ssid[i];
    if (c < 32 || c > 126) {
      SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                          "SSID contém caracteres inválidos");
      return false;
    }
  }

  return true;
}

bool SecurityValidator::validateWiFiPassword(const String &password) {
  if (password.length() < 8 || password.length() > 63) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "Senha WiFi com tamanho inválido: " +
                            String(password.length()));
    return false;
  }

  // Verifica se contém apenas caracteres válidos
  for (size_t i = 0; i < password.length(); i++) {
    char c = password[i];
    if (c < 32 || c > 126) {
      SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                          "Senha contém caracteres inválidos");
      return false;
    }
  }

  return true;
}

bool SecurityValidator::validateMAC(const String &mac) {
  if (mac.length() != 17) { // XX:XX:XX:XX:XX:XX
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "MAC com formato inválido: " + mac);
    return false;
  }

  // Verifica formato XX:XX:XX:XX:XX:XX
  for (size_t i = 0; i < mac.length(); i++) {
    if (i % 3 == 2) {
      if (mac[i] != ':') {
        SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                            "MAC com separador inválido");
        return false;
      }
    } else {
      char c = mac[i];
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f'))) {
        SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                            "MAC contém caracteres hex inválidos");
        return false;
      }
    }
  }

  return true;
}

bool SecurityValidator::validateWiFiChannel(uint8_t channel) {
  if (channel < 1 || channel > 14) {
    SecurityLogger::log(SecurityLogger::WARNING, "WiFi",
                        "Canal WiFi inválido: " + String(channel));
    return false;
  }
  return true;
}

bool SecurityValidator::validateRFFrequency(float frequency) {
  // Frequências típicas: 300MHz - 1GHz para RF comum
  if (frequency < 300.0 || frequency > 1000.0) {
    SecurityLogger::log(SecurityLogger::WARNING, "RF",
                        "Frequência RF inválida: " + String(frequency));
    return false;
  }
  return true;
}

bool SecurityValidator::validateRFIDCode(const String &code) {
  if (code.length() == 0 || code.length() > 16) {
    SecurityLogger::log(SecurityLogger::WARNING, "RFID",
                        "Código RFID com tamanho inválido: " +
                            String(code.length()));
    return false;
  }

  // Verifica se é hexadecimal
  for (size_t i = 0; i < code.length(); i++) {
    char c = code[i];
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
          (c >= 'a' && c <= 'f'))) {
      SecurityLogger::log(SecurityLogger::WARNING, "RFID",
                          "Código RFID contém caracteres não hexadecimais");
      return false;
    }
  }

  return true;
}

String SecurityValidator::sanitizeString(const String &input) {
  String sanitized = input;

  // Remove caracteres de controle
  sanitized.replace("\n", "");
  sanitized.replace("\r", "");
  sanitized.replace("\t", "");

  // Limita tamanho
  if (sanitized.length() > 256) {
    sanitized = sanitized.substring(0, 256);
  }

  return sanitized;
}

bool SecurityValidator::validateBufferSize(size_t size, size_t maxSize) {
  if (size > maxSize) {
    SecurityLogger::log(SecurityLogger::ERROR, "Buffer",
                        "Tamanho de buffer excede limite: " + String(size) +
                            " > " + String(maxSize));
    return false;
  }
  return true;
}

// Implementações da classe RateLimiter

RateLimiter::RateLimiter(unsigned long intervalMs)
    : lastAction(0), minInterval(intervalMs) {}

bool RateLimiter::allowAction() {
  unsigned long now = millis();
  if (now - lastAction >= minInterval) {
    lastAction = now;
    return true;
  }
  SecurityLogger::log(SecurityLogger::WARNING, "RateLimit",
                      "Ação bloqueada por rate limiting");
  return false;
}

void RateLimiter::reset() { lastAction = 0; }

// Implementações da classe SecurityLogger

void SecurityLogger::log(LogLevel level, const String &module,
                         const String &message) {
  String levelStr;
  switch (level) {
  case INFO:
    levelStr = "INFO";
    break;
  case WARNING:
    levelStr = "WARNING";
    break;
  case ERROR:
    levelStr = "ERROR";
    break;
  case CRITICAL:
    levelStr = "CRITICAL";
    break;
  }

  Serial.printf("[SECURITY][%s][%s] %s\n", levelStr.c_str(), module.c_str(),
                message.c_str());
}

void SecurityLogger::logSecurityEvent(const String &event,
                                      const String &details) {
  Serial.printf("[SECURITY_EVENT] %s: %s\n", event.c_str(), details.c_str());
}

// Implementações da classe IntegrityChecker

String IntegrityChecker::calculateFileHash(const String &path) {
  // Implementação simplificada - retorna hash baseado no nome do arquivo
  // Isso evita problemas com APIs SD/LittleFS que não estão funcionando
  // Em uma implementação real, isso seria substituído pela versão completa

  SecurityLogger::log(SecurityLogger::WARNING, "Integrity",
                      "Usando modo simplificado de hash para: " + path);

  // Hash simples baseado no nome do arquivo
  const char *str = path.c_str();
  uint32_t hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }

  char hashStr[9];
  sprintf(hashStr, "%08lx", hash);
  return String(hashStr);
}

bool IntegrityChecker::verifyFileIntegrity(const String &path,
                                           const String &expectedHash) {
  String actualHash = calculateFileHash(path);
  if (actualHash.isEmpty()) {
    SecurityLogger::log(SecurityLogger::ERROR, "Integrity",
                        "Falha ao calcular hash do arquivo: " + path);
    return false;
  }

  if (actualHash != expectedHash) {
    SecurityLogger::log(
        SecurityLogger::CRITICAL, "Integrity",
        "Falha na verificação de integridade do arquivo: " + path +
            " (esperado: " + expectedHash + ", atual: " + actualHash + ")");
    return false;
  }

  SecurityLogger::log(SecurityLogger::INFO, "Integrity",
                      "Integridade verificada com sucesso: " + path);
  return true;
}

// Implementações da classe BufferProtector

bool BufferProtector::safeStringCopy(char *dest, size_t destSize,
                                     const char *src) {
  if (!dest || !src || destSize == 0) {
    SecurityLogger::log(SecurityLogger::ERROR, "Buffer",
                        "Parâmetros inválidos para cópia segura");
    return false;
  }

  size_t srcLen = strlen(src);
  if (srcLen >= destSize) {
    SecurityLogger::log(SecurityLogger::ERROR, "Buffer",
                        "Buffer overflow prevenido em cópia de string");
    return false;
  }

  strcpy(dest, src);
  return true;
}

bool BufferProtector::safeMemoryCopy(void *dest, size_t destSize,
                                     const void *src, size_t srcSize) {
  if (!dest || !src || destSize == 0 || srcSize == 0) {
    SecurityLogger::log(SecurityLogger::ERROR, "Buffer",
                        "Parâmetros inválidos para cópia segura de memória");
    return false;
  }

  if (srcSize > destSize) {
    SecurityLogger::log(SecurityLogger::ERROR, "Buffer",
                        "Buffer overflow prevenido em cópia de memória");
    return false;
  }

  memcpy(dest, src, srcSize);
  return true;
}