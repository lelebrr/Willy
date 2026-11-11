/**
 * @file advanced_ble_atks.h
 * @brief Módulo de ataques BLE avançados para fins acadêmicos e pesquisa em segurança.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 *
 * @author Willy Team
 * @date 2026
 */

#ifndef __ADVANCED_BLE_ATKS_H__
#define __ADVANCED_BLE_ATKS_H__

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEAdvertisedDevice.h>
#include <vector>

//================================================================================
// ESTRUTURAS DE DADOS
//================================================================================

/**
 * @brief Estrutura para armazenar informações de dispositivos BLE detectados
 */
struct BLEDeviceInfo {
    NimBLEAddress address;
    String name;
    int rssi;
    uint16_t appearance;
    bool connectable;
    bool hasAppearance;
    std::vector<NimBLEUUID> serviceUUIDs;
};

/**
 * @brief Estrutura para exfiltração de dados via advertising
 */
struct ExfilData {
    uint8_t dataType;
    std::vector<uint8_t> payload;
    uint32_t timestamp;
};

/**
 * @brief Estrutura para AirTag spoof
 */
struct AirTagInfo {
    uint8_t publicKey[28];
    uint8_t hashCode[4];
    uint8_t status;
    uint8_t batteryLevel;
};

//================================================================================
// 1. MALFORMED ATT FUZZER
//================================================================================
/**
 * @brief Malformed ATT Fuzzer - Envia pacotes ATT malformados para crashar stacks BLE
 *
 * O que faz: Envia pacotes ATT aleatórios para crashar stacks BLE.
 * Como faz: NimBLE raw packet send em loop com payloads malformados.
 * Para que serve: Crash de fones, relógios, fechaduras (destrutivo).
 */
void malformedATTFuzzer();

/**
 * @brief Configurações do ATT Fuzzer
 */
struct ATTFuzzerConfig {
    uint16_t packets_per_second = 100;
    bool randomize_opcode = true;
    bool invalid_handles = true;
    bool overflow_attibutes = true;
    uint32_t duration_ms = 0; // 0 = até cancelar
};

void malformedATTFuzzer(const ATTFuzzerConfig& config);

//================================================================================
// 2. GATT PHISHING SERVER
//================================================================================
/**
 * @brief GATT Phishing Server - Emula serviços para roubar credenciais
 *
 * O que faz: Emula impressora/headset e rouba credenciais digitadas.
 * Como faz: Advertise + servidor GATT interativo.
 * Para que serve: Phishing sem usuário perceber.
 */
void gattPhishingServer();

/**
 * @brief Tipos de serviços GATT para phishing
 */
enum class GATTPhishingType {
    PHISH_KEYBOARD,      // Emula teclado
    PHISH_PRINTER,       // Emula impressora
    PHISH_HEADSET,       // Emula headset
    PHISH_SMART_LOCK,    // Emula fechadura
    PHISH_FITNESS,       // Emula rastreador fitness
    PHISH_THERMOMETER,   // Emula termômetro
    PHISH_HEART_RATE     // Emula monitor cardíaco
};

void gattPhishingServer(GATTPhishingType type);

//================================================================================
// 3. BLE ADVERTISING CHANNEL JAMMER
//================================================================================
/**
 * @brief BLE Advertising Channel Jammer - Bloqueia todos os dispositivos BLE
 *
 * O que faz: Inunda canais de advertising para bloquear todos os dispositivos BLE.
 * Como faz: Flood directed em 3 canais simultâneos (37, 38, 39).
 * Para que serve: Blackout total de BLE na área.
 */
void advertisingChannelJammer();

/**
 * @brief Configurações do jammer
 */
struct AdvertisingJammerConfig {
    bool channel_37 = true;
    bool channel_38 = true;
    bool channel_39 = true;
    uint16_t packets_per_second = 500;
    bool randomize_payload = true;
};

void advertisingChannelJammer(const AdvertisingJammerConfig& config);

//================================================================================
// 4. CONNECTION PARAMETER ABUSE DRAIN
//================================================================================
/**
 * @brief Connection Parameter Abuse Drain - Drena bateria de dispositivos BLE
 *
 * O que faz: Força intervalos de conexão extremos para drenar bateria.
 * Como faz: Update parameter requests maliciosos.
 * Para que serve: Drena AirPods, smartwatches em minutos.
 */
void connectionParameterDrain();

/**
 * @brief Configurações de abuso de parâmetros
 */
struct ConnectionDrainConfig {
    uint16_t min_interval = 6;     // 7.5ms mínimo
    uint16_t max_interval = 6;     // Força intervalo mínimo
    uint16_t latency = 0;          // Sem latência
    uint16_t timeout = 100;        // Timeout curto
    bool aggressive_mode = true;   // Solicitações constantes
};

void connectionParameterDrain(const ConnectionDrainConfig& config);

//================================================================================
// 5. MASS FINDMY / AIRTAG SPOOF CHAOS
//================================================================================
/**
 * @brief Mass FindMy/AirTag Spoof Chaos - Emula centenas de AirTags falsos
 *
 * O que faz: Emula centenas de AirTags falsos.
 * Como faz: Advertising em massa com random IDs no formato Apple.
 * Para que serve: Caos no ecossistema Apple.
 */
void massAirTagSpoof();

/**
 * @brief Configurações do AirTag spoof
 */
struct AirTagSpoofConfig {
    uint16_t num_tags = 100;
    bool randomize_ids = true;
    bool rotate_ids = true;
    uint16_t rotation_interval_ms = 1000;
    bool use_offline_finding = true;
};

void massAirTagSpoof(const AirTagSpoofConfig& config);

//================================================================================
// 6. BLE 5 LONG-RANGE EXTENDED ATTACK
//================================================================================
/**
 * @brief BLE 5 Long-Range Extended Attack - Ataques de longa distância
 *
 * O que faz: Ataques em alcance dobrado (até 200m).
 * Como faz: Usa PHY LR (Long Range) do ESP32.
 * Para que serve: Ataques de longe em estacionamentos.
 */
void longRangeExtendedAttack();

/**
 * @brief Configurações de ataque long-range
 */
struct LongRangeConfig {
    bool use_coded_phy = true;     // PHY codificado (125kbps ou 500kbps)
    bool extended_advertising = true;
    uint8_t primary_phy = 3;        // BLE_HCI_LE_1M_PHY = 1, BLE_HCI_LE_CODED_PHY = 3
    uint8_t secondary_phy = 3;
    uint32_t primary_adv_interval = 1000;
};

void longRangeExtendedAttack(const LongRangeConfig& config);

//================================================================================
// 7. BLE PAIRING MITM SIMULATOR
//================================================================================
/**
 * @brief BLE Pairing MITM Simulator - Intercepta pairing
 *
 * O que faz: Intercepta e força pairing JustWorks.
 * Como faz: Relay em tempo real entre duas conexões.
 * Para que serve: Roubo de dispositivos pareados.
 */
void pairingMITMSimulator();

/**
 * @brief Callback para captura de chaves de pairing
 */
typedef void (*PairingKeyCallback)(const NimBLEAddress& device, const uint8_t* ltk, size_t ltk_len);

void setPairingKeyCallback(PairingKeyCallback callback);

//================================================================================
// 8. ADVERTISEMENT STEALTH EXFIL
//================================================================================
/**
 * @brief Advertisement Stealth Exfil - Exfiltração de dados via advertising
 *
 * O que faz: Codifica dados roubados em pacotes de advertising.
 * Como faz: Encode base64 nos campos manufacturer specific data.
 * Para que serve: Exfiltração indetectável.
 */
void advertisementStealthExfil();

/**
 * @brief Envia dados via advertising de forma oculta
 * @param data Dados a serem exfiltrados
 * @param encode_type Tipo de codificação (0=raw, 1=base64, 2=xor, 3=custom)
 */
void exfiltrateData(const uint8_t* data, size_t len, uint8_t encode_type = 1);

/**
 * @brief Configura dados para exfiltração contínua
 */
void setupContinuousExfil(const std::vector<ExfilData>& data);

//================================================================================
// 9. BLE MESH WORM PROPAGATOR
//================================================================================
/**
 * @brief BLE Mesh Worm Propagator - Propaga serviço malicioso em redes mesh
 *
 * O que faz: Propaga serviço malicioso em redes mesh BLE.
 * Como faz: Advertise + auto-conexão + propagação.
 * Para que serve: Infecção em massa de lâmpadas IoT.
 */
void meshWormPropagator();

/**
 * @brief Estrutura para nó mesh infectado
 */
struct MeshNode {
    NimBLEAddress address;
    bool infected;
    uint8_t hop_count;
    uint32_t last_seen;
};

/**
 * @brief Configurações do worm
 */
struct MeshWormConfig {
    uint16_t scan_duration_ms = 5000;
    bool auto_spread = true;
    uint32_t spread_interval_ms = 30000;
    uint8_t max_hops = 10;
};

void meshWormPropagator(const MeshWormConfig& config);

//================================================================================
// 10. DIRECTED DISCONNECT FLOOD
//================================================================================
/**
 * @brief Directed Disconnect Flood - Desconecta dispositivos selecionados
 *
 * O que faz: Desconecta apenas dispositivos selecionados por MAC.
 * Como faz: Connect + immediate disconnect loop.
 * Para que serve: Ataque cirúrgico.
 */
void directedDisconnectFlood();

/**
 * @brief Configurações de disconnect flood
 */
struct DisconnectFloodConfig {
    std::vector<NimBLEAddress> target_addresses;
    uint16_t disconnect_per_second = 10;
    bool scan_for_targets = true;
    uint8_t target_appearance = 0; // 0 = qualquer dispositivo
};

void directedDisconnectFlood(const DisconnectFloodConfig& config);

/**
 * @brief Adiciona alvo à lista de disconnect
 */
void addDisconnectTarget(const NimBLEAddress& address);

/**
 * @brief Limpa lista de alvos
 */
void clearDisconnectTargets();

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

/**
 * @brief Menu principal de ataques BLE avançados
 */
void advancedBLEAtksMenu();

/**
 * @brief Inicializa modo de ataque BLE
 */
bool initBLEAttackMode();

/**
 * @brief Finaliza modo de ataque BLE
 */
void deinitBLEAttackMode();

/**
 * @brief Gera endereço BLE aleatório
 */
void generateRandomBLEAddress(uint8_t* addr);

/**
 * @brief Gera Apple-style advertising data
 */
std::vector<uint8_t> generateAppleAdvertisingData();

/**
 * @brief Calcula checksum para payload BLE
 */
uint8_t calculateBLEChecksum(const uint8_t* data, size_t len);

/**
 * @brief Converte endereço BLE para string
 */
String bleAddressToString(const NimBLEAddress& addr);

/**
 * @brief Detecta tipo de dispositivo por appearance
 */
String getDeviceTypeByAppearance(uint16_t appearance);

//================================================================================
// BLUETOOTH SNIFFERS ADICIONAIS
//================================================================================

/**
 * @brief Bluetooth Analyzer - Análise detalhada de pacotes BLE
 */
void bluetoothAnalyzer();

/**
 * @brief Flipper Sniff - Detecta Flippers especificamente
 */
void flipperSniff();

/**
 * @brief Airtag Monitor - Monitora AirTags continuamente
 */
void airtagMonitor();

/**
 * @brief BT Wardrive Continuous - Wardriving BLE contínuo
 */
void btWardriveContinuous();

/**
 * @brief Flock Sniff/Wardrive - Detecta/wardrive flocks IoT
 */
void flockSniff();

/**
 * @brief Detect Card Skimmers - Identifica skimmers BLE
 */
void detectCardSkimmers();

//================================================================================
// BLUETOOTH ATTACKS EXCLUSIVOS
//================================================================================

/**
 * @brief Google BLE Spam - Spam específico para Google devices
 */
void googleBLESpam();

/**
 * @brief Flipper BLE Spam - Spam para Flippers
 */
void flipperBLESpam();

/**
 * @brief Samsung BLE Spam - Spam para dispositivos Samsung
 */
void samsungBLESpam();

/**
 * @brief Swift Pair Spam - Spam via Swift Pair
 */
void swiftPairSpam();

/**
 * @brief BLE Spam All - Spam para todos os dispositivos
 */
void bleSpamAll();

//================================================================================
// ESTRUTURAS ADICIONAIS
//================================================================================

struct BLESpamConfig {
    uint16_t packets_per_second = 100;
    bool randomize_mac = true;
    uint32_t duration_ms = 0;
    uint8_t spam_type; // 0=all, 1=google, 2=samsung, 3=flipper, 4=swift
};

struct SkimmerInfo {
    NimBLEAddress address;
    String name;
    int rssi;
    uint8_t risk_level; // 0-100
    String detection_reason;
};

struct FlipperInfo {
    NimBLEAddress address;
    String name;
    int rssi;
    String firmware;
    uint32_t last_seen;
};

//================================================================================
// FUNÇÕES DE MENU BLE
//================================================================================

/**
 * @brief Menu de BLE Sniffers
 */
void bleSniffersMenu();

/**
 * @brief Menu de BLE Spam Avançado
 */
void bleSpamMenu();

#endif // __ADVANCED_BLE_ATKS_H__
