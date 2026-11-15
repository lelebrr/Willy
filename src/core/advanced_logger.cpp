#include "advanced_logger.h"
#include "compression_utils.h"
#include "sd_functions.h"
#include <ESP32Time.h>
#include <cstdarg>
#include <ctime>
#include <globals.h>


// Declaração externa do RTC (definido em main.cpp)
extern ESP32Time rtc;

// Strings constantes
const char *AdvancedLogger::LEVEL_STRINGS[] = {"DEBUG", "INFO", "WARNING",
                                               "ERROR", "CRITICAL"};
const char *AdvancedLogger::MODULE_STRINGS[] = {
    "SYSTEM",  "WIFI",  "RFID", "RF",    "BLE",  "IR", "GPS",      "SDCARD",
    "DISPLAY", "POWER", "WEB",  "NRF24", "LORA", "FM", "ETHERNET", "OTHER"};

// Instância singleton
AdvancedLogger &AdvancedLogger::getInstance() {
  static AdvancedLogger instance;
  return instance;
}

AdvancedLogger::AdvancedLogger()
    : _mutex(nullptr), _startTime(0), _lastRotationCheck(0), _logCount(0),
      _initialized(false) {
  // Inicializar filtros de módulo - todos habilitados por padrão
  for (int i = 0; i <= OTHER; ++i) {
    _config.moduleFilters[static_cast<LogModule>(i)] = true;
  }
}

AdvancedLogger::~AdvancedLogger() { end(); }

bool AdvancedLogger::begin() {
  if (_initialized)
    return true;

  // Criar mutex
  _mutex = xSemaphoreCreateRecursiveMutex();
  if (!_mutex) {
    Serial.println("[AdvancedLogger] Erro ao criar mutex");
    return false;
  }

  // Carregar configuração
  loadConfig();

  // Verificar SD
  if (_config.logToSD && !sdcardMounted) {
    Serial.println("[AdvancedLogger] SD não montado - logging limitado");
    _config.logToSD = false;
  }

  // Criar diretório de logs
  if (_config.logToSD && !ensureLogDirectory()) {
    Serial.println("[AdvancedLogger] Erro ao criar diretório de logs");
    _config.logToSD = false;
  }

  // Criar arquivo de log inicial
  if (_config.logToSD) {
    _currentLogFile = generateLogFilename();
    _logFile = SD.open(_currentLogFile, FILE_WRITE);
    if (!_logFile) {
      Serial.println("[AdvancedLogger] Erro ao criar arquivo de log");
      _config.logToSD = false;
    }
  }

  _startTime = millis();
  _lastRotationCheck = _startTime;
  _initialized = true;

  // Log inicial
  info(SYSTEM, "Advanced Logger iniciado");
  return true;
}

void AdvancedLogger::end() {
  if (!_initialized)
    return;

  // Flush final
  if (_logFile) {
    _logFile.flush();
    _logFile.close();
  }

  if (_mutex) {
    vSemaphoreDelete(_mutex);
    _mutex = nullptr;
  }

  _initialized = false;
}

void AdvancedLogger::setConfig(const LogConfig &config) {
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    _config = config;
    saveConfig();
    xSemaphoreGive(_mutex);
  }
}

LogConfig AdvancedLogger::getConfig() const { return _config; }

void AdvancedLogger::log(LogLevel level, LogModule module, const char *format,
                         ...) {
  // Validações básicas
  if (!_initialized) {
    return;
  }

  if (!_config.enabled) {
    return; // Silenciosamente ignorado
  }

  if (level < _config.minLevel || !_config.moduleFilters[module]) {
    return; // Filtrado
  }

  // Rate limiting
  uint32_t currentTime = millis();
  if (currentTime - _rateLimitWindowStart >= RATE_LIMIT_WINDOW_MS) {
    _rateLimitCounter = 0;
    _rateLimitWindowStart = currentTime;
  }

  if (_rateLimitCounter >= MAX_LOGS_PER_SECOND) {
    _droppedLogCount++;
    return;
  }

  // Validação de entrada
  if (!format || strlen(format) > MAX_LOG_MESSAGE_SIZE - 50) {
    return;
  }

  va_list args;
  va_start(args, format);
  char buffer[MAX_LOG_BUFFER_SIZE];
  int result = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (result < 0 || static_cast<size_t>(result) >= sizeof(buffer)) {
    return;
  }

  log(level, module, String(buffer));
  _rateLimitCounter++;
}

void AdvancedLogger::log(LogLevel level, LogModule module,
                         const String &message) {
  JsonDocument emptyDoc;
  log(level, module, message, emptyDoc);
}

void AdvancedLogger::log(LogLevel level, LogModule module,
                         const String &message, const JsonDocument &extraData) {
  if (!_initialized) {
    return;
  }

  if (!_config.enabled) {
    return;
  }

  if (level < _config.minLevel || !_config.moduleFilters[module]) {
    return;
  }

  // Validação de tamanho da mensagem
  if (message.length() >= MAX_LOG_MESSAGE_SIZE) {
    return;
  }

  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) {
    return;
  }

  LogEntry entry;
  entry.timestamp = rtc.getEpoch(); // Usar RTC se disponível
  if (entry.timestamp == 0) {
    entry.timestamp = millis() / 1000; // Fallback
  }
  entry.level = level;
  entry.module = module;
  strncpy(entry.message, message.c_str(), MAX_LOG_MESSAGE_SIZE - 1);
  entry.message[MAX_LOG_MESSAGE_SIZE - 1] = '\0';
  entry.messageLength = strlen(entry.message);
  entry.extraData = extraData;
  entry.heapFree = ESP.getFreeHeap();
  entry.errorCode = LogError::SUCCESS; // Pode ser expandido

  String formattedEntry = formatLogEntry(entry);

  // Escrever para Serial
  if (_config.logToSerial) {
    writeToSerial(formattedEntry);
  }

  // Escrever para arquivo
  if (_config.logToSD) {
    writeToFile(formattedEntry);
  }

  _logCount++;

  // Verificar rotação
  checkRotation();

  xSemaphoreGive(_mutex);
}

// Métodos convenientes
void AdvancedLogger::debug(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(DEBUG, module, String(buffer));
}

void AdvancedLogger::info(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(INFO, module, String(buffer));
}

void AdvancedLogger::warning(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(WARNING, module, String(buffer));
}

void AdvancedLogger::error(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(ERROR, module, String(buffer));
}

void AdvancedLogger::critical(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(CRITICAL, module, String(buffer));
}

void AdvancedLogger::enableModule(LogModule module, bool enable) {
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    _config.moduleFilters[module] = enable;
    xSemaphoreGive(_mutex);
  }
}

bool AdvancedLogger::isModuleEnabled(LogModule module) const {
  // std::array não tem find(), usar acesso direto com verificação de bounds
  if (module >= 0 && module < MAX_MODULE_FILTERS) {
    return _config.moduleFilters[static_cast<size_t>(module)];
  }
  return true; // Padrão: habilitado para módulos desconhecidos
}

void AdvancedLogger::rotateLogFile() {
  if (!_initialized || !_config.logToSD)
    return;

  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE)
    return;

  if (_logFile) {
    _logFile.flush();
    _logFile.close();
  }

  // Comprimir arquivo antigo se necessário
  if (_config.compressOldLogs && !_currentLogFile.isEmpty()) {
    String compressedFile = _currentLogFile + ".lz4";
    if (compressFile(_currentLogFile, compressedFile)) {
      SD.remove(_currentLogFile); // Remover original após compressão
    }
  }

  // Criar novo arquivo
  _currentLogFile = generateLogFilename();
  _logFile = SD.open(_currentLogFile, FILE_WRITE);
  if (_logFile) {
    // Escrever cabeçalho
    _logFile.println("# Advanced Logger - Rotated at " +
                     String(rtc.getEpoch()));
    _logFile.flush();
  }

  xSemaphoreGive(_mutex);
}

void AdvancedLogger::compressOldLogs() {
  if (!_initialized || !_config.logToSD)
    return;

  if (!SD.exists("/logs"))
    return;

  File root = SD.open("/logs");
  if (!root || !root.isDirectory()) {
    root.close();
    return;
  }

  while (File file = root.openNextFile()) {
    if (!file.isDirectory() && file.name()) {
      String name = String(file.name());
      if (name.endsWith(".log") || name.endsWith(".json")) {
        String compressedPath = "/logs/" + name + ".gz";
        if (!SD.exists(compressedPath)) {
          // Ler arquivo
          size_t fileSize = file.size();
          uint8_t *input = new uint8_t[fileSize];
          if (input && file.read(input, fileSize) == fileSize) {
            size_t maxOut = fileSize + 16;
            uint8_t *output = new uint8_t[maxOut];
            if (output) {
              size_t outSize = 0;
              if (Compression::compressData(input, fileSize, output, outSize)) {
                File outFile = SD.open(compressedPath, FILE_WRITE);
                if (outFile) {
                  outFile.write(output, outSize);
                  outFile.close();
                  info(SYSTEM, "Log comprimido: %s -> %s (%d bytes)", name.c_str(), compressedPath.c_str(), outSize);
                }
                delete[] output;
              } else {
                delete[] output;
              }
            }
            delete[] input;
          }
        }
      }
    }
    file.close();
  }
  root.close();
}

void AdvancedLogger::cleanupOldLogs(uint32_t maxAgeDays) {
  if (!_initialized || !_config.logToSD)
    return;

  if (!SD.exists("/logs"))
    return;

  uint32_t now = rtc.getEpoch();
  if (now == 0)
    now = millis() / 1000;

  File root = SD.open("/logs");
  if (!root || !root.isDirectory()) {
    root.close();
    return;
  }

  uint32_t maxAgeSeconds = maxAgeDays * 24 * 60 * 60;

  while (File file = root.openNextFile()) {
    if (!file.isDirectory() && file.name()) {
      String name = String(file.name());
      // Extrair timestamp do nome do arquivo: log_YYYYMMDD_HHMMSS.json
      int underscoreIdx = name.indexOf('_');
      int dotIdx = name.lastIndexOf('.');
      if (underscoreIdx > 0 && dotIdx > underscoreIdx) {
        String dateStr = name.substring(underscoreIdx + 1, dotIdx);
        // Tentar interpretar como timestamp YYYYMMDDHHMMSS
        if (dateStr.length() >= 14) {
          int year = dateStr.substring(0, 4).toInt();
          int month = dateStr.substring(4, 6).toInt();
          int day = dateStr.substring(6, 8).toInt();
          // Calcular idade aproximada em dias
          uint32_t fileTime = (year - 1970) * 365 * 24 * 60 * 60 +
                              (month - 1) * 30 * 24 * 60 * 60 +
                              day * 24 * 60 * 60;
          uint32_t ageDays = (now - fileTime) / (24 * 60 * 60);
          if (ageDays > maxAgeDays) {
            String path = "/logs/" + name;
            SD.remove(path);
            info(SYSTEM, "Log antigo removido: %s (idade: %d dias)", name.c_str(), ageDays);
          }
        }
      }
    }
    file.close();
  }
  root.close();
}

void AdvancedLogger::handleSerialCommand(const String &command,
                                         const std::vector<String> &args) {
  if (command == "log_level") {
    if (args.size() >= 1) {
      LogLevel level = static_cast<LogLevel>(args[0].toInt());
      if (level >= DEBUG && level <= CRITICAL) {
        _config.minLevel = level;
        Serial.printf("[AdvancedLogger] Nível mínimo alterado para %s\n",
                      LEVEL_STRINGS[level]);
      }
    }
  } else if (command == "log_module") {
    if (args.size() >= 2) {
      LogModule module = static_cast<LogModule>(args[0].toInt());
      bool enable = args[1] == "1" || args[1] == "true";
      enableModule(module, enable);
      Serial.printf("[AdvancedLogger] Módulo %s %s\n", MODULE_STRINGS[module],
                    enable ? "habilitado" : "desabilitado");
    }
  } else if (command == "log_rotate") {
    rotateLogFile();
    Serial.println("[AdvancedLogger] Rotação de log executada");
  } else if (command == "log_stats") {
    Serial.printf("[AdvancedLogger] Arquivo atual: %s\n",
                  _currentLogFile.c_str());
    Serial.printf("[AdvancedLogger] Tamanho: %d bytes\n", getLogFileSize());
    Serial.printf("[AdvancedLogger] Contagem: %lu\n", _logCount);
  }
}

size_t AdvancedLogger::getLogFileSize() const {
  if (!_logFile)
    return 0;
  return _logFile.size();
}

uint32_t AdvancedLogger::getLogCount() const { return _logCount; }

uint32_t AdvancedLogger::getDroppedLogCount() const { return _droppedLogCount; }

String AdvancedLogger::getCurrentLogFile() const { return _currentLogFile; }

bool AdvancedLogger::isRateLimited() const {
  uint32_t currentTime = millis();
  if (currentTime - _rateLimitWindowStart >= RATE_LIMIT_WINDOW_MS) {
    return false; // Nova janela
  }
  return _rateLimitCounter >= MAX_LOGS_PER_SECOND;
}

void AdvancedLogger::resetRateLimit() {
  _rateLimitCounter = 0;
  _rateLimitWindowStart = millis();
}

// Compatibilidade com WillyLogger
void AdvancedLogger::trace(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(DEBUG, module, String(buffer)); // TRACE mapeia para DEBUG
}

void AdvancedLogger::notice(LogModule module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  log(INFO, module, String(buffer)); // NOTICE mapeia para INFO
}

// Métodos privados
void AdvancedLogger::writeToSerial(const String &entry) {
  Serial.println(entry);
}

void AdvancedLogger::writeToFile(const String &entry) {
  if (_logFile) {
    _logFile.println(entry);
    _logFile.flush();
  }
}

String AdvancedLogger::formatLogEntry(const LogEntry &entry) {
  if (_config.structuredJSON) {
    JsonDocument doc;
    doc["timestamp"] = entry.timestamp;
    doc["level"] = LEVEL_STRINGS[entry.level];
    doc["module"] = MODULE_STRINGS[entry.module];
    doc["message"] = entry.message;
    if (!entry.extraData.isNull()) {
      doc["extra"] = entry.extraData;
    }
    doc["heap_free"] = entry.heapFree;
    doc["error_code"] = static_cast<uint8_t>(entry.errorCode);

    String output;
    serializeJson(doc, output);
    return output;
  } else {
    // Formato CSV simples
    return String(entry.timestamp) + "," + LEVEL_STRINGS[entry.level] + "," +
           MODULE_STRINGS[entry.module] + "," + String(entry.message) + "," +
           String(entry.heapFree) + "," +
           String(static_cast<uint8_t>(entry.errorCode));
  }
}

String AdvancedLogger::generateLogFilename() {
  time_t now = rtc.getEpoch();
  if (now == 0) {
    now = millis() / 1000;
  }

  struct tm *timeinfo = localtime(&now);
  char filename[32];
  sprintf(filename, "/logs/log_%04d%02d%02d_%02d%02d%02d.json",
          timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
          timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  return String(filename);
}

bool AdvancedLogger::ensureLogDirectory() {
  if (!SD.exists("/logs")) {
    return SD.mkdir("/logs");
  }
  return true;
}

void AdvancedLogger::checkRotation() {
  uint32_t now = millis();
  if (now - _lastRotationCheck < 60000)
    return; // Verificar a cada minuto

  _lastRotationCheck = now;

  bool shouldRotate = false;

  // Verificar tamanho
  if (_config.maxFileSize > 0 && getLogFileSize() >= _config.maxFileSize) {
    shouldRotate = true;
  }

  // Verificar tempo
  if (_config.rotationTimeHours > 0) {
    uint32_t elapsedHours = (now - _startTime) / (1000 * 60 * 60);
    if (elapsedHours >= _config.rotationTimeHours) {
      shouldRotate = true;
    }
  }

  if (shouldRotate) {
    rotateLogFile();
  }
}

void AdvancedLogger::loadConfig() {
  // Carregar configuração do SD ou usar padrões
  String configPath = "/logs/logger_config.json";
  if (!SD.exists(configPath))
    return;

  File configFile = SD.open(configPath, FILE_READ);
  if (!configFile)
    return;

  size_t size = configFile.size();
  uint8_t *buf = new uint8_t[size + 1];
  if (buf) {
    size_t read = configFile.read(buf, size);
    buf[read] = '\0';
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, (char *)buf);
    if (!err) {
      if (doc.containsKey("minLevel"))
        _config.minLevel = (LogLevel)doc["minLevel"].as<int>();
      if (doc.containsKey("logToSerial"))
        _config.logToSerial = doc["logToSerial"].as<bool>();
      if (doc.containsKey("logToSD"))
        _config.logToSD = doc["logToSD"].as<bool>();
      if (doc.containsKey("structuredJSON"))
        _config.structuredJSON = doc["structuredJSON"].as<bool>();
      if (doc.containsKey("maxFileSize"))
        _config.maxFileSize = doc["maxFileSize"].as<size_t>();
      if (doc.containsKey("rotationTimeHours"))
        _config.rotationTimeHours = doc["rotationTimeHours"].as<uint32_t>();
      if (doc.containsKey("compressOldLogs"))
        _config.compressOldLogs = doc["compressOldLogs"].as<bool>();
      info(SYSTEM, "Configuração do logger carregada do SD");
    } else {
      warning(SYSTEM, "Falha ao analisar config do logger: %s", err.c_str());
    }
    delete[] buf;
  }
  configFile.close();
}

void AdvancedLogger::saveConfig() {
  // Salvar configuração no SD
  String configPath = "/logs/logger_config.json";
  File configFile = SD.open(configPath, FILE_WRITE);
  if (!configFile)
    return;

  JsonDocument doc;
  doc["minLevel"] = (int)_config.minLevel;
  doc["logToSerial"] = _config.logToSerial;
  doc["logToSD"] = _config.logToSD;
  doc["structuredJSON"] = _config.structuredJSON;
  doc["maxFileSize"] = _config.maxFileSize;
  doc["rotationTimeHours"] = _config.rotationTimeHours;
  doc["compressOldLogs"] = _config.compressOldLogs;

  serializeJson(doc, configFile);
  configFile.close();
  info(SYSTEM, "Configuração do logger salva no SD");
}

bool AdvancedLogger::compressFile(const String &inputFile,
                                  const String &outputFile) {
  if (!SD.exists(inputFile)) {
    error(SYSTEM, "Arquivo de entrada não existe: %s", inputFile.c_str());
    return false;
  }

  File inFile = SD.open(inputFile, FILE_READ);
  if (!inFile) {
    error(SYSTEM, "Erro ao abrir arquivo de entrada: %s", inputFile.c_str());
    return false;
  }

  size_t fileSize = inFile.size();
  uint8_t *input = new uint8_t[fileSize];
  if (!input) {
    inFile.close();
    error(SYSTEM, "Sem memória para buffer de entrada (%d bytes)", fileSize);
    return false;
  }

  if (inFile.read(input, fileSize) != fileSize) {
    delete[] input;
    inFile.close();
    error(SYSTEM, "Erro ao ler arquivo de entrada: %s", inputFile.c_str());
    return false;
  }
  inFile.close();

  size_t maxOut = fileSize + 16;
  uint8_t *output = new uint8_t[maxOut];
  if (!output) {
    delete[] input;
    error(SYSTEM, "Sem memória para buffer de saída (%d bytes)", maxOut);
    return false;
  }

  size_t outSize = 0;
  bool success = Compression::compressData(input, fileSize, output, outSize);

  if (success) {
    File outFile = SD.open(outputFile, FILE_WRITE);
    if (outFile) {
      outFile.write(output, outSize);
      outFile.close();
      info(SYSTEM, "Arquivo comprimido: %s -> %s (%d -> %d bytes)",
           inputFile.c_str(), outputFile.c_str(), fileSize, outSize);
      success = true;
    } else {
      error(SYSTEM, "Erro ao criar arquivo de saída: %s", outputFile.c_str());
      success = false;
    }
  } else {
    error(SYSTEM, "Falha na compressão RLE de: %s", inputFile.c_str());
    success = false;
  }

  delete[] input;
  delete[] output;
  return success;
}

bool AdvancedLogger::decompressFile(const String &inputFile,
                                    const String &outputFile) {
  if (!SD.exists(inputFile)) {
    error(SYSTEM, "Arquivo comprimido não existe: %s", inputFile.c_str());
    return false;
  }

  File inFile = SD.open(inputFile, FILE_READ);
  if (!inFile) {
    error(SYSTEM, "Erro ao abrir arquivo comprimido: %s", inputFile.c_str());
    return false;
  }

  size_t fileSize = inFile.size();
  uint8_t *input = new uint8_t[fileSize];
  if (!input) {
    inFile.close();
    error(SYSTEM, "Sem memória para buffer de entrada (%d bytes)", fileSize);
    return false;
  }

  if (inFile.read(input, fileSize) != fileSize) {
    delete[] input;
    inFile.close();
    error(SYSTEM, "Erro ao ler arquivo comprimido: %s", inputFile.c_str());
    return false;
  }
  inFile.close();

  // Para descompressão, alocar buffer generoso (2x o tamanho comprimido)
  size_t maxOut = fileSize * 4;
  uint8_t *output = new uint8_t[maxOut];
  if (!output) {
    delete[] input;
    error(SYSTEM, "Sem memória para buffer de saída (%d bytes)", maxOut);
    return false;
  }

  size_t outSize = 0;
  bool success = Compression::decompressData(input, fileSize, output, outSize);

  if (success) {
    File outFile = SD.open(outputFile, FILE_WRITE);
    if (outFile) {
      outFile.write(output, outSize);
      outFile.close();
      info(SYSTEM, "Arquivo descomprimido: %s -> %s (%d -> %d bytes)",
           inputFile.c_str(), outputFile.c_str(), fileSize, outSize);
      success = true;
    } else {
      error(SYSTEM, "Erro ao criar arquivo de saída: %s", outputFile.c_str());
      success = false;
    }
  } else {
    error(SYSTEM, "Falha na descompressão RLE de: %s", inputFile.c_str());
    success = false;
  }

  delete[] input;
  delete[] output;
  return success;
}
