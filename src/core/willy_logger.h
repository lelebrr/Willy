/**
 * @file willy_logger.h
 * @author Willy Firmware
 * @brief Sistema de Logging Centralizado para Willy ESP32-S3
 * @version 1.0
 */

#ifndef WILLY_LOGGER_H
#define WILLY_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include <vector>

// Níveis de log
enum WillyLogLevel {
  WILLY_LOG_LEVEL_TRACE = 0,
  WILLY_LOG_LEVEL_DEBUG = 1,
  WILLY_LOG_LEVEL_INFO = 2,
  WILLY_LOG_LEVEL_NOTICE = 3,
  WILLY_LOG_LEVEL_WARNING = 4,
  WILLY_LOG_LEVEL_ERROR = 5,
  WILLY_LOG_LEVEL_CRITICAL = 6,
  WILLY_LOG_LEVEL_OFF = 7
};

// Componentes do sistema
enum WillyLogComponent {
  COMP_SYSTEM = 0,
  COMP_GPS,
  COMP_IR,
  COMP_WIFI,
  COMP_BLE,
  COMP_CC1101,
  COMP_NRF24,
  COMP_NFC,
  COMP_RF,
  COMP_BADUSB,
  COMP_ETHERNET,
  COMP_SDCARD,
  COMP_DISPLAY,
  COMP_POWER,
  COMP_WEBUI,
  COMP_INTERPRETER,
  COMP_LORA,
  COMP_FM,
  COMP_OTHER,
  // Compatibilidade
  COMP_ETH = COMP_ETHERNET,
  COMP_SD = COMP_SDCARD,
  COMP_DISP = COMP_DISPLAY,
  COMP_PWR = COMP_POWER,
  COMP_WEB = COMP_WEBUI,
  COMP_INTERP = COMP_INTERPRETER
};

// Estrutura de entrada de log
struct WillyLogEntry {
  uint32_t timestamp;
  WillyLogLevel level;
  WillyLogComponent component;
  char message[192]; // Match include/willy_logger.h size
  int32_t value1;
  int32_t value2;
  int32_t value3;
  uint32_t heapFree;
  uint16_t errorCode;
};

// Configuração do logger
struct WillyLoggerConfig {
  bool enabled = true;
  WillyLogLevel minLevel = WILLY_LOG_LEVEL_INFO;
  bool logToSerial = true;
  bool logToSD = true;
  String logDirectory = "/WillyLogs";
  String logFilePrefix = "willy";
  uint32_t flushInterval = 5000; // ms
  uint32_t maxLogSize = 1048576; // 1MB
  int bufferEntries = 10;
  int maxLogFiles = 10;
  bool includeHeap = true;
};

class WillyLogger {
public:
  WillyLogger();
  ~WillyLogger();

  // Inicialização
  bool begin();
  void end();

  // Configuração
  void setEnabled(bool enabled);
  void setMinLevel(WillyLogLevel level);
  void setLogToSerial(bool enabled);
  void setLogToSD(bool enabled);
  void setLogDirectory(const String &dir);
  void setMaxLogSize(size_t size);
  void setMaxLogFiles(int files);
  void setFlushInterval(uint32_t interval);

  // Métodos de logging
  void log(WillyLogLevel level, WillyLogComponent component,
           const char *message, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0,
           uint16_t errorCode = 0);
  void logf(WillyLogLevel level, WillyLogComponent component,
            const char *format, ...);

  // Métodos convenientes
  void trace(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
             int32_t v2 = 0, int32_t v3 = 0);
  void debug(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
             int32_t v2 = 0, int32_t v3 = 0);
  void info(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
            int32_t v2 = 0, int32_t v3 = 0);
  void notice(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
              int32_t v2 = 0, int32_t v3 = 0);
  void warning(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
               int32_t v2 = 0, int32_t v3 = 0);
  void error(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
             int32_t v2 = 0, int32_t v3 = 0, uint16_t code = 0);
  void critical(WillyLogComponent comp, const char *msg, int32_t v1 = 0,
                int32_t v2 = 0, int32_t v3 = 0, uint16_t code = 0);

  // Logging específico por componente
  void logGPS(float lat, float lon, float alt, int satellites, uint32_t age,
              bool valid);
  void logIR(uint64_t code, uint16_t protocol, uint16_t bits, bool raw);
  void logWiFiScan(int apCount, int rssi, const char *ssid);
  void logBLEScan(int deviceCount, const char *name);
  void logCC1101(float freq, int rssi, const char *mode);
  void logNRF24(int channel, int packetCount, const char *mode);
  void logNFC(const char *uid, const char *type, bool readSuccess);
  void logRF(uint64_t code, float freq, int protocol, const char *raw);
  void logError(WillyLogComponent comp, const char *context, int errorCode,
                const char *details);
  void logSystemStatus();
  void logModuleInit(const char *moduleName, bool success, const char *details);

  // Gerenciamento de logs
  void flush();
  bool rotateLog();
  void cleanupOldLogs(int maxFiles = -1);
  bool needsRotation();

  // Estatísticas
  void getStats(uint32_t &totalEntries, uint32_t &errorCount,
                uint32_t &warningCount);
  uint64_t getTotalLogSize();
  String getCurrentLogFile() const { return _currentLogFile; }
  std::vector<String> listLogFiles();

  // Interface TFT
  void showLogWarning();
  void hideLogWarning();
  bool isLogWarningVisible() const;

  // Interface Web
  std::vector<String> getLogs();
  // Utilitários
  static const char *getLevelString(WillyLogLevel level);
  static const char *getComponentString(WillyLogComponent comp);
  String getFormattedTimestamp();
  uint32_t getUnixTimestamp();

private:
  WillyLoggerConfig _config;
  WillyLogEntry
      *_buffer_unused; // Keep the same name if needed but vector is better
  std::vector<WillyLogEntry> _buffer;
  int _bufferIndex;
  int _bufferCount;
  File _logFile;
  String _currentLogFile;
  uint32_t _startTime;
  uint32_t _lastFlush;
  uint32_t _totalEntries;
  uint32_t _errorCount;
  uint32_t _warningCount;
  SemaphoreHandle_t _mutex;
  bool _initialized;

  // Métodos privados
  void loadConfig();
  void saveConfig();
  void writeEntry(const WillyLogEntry &entry);
  void writeToFile(const WillyLogEntry &entry);
  void writeToSerial(const WillyLogEntry &entry);
  String generateLogFilename();
  bool ensureLogDirectory();
  void pushToBuffer(const WillyLogEntry &entry);
  void processBuffer();
};

// Instância global externa
extern WillyLogger willyLogger;

// Macros de logging convenientes removidas para evitar conflitos com
// advanced_logger.h Use as macros do advanced_logger.h que são compatíveis

#endif // WILLY_LOGGER_H
