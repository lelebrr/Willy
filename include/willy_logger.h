/**
 * @file willy_logger.h
 * @author Willy Firmware
 * @brief Sistema de Logging Centralizado para Willy
 * @version 1.0
 * 
 * Sistema de logging forense completo que registra todas as atividades
 * do sistema no cartão SD para análise pós-operação.
 * 
 * Funcionalidades:
 * - Logging de GPS, IR, WiFi, BLE, CC1101, NRF24, NFC
 * - Registro de erros e stack traces
 * - Timestamps precisos
 * - Rotação automática de logs
 * - Buffer em RAM para menor desgaste do SD
 */

#ifndef __WILLY_LOGGER_H__
#define __WILLY_LOGGER_H__

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <vector>

// Include globals for sdcardMounted, tft, tftWidth, tftHeight, etc.
#include "globals.h"

// Níveis de log (prefixo para evitar conflito com macros do NimBLE)
enum WillyLogLevel {
    WILLY_LOG_LEVEL_TRACE = 0,    // Detalhado para debug
    WILLY_LOG_LEVEL_DEBUG = 1,    // Debug normal
    WILLY_LOG_LEVEL_INFO = 2,     // Informações gerais
    WILLY_LOG_LEVEL_NOTICE = 3,   // Avisos importantes
    WILLY_LOG_LEVEL_WARNING = 4,  // Alertas
    WILLY_LOG_LEVEL_ERROR = 5,    // Erros
    WILLY_LOG_LEVEL_CRITICAL = 6, // Erros críticos
    WILLY_LOG_LEVEL_OFF = 7       // Log desativado
};

// Componentes do sistema
enum WillyLogComponent {
    COMP_SYSTEM,        // Sistema geral
    COMP_GPS,           // Módulo GPS
    COMP_IR,            // Infravermelho
    COMP_WIFI,          // WiFi
    COMP_BLE,           // Bluetooth
    COMP_CC1101,        // Rádio CC1101 (Sub-GHz)
    COMP_NRF24,         // Rádio NRF24
    COMP_NFC,           // NFC/RFID
    COMP_RF,            // RF genérico
    COMP_BADUSB,        // BadUSB
    COMP_ETHERNET,      // Ethernet
    COMP_SDCARD,        // Cartão SD
    COMP_DISPLAY,       // Display
    COMP_POWER,         // Energia/Bateria
    COMP_WEBUI,         // Interface Web
    COMP_INTERPRETER,   // Interpretador JS/Lua
    COMP_LORA,          // LoRa
    COMP_FM,            // FM Radio
    COMP_OTHER          // Outros
};

// Estrutura de entrada de log
struct WillyLogEntry {
    uint32_t timestamp;         // millis() ou Unix timestamp
    WillyLogLevel level;
    WillyLogComponent component;
    char message[192];          // Mensagem principal
    int32_t value1;             // Valores auxiliares
    int32_t value2;
    int32_t value3;
    uint32_t heapFree;          // Memória livre no momento
    uint16_t errorCode;         // Código de erro específico
};

// Configuração do logger
struct WillyLoggerConfig {
    bool enabled = true;                    // Log ativo
    WillyLogLevel minLevel = WILLY_LOG_LEVEL_INFO; // Nível mínimo para registrar
    bool logToSerial = true;                // Enviar para Serial também
    bool logToSD = true;                    // Salvar no SD
    bool includeHeap = true;                // Incluir heap livre
    bool includeTimestamp = true;           // Incluir timestamp
    uint32_t flushInterval = 5000;          // Intervalo de flush (ms)
    uint32_t maxLogSize = 1048576;          // Tamanho máximo por arquivo (1MB)
    uint32_t maxTotalSize = 10485760;       // Tamanho total máximo (10MB)
    uint8_t bufferEntries = 10;             // Entradas no buffer antes de flush
    String logDirectory = "/WILLY_LOGS";    // Diretório de logs
};

/**
 * @brief Classe principal do Sistema de Logging Willy
 */
class WillyLogger {
public:
    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    WillyLogger();
    ~WillyLogger();
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Inicialização e Configuração
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Inicializa o sistema de logging
     * @return true se inicializado com sucesso
     */
    bool begin();
    
    /**
     * @brief Para o sistema de logging
     */
    void end();
    
    /**
     * @brief Verifica se o logging está ativo
     * @return true se ativo
     */
    bool isEnabled() { return _config.enabled && _initialized; }
    
    /**
     * @brief Ativa/desativa o logging
     * @param enabled Estado desejado
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief Define o nível mínimo de log
     * @param level Nível mínimo
     */
    void setMinLevel(WillyLogLevel level);
    
    /**
     * @brief Obtém a configuração atual
     * @return Referência para a configuração
     */
    WillyLoggerConfig& getConfig() { return _config; }
    
    /**
     * @brief Carrega configuração do arquivo
     */
    void loadConfig();
    
    /**
     * @brief Salva configuração no arquivo
     */
    void saveConfig();
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Logging - Métodos Principais
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Registra uma entrada de log
     * @param level Nível do log
     * @param component Componente
     * @param message Mensagem formatada
     * @param v1 Valor auxiliar 1
     * @param v2 Valor auxiliar 2
     * @param v3 Valor auxiliar 3
     * @param errorCode Código de erro
     */
    void log(WillyLogLevel level, WillyLogComponent component, const char* message, 
             int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0, uint16_t errorCode = 0);
    
    /**
     * @brief Log formatado com printf-style
     */
    void logf(WillyLogLevel level, WillyLogComponent component, const char* format, ...);
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Logging - Atalhos por Nível
    /////////////////////////////////////////////////////////////////////////////////////
    
    void trace(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0);
    void debug(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0);
    void info(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0);
    void notice(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0);
    void warning(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0);
    void error(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0, uint16_t code = 0);
    void critical(WillyLogComponent comp, const char* msg, int32_t v1 = 0, int32_t v2 = 0, int32_t v3 = 0, uint16_t code = 0);
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Logging - Métodos Específicos por Componente
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Log de dados GPS
     */
    void logGPS(float lat, float lon, float alt, int satellites, uint32_t age, bool valid);
    
    /**
     * @brief Log de código IR capturado
     */
    void logIR(uint64_t code, uint16_t protocol, uint16_t bits, bool raw = false);
    
    /**
     * @brief Log de scan WiFi
     */
    void logWiFiScan(int apCount, int rssi, const char* ssid = nullptr);
    
    /**
     * @brief Log de dispositivo BLE
     */
    void logBLEScan(int deviceCount, const char* name = nullptr);
    
    /**
     * @brief Log de atividade CC1101
     */
    void logCC1101(float freq, int rssi, const char* mode);
    
    /**
     * @brief Log de atividade NRF24
     */
    void logNRF24(int channel, int packetCount, const char* mode);
    
    /**
     * @brief Log de tag NFC detectada
     */
    void logNFC(const char* uid, const char* type, bool readSuccess);
    
    /**
     * @brief Log de código RF Sub-GHz
     */
    void logRF(uint64_t code, float freq, int protocol, const char* raw = nullptr);
    
    /**
     * @brief Log de erro genérico com contexto
     */
    void logError(WillyLogComponent comp, const char* context, int errorCode, const char* details = nullptr);
    
    /**
     * @brief Log de status do sistema
     */
    void logSystemStatus();
    
    /**
     * @brief Log de inicialização de módulo
     */
    void logModuleInit(const char* moduleName, bool success, const char* details = nullptr);
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Gerenciamento de Arquivos
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Força flush do buffer para o SD
     */
    void flush();
    
    /**
     * @brief Cria novo arquivo de log
     * @return true se criado com sucesso
     */
    bool rotateLog();
    
    /**
     * @brief Limpa logs antigos
     * @param maxFiles Número máximo de arquivos a manter
     */
    void cleanupOldLogs(int maxFiles = 10);
    
    /**
     * @brief Obtém tamanho total dos logs
     * @return Tamanho em bytes
     */
    uint64_t getTotalLogSize();
    
    /**
     * @brief Obtém caminho do arquivo de log atual
     * @return Caminho completo
     */
    String getCurrentLogFile() { return _currentLogFile; }
    
    /**
     * @brief Lista arquivos de log
     * @return Vector com nomes dos arquivos
     */
    std::vector<String> listLogFiles();
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Métodos de Status
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Obtém nome string do nível
     */
    static const char* getLevelString(WillyLogLevel level);
    
    /**
     * @brief Obtém nome string do componente
     */
    static const char* getComponentString(WillyLogComponent comp);
    
    /**
     * @brief Obtém estatísticas do logger
     */
    void getStats(uint32_t& totalEntries, uint32_t& errorCount, uint32_t& warningCount);
    
    /**
     * @brief Exibe aviso de log ativo no display
     */
    void showLogWarning();

private:
    /////////////////////////////////////////////////////////////////////////////////////
    // Variáveis Privadas
    /////////////////////////////////////////////////////////////////////////////////////
    
    bool _initialized = false;
    WillyLoggerConfig _config;
    File _logFile;
    String _currentLogFile;
    
    // Buffer de entradas
    std::vector<WillyLogEntry> _buffer;
    uint8_t _bufferIndex = 0;
    
    // Estatísticas
    uint32_t _totalEntries = 0;
    uint32_t _errorCount = 0;
    uint32_t _warningCount = 0;
    
    // Controle de tempo
    uint32_t _lastFlush = 0;
    uint32_t _startTime = 0;
    
    // Mutex para thread safety
    SemaphoreHandle_t _mutex = nullptr;
    
    /////////////////////////////////////////////////////////////////////////////////////
    // Métodos Privados
    /////////////////////////////////////////////////////////////////////////////////////
    
    /**
     * @brief Escreve entrada no buffer
     */
    void writeEntry(const WillyLogEntry& entry);
    
    /**
     * @brief Escreve entrada formatada no arquivo
     */
    void writeToFile(const WillyLogEntry& entry);
    
    /**
     * @brief Escreve entrada na serial
     */
    void writeToSerial(const WillyLogEntry& entry);
    
    /**
     * @brief Gera nome de arquivo de log
     */
    String generateLogFilename();
    
    /**
     * @brief Verifica se precisa rotação
     */
    bool needsRotation();
    
    /**
     * @brief Cria diretório de logs se não existir
     */
    bool ensureLogDirectory();
    
    /**
     * @brief Obtém timestamp formatado
     */
    String getFormattedTimestamp();
    
    /**
     * @brief Obtém timestamp Unix (se disponível)
     */
    uint32_t getUnixTimestamp();
};

// Instância global do logger
extern WillyLogger willyLogger;

// Macros de conveniência para logging rápido
#define WILLY_TRACE(comp, msg, ...) willyLogger.trace(comp, msg, ##__VA_ARGS__)
#define WILLY_DEBUG(comp, msg, ...) willyLogger.debug(comp, msg, ##__VA_ARGS__)
#define WILLY_INFO(comp, msg, ...) willyLogger.info(comp, msg, ##__VA_ARGS__)
#define WILLY_NOTICE(comp, msg, ...) willyLogger.notice(comp, msg, ##__VA_ARGS__)
#define WILLY_WARNING(comp, msg, ...) willyLogger.warning(comp, msg, ##__VA_ARGS__)
#define WILLY_ERROR(comp, msg, ...) willyLogger.error(comp, msg, ##__VA_ARGS__)
#define WILLY_CRITICAL(comp, msg, ...) willyLogger.critical(comp, msg, ##__VA_ARGS__)

// Log de sistema
#define LOG_SYSTEM(msg, ...) willyLogger.info(COMP_SYSTEM, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) willyLogger.error(COMP_SYSTEM, msg, ##__VA_ARGS__)

// Logs específicos por módulo
#define LOG_GPS(msg, ...) willyLogger.info(COMP_GPS, msg, ##__VA_ARGS__)
#define LOG_IR(msg, ...) willyLogger.info(COMP_IR, msg, ##__VA_ARGS__)
#define LOG_WIFI(msg, ...) willyLogger.info(COMP_WIFI, msg, ##__VA_ARGS__)
#define LOG_BLE(msg, ...) willyLogger.info(COMP_BLE, msg, ##__VA_ARGS__)
#define LOG_CC1101(msg, ...) willyLogger.info(COMP_CC1101, msg, ##__VA_ARGS__)
#define LOG_NRF24(msg, ...) willyLogger.info(COMP_NRF24, msg, ##__VA_ARGS__)
#define LOG_NFC(msg, ...) willyLogger.info(COMP_NFC, msg, ##__VA_ARGS__)
#define LOG_RF(msg, ...) willyLogger.info(COMP_RF, msg, ##__VA_ARGS__)

#endif // __WILLY_LOGGER_H__