/**
 * @file advanced_atks.h
 * @brief Módulo de ataques WiFi avançados para fins acadêmicos e pesquisa em segurança.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 *
 * @author Willy Team
 * @date 2026
 */

#ifndef __ADVANCED_ATKS_H__
#define __ADVANCED_ATKS_H__

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

//================================================================================
// ESTRUTURAS DE DADOS
//================================================================================

/**
 * @brief Estrutura para armazenar informações de clientes detectados
 */
struct ClientInfo {
    uint8_t mac[6];
    uint8_t ap_mac[6];
    int8_t rssi;
    uint8_t channel;
    uint32_t last_seen;
    uint16_t packet_count;
    bool is_iot;
    char device_type[32];
};

/**
 * @brief Estrutura para fingerprints de tráfego
 */
struct TrafficFingerprint {
    uint16_t avg_packet_size;
    uint16_t avg_interval_ms;
    uint16_t variance;
    char detected_os[32];
    char device_type[32];
    uint8_t confidence;
};

/**
 * @brief Estrutura para entradas OUI de IoT
 */
struct OUIEntry {
    uint8_t prefix[3];
    const char* vendor;
    const char* device_type;
    bool is_iot;
};

/**
 * @brief Estrutura para dados do heatmap
 */
struct HeatmapPoint {
    uint8_t ap_mac[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    int16_t x;
    int16_t y;
};

//================================================================================
// 1. DYNAMIC BEACON FUZZER
//================================================================================
/**
 * @brief Dynamic Beacon Fuzzer - Envia beacons malformados para crashar roteadores
 *
 * O que faz: Envia beacons malformados aleatórios para testar robustez de roteadores.
 * Como faz: esp_wifi_80211_tx com buffer custom + random IE fields (loop 1000/s).
 * Para que serve: Descoberta de vulnerabilidades em equipamentos (fuzzing).
 */
void dynamicBeaconFuzzer();

/**
 * @brief Configurações do Beacon Fuzzer
 */
struct BeaconFuzzerConfig {
    uint16_t packets_per_second = 1000;
    bool randomize_ssid = true;
    bool randomize_mac = true;
    bool malformed_ie = true;
    uint8_t target_channel = 0; // 0 = todos os canais
    uint32_t duration_ms = 0;   // 0 = até cancelar
};

void dynamicBeaconFuzzer(const BeaconFuzzerConfig& config);

//================================================================================
// 2. CLIENT BATTERY DRAIN (PS-POLL SPOOF)
//================================================================================
/**
 * @brief Client Battery Drain - Impede dispositivos de entrar em modo sleep
 *
 * O que faz: Impede que dispositivos entrem em modo sleep enviando PS-Poll spoofados.
 * Como faz: Monitora clientes + injeta respostas spoofadas continuamente.
 * Para que serve: Dreno de bateria para testes de autonomia.
 */
void clientBatteryDrain();

/**
 * @brief Inicia ataque de drenagem de bateria em clientes específicos
 * @param target_macs Lista de MACs alvo (vazio = todos)
 */
void clientBatteryDrain(const std::vector<uint8_t[6]>& target_macs);

//================================================================================
// 3. VISUAL WIFI HEATMAP NO TFT
//================================================================================
/**
 * @brief Visual WiFi Heatmap - Mapa de calor em tempo real
 *
 * O que faz: Mapa de calor em tempo real de APs e clientes por RSSI.
 * Como faz: Armazena posições + desenha com LVGL/TFT no CYD.
 * Para que serve: Reconhecimento físico em pentest de escritório.
 */
void visualWifiHeatmap();

/**
 * @brief Adiciona ponto ao heatmap
 */
void addHeatmapPoint(const HeatmapPoint& point);

/**
 * @brief Renderiza heatmap no display
 */
void renderHeatmap();

//================================================================================
// 4. WPA3 DOWNGRADE + MITM COORDINATOR
//================================================================================
/**
 * @brief WPA3 Downgrade + MITM Coordinator
 *
 * O que faz: Força downgrade de WPA3 para WPA2 e faz MITM automático.
 * Como faz: Beacon spoof sem PMF/SAE + evil portal integrado.
 * Para que serve: Teste de segurança em redes modernas WPA3.
 */
void wpa3DowngradeAttack();

/**
 * @brief Estrutura de configuração do ataque WPA3 Downgrade
 */
struct WPA3DowngradeConfig {
    String target_ssid;
    uint8_t target_bssid[6];
    uint8_t channel;
    bool enable_mitm = true;
    bool capture_handshake = true;
    uint32_t deauth_interval_ms = 5000;
};

void wpa3DowngradeAttack(const WPA3DowngradeConfig& config);

//================================================================================
// 5. IoT TARGETED EXPLOIT INJECTOR
//================================================================================
/**
 * @brief IoT Targeted Exploit Injector
 *
 * O que faz: Detecta IoT por OUI/MAC e injeta payloads conhecidos.
 * Como faz: Banco offline no SD + packet injection.
 * Para que serve: Teste de vulnerabilidades em dispositivos IoT.
 */
void iotExploitInjector();

/**
 * @brief Estrutura para exploit IoT
 */
struct IoTExploit {
    char name[64];
    char vendor[32];
    char device_type[32];
    uint8_t oui_prefix[3];
    uint8_t payload[256];
    uint8_t payload_len;
    uint16_t port;
    const char* protocol; // "TCP", "UDP", "HTTP"
};

/**
 * @brief Carrega exploits do SD card
 */
bool loadIoTExploits();

/**
 * @brief Detecta dispositivos IoT por OUI
 */
bool detectIoTDevice(const uint8_t* mac, OUIEntry& entry);

//================================================================================
// 6. 802.11s MESH DISRUPTOR
//================================================================================
/**
 * @brief 802.11s Mesh Disruptor
 *
 * O que faz: Quebra redes mesh inundando frames de controle.
 * Como faz: Detecta mesh + flood path-request frames.
 * Para que serve: Teste de resiliência de redes mesh IoT empresariais.
 */
void meshDisruptor();

/**
 * @brief Estrutura para rede mesh detectada
 */
struct MeshNetwork {
    uint8_t mesh_id[32];
    uint8_t mesh_id_len;
    uint8_t bssid[6];
    uint8_t channel;
    int8_t rssi;
    uint8_t path_protocol;
    bool is_detected;
};

/**
 * @brief Detecta redes mesh no ambiente
 */
std::vector<MeshNetwork> detectMeshNetworks();

//================================================================================
// 7. SMART DEAUTH SCHEDULER
//================================================================================
/**
 * @brief Smart Deauth Scheduler - Deauth programado
 *
 * O que faz: Deauth programado por horário ou condição.
 * Como faz: Usa o interpretador JS já presente no Bruce.
 * Para que serve: Manter rede offline sem flood constante (stealth).
 */
void smartDeauthScheduler();

/**
 * @brief Estrutura de agendamento de deauth
 */
struct DeauthSchedule {
    uint8_t target_bssid[6];
    String target_ssid;
    uint8_t channel;

    // Condições de tempo
    int8_t start_hour = -1;    // -1 = qualquer hora
    int8_t end_hour = -1;
    int8_t start_minute = -1;
    int8_t end_minute = -1;

    // Condições de evento
    bool on_reconnect = false;  // Deauth quando cliente reconecta
    bool on_new_client = false; // Deauth quando novo cliente aparece
    uint16_t packets_per_burst = 10;
    uint32_t interval_ms = 60000; // Intervalo entre bursts

    // Script JS personalizado
    String condition_script;
};

void startDeauthScheduler(const DeauthSchedule& schedule);
void stopDeauthScheduler();

//================================================================================
// 8. ENCRYPTED TRAFFIC FINGERPRINT
//================================================================================
/**
 * @brief Encrypted Traffic Fingerprint - Identificação passiva
 *
 * O que faz: Identifica SO/dispositivo por estatística de pacotes criptografados.
 * Como faz: Análise simples de tamanho/intervals (mostra na tela).
 * Para que serve: Reconhecimento passivo avançado.
 */
void encryptedTrafficFingerprint();

/**
 * @brief Analisa tráfego e gera fingerprint
 */
TrafficFingerprint analyzeTraffic(const std::vector<uint16_t>& packet_sizes,
                                   const std::vector<uint32_t>& intervals);

/**
 * @brief Detecta OS baseado em padrões de tráfego
 */
const char* detectOSFromFingerprint(const TrafficFingerprint& fp);

//================================================================================
// 9. ROGUE AP SESSION HIJACK PORTAL
//================================================================================
/**
 * @brief Rogue AP Session Hijack Portal - Portal que rouba sessões
 *
 * O que faz: Portal que rouba sessões ativas (cookies).
 * Como faz: Evil Portal + JS injection no tráfego.
 * Para que serve: Teste de sequestro de sessão pós-phishing.
 */
void sessionHijackPortal();

/**
 * @brief Estrutura para sessão capturada
 */
struct CapturedSession {
    uint8_t client_mac[6];
    String domain;
    String cookies;
    String session_token;
    String user_agent;
    uint32_t timestamp;
};

/**
 * @brief Callback para nova sessão capturada
 */
typedef void (*SessionCallback)(const CapturedSession& session);

void setSessionCallback(SessionCallback callback);

/**
 * @brief Gera página HTML com JS de sequestro de sessão
 */
String generateHijackPage(const String& target_domain);

//================================================================================
// 10. MANAGEMENT FRAME LIMITER BYPASS FLOOD
//================================================================================
/**
 * @brief Management Frame Limiter Bypass Flood
 *
 * O que faz: Flood de management frames acima do limite de detecção.
 * Como faz: Rate control inteligente + random MAC.
 * Para que serve: DoS stealth em WIDS modernos.
 */
void mgmtFrameBypassFlood();

/**
 * @brief Configuração do bypass flood
 */
struct MgmtFloodConfig {
    uint16_t frames_per_second = 50;  // Abaixo do threshold típico de WIDS
    bool randomize_mac = true;
    bool rotate_channels = true;
    uint8_t start_channel = 1;
    uint8_t end_channel = 11;
    uint32_t duration_ms = 0; // 0 = até cancelar
    uint8_t frame_types;      // Bitmask: bit0=beacon, bit1=probe, bit2=auth, etc.
};

void mgmtFrameBypassFlood(const MgmtFloodConfig& config);

//================================================================================
// WIFI SNIFFERS ADICIONAIS
//================================================================================

/**
 * @brief Channel Analyzer - Analisa canais por ruído/interferência
 */
void channelAnalyzer();

/**
 * @brief Packet Counter - Conta pacotes em tempo real
 */
void packetCounter();

/**
 * @brief Detect Pwnagotchi - Identifica Pwnagotchis ativos no range
 */
void detectPwnagotchi();

/**
 * @brief Scan All - Combina múltiplos scans
 */
void scanAllNetworks();

/**
 * @brief SAE Commit Capture - Captura commits SAE para WPA3
 */
void saeCommitCapture();

/**
 * @brief Ping Scan - Descobre hosts via ICMP
 */
void pingScan();

/**
 * @brief ARP Scan - Descobre hosts via ARP
 */
void arpScan();

/**
 * @brief Port Scan All - Varre todas as portas comuns
 */
void portScanAll();

/**
 * @brief SSH Scan - Varre especificamente SSH
 */
void sshScan();

/**
 * @brief Telnet Scan - Varre Telnet
 */
void telnetScan();

//================================================================================
// WIFI ATTACKS EXCLUSIVOS
//================================================================================

/**
 * @brief Probe Request Flood - Flood de probe requests
 */
void probeRequestFlood();

/**
 * @brief AP Clone Spam - Clona e spama APs
 */
void apCloneSpam();

/**
 * @brief Karma Attack - Responde a probes para atrair devices
 */
void karmaAttack();

/**
 * @brief Bad Message Attack - Envia mensagens ruins para crashar devices
 */
void badMessageAttack();

/**
 * @brief Sleep Attack - Ataque de sleep em associações
 */
void sleepAttack();

/**
 * @brief SAE Commit Flood - Flood para WPA3
 */
void saeCommitFlood();

//================================================================================
// ESTRUTURAS PARA SNIFFERS
//================================================================================

struct ChannelInfo {
    uint8_t channel;
    uint16_t packet_count;
    int16_t noise_level;
    uint8_t ap_count;
    uint8_t client_count;
    uint32_t last_update;
};

struct PwnagotchiInfo {
    uint8_t mac[6];
    uint8_t channel;
    int8_t rssi;
    String identity;
    uint32_t last_seen;
};

struct SAECapture {
    uint8_t client_mac[6];
    uint8_t ap_mac[6];
    uint8_t commit[128];
    uint8_t commit_len;
    uint32_t timestamp;
};

struct PortScanResult {
    String ip;
    uint16_t port;
    bool open;
    String service;
};

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

/**
 * @brief Menu principal de ataques avançados
 */
void advancedAtksMenu();

/**
 * @brief Inicializa modo de ataque avançado
 */
bool initAdvancedAttackMode();

/**
 * @brief Finaliza modo de ataque avançado
 */
void deinitAdvancedAttackMode();

/**
 * @brief Gera MAC aleatório
 */
void generateRandomMac(uint8_t* mac);

/**
 * @brief Verifica se MAC é multicast/broadcast
 */
bool isMulticastMac(const uint8_t* mac);

/**
 * @brief Converte MAC para string
 */
String macToString(const uint8_t* mac);

/**
 * @brief Converte string para MAC
 */
bool stringToMac(const String& str, uint8_t* mac);

/**
 * @brief Calcula checksum do frame 802.11
 */
uint32_t calculateFCS(const uint8_t* data, size_t len);

//================================================================================
// FUNÇÕES DE MENU
//================================================================================

/**
 * @brief Menu de WiFi Sniffers
 */
void wifiSniffersMenu();

/**
 * @brief Menu de WiFi Attacks
 */
void wifiAttacksMenu();

/**
 * @brief Obtém nome do serviço por porta
 */
String getServiceName(uint16_t port);

#endif // __ADVANCED_ATKS_H__
