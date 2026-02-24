/**
 * @file willy_logger.cpp
 * @author Willy Firmware
 * @brief Implementação do Sistema de Logging Centralizado
 * @version 1.0
 */

#include "willy_logger.h"
#include "sd_functions.h"
#include <cstdarg>
#include <time.h>

// Forward declaration
const char* getProtocolString(uint16_t protocol);

// Instância global
WillyLogger willyLogger;

// Strings de nível de log
const char* LEVEL_STRINGS[] = {
    "TRACE", "DEBUG", "INFO", "NOTICE", "WARN", "ERROR", "CRIT", "OFF"
};

// Strings de componente
const char* COMPONENT_STRINGS[] = {
    "SYSTEM", "GPS", "IR", "WIFI", "BLE", "CC1101", "NRF24", "NFC",
    "RF", "BADUSB", "ETH", "SD", "DISP", "PWR", "WEB", "INTERP",
    "LORA", "FM", "OTHER"
};

/////////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
/////////////////////////////////////////////////////////////////////////////////////

WillyLogger::WillyLogger() {
    _buffer.reserve(20);
    _startTime = millis();
}

WillyLogger::~WillyLogger() {
    end();
}

/////////////////////////////////////////////////////////////////////////////////////
// Inicialização
/////////////////////////////////////////////////////////////////////////////////////

bool WillyLogger::begin() {
    // Cria mutex para thread safety
    if (_mutex == nullptr) {
        _mutex = xSemaphoreCreateRecursiveMutex();
    }

    // Carrega configuração
    loadConfig();

    // Verifica se o SD está montado
    if (!sdcardMounted) {
        Serial.println(F("[WillyLogger] SD não montado - logging limitado"));
        _config.logToSD = false;
    }

    // Cria diretório de logs
    if (_config.logToSD && !ensureLogDirectory()) {
        Serial.println(F("[WillyLogger] Erro ao criar diretório de logs"));
        _config.logToSD = false;
    }

    // Cria arquivo de log inicial
    if (_config.logToSD) {
        _currentLogFile = generateLogFilename();
        _logFile = SD.open(_currentLogFile, FILE_APPEND);
        if (!_logFile) {
            Serial.println(F("[WillyLogger] Erro ao criar arquivo de log"));
            _config.logToSD = false;
        } else {
            // Escreve cabeçalho CSV
            _logFile.println(F("Timestamp,Level,Component,Message,Value1,Value2,Value3,HeapFree,ErrorCode"));
            _logFile.flush();
        }
    }

    _initialized = true;
    _lastFlush = millis();

    // Log inicial
    info(COMP_SYSTEM, "Willy Logger iniciado");
    logSystemStatus();

    Serial.println(F("[WillyLogger] Sistema de logging ativo"));
    return true;
}

void WillyLogger::end() {
    if (!_initialized) return;

    // Flush final
    flush();

    if (_logFile) {
        _logFile.println(F("# Log encerrado"));
        _logFile.close();
    }

    if (_mutex != nullptr) {
        vSemaphoreDelete(_mutex);
        _mutex = nullptr;
    }

    _initialized = false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Configuração
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::setEnabled(bool enabled) {
    _config.enabled = enabled;
    saveConfig();

    if (enabled && !_initialized) {
        begin();
    } else if (!enabled && _initialized) {
        flush();
    }
}

void WillyLogger::setMinLevel(WillyLogLevel level) {
    _config.minLevel = level;
    saveConfig();
}

void WillyLogger::loadConfig() {
    // Tenta carregar do arquivo de config principal
    if (LittleFS.exists("/willy_log.conf")) {
        File f = LittleFS.open("/willy_log.conf", FILE_READ);
        if (f) {
            JsonDocument doc;
            if (deserializeJson(doc, f) == DeserializationError::Ok) {
                _config.enabled = doc["enabled"] | true;
                _config.minLevel = (WillyLogLevel)(doc["minLevel"] | 2);
                _config.logToSerial = doc["logToSerial"] | true;
                _config.logToSD = doc["logToSD"] | true;
                _config.includeHeap = doc["includeHeap"] | true;
                _config.flushInterval = doc["flushInterval"] | 5000;
                _config.maxLogSize = doc["maxLogSize"] | 1048576;
                _config.bufferEntries = doc["bufferEntries"] | 10;
            }
            f.close();
        }
    }
}

void WillyLogger::saveConfig() {
    File f = LittleFS.open("/willy_log.conf", FILE_WRITE);
    if (f) {
        JsonDocument doc;
        doc["enabled"] = _config.enabled;
        doc["minLevel"] = (int)_config.minLevel;
        doc["logToSerial"] = _config.logToSerial;
        doc["logToSD"] = _config.logToSD;
        doc["includeHeap"] = _config.includeHeap;
        doc["flushInterval"] = _config.flushInterval;
        doc["maxLogSize"] = _config.maxLogSize;
        doc["bufferEntries"] = _config.bufferEntries;

        serializeJson(doc, f);
        f.close();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Logging Principal
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::log(WillyLogLevel level, WillyLogComponent component,
                       const char* message, int32_t v1, int32_t v2, int32_t v3,
                       uint16_t errorCode) {
    // Verifica se deve logar
    if (!_config.enabled || level < _config.minLevel) return;

    // Adquire mutex
    if (_mutex != nullptr) {
        if (xSemaphoreTakeRecursive(_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            return; // Timeout, skip this log
        }
    }

    // Cria entrada
    WillyLogEntry entry;
    entry.timestamp = millis();
    entry.level = level;
    entry.component = component;
    strncpy(entry.message, message, sizeof(entry.message) - 1);
    entry.message[sizeof(entry.message) - 1] = '\0';
    entry.value1 = v1;
    entry.value2 = v2;
    entry.value3 = v3;
    entry.heapFree = ESP.getFreeHeap();
    entry.errorCode = errorCode;

    // Atualiza estatísticas
    _totalEntries++;
    if (level == WILLY_LOG_LEVEL_ERROR) _errorCount++;
    if (level == WILLY_LOG_LEVEL_WARNING) _warningCount++;

    // Escreve na serial
    if (_config.logToSerial) {
        writeToSerial(entry);
    }

    // Adiciona ao buffer ou escreve direto
    if (_config.logToSD) {
        writeEntry(entry);
    }

    // Libera mutex
    if (_mutex != nullptr) {
        xSemaphoreGiveRecursive(_mutex);
    }
}

void WillyLogger::logf(WillyLogLevel level, WillyLogComponent component,
                        const char* format, ...) {
    if (!_config.enabled || level < _config.minLevel) return;

    char buffer[192];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    log(level, component, buffer);
}

/////////////////////////////////////////////////////////////////////////////////////
// Atalhos por Nível
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::trace(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3) {
    log(WILLY_LOG_LEVEL_TRACE, comp, msg, v1, v2, v3);
}

void WillyLogger::debug(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3) {
    log(WILLY_LOG_LEVEL_DEBUG, comp, msg, v1, v2, v3);
}

void WillyLogger::info(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3) {
    log(WILLY_LOG_LEVEL_INFO, comp, msg, v1, v2, v3);
}

void WillyLogger::notice(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3) {
    log(WILLY_LOG_LEVEL_NOTICE, comp, msg, v1, v2, v3);
}

void WillyLogger::warning(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3) {
    log(WILLY_LOG_LEVEL_WARNING, comp, msg, v1, v2, v3);
}

void WillyLogger::error(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3, uint16_t code) {
    log(WILLY_LOG_LEVEL_ERROR, comp, msg, v1, v2, v3, code);
}

void WillyLogger::critical(WillyLogComponent comp, const char* msg, int32_t v1, int32_t v2, int32_t v3, uint16_t code) {
    log(WILLY_LOG_LEVEL_CRITICAL, comp, msg, v1, v2, v3, code);
}

/////////////////////////////////////////////////////////////////////////////////////
// Logging Específico por Componente
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::logGPS(float lat, float lon, float alt, int satellites, uint32_t age, bool valid) {
    if (valid) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_GPS, "GPS Fix: lat=%.6f, lon=%.6f, alt=%.1fm, sats=%d, age=%lums",
             lat, lon, alt, satellites, age);
    } else {
        logf(WILLY_LOG_LEVEL_WARNING, COMP_GPS, "GPS sem fix: sats=%d, age=%lums", satellites, age);
    }
}

void WillyLogger::logIR(uint64_t code, uint16_t protocol, uint16_t bits, bool raw) {
    if (raw) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_IR, "IR Raw: code=0x%llX, bits=%d", code, bits);
    } else {
        logf(WILLY_LOG_LEVEL_INFO, COMP_IR, "IR Code: 0x%llX, protocol=%d (%s), bits=%d",
             code, protocol, getProtocolString(protocol), bits);
    }
}

void WillyLogger::logWiFiScan(int apCount, int rssi, const char* ssid) {
    if (ssid) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_WIFI, "WiFi Scan: %d APs, RSSI=%d dBm, SSID=%s", apCount, rssi, ssid);
    } else {
        logf(WILLY_LOG_LEVEL_INFO, COMP_WIFI, "WiFi Scan: %d APs encontrados, RSSI médio=%d dBm", apCount, rssi);
    }
}

void WillyLogger::logBLEScan(int deviceCount, const char* name) {
    if (name) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_BLE, "BLE Scan: %d devices, found=%s", deviceCount, name);
    } else {
        logf(WILLY_LOG_LEVEL_INFO, COMP_BLE, "BLE Scan: %d devices encontrados", deviceCount);
    }
}

void WillyLogger::logCC1101(float freq, int rssi, const char* mode) {
    logf(WILLY_LOG_LEVEL_INFO, COMP_CC1101, "CC1101: freq=%.3f MHz, RSSI=%d dBm, mode=%s", freq, rssi, mode);
}

void WillyLogger::logNRF24(int channel, int packetCount, const char* mode) {
    logf(WILLY_LOG_LEVEL_INFO, COMP_NRF24, "NRF24: ch=%d, packets=%d, mode=%s", channel, packetCount, mode);
}

void WillyLogger::logNFC(const char* uid, const char* type, bool readSuccess) {
    if (readSuccess) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_NFC, "NFC Tag: UID=%s, type=%s", uid, type);
    } else {
        logf(WILLY_LOG_LEVEL_WARNING, COMP_NFC, "NFC Erro na leitura: UID=%s", uid);
    }
}

void WillyLogger::logRF(uint64_t code, float freq, int protocol, const char* raw) {
    if (raw) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_RF, "RF Raw: freq=%.3f MHz, data=%s", freq, raw);
    } else {
        logf(WILLY_LOG_LEVEL_INFO, COMP_RF, "RF Code: 0x%llX, freq=%.3f MHz, protocol=%d", code, freq, protocol);
    }
}

void WillyLogger::logError(WillyLogComponent comp, const char* context, int errorCode, const char* details) {
    if (details) {
        logf(WILLY_LOG_LEVEL_ERROR, comp, "ERRO [%s]: code=%d, %s", context, errorCode, details);
    } else {
        logf(WILLY_LOG_LEVEL_ERROR, comp, "ERRO [%s]: code=%d", context, errorCode);
    }
}

void WillyLogger::logSystemStatus() {
    logf(WILLY_LOG_LEVEL_INFO, COMP_SYSTEM, "Status: heap=%d, freePSRAM=%d, millis=%lu",
         ESP.getFreeHeap(), ESP.getFreePsram(), millis());
}

void WillyLogger::logModuleInit(const char* moduleName, bool success, const char* details) {
    if (success) {
        logf(WILLY_LOG_LEVEL_INFO, COMP_SYSTEM, "Modulo iniciado: %s", moduleName);
    } else {
        logf(WILLY_LOG_LEVEL_ERROR, COMP_SYSTEM, "Falha ao iniciar modulo: %s - %s",
             moduleName, details ? details : "unknown");
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Gerenciamento de Arquivos
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::flush() {
    if (_mutex != nullptr) xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    if (!_config.logToSD || !_logFile) {
        if (_mutex != nullptr) xSemaphoreGiveRecursive(_mutex);
        return;
    }

    // Escreve todas as entradas do buffer
    for (const auto& entry : _buffer) {
        writeToFile(entry);
    }

    _buffer.clear();
    _bufferIndex = 0;

    if (_logFile) {
        _logFile.flush();
    }

    _lastFlush = millis();
    if (_mutex != nullptr) xSemaphoreGiveRecursive(_mutex);
}

bool WillyLogger::rotateLog() {
    if (_mutex != nullptr) xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    // Fecha arquivo atual
    if (_logFile) {
        _logFile.println(F("# Log rotacionado"));
        _logFile.close();
    }

    // Gera novo nome
    _currentLogFile = generateLogFilename();

    // Cria novo arquivo
    _logFile = SD.open(_currentLogFile, FILE_APPEND);
    if (!_logFile) {
        Serial.println(F("[WillyLogger] Erro ao rotacionar log"));
        if (_mutex != nullptr) xSemaphoreGiveRecursive(_mutex);
        return false;
    }

    // Cabeçalho
    _logFile.println(F("Timestamp,Level,Component,Message,Value1,Value2,Value3,HeapFree,ErrorCode"));
    _logFile.flush();

    // Limpa logs antigos
    cleanupOldLogs();

    if (_mutex != nullptr) xSemaphoreGiveRecursive(_mutex);
    return true;
}

void WillyLogger::cleanupOldLogs(int maxFiles) {
    File root = SD.open(_config.logDirectory);
    if (!root || !root.isDirectory()) return;

    std::vector<String> files;

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = file.name();
            if (name.startsWith("willy_") && name.endsWith(".csv")) {
                files.push_back(_config.logDirectory + "/" + name);
            }
        }
        file = root.openNextFile();
    }
    root.close();

    // Remove os mais antigos se exceder limite
    while (files.size() > (size_t)maxFiles) {
        // Encontra o mais antigo (menor timestamp no nome)
        int oldestIdx = 0;
        for (size_t i = 1; i < files.size(); i++) {
            if (files[i] < files[oldestIdx]) {
                oldestIdx = i;
            }
        }

        SD.remove(files[oldestIdx]);
        files.erase(files.begin() + oldestIdx);
    }
}

uint64_t WillyLogger::getTotalLogSize() {
    uint64_t total = 0;

    File root = SD.open(_config.logDirectory);
    if (!root || !root.isDirectory()) return 0;

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            total += file.size();
        }
        file = root.openNextFile();
    }
    root.close();

    return total;
}

std::vector<String> WillyLogger::listLogFiles() {
    std::vector<String> files;

    File root = SD.open(_config.logDirectory);
    if (!root || !root.isDirectory()) return files;

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = file.name();
            if (name.endsWith(".csv")) {
                files.push_back(_config.logDirectory + "/" + name);
            }
        }
        file = root.openNextFile();
    }
    root.close();

    return files;
}

/////////////////////////////////////////////////////////////////////////////////////
// Status e Informação
/////////////////////////////////////////////////////////////////////////////////////

const char* WillyLogger::getLevelString(WillyLogLevel level) {
    if (level >= WILLY_LOG_LEVEL_TRACE && level <= WILLY_LOG_LEVEL_OFF) {
        return LEVEL_STRINGS[level];
    }
    return "UNKNOWN";
}

const char* WillyLogger::getComponentString(WillyLogComponent comp) {
    if (comp >= COMP_SYSTEM && comp <= COMP_OTHER) {
        return COMPONENT_STRINGS[comp];
    }
    return "UNKNOWN";
}

void WillyLogger::getStats(uint32_t& totalEntries, uint32_t& errorCount, uint32_t& warningCount) {
    totalEntries = _totalEntries;
    errorCount = _errorCount;
    warningCount = _warningCount;
}

void WillyLogger::showLogWarning() {
    if (!_config.enabled) return;

    // Desenha aviso no display
    #if defined(HAS_SCREEN)
    tft.fillScreen(TFT_BLACK);

    // Ícone de aviso (triângulo)
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextSize(FM);
    tft.drawCentreString("LOG ATIVO", tftWidth / 2, 30, 1);

    // Informações
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(FP);
    tft.drawCentreString("Gravando em:", tftWidth / 2, 60, 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString(_currentLogFile.c_str(), tftWidth / 2, 78, 1);

    // Estatísticas
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString("Entradas: " + String(_totalEntries), tftWidth / 2, 105, 1);

    if (_errorCount > 0) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("Erros: " + String(_errorCount), tftWidth / 2, 123, 1);
    }

    // Heap
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawCentreString("Heap: " + String(ESP.getFreeHeap()) + " bytes", tftWidth / 2, tftHeight - 20, 1);

    delay(2000);
    tft.fillScreen(TFT_BLACK);
    #endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Métodos Privados
/////////////////////////////////////////////////////////////////////////////////////

void WillyLogger::writeEntry(const WillyLogEntry& entry) {
    // Adiciona ao buffer
    _buffer.push_back(entry);
    _bufferIndex++;

    // Verifica se precisa flush
    if (_bufferIndex >= _config.bufferEntries ||
        millis() - _lastFlush > _config.flushInterval) {
        flush();
    }

    // Verifica rotação
    if (needsRotation()) {
        rotateLog();
    }
}

void WillyLogger::writeToFile(const WillyLogEntry& entry) {
    if (!_logFile) return;

    // Formato CSV
    _logFile.print(millis());
    _logFile.print(",");
    _logFile.print(getLevelString(entry.level));
    _logFile.print(",");
    _logFile.print(getComponentString(entry.component));
    _logFile.print(",\"");
    _logFile.print(entry.message);
    _logFile.print("\",");
    _logFile.print(entry.value1);
    _logFile.print(",");
    _logFile.print(entry.value2);
    _logFile.print(",");
    _logFile.print(entry.value3);
    _logFile.print(",");
    _logFile.print(entry.heapFree);
    _logFile.print(",");
    _logFile.println(entry.errorCode);
}

void WillyLogger::writeToSerial(const WillyLogEntry& entry) {
    // Formato: [timestamp] LEVEL [COMP]: message
    Serial.print(F("["));
    Serial.print(entry.timestamp / 1000);
    Serial.print(F("s] "));
    Serial.print(getLevelString(entry.level));
    Serial.print(F(" ["));
    Serial.print(getComponentString(entry.component));
    Serial.print(F("]: "));
    Serial.print(entry.message);

    // Valores auxiliares
    if (entry.value1 != 0 || entry.value2 != 0 || entry.value3 != 0) {
        Serial.print(F(" | v1="));
        Serial.print(entry.value1);
        Serial.print(F(" v2="));
        Serial.print(entry.value2);
        Serial.print(F(" v3="));
        Serial.print(entry.value3);
    }

    // Heap
    if (_config.includeHeap) {
        Serial.print(F(" | heap="));
        Serial.print(entry.heapFree);
    }

    // ErrorCode
    if (entry.errorCode > 0) {
        Serial.print(F(" | err="));
        Serial.print(entry.errorCode);
    }

    Serial.println();
}

String WillyLogger::generateLogFilename() {
    // Formato: /WILLY_LOGS/willy_YYYYMMDD_HHMMSS.csv
    char filename[64];

    uint32_t ts = millis();

    // Tenta usar tempo real se disponível
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        snprintf(filename, sizeof(filename), "%s/willy_%04d%02d%02d_%02d%02d%02d.csv",
                 _config.logDirectory.c_str(),
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        snprintf(filename, sizeof(filename), "%s/willy_%lu.csv",
                 _config.logDirectory.c_str(), ts);
    }

    return String(filename);
}

bool WillyLogger::needsRotation() {
    if (!_logFile) return false;
    return _logFile.size() > _config.maxLogSize;
}

bool WillyLogger::ensureLogDirectory() {
    if (!SD.exists(_config.logDirectory)) {
        if (!SD.mkdir(_config.logDirectory)) {
            return false;
        }
    }
    return true;
}

String WillyLogger::getFormattedTimestamp() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return String(buf);
    }
    return String(millis() / 1000) + "s";
}

uint32_t WillyLogger::getUnixTimestamp() {
    time_t now;
    time(&now);
    return (uint32_t)now;
}

// Função auxiliar para obter string de protocolo IR
const char* getProtocolString(uint16_t protocol) {
    static const char* protocols[] = {
        "UNKNOWN", "RC5", "RC6", "NEC", "SONY", "PANASONIC", "JVC",
        "SAMSUNG", "WHYNTER", "AIWA_RC_T501", "LG", "SANYO", "MITSUBISHI",
        "DISH", "SHARP", "COOLIX", "DAIKIN", "DENON", "KELVINATOR",
        "SHERWOOD", "MITSUBISHI_AC", "RCMM", "GREE", "PRONTO", "NEC_LIKE"
    };

    if (protocol < sizeof(protocols) / sizeof(protocols[0])) {
        return protocols[protocol];
    }
    return "UNKNOWN";
}
