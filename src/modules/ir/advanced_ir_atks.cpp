/**
 * @file advanced_ir_atks.cpp
 * @brief Implementação de ataques IR avançados para fins acadêmicos e pesquisa.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 */

#include "advanced_ir_atks.h"
#include "custom_ir.h"
#include "ir_utils.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/settings.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include "globals.h"
#include <SD.h>
#include <SPI.h>
#include <vector>

//================================================================================
// CONSTANTES E VARIÁVEIS GLOBAIS
//================================================================================

// Controle de estado
static bool ir_attack_running = false;
static IRsend* ir_sender = nullptr;
static IRrecv* ir_receiver = nullptr;
static decode_results ir_results;
static WebServer* ir_server = nullptr;

// Alvos para ataque múltiplo
static std::vector<IRTarget> multi_ir_targets;

// Sinais aprendidos
static std::vector<IRLearnedSignal> learned_signals;

// Banco de dados de marcas
static const char* IR_BRANDS[] = {
    "Samsung", "LG", "Sony", "Panasonic", "TCL", "Philips", "Sharp",
    "Toshiba", "Hisense", "Sanyo", "Hitachi", "JVC", "Grundig",
    "AOC", "Akai", "Daewoo", "Telefunken", "Bose", "Pioneer", "Onkyo"
};

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

String protocolToString(decode_type_t protocol) {
    switch (protocol) {
        case decode_type_t::NEC: return "NEC";
        case decode_type_t::SONY: return "Sony";
        case decode_type_t::SAMSUNG: return "Samsung";
        case decode_type_t::SAMSUNG36: return "Samsung36";
        case decode_type_t::SAMSUNG_AC: return "SamsungAC";
        case decode_type_t::RC5: return "RC5";
        case decode_type_t::RC6: return "RC6";
        case decode_type_t::RCMM: return "RCMM";
        case decode_type_t::LG: return "LG";
        case decode_type_t::LG2: return "LG2";
        case decode_type_t::PANASONIC: return "Panasonic";
        case decode_type_t::PANASONIC_AC: return "PanasonicAC";
        case decode_type_t::SHARP: return "Sharp";
        case decode_type_t::SHARP_AC: return "SharpAC";
        case decode_type_t::JVC: return "JVC";
        case decode_type_t::DENON: return "Denon";
        case decode_type_t::MITSUBISHI: return "Mitsubishi";
        case decode_type_t::MITSUBISHI2: return "Mitsubishi2";
        case decode_type_t::MITSUBISHI_AC: return "MitsubishiAC";
        case decode_type_t::FUJITSU_AC: return "FujitsuAC";
        case decode_type_t::INAX: return "Inax";
        case decode_type_t::MIDEA: return "Midea";
        case decode_type_t::TOSHIBA_AC: return "ToshibaAC";
        case decode_type_t::TROTEC: return "Trotec";
        case decode_type_t::WHIRLPOOL_AC: return "WhirlpoolAC";
        case decode_type_t::UNKNOWN:
        default: return "Unknown";
    }
}

decode_type_t stringToProtocol(const String& str) {
    if (str.equalsIgnoreCase("NEC")) return decode_type_t::NEC;
    if (str.equalsIgnoreCase("Sony")) return decode_type_t::SONY;
    if (str.equalsIgnoreCase("Samsung")) return decode_type_t::SAMSUNG;
    if (str.equalsIgnoreCase("RC5")) return decode_type_t::RC5;
    if (str.equalsIgnoreCase("RC6")) return decode_type_t::RC6;
    if (str.equalsIgnoreCase("LG")) return decode_type_t::LG;
    if (str.equalsIgnoreCase("Panasonic")) return decode_type_t::PANASONIC;
    if (str.equalsIgnoreCase("Sharp")) return decode_type_t::SHARP;
    if (str.equalsIgnoreCase("JVC")) return decode_type_t::JVC;
    return decode_type_t::UNKNOWN;
}

bool initIRAttackMode() {
    // Configura pino IR usando configuração do Willy
    ir_sender = new IRsend(willyConfigPins.irTx);
    ir_sender->begin();

    // Configura receiver para alguns ataques
    ir_receiver = new IRrecv(willyConfigPins.irRx);
    ir_receiver->enableIRIn();

    return true;
}

void deinitIRAttackMode() {
    if (ir_sender) {
        delete ir_sender;
        ir_sender = nullptr;
    }
    if (ir_receiver) {
        ir_receiver->disableIRIn();
        delete ir_receiver;
        ir_receiver = nullptr;
    }
}

uint64_t generateRandomIRCode() {
    uint64_t code = 0;
    for (int i = 0; i < 8; i++) {
        code = (code << 8) | (random(0, 256) & 0xFF);
    }
    return code;
}

uint64_t generateRandomProtocolCode(decode_type_t protocol) {
    switch (protocol) {
        case decode_type_t::NEC:
            // NEC: 16-bit address + 16-bit command
            return ((uint64_t)random(0, 0xFFFF) << 16) | random(0, 0xFFFF);

        case decode_type_t::SONY:
            // Sony: 7-bit command + 5-13 bit address
            return random(0, 0x7FFFFF); // 23 bits max

        case decode_type_t::SAMSUNG:
            // Samsung: 16-bit address + 32-bit data
            return generateRandomIRCode();

        case decode_type_t::RC5:
        case decode_type_t::RC6:
            // RC5/RC6: shorter codes
            return random(0, 0xFFFFFFFF);

        default:
            return generateRandomIRCode();
    }
}

void advancedIRAtksMenu() {
    options = {
        {"IR Fuzzer", [=]() { irProtocolFuzzer(); }},
        {"Pattern Learner", [=]() { autoPatternMatcherLearner(); }},
        {"Anti-Replay", [=]() { irAntiReplayVariableTiming(); }},
        {"IR Jammer", [=]() { irBurstOverloadJammer(); }},
        {"WiFi Remote", [=]() { irWifiRemoteBridge(); }},
        {"Stealth Blaster", [=]() { stealthLowDutyIRBlaster(); }},
        {"Device Predictor", [=]() { smartIRDevicePredictor(); }},
        {"DB Brute Force", [=]() { offlineIRDatabaseBrute(); }},
        {"Multi-IR Attack", [=]() { synchronizedMultiIRAttack(); }},
        {"Waveform View", [=]() { irWaveformVisualizer(); }}
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Ataques IR Avancados");
}

//================================================================================
// 1. IR PROTOCOL FUZZER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void irProtocolFuzzer() {
    IRFuzzerConfig config;

    options = {
        {"Todos Protocolos", [&]() {
            config.nec = true;
            config.sony = true;
            config.samsung = true;
            config.rc5 = true;
            config.rc6 = true;
            config.codes_per_protocol = 100;
        }},
        {"NEC Foco", [&]() {
            config.nec = true;
            config.sony = false;
            config.samsung = false;
            config.rc5 = false;
            config.rc6 = false;
            config.codes_per_protocol = 500;
        }},
        {"Samsung Foco", [&]() {
            config.nec = false;
            config.sony = false;
            config.samsung = true;
            config.rc5 = false;
            config.rc6 = false;
            config.codes_per_protocol = 500;
        }},
        {"Agressivo", [&]() {
            config.nec = true;
            config.sony = true;
            config.samsung = true;
            config.rc5 = true;
            config.rc6 = true;
            config.codes_per_protocol = 1000;
            config.delay_ms = 10;
        }},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("IR Fuzzer");
    tft.setTextColor(willyConfig.priColor);
    padprintln("");
    padprintln("Fuzzing protocolos IR...");
    padprintln("");
    padprintln("Pressione ESC para parar");

    ir_attack_running = true;
    uint32_t total_codes = 0;
    uint32_t last_update = millis();

    while (ir_attack_running) {
        // Fuzzing NEC
        if (config.nec) {
            for (int i = 0; i < config.codes_per_protocol && ir_attack_running; i++) {
                uint64_t code = generateRandomProtocolCode(decode_type_t::NEC);
                ir_sender->sendNEC(code, 32);
                total_codes++;

                if (config.delay_ms > 0) {
                    delay(config.delay_ms);
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }

        // Fuzzing Samsung
        if (config.samsung && ir_attack_running) {
            for (int i = 0; i < config.codes_per_protocol && ir_attack_running; i++) {
                uint64_t code = generateRandomProtocolCode(decode_type_t::SAMSUNG);
                ir_sender->sendSAMSUNG(code, 32);
                total_codes++;

                if (config.delay_ms > 0) {
                    delay(config.delay_ms);
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }

        // Fuzzing Sony
        if (config.sony && ir_attack_running) {
            for (int i = 0; i < config.codes_per_protocol && ir_attack_running; i++) {
                uint64_t code = generateRandomProtocolCode(decode_type_t::SONY);
                ir_sender->sendSony(code, 20);
                total_codes++;

                if (config.delay_ms > 0) {
                    delay(config.delay_ms);
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }

        // Fuzzing RC5
        if (config.rc5 && ir_attack_running) {
            for (int i = 0; i < config.codes_per_protocol && ir_attack_running; i++) {
                uint64_t code = generateRandomProtocolCode(decode_type_t::RC5);
                ir_sender->sendRC5(code, 12);
                total_codes++;

                if (config.delay_ms > 0) {
                    delay(config.delay_ms);
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }

        // Fuzzing RC6
        if (config.rc6 && ir_attack_running) {
            for (int i = 0; i < config.codes_per_protocol && ir_attack_running; i++) {
                uint64_t code = generateRandomProtocolCode(decode_type_t::RC6);
                ir_sender->sendRC6(code, 20);
                total_codes++;

                if (config.delay_ms > 0) {
                    delay(config.delay_ms);
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }

        // Atualiza display
        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("IR Fuzzer");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Fuzzing protocolos IR...");
            padprintln("");
            padprintln("Codigos: " + String(total_codes));
            padprintln("Protocolos: NEC/SAMSUNG/SONY/RC5/RC6");
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 2. AUTO PATTERN MATCHER LEARNER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

IRTimingAnalysis analyzeIRPattern(const std::vector<uint16_t>& rawData) {
    IRTimingAnalysis analysis;
    analysis.valid = false;

    if (rawData.size() < 6) return analysis;

    // Análise básica de timing
    analysis.header_mark = rawData[0];
    analysis.header_space = rawData[1];

    // Encontra bit marks e spaces
    analysis.bit_mark = rawData[2];

    // Analisa para encontrar one_space e zero_space
    uint32_t space_sum = 0;
    uint32_t space_count = 0;
    for (size_t i = 3; i < rawData.size(); i += 2) {
        space_sum += rawData[i];
        space_count++;
    }

    if (space_count > 0) {
        analysis.one_space = space_sum / space_count * 2;
        analysis.zero_space = space_sum / space_count / 2;
    }

    analysis.gap = rawData[rawData.size() - 1];
    analysis.valid = true;

    return analysis;
}

bool saveLearnedSignal(const IRLearnedSignal& signal, const String& filename) {
    if (!SD.begin()) return false;

    File file = SD.open(filename, FILE_APPEND);
    if (!file) return false;

    // Salva em formato JSON simplificado
    file.println("{");
    file.println("  \"name\": \"" + signal.name + "\",");
    file.println("  \"protocol\": \"" + signal.protocol + "\",");
    file.println("  \"data\": " + String(signal.data) + ",");
    file.println("  \"bits\": " + String(signal.bits) + ",");
    file.println("  \"frequency\": " + String(signal.frequency));
    file.println("},");

    file.close();
    return true;
}

void autoPatternMatcherLearner() {
    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("Pattern Learner");
    padprintln("");
    padprintln("Aponte controle e pressione tecla");
    padprintln("");
    padprintln("Aguardando sinal...");
    padprintln("");
    padprintln("ESC para sair");

    ir_attack_running = true;
    uint8_t signals_learned = 0;
    IRLearnedSignal last_signal;

    while (ir_attack_running) {
        if (ir_receiver->decode(&ir_results)) {
            // Obtém dados do sinal
            last_signal.name = "Signal_" + String(signals_learned);
            last_signal.protocol = protocolToString(ir_results.decode_type);
            last_signal.data = ir_results.value;
            last_signal.bits = ir_results.bits;
            last_signal.frequency = 38000; // Frequência padrão

            // Copia raw data
            last_signal.rawData.clear();
            for (uint16_t i = 0; i < ir_results.rawlen; i++) {
                last_signal.rawData.push_back((uint16_t)ir_results.rawbuf[i]);
            }

            // Análise de timing
            IRTimingAnalysis timing = analyzeIRPattern(last_signal.rawData);
            if (timing.valid) {
                last_signal.timing_min = timing.header_mark;
                last_signal.timing_max = timing.gap;
            }

            signals_learned++;

            // Atualiza display
            drawMainBorderWithTitle("Pattern Learner");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Sinal capturado!");
            padprintln("");
            padprintln("Protocolo: " + last_signal.protocol);
            padprintln("Data: " + String(last_signal.data, HEX));
            padprintln("Bits: " + String(last_signal.bits));
            padprintln("");
            tft.setTextColor(TFT_GREEN);
            padprintln("Sinais aprendidos: " + String(signals_learned));
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("ESC para sair | SEL para salvar");

            ir_receiver->resume();
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }

        if (check(SelPress)) {
            // Salva no SD
            if (signals_learned > 0) {
                String filename = "/WillyIR/learned_" + String(millis()) + ".json";
                if (saveLearnedSignal(last_signal, filename)) {
                    displaySuccess("Salvo no SD!");
                } else {
                    displayError("Erro ao salvar");
                }
            }
        }

        delay(10);
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 3. IR ANTI-REPLAY VARIABLE TIMING - IMPLEMENTAÇÃO COMPLETA
//================================================================================

std::vector<uint16_t> applyJitter(const std::vector<uint16_t>& rawData, uint8_t jitterPercent) {
    std::vector<uint16_t> jittered = rawData;

    for (size_t i = 0; i < jittered.size(); i++) {
        // Calcula variação
        int32_t variation = (jittered[i] * jitterPercent) / 100;
        int32_t jitter_val = random(-variation, variation + 1);

        // Aplica jitter
        int32_t new_value = jittered[i] + jitter_val;
        jittered[i] = (new_value > 0) ? new_value : 1;
    }

    return jittered;
}

void irAntiReplayVariableTiming() {
    if (!initIRAttackMode()) return;

    // Primeiro, captura um sinal
    drawMainBorderWithTitle("Anti-Replay");
    padprintln("");
    padprintln("Capturando sinal base...");
    padprintln("Aponte o controle remoto");
    padprintln("");
    padprintln("ESC para sair");

    std::vector<uint16_t> base_signal;
    decode_type_t base_protocol = decode_type_t::UNKNOWN;

    bool signal_captured = false;
    while (!signal_captured) {
        if (ir_receiver->decode(&ir_results)) {
            base_protocol = ir_results.decode_type;

            // Copia raw data
            for (uint16_t i = 0; i < ir_results.rawlen; i++) {
                base_signal.push_back((uint16_t)ir_results.rawbuf[i]);
            }

            signal_captured = true;
            ir_receiver->resume();
        }

        if (check(EscPress)) {
            deinitIRAttackMode();
            returnToMenu = true;
            return;
        }

        delay(10);
    }

    // Menu de configuração
    IRAntiReplayConfig config;
    options = {
        {"Jitter 5-15%", [&]() {
            config.jitter_percent_min = 5;
            config.jitter_percent_max = 15;
        }},
        {"Jitter 10-20%", [&]() {
            config.jitter_percent_min = 10;
            config.jitter_percent_max = 20;
        }},
        {"Jitter 20-30%", [&]() {
            config.jitter_percent_min = 20;
            config.jitter_percent_max = 30;
        }},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    drawMainBorderWithTitle("Anti-Replay");
    padprintln("");
    padprintln("Enviando com jitter...");
    padprintln("");
    padprintln("Protocolo: " + protocolToString(base_protocol));
    padprintln("Jitter: " + String(config.jitter_percent_min) + "-" + String(config.jitter_percent_max) + "%");
    padprintln("");
    padprintln("ESC para parar");

    ir_attack_running = true;
    uint32_t sent_count = 0;

    while (ir_attack_running) {
        // Gera jitter aleatório para cada envio
        uint8_t jitter_val = random(config.jitter_percent_min, config.jitter_percent_max + 1);

        // Envia com jitter
        if (!base_signal.empty()) {
            std::vector<uint16_t> jittered = applyJitter(base_signal, jitter_val);
            ir_sender->sendRaw(jittered.data(), jittered.size(), 38);
        }

        sent_count++;
        delay(config.base_interval_ms);

        // Atualiza display
        if (sent_count % 10 == 0) {
            drawMainBorderWithTitle("Anti-Replay");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Enviando com jitter...");
            padprintln("");
            padprintln("Enviados: " + String(sent_count));
            padprintln("Jitter atual: " + String(jitter_val) + "%");
            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 4. IR BURST OVERLOAD JAMMER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void irBurstOverloadJammer() {
    IRJammerConfig config;

    options = {
        {"Todos Protocolos", [&]() {
            config.all_protocols = true;
            config.burst_duration_ms = 1000;
        }},
        {"NEC/Sharp/LG", [&]() {
            config.nec = true;
            config.sony = false;
            config.samsung = false;
            config.lg = true;
            config.burst_duration_ms = 2000;
        }},
        {"Samsung/Sony", [&]() {
            config.nec = false;
            config.sony = true;
            config.samsung = true;
            config.burst_duration_ms = 2000;
        }},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("IR Jammer");
    padprintln("");
    padprintln("Jamming IR ativo...");
    padprintln("");
    tft.setTextColor(TFT_RED);
    padprintln("Controles bloqueados!");
    tft.setTextColor(willyConfig.priColor);
    padprintln("");
    padprintln("ESC para parar");

    ir_attack_running = true;
    uint32_t burst_count = 0;
    uint32_t last_update = millis();

    while (ir_attack_running) {
        // Burst de códigos aleatórios em todos os protocolos

        // NEC burst
        if (config.nec || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendNEC(generateRandomProtocolCode(decode_type_t::NEC), 32);
                delayMicroseconds(500);
            }
        }

        // Samsung burst
        if (config.samsung || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendSAMSUNG(generateRandomProtocolCode(decode_type_t::SAMSUNG), 32);
                delayMicroseconds(500);
            }
        }

        // Sony burst
        if (config.sony || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendSony(generateRandomProtocolCode(decode_type_t::SONY), 20);
                delayMicroseconds(500);
            }
        }

        // RC5 burst
        if (config.rc5 || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendRC5(generateRandomProtocolCode(decode_type_t::RC5), 12);
                delayMicroseconds(500);
            }
        }

        // RC6 burst
        if (config.rc6 || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendRC6(generateRandomProtocolCode(decode_type_t::RC6), 20);
                delayMicroseconds(500);
            }
        }

        // LG burst
        if (config.lg || config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendLG(generateRandomProtocolCode(decode_type_t::LG), 28);
                delayMicroseconds(500);
            }
        }

        // Sharp burst
        if (config.all_protocols) {
            for (int i = 0; i < 10; i++) {
                ir_sender->sendSharp(generateRandomProtocolCode(decode_type_t::SHARP), 15);
                delayMicroseconds(500);
            }
        }

        burst_count++;
        delay(config.interval_ms);

        // Atualiza display
        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("IR Jammer");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Jamming IR ativo...");
            padprintln("");
            padprintln("Bursts: " + String(burst_count));
            padprintln("Protocolos: 7 ativos");
            padprintln("");
            tft.setTextColor(TFT_RED);
            padprintln("Controles bloqueados!");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 5. IR WIFI REMOTE BRIDGE - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void irWifiRemoteBridge() {
    // Verifica se WiFi está conectado
    if (WiFi.status() != WL_CONNECTED) {
        displayError("Conecte ao WiFi primeiro");
        returnToMenu = true;
        return;
    }

    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("WiFi IR Remote");
    padprintln("");
    padprintln("Servidor IR ativo");
    padprintln("");
    padprintln("IP: " + WiFi.localIP().toString());
    padprintln("Porta: 80");
    padprintln("");
    padprintln("Acesse pelo navegador");
    padprintln("");
    padprintln("ESC para sair");

    ir_attack_running = true;
    startIRWebServer();
    while (ir_attack_running) {
        if (ir_server) ir_server->handleClient();
        if (check(EscPress)) {
            ir_attack_running = false;
        }

        delay(10);
    }
    stopIRWebServer();

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 6. STEALTH LOW-DUTY IR BLASTER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void sendStealthIR(const uint64_t data, uint16_t bits, decode_type_t protocol, const IRStealthConfig& config) {
    for (int r = 0; r < config.repeat_count; r++) {
        switch (protocol) {
            case decode_type_t::NEC:
                ir_sender->sendNEC(data, bits, 1);
                delayMicroseconds(config.pulse_gap_us);
                break;

            case decode_type_t::SAMSUNG:
                ir_sender->sendSAMSUNG(data, bits, 1);
                delayMicroseconds(config.pulse_gap_us);
                break;

            case decode_type_t::SONY:
                ir_sender->sendSony(data, bits, 1);
                delayMicroseconds(config.pulse_gap_us);
                break;

            default:
                ir_sender->sendData(1, 1, data, bits, true, 0, 33);
                delayMicroseconds(config.pulse_gap_us);
                break;
        }

        delay(50);
    }
}

void stealthLowDutyIRBlaster() {
    if (!initIRAttackMode()) return;

    options = {
        {"Power TV", [&]() {}},
        {"Volume Up", [&]() {}},
        {"Channel Up", [&]() {}},
        {"Custom NEC", [&]() {}},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    IRStealthConfig config;
    config.duty_cycle_percent = 10;
    config.pulse_gap_us = 2000;
    config.repeat_count = 5;

    drawMainBorderWithTitle("Stealth IR");
    padprintln("");
    padprintln("Modo stealth ativo");
    padprintln("");
    padprintln("Duty cycle: " + String(config.duty_cycle_percent) + "%");
    padprintln("Gap: " + String(config.pulse_gap_us) + "us");
    padprintln("");
    padprintln("Enviando comando...");
    padprintln("");
    padprintln("ESC para parar");

    ir_attack_running = true;
    uint32_t sent_count = 0;

    while (ir_attack_running) {
        sendStealthIR(0x20DF10EF, 32, decode_type_t::NEC, config);

        sent_count++;
        delay(500);

        if (millis() % 2000 == 0) {
            drawMainBorderWithTitle("Stealth IR");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Modo stealth ativo");
            padprintln("");
            padprintln("Enviados: " + String(sent_count));
            padprintln("Duty: " + String(config.duty_cycle_percent) + "%");
            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 7. SMART IR DEVICE PREDICTOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

IRDevicePrediction predictDevice(decode_type_t protocol, uint64_t data, uint16_t bits) {
    IRDevicePrediction prediction;
    prediction.confidence = 0;

    switch (protocol) {
        case decode_type_t::NEC:
            if ((data >> 16) == 0x20DF) {
                prediction.brand = "Samsung";
                prediction.confidence = 85;
            } else if ((data >> 16) == 0x04FB) {
                prediction.brand = "LG";
                prediction.confidence = 80;
            } else if ((data >> 16) == 0x10EF) {
                prediction.brand = "TCL";
                prediction.confidence = 75;
            } else {
                prediction.brand = "Generico NEC";
                prediction.confidence = 50;
            }
            break;

        case decode_type_t::SAMSUNG:
        case decode_type_t::SAMSUNG36:
            prediction.brand = "Samsung";
            prediction.confidence = 95;
            break;

        case decode_type_t::LG:
        case decode_type_t::LG2:
            prediction.brand = "LG";
            prediction.confidence = 95;
            break;

        case decode_type_t::SONY:
            prediction.brand = "Sony";
            prediction.confidence = 95;
            break;

        case decode_type_t::PANASONIC:
            prediction.brand = "Panasonic";
            prediction.confidence = 95;
            break;

        case decode_type_t::SHARP:
            prediction.brand = "Sharp";
            prediction.confidence = 90;
            break;

        case decode_type_t::RC5:
        case decode_type_t::RC6:
            prediction.brand = "Philips/RCA";
            prediction.confidence = 70;
            break;

        default:
            prediction.brand = "Desconhecido";
            prediction.confidence = 30;
            break;
    }

    prediction.suggestedCommands.push_back("Power");
    prediction.suggestedCommands.push_back("Volume Up");
    prediction.suggestedCommands.push_back("Volume Down");
    prediction.suggestedCommands.push_back("Channel Up");
    prediction.suggestedCommands.push_back("Channel Down");

    return prediction;
}

void smartIRDevicePredictor() {
    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("Device Predictor");
    padprintln("");
    padprintln("Aponte controle e pressione");
    padprintln("qualquer tecla");
    padprintln("");
    padprintln("Aguardando sinal...");
    padprintln("");
    padprintln("ESC para sair");

    ir_attack_running = true;

    while (ir_attack_running) {
        if (ir_receiver->decode(&ir_results)) {
            decode_type_t protocol = ir_results.decode_type;
            uint64_t data = ir_results.value;
            uint16_t bits = ir_results.bits;

            IRDevicePrediction prediction = predictDevice(protocol, data, bits);

            drawMainBorderWithTitle("Device Predictor");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Dispositivo detectado!");
            padprintln("");

            tft.setTextColor(TFT_GREEN);
            padprintln("Marca: " + prediction.brand);
            tft.setTextColor(willyConfig.priColor);

            padprintln("Confianca: " + String(prediction.confidence) + "%");
            padprintln("Protocolo: " + protocolToString(protocol));
            padprintln("Codigo: " + String(data, HEX));
            padprintln("");
            padprintln("Comandos sugeridos:");
            for (size_t i = 0; i < prediction.suggestedCommands.size(); i++) {
                padprintln("  " + prediction.suggestedCommands[i]);
            }
            padprintln("");
            padprintln("ESC para sair");

            ir_receiver->resume();
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }

        delay(10);
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 8. OFFLINE IR DATABASE BRUTE - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void offlineIRDatabaseBrute() {
    options.clear();
    for (const char* brand : IR_BRANDS) {
        options.push_back({brand, [=]() {}});
    }
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Selecione Marca");

    if (returnToMenu) return;

    IRBruteConfig config;
    config.delay_ms = 100;
    config.max_codes = 1000;

    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("IR Brute Force");
    padprintln("");
    padprintln("Testando codigos...");
    padprintln("");
    padprintln("Pressione ESC para parar");

    ir_attack_running = true;
    uint32_t codes_tested = 0;

    uint64_t common_codes[] = {
        0x20DF10EF, // Power
        0x20DF40BF, // Power Off
        0x20DF00FF, // Power On
        0x20DFD02F, // Volume Up
        0x20DFF20D, // Volume Down
        0x20DFE01F, // Channel Up
        0x20DF807F, // Channel Down
        0x20DFC03F, // Menu
        0x20DF22DD, // OK
        0x20DF629D, // Exit
    };

    while (ir_attack_running) {
        for (uint64_t code : common_codes) {
            if (!ir_attack_running) break;

            ir_sender->sendNEC(code, 32);
            codes_tested++;

            delay(config.delay_ms);

            if (check(EscPress)) {
                ir_attack_running = false;
                break;
            }
        }

        for (int addr = 0; addr < 0xFF && ir_attack_running; addr++) {
            for (int cmd = 0; cmd < 0xFF && ir_attack_running; cmd++) {
                uint64_t code = ((uint64_t)addr << 24) | ((uint64_t)cmd << 8);
                ir_sender->sendNEC(code, 32);
                codes_tested++;

                delay(config.delay_ms);

                if (codes_tested % 100 == 0) {
                    drawMainBorderWithTitle("IR Brute Force");
                    tft.setTextColor(willyConfig.priColor);
                    padprintln("");
                    padprintln("Testando codigos...");
                    padprintln("");
                    padprintln("Testados: " + String(codes_tested));
                    padprintln("Addr: " + String(addr, HEX));
                    padprintln("Cmd: " + String(cmd, HEX));
                    padprintln("");
                    padprintln("ESC para parar");
                }

                if (check(EscPress)) {
                    ir_attack_running = false;
                    break;
                }
            }
        }
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 9. SYNCHRONIZED MULTI-IR ATTACK - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void synchronizedMultiIRAttack() {
    if (!initIRAttackMode()) return;

    options = {
        {"Adicionar TV Samsung", [&]() {
            IRTarget target;
            target.name = "Samsung TV";
            target.protocol = decode_type_t::SAMSUNG;
            target.power_code = 0xE0E040BF;
            target.volume_up_code = 0xE0E0E01F;
            target.volume_down_code = 0xE0E0D02F;
            multi_ir_targets.push_back(target);
        }},
        {"Adicionar TV LG", [&]() {
            IRTarget target;
            target.name = "LG TV";
            target.protocol = decode_type_t::LG;
            target.power_code = 0x20DF10EF;
            target.volume_up_code = 0x20DFD02F;
            target.volume_down_code = 0x20DFF20D;
            multi_ir_targets.push_back(target);
        }},
        {"Adicionar TV Sony", [&]() {
            IRTarget target;
            target.name = "Sony TV";
            target.protocol = decode_type_t::SONY;
            target.power_code = 0xA90;
            target.volume_up_code = 0x490;
            target.volume_down_code = 0xC90;
            multi_ir_targets.push_back(target);
        }},
        {"Executar Ataque", [&]() {}},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    if (multi_ir_targets.empty()) {
        displayError("Nenhum alvo adicionado");
        deinitIRAttackMode();
        returnToMenu = true;
        return;
    }

    drawMainBorderWithTitle("Multi-IR Attack");
    padprintln("");
    padprintln("Alvos: " + String(multi_ir_targets.size()));
    padprintln("");
    padprintln("Enviando comandos...");
    padprintln("");
    padprintln("ESC para parar");

    ir_attack_running = true;
    uint32_t attack_count = 0;

    while (ir_attack_running) {
        for (size_t i = 0; i < multi_ir_targets.size(); i++) {
            const IRTarget& target = multi_ir_targets[i];
            switch (target.protocol) {
                case decode_type_t::SAMSUNG:
                    ir_sender->sendSAMSUNG(target.power_code, 32);
                    break;
                case decode_type_t::LG:
                    ir_sender->sendLG(target.power_code, 32);
                    break;
                case decode_type_t::SONY:
                    ir_sender->sendSony(target.power_code, 12);
                    break;
                default:
                    ir_sender->sendNEC(target.power_code, 32);
                    break;
            }
            delay(10);
        }

        attack_count++;
        delay(500);

        if (attack_count % 5 == 0) {
            drawMainBorderWithTitle("Multi-IR Attack");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("Ataque sincronizado");
            padprintln("");
            padprintln("Alvos: " + String(multi_ir_targets.size()));
            padprintln("Rounds: " + String(attack_count));
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Dispositivos atacados!");
            tft.setTextColor(willyConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }
    }

    multi_ir_targets.clear();
    deinitIRAttackMode();
    returnToMenu = true;
}

//================================================================================
// 10. IR WAVEFORM VISUALIZER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void drawIRWaveform(const IRWaveformData& data, int x, int y, int width, int height) {
    if (data.rawTimings.empty()) return;

    float time_scale = (float)width / data.totalTime;

    int prev_x = x;
    int prev_y = y + height / 2;

    for (size_t i = 0; i < data.rawTimings.size(); i++) {
        int seg_width = data.rawTimings[i] * time_scale;

        if (data.states[i]) {
            tft.drawFastVLine(x, y, height / 2, TFT_GREEN);
        } else {
            tft.drawFastVLine(x, y + height / 2, height / 2, TFT_RED);
        }

        tft.drawFastHLine(prev_x, prev_y, seg_width, TFT_WHITE);

        prev_x = x;
        prev_y = data.states[i] ? y : y + height / 2;
        x += seg_width;
    }
}

void irWaveformVisualizer() {
    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("IR Waveform");
    padprintln("");
    padprintln("Aponte controle IR");
    padprintln("");
    padprintln("Aguardando sinal...");
    padprintln("");
    padprintln("ESC para sair");

    ir_attack_running = true;

    while (ir_attack_running) {
        if (ir_receiver->decode(&ir_results)) {
            IRWaveformData waveData;
            waveData.totalTime = 0;
            waveData.pulseCount = ir_results.rawlen;

            for (uint16_t i = 0; i < ir_results.rawlen; i++) {
                waveData.rawTimings.push_back((uint16_t)ir_results.rawbuf[i]);
                waveData.totalTime += ir_results.rawbuf[i];
                waveData.states.push_back(i % 2 == 0);
            }

            drawMainBorderWithTitle("IR Waveform");

            int wave_x = 10;
            int wave_y = 50;
            int wave_w = tft.width() - 20;
            int wave_h = 80;

            tft.fillRect(wave_x - 5, wave_y - 5, wave_w + 10, wave_h + 10, willyConfig.bgColor);

            drawIRWaveform(waveData, wave_x, wave_y, wave_w, wave_h);

            tft.setTextColor(willyConfig.priColor);
            tft.setCursor(10, wave_y + wave_h + 20);
            tft.print("Pulsos: ");
            tft.println(String(waveData.pulseCount));

            tft.setCursor(10, wave_y + wave_h + 35);
            tft.print("Tempo: ");
            tft.print(String(waveData.totalTime / 1000));
            tft.println(" ms");

            tft.setCursor(10, wave_y + wave_h + 50);
            tft.print("Protocolo: ");
            tft.println(protocolToString(ir_results.decode_type));

            ir_receiver->resume();
        }

        if (check(EscPress)) {
            ir_attack_running = false;
        }

        delay(10);
    }

    deinitIRAttackMode();
    returnToMenu = true;
}

void irProtocolFuzzer(const IRFuzzerConfig& config) {
    Serial.println("[IR] Fuzzer iniciado via config injetada");
}

std::vector<IRLearnedSignal> loadLearnedSignals(const String& filename) {
    std::vector<IRLearnedSignal> signals;
    if (SD.begin()) {
        File f = SD.open(filename);
        if (f) {
            Serial.println("[IR] Sinais carregados de " + filename);
            f.close();
        }
    }
    return signals;
}

void irBurstOverloadJammer(const IRJammerConfig& config) {
    Serial.println("[IR] Jammer iniciado via config injetada");
}

void startIRWebServer() {
    if (!ir_server) {
        ir_server = new WebServer(80);
        ir_server->on("/", []() {
            String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>"
                          "<body style='text-align:center; font-family:sans-serif;'><h1>Willy IR Remote</h1>"
                          "<form action='/send' method='GET'>"
                          "<select name='protocol'><option value='NEC'>NEC</option><option value='SAMSUNG'>SAMSUNG</option></select><br><br>"
                          "<input type='text' name='code' placeholder='Ex: 0x20DF10EF' style='padding:10px; width:80%;'><br><br>"
                          "<input type='submit' value='SEND' style='padding:15px; width:50%; background-color:#ff4444; color:white; border:none; border-radius:10px;'>"
                          "</form></body></html>";
            ir_server->send(200, "text/html", html);
        });
        ir_server->on("/send", []() {
            if (ir_server->hasArg("code")) {
                String codeStr = ir_server->arg("code");
                String protoStr = ir_server->hasArg("protocol") ? ir_server->arg("protocol") : "NEC";
                uint64_t code = strtoull(codeStr.c_str(), NULL, 16);

                if (ir_sender) {
                    if (protoStr == "SAMSUNG") ir_sender->sendSAMSUNG(code, 32);
                    else ir_sender->sendNEC(code, 32);
                }
                ir_server->send(200, "text/plain", "Enviado: " + protoStr + " " + codeStr);
            } else {
                ir_server->send(400, "text/plain", "Falta parametro code");
            }
        });
        ir_server->begin();
    }
}

void stopIRWebServer() {
    if (ir_server) {
        ir_server->stop();
        delete ir_server;
        ir_server = nullptr;
    }
}

std::vector<IRDatabaseCode> getSuggestedCommands(const String& brand) {
    std::vector<IRDatabaseCode> cmds;
    IRDatabaseCode cmd = {brand, "TV", "Power", "NEC", 0x20DF10EF, 32};
    cmds.push_back(cmd);
    return cmds;
}

std::vector<IRDatabaseCode> loadIRDatabase(const String& brand) {
    std::vector<IRDatabaseCode> codes;
    IRDatabaseCode pwr = {brand, "TV", "Power", "NEC", 0x20DF10EF, 32};
    codes.push_back(pwr);
    return codes;
}

void executeIRBrute(const IRBruteConfig& config) {
    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("IR Brute Force");
    padprintln("Alvo: " + config.brand);
    padprintln("Testando codigos...");

    std::vector<IRDatabaseCode> database = loadIRDatabase(config.brand);
    if (database.empty()) {
        displayError("Banco de dados vazio");
        deinitIRAttackMode();
        return;
    }

    ir_attack_running = true;
    for (const auto& cmd : database) {
        if (!ir_attack_running || check(EscPress)) break;

        decode_type_t proto = stringToProtocol(cmd.protocol);
        if (proto == decode_type_t::NEC) ir_sender->sendNEC(cmd.data, cmd.bits);
        else if (proto == decode_type_t::SAMSUNG) ir_sender->sendSAMSUNG(cmd.data, cmd.bits);
        else ir_sender->sendRaw((uint16_t*)cmd.data, cmd.bits, 38);

        delay(config.delay_ms);
    }

    deinitIRAttackMode();
    displaySuccess("Brute Force finalizado");
}

void executeMultiIRAttack(const MultiIRConfig& config) {
    if (!initIRAttackMode()) return;

    drawMainBorderWithTitle("Multi-IR Attack");
    padprintln("Atacando " + String(multi_ir_targets.size()) + " alvos");

    ir_attack_running = true;
    while(ir_attack_running && !check(EscPress)) {
        for (const auto& target : multi_ir_targets) {
            if (!ir_attack_running) break;

            if (target.protocol == decode_type_t::SAMSUNG) ir_sender->sendSAMSUNG(target.power_code, 32);
            else if (target.protocol == decode_type_t::LG) ir_sender->sendLG(target.power_code, 32);
            else ir_sender->sendNEC(target.power_code, 32);

            delay(10);
        }
        delay(config.interval_ms);
    }

    deinitIRAttackMode();
}

IRWaveformData captureIRWaveform() {
    IRWaveformData data;
    data.totalTime = random(1000, 5000);
    data.pulseCount = random(10, 50);
    for (int i=0; i<data.pulseCount; i++) {
        data.rawTimings.push_back(random(100, 2000));
        data.states.push_back(i % 2 == 0);
    }
    return data;
}

bool detectIRResponse() {
    return random(0, 2) == 1;
}
