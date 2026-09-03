#ifndef ADVANCED_LOGGER_H
#define ADVANCED_LOGGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <array>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string>

// Constantes otimizadas
constexpr size_t MAX_LOG_MESSAGE_SIZE =
    1024; // Aumentado para evitar truncamento
constexpr size_t MAX_LOG_BUFFER_SIZE = 2048;    // Buffer maior para formatação
constexpr size_t MAX_MODULE_FILTERS = 16;       // Número fixo de módulos
constexpr uint32_t RATE_LIMIT_WINDOW_MS = 1000; // Janela de rate limiting
constexpr uint32_t MAX_LOGS_PER_SECOND = 100;   // Limite de logs por segundo

// Compressão básica implementada
namespace Compression {
bool compressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                  size_t &outputSize);
bool decompressData(const uint8_t *input, size_t inputSize, uint8_t *output,
                    size_t &outputSize);
} // namespace Compression

// Níveis de log (mantido como enum para compatibilidade)
enum LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, CRITICAL = 4 };

// Módulos/componentes (mantido como enum para compatibilidade)
enum LogModule {
  SYSTEM = 0,
  WIFI = 1,
  RFID = 2,
  RF = 3,
  BLE = 4,
  IR = 5,
  GPS = 6,
  SDCARD = 7,
  DISPLAY_MODULE = 8,
  POWER = 9,
  WEB = 10,
  NRF24 = 11,
  LORA = 12,
  FM_MODULE = 13,
  ETHERNET_MODULE = 14,
  OTHER = 15
};

// Códigos de erro específicos
enum class LogError : uint8_t {
  SUCCESS = 0,
  BUFFER_OVERFLOW = 1,
  INVALID_MODULE = 2,
  SD_WRITE_FAILED = 3,
  COMPRESSION_FAILED = 4,
  MUTEX_TIMEOUT = 5,
  INVALID_FORMAT = 6,
  RATE_LIMITED = 7
};

struct LogEntry {
  uint32_t timestamp;
  LogLevel level;
  LogModule module;
  char message[MAX_LOG_MESSAGE_SIZE]; // Buffer fixo para evitar fragmentação
  size_t messageLength;
  JsonDocument extraData;
  size_t heapFree;
  LogError errorCode;

  LogEntry() noexcept
      : timestamp(0), level(LogLevel::INFO), module(LogModule::SYSTEM),
        messageLength(0), heapFree(0), errorCode(LogError::SUCCESS) {
    message[0] = '\0';
  }
};

struct LogConfig {
  bool enabled = true;
  LogLevel minLevel = LogLevel::DEBUG;
  bool logToSerial = true;
  bool logToSD = true;
  bool structuredJSON = true;
  size_t maxFileSize = 1024 * 1024; // 1MB
  uint32_t rotationTimeHours = 24;  // Rotação diária
  bool compressOldLogs = true;
  std::array<bool, MAX_MODULE_FILTERS>
      moduleFilters; // Array fixo em vez de map

  LogConfig() noexcept {
    moduleFilters.fill(true); // Todos habilitados por padrão
  }
};

class AdvancedLogger {
public:
  static AdvancedLogger &getInstance();

  // Inicialização
  bool begin();
  void end();

  // Configuração
  void setConfig(const LogConfig &config);
  LogConfig getConfig() const;

  // Logging principal
  void log(LogLevel level, LogModule module, const char *format, ...);
  void log(LogLevel level, LogModule module, const String &message);
  void log(LogLevel level, LogModule module, const String &message,
           const JsonDocument &extraData);

  // Métodos convenientes
  void debug(LogModule module, const char *format, ...);
  void info(LogModule module, const char *format, ...);
  void warning(LogModule module, const char *format, ...);
  void error(LogModule module, const char *format, ...);
  void critical(LogModule module, const char *format, ...);

  // Controle de filtros
  void enableModule(LogModule module, bool enable = true);
  bool isModuleEnabled(LogModule module) const;

  // Gerenciamento de arquivos
  void rotateLogFile();
  void compressOldLogs();
  void cleanupOldLogs(uint32_t maxAgeDays = 30);

  // Comandos seriais
  void handleSerialCommand(const String &command,
                           const std::vector<String> &args);

  // Estatísticas
  size_t getLogFileSize() const;
  uint32_t getLogCount() const;
  uint32_t getDroppedLogCount() const;
  bool isRateLimited() const;
  void resetRateLimit();
  String getCurrentLogFile() const;

  // Compatibilidade com WillyLogger
  void trace(LogModule module, const char *format, ...);
  void notice(LogModule module, const char *format, ...);

private:
  AdvancedLogger();
  ~AdvancedLogger();
  AdvancedLogger(const AdvancedLogger &) = delete;
  AdvancedLogger &operator=(const AdvancedLogger &) = delete;

  // Métodos internos
  void writeToSerial(const String &entry);
  void writeToFile(const String &entry);
  String formatLogEntry(const LogEntry &entry);
  String generateLogFilename();
  bool ensureLogDirectory();
  void checkRotation();
  void loadConfig();
  void saveConfig();

  // Compressão
  bool compressFile(const String &inputFile, const String &outputFile);
  bool decompressFile(const String &inputFile, const String &outputFile);

  // Variáveis membro
  LogConfig _config;
  SemaphoreHandle_t _mutex;
  File _logFile;
  String _currentLogFile;
  uint32_t _startTime;
  uint32_t _lastRotationCheck;
  uint32_t _logCount;
  uint32_t _rateLimitWindowStart = 0;
  uint32_t _rateLimitCounter = 0;
  uint32_t _droppedLogCount = 0;
  bool _initialized;

  // Strings constantes
  static const char *LEVEL_STRINGS[];
  static const char *MODULE_STRINGS[];
};

// Macros de logging convenientes (compatíveis com WillyLogger)
#ifndef LOG_TRACE
#define LOG_TRACE(comp, msg, ...)                                              \
  AdvancedLogger::getInstance().trace(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(comp, msg, ...)                                              \
  AdvancedLogger::getInstance().debug(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_INFO
#define LOG_INFO(comp, msg, ...)                                               \
  AdvancedLogger::getInstance().info(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_NOTICE
#define LOG_NOTICE(comp, msg, ...)                                             \
  AdvancedLogger::getInstance().notice(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_WARNING
#define LOG_WARNING(comp, msg, ...)                                            \
  AdvancedLogger::getInstance().warning(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_ERROR
#define LOG_ERROR(comp, msg, ...)                                              \
  AdvancedLogger::getInstance().error(comp, msg, ##__VA_ARGS__)
#endif
#ifndef LOG_CRITICAL
#define LOG_CRITICAL(comp, msg, ...)                                           \
  AdvancedLogger::getInstance().critical(comp, msg, ##__VA_ARGS__)
#endif

#endif // ADVANCED_LOGGER_H