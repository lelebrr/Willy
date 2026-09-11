/**
 * @file subghz_advanced.h
 * @brief Willy SubGHz Advanced Suite - 55+ CC1101 Sub-GHz Functions
 * @author Willy Team
 * @date 2026
 */

#ifndef __SUBGHZ_ADVANCED_H__
#define __SUBGHZ_ADVANCED_H__

#include <Arduino.h>
#include <vector>

// ============================================================================
// Main entry point
// ============================================================================
void subghzAdvancedSuiteMenu();

// ============================================================================
// 1. SPECTRUM ANALYSIS (10 functions)
// ============================================================================
void subghzSpectrumAnalyzer();    // Analisador de espectro completo
void subghzFreqScanner();         // Scanner de frequencias
void subghzSignalDetector();      // Detector de sinais
void subghzNoiseFloor();          // Piso de ruido
void subghzPeakFinder();          // Encontra picos de sinal
void subghzBandwidthAnalyzer();   // Analise de largura de banda
void subghzModulationDetector();  // Detector de modulacao (ASK/FSK/OOK)
void subghzHarmonicFinder();      // Encontra harmonicas
void subghzInterferenceMap();     // Mapa de interferencia
void subghzSignalStrengthLog();   // Log de intensidade de sinal

// ============================================================================
// 2. SIGNAL CAPTURE & DECODE (10 functions)
// ============================================================================
void subghzSignalCapture();       // Captura de sinal bruto
void subghzProtocolDecoder();     // Decodificador de protocolo
void subghzBitAnalyzer();         // Analisador de bits
void subghzTimingDecoder();       // Decodificador de timing
void subghzPulseAnalyzer();       // Analisador de pulsos
void subghzMultiProtocolCapture();// Captura multi-protocolo
void subghzSignalComparison();    // Comparacao de sinais
void subghzSignalFingerprint();   // Fingerprint de sinal
void subghzSignalExporter();      // Export de sinal
void subghzSignalValidator();     // Validacao de sinal

// ============================================================================
// 3. TRANSMISSION & CLONING (10 functions)
// ============================================================================
void subghzSignalCloner();        // Clonador de sinal
void subghzProtocolGenerator();   // Gerador de protocolo
void subghzCodeBuilder();         // Construtor de codigo
void subghzRollingCodeGen();      // Gerador de rolling code
void subghzBruteForce();          // Brute force de codigo
void subghzSignalReplay();        // Replay de sinal
void subghzMultiFreqTransmit();   // Transmissao multi-frequencia
void subghzTimedTransmit();       // Transmissao temporizada
void subghzBulkTransmit();        // Transmissao em lote
void subghzSignalModulator();     // Modulador de sinal

// ============================================================================
// 4. DEVICE EMULATION (8 functions)
// ============================================================================
void subghzRemoteEmulator();      // Emulador de controle remoto
void subghzGarageOpener();        // Abridor de garagem
void subghzGateOpener();          // Abridor de portao
void subghzDoorbellEmulator();    // Emulador de campainha
void subghzAlarmEmulator();       // Emulador de alarme
void subghzSensorEmulator();      // Emulador de sensor (TPMS, etc.)
void subghzLightSwitch();         // Interruptor de luz
void subghzUniversalRemote();     // Controle universal

// ============================================================================
// 5. ATTACK VECTORS (8 functions)
// ============================================================================
void subghzReplayAttack();        // Ataque de replay
void subghzRollingCodeAttack();   // Ataque de rolling code
void subghzBruteForceAttack();    // Ataque de brute force
void subghzProtocolFuzzing();     // Fuzzing de protocolo
void subghzSignalJamming();       // Jamming de sinal
void subghzRollingCodeSniff();    // Sniff de rolling code
void subghzKeeLoqAttack();        // Ataque KeeLoq
void subghzFixedCodeAttack();     // Ataque de codigo fixo

// ============================================================================
// 6. ENVIRONMENTAL MONITORING (6 functions)
// ============================================================================
void subghzWeatherStation();      // Estacao meteorologica
void subghzTPMSMonitor();         // Monitor TPMS (pneus)
void subghzMotionDetector();      // Detector de movimento
void subghzDoorWindowSensor();    // Sensor porta/janela
void subghzTemperatureSensor();   // Sensor de temperatura
void subghzEnergyMonitor();       // Monitor de energia

// ============================================================================
// 7. UTILITIES & TOOLS (7 functions)
// ============================================================================
void subghzFrequencyConverter();  // Conversor de frequencia
void subghzCodeDatabase();        // Banco de codigos
void subghzSignalLibrary();       // Biblioteca de sinais
void subghzRecordingManager();    // Gerenciador de gravacoes
void subghzProtocolInfo();        // Info de protocolos
void subghzHardwareTest();        // Teste de hardware
void subghzExportToSD();          // Export para SD

// ============================================================================
// Menu functions
// ============================================================================
void subghzSpectrumMenu();        // Submenu de espectro
void subghzCaptureMenu();         // Submenu de captura
void subghzTransmitMenu();        // Submenu de transmissao
void subghzEmulateMenu();         // Submenu de emulacao
void subghzAttackMenu();          // Submenu de ataques
void subghzMonitorMenu();         // Submenu de monitoramento
void subghzUtilityMenu();         // Submenu de utilitarios

// ============================================================================
// Configuration
// ============================================================================
struct SubGHzAdvConfig {
    float baseFrequency;           // Frequencia base (315/433/868/915 MHz)
    int modulationType;            // 0=ASK, 1=FSK, 2=OOK
    float dataRate;                // Taxa de dados (kbps)
    float deviation;               // Desviacao de frequencia (kHz)
    float rxBW;                    // Largura de banda RX (kHz)
    int txPower;                   // Potencia TX (dBm)
    bool randomizeAddress;         // Randomizar endereco
    int repeatCount;               // Repeticoes por transmissao
    int delayBetween;              // Delay entre transmissoes (ms)
    bool autoDetect;               // Auto-detectar protocolo
    bool saveToSD;                 // Salvar automaticamente no SD
    bool showRawData;              // Mostrar dados brutos
    int protocolIndex;             // Indice do protocolo selecionado

    SubGHzAdvConfig() :
        baseFrequency(433.92f),
        modulationType(0),
        dataRate(4.8f),
        deviation(47.60f),
        rxBW(270.0f),
        txPower(10),
        randomizeAddress(false),
        repeatCount(10),
        delayBetween(1000),
        autoDetect(true),
        saveToSD(false),
        showRawData(true),
        protocolIndex(0) {}
};

extern SubGHzAdvConfig subghzAdvConfig;

void subghzAdvancedSetup();
void subghzAdvancedCleanup();
void subghzAdvancedDrawMenu();
void subghzAdvancedSuiteMenu();
void subghzAdvConfigMenu();

#endif // __SUBGHZ_ADVANCED_H__
