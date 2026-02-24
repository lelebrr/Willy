/**
 * @file advanced_ir_atks.h
 * @brief Módulo de ataques IR avançados para fins acadêmicos e pesquisa em segurança.
 * 
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 * 
 * @author Willy Team
 * @date 2026
 */

#ifndef __ADVANCED_IR_ATKS_H__
#define __ADVANCED_IR_ATKS_H__

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <vector>

//================================================================================
// ESTRUTURAS DE DADOS
//================================================================================

/**
 * @brief Estrutura para armazenar sinais IR aprendidos
 */
struct IRLearnedSignal {
    String name;
    String protocol;
    uint64_t data;
    uint16_t bits;
    uint16_t frequency;
    std::vector<uint16_t> rawData;
    uint32_t timing_min;
    uint32_t timing_max;
    uint32_t timing_avg;
    uint8_t duty_cycle;
};

/**
 * @brief Estrutura para predição de dispositivo IR
 */
struct IRDevicePrediction {
    String brand;
    String model;
    uint8_t confidence;
    std::vector<String> suggestedCommands;
};

/**
 * @brief Estrutura para código IR no banco de dados
 */
struct IRDatabaseCode {
    String brand;
    String device;
    String command;
    String protocol;
    uint64_t data;
    uint16_t bits;
};

/**
 * @brief Estrutura para análise de timing
 */
struct IRTimingAnalysis {
    uint32_t header_mark;
    uint32_t header_space;
    uint32_t bit_mark;
    uint32_t one_space;
    uint32_t zero_space;
    uint32_t gap;
    bool valid;
};

//================================================================================
// 1. IR PROTOCOL FUZZER
//================================================================================
/**
 * @brief IR Protocol Fuzzer - Testa códigos aleatórios por protocolo
 * 
 * O que faz: Testa códigos aleatórios por protocolo até achar crash.
 * Como faz: Loop random data em NEC/SIRC etc.
 * Para que serve: Descoberta de backdoors em TVs.
 */
void irProtocolFuzzer();

/**
 * @brief Configurações do IR Fuzzer
 */
struct IRFuzzerConfig {
    bool nec = true;
    bool sony = true;
    bool samsung = true;
    bool rc5 = true;
    bool rc6 = true;
    uint16_t codes_per_protocol = 100;
    uint16_t delay_ms = 50;
    bool random_address = true;
    bool random_command = true;
};

void irProtocolFuzzer(const IRFuzzerConfig& config);

//================================================================================
// 2. AUTO PATTERN MATCHER LEARNER
//================================================================================
/**
 * @brief Auto Pattern Matcher Learner - Aprende protocolos desconhecidos automaticamente
 * 
 * O que faz: Aprende protocolos desconhecidos automaticamente.
 * Como faz: Análise de timing + salvamento no SD.
 * Para que serve: Suporte a qualquer controle novo.
 */
void autoPatternMatcherLearner();

/**
 * @brief Analisa padrão de sinal IR
 */
IRTimingAnalysis analyzeIRPattern(const std::vector<uint16_t>& rawData);

/**
 * @brief Salva sinal aprendado no SD
 */
bool saveLearnedSignal(const IRLearnedSignal& signal, const String& filename);

/**
 * @brief Carrega sinais aprendados do SD
 */
std::vector<IRLearnedSignal> loadLearnedSignals(const String& filename);

//================================================================================
// 3. IR ANTI-REPLAY VARIABLE TIMING
//================================================================================
/**
 * @brief IR Anti-Replay Variable Timing - Replay com jitter
 * 
 * O que faz: Replay com jitter para burlar anti-replay.
 * Como faz: Adiciona variação de 5-15% nos tempos.
 * Para que serve: Funciona em sistemas modernos.
 */
void irAntiReplayVariableTiming();

/**
 * @brief Configurações de anti-replay
 */
struct IRAntiReplayConfig {
    uint8_t jitter_percent_min = 5;
    uint8_t jitter_percent_max = 15;
    uint8_t repeat_count = 3;
    uint32_t base_interval_ms = 100;
    bool randomize_order = false;
};

/**
 * @brief Aplica jitter a um sinal IR
 */
std::vector<uint16_t> applyJitter(const std::vector<uint16_t>& rawData, uint8_t jitterPercent);

//================================================================================
// 4. IR BURST OVERLOAD JAMMER
//================================================================================
/**
 * @brief IR Burst Overload Jammer - Flood multi-protocolo
 * 
 * O que faz: Flood multi-protocol para cegar todos os receptores IR da sala.
 * Como faz: Envio simultâneo de todos protocolos.
 * Para que serve: Bloqueio total de controles remotos.
 */
void irBurstOverloadJammer();

/**
 * @brief Configurações do jammer
 */
struct IRJammerConfig {
    bool all_protocols = true;
    bool nec = true;
    bool sony = true;
    bool samsung = true;
    bool rc5 = true;
    bool rc6 = true;
    bool lg = true;
    bool panasonic = true;
    uint16_t burst_duration_ms = 1000;
    uint16_t interval_ms = 100;
    uint32_t total_duration_ms = 0; // 0 = até cancelar
};

void irBurstOverloadJammer(const IRJammerConfig& config);

//================================================================================
// 5. IR WIFI REMOTE BRIDGE
//================================================================================
/**
 * @brief IR WiFi Remote Bridge - Controla TV via celular
 * 
 * O que faz: Controla TV via celular (web interface).
 * Como faz: WebUI do Bruce + IR send.
 * Para que serve: Controle remoto em pentest físico.
 */
void irWifiRemoteBridge();

/**
 * @brief Inicia servidor web IR
 */
void startIRWebServer();

/**
 * @brief Para servidor web IR
 */
void stopIRWebServer();

//================================================================================
// 6. STEALTH LOW-DUTY IR BLASTER
//================================================================================
/**
 * @brief Stealth Low-Duty IR Blaster - Envia comandos covert
 * 
 * O que faz: Envia comandos com duty cycle mínimo (quase invisível).
 * Como faz: Pulses espaçados.
 * Para que serve: Operações covert.
 */
void stealthLowDutyIRBlaster();

/**
 * @brief Configurações stealth
 */
struct IRStealthConfig {
    uint8_t duty_cycle_percent = 10; // 10% do normal
    uint32_t pulse_gap_us = 1000;    // Gap entre pulsos
    uint8_t repeat_count = 3;
    bool spread_spectrum = false;
};

/**
 * @brief Envia sinal IR stealth
 */
void sendStealthIR(const uint64_t data, uint16_t bits, decode_type_t protocol, const IRStealthConfig& config);

//================================================================================
// 7. SMART IR DEVICE PREDICTOR
//================================================================================
/**
 * @brief Smart IR Device Predictor - Adivinha marca por 1 sinal
 * 
 * O que faz: Depois de 1 sinal, adivinha marca e sugere comandos.
 * Como faz: Banco offline + matching.
 * Para que serve: Uso imediato sem aprender tudo.
 */
void smartIRDevicePredictor();

/**
 * @brief Prediz dispositivo baseado no sinal
 */
IRDevicePrediction predictDevice(decode_type_t protocol, uint64_t data, uint16_t bits);

/**
 * @brief Obtém comandos sugeridos para uma marca
 */
std::vector<IRDatabaseCode> getSuggestedCommands(const String& brand);

//================================================================================
// 8. OFFLINE IR DATABASE BRUTE
//================================================================================
/**
 * @brief Offline IR Database Brute - Testa todos códigos de uma marca
 * 
 * O que faz: Testa todos códigos conhecidos de uma marca.
 * Como faz: SD com 10k+ códigos.
 * Para que serve: Universal remoto destrutivo.
 */
void offlineIRDatabaseBrute();

/**
 * @brief Configurações de brute force
 */
struct IRBruteConfig {
    String brand;
    String device_type; // TV, AC, Projector, etc.
    uint16_t delay_ms = 100;
    bool stop_on_response = true;
    uint32_t max_codes = 10000;
};

/**
 * @brief Carrega banco de dados IR do SD
 */
std::vector<IRDatabaseCode> loadIRDatabase(const String& brand);

/**
 * @brief Executa brute force com configuração
 */
void executeIRBrute(const IRBruteConfig& config);

//================================================================================
// 9. SYNCHRONIZED MULTI-IR ATTACK
//================================================================================
/**
 * @brief Synchronized Multi-IR Attack - Controla múltiplos dispositivos
 * 
 * O que faz: Controla 5 dispositivos ao mesmo tempo.
 * Como faz: Envio paralelo.
 * Para que serve: Chaos em sala de reunião.
 */
void synchronizedMultiIRAttack();

/**
 * @brief Estrutura para dispositivo alvo
 */
struct IRTarget {
    String name;
    decode_type_t protocol;
    uint64_t power_code;
    uint64_t volume_up_code;
    uint64_t volume_down_code;
    uint64_t channel_up_code;
    uint64_t channel_down_code;
};

/**
 * @brief Configurações de ataque múltiplo
 */
struct MultiIRConfig {
    std::vector<IRTarget> targets;
    bool sync_power = false;
    bool sync_volume = false;
    uint8_t repeat_count = 2;
};

/**
 * @brief Adiciona alvo ao ataque
 */
void addIRTarget(const IRTarget& target);

/**
 * @brief Executa ataque em todos os dispositivos
 */
void executeMultiIRAttack(const MultiIRConfig& config);

//================================================================================
// 10. IR WAVEFORM VISUALIZER
//================================================================================
/**
 * @brief IR Waveform Visualizer - Mostra forma de onda no TFT
 * 
 * O que faz: Mostra forma de onda do sinal recebido no TFT.
 * Como faz: Plot LVGL em tempo real.
 * Para que serve: Análise avançada (útil + bonito no demo).
 */
void irWaveformVisualizer();

/**
 * @brief Estrutura para dados de visualização
 */
struct IRWaveformData {
    std::vector<uint16_t> rawTimings;
    std::vector<bool> states; // true = mark, false = space
    uint32_t totalTime;
    uint16_t pulseCount;
};

/**
 * @brief Captura waveform para visualização
 */
IRWaveformData captureIRWaveform();

/**
 * @brief Desenha waveform no display
 */
void drawIRWaveform(const IRWaveformData& data, int x, int y, int width, int height);

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

/**
 * @brief Menu principal de ataques IR avançados
 */
void advancedIRAtksMenu();

/**
 * @brief Inicializa modo de ataque IR
 */
bool initIRAttackMode();

/**
 * @brief Finaliza modo de ataque IR
 */
void deinitIRAttackMode();

/**
 * @brief Gera código IR aleatório
 */
uint64_t generateRandomIRCode();

/**
 * @brief Gera código aleatório para protocolo específico
 */
uint64_t generateRandomProtocolCode(decode_type_t protocol);

/**
 * @brief Converte protocolo para string
 */
String protocolToString(decode_type_t protocol);

/**
 * @brief Converte string para protocolo
 */
decode_type_t stringToProtocol(const String& str);

/**
 * @brief Detecta se houve resposta do dispositivo
 */
bool detectIRResponse();

#endif // __ADVANCED_IR_ATKS_H__
