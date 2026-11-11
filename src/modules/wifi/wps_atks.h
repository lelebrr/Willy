/**
 * @file wps_atks.h
 * @brief Módulo de ataques WPS (Wi-Fi Protected Setup) para fins acadêmicos.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 *
 * @author Willy Team
 * @date 2026
 */

#ifndef __WPS_ATKS_H__
#define __WPS_ATKS_H__

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

//================================================================================
// ESTRUTURAS DE DADOS WPS
//================================================================================

/**
 * @brief Informações de uma rede com WPS habilitado
 */
struct WPSNetworkInfo {
    uint8_t bssid[6];
    char ssid[33];
    uint8_t channel;
    int8_t rssi;
    bool wps_enabled;
    bool wps_locked;
    uint8_t wps_version;       // 1 ou 2
    bool pbc_supported;
    bool pin_supported;
    char device_name[33];
    char manufacturer[65];
    char model[33];
    uint16_t config_methods;   // Bitmask de métodos suportados
    bool cracked;              // Se a rede já foi quebrada
    char cracked_pin[9];       // PIN encontrado (se houver)
};

/**
 * @brief Resultado de tentativa de PIN WPS
 */
struct WPSPinResult {
    char pin[9];               // PIN de 8 dígitos + null
    bool success;
    uint8_t error_code;
    uint32_t attempt_time_ms;
    uint8_t message_reached;   // M1-M7 alcançado
};

/**
 * @brief Dados capturados para Pixie Dust
 */
struct WPSPixieData {
    uint8_t enrollee_nonce[16];
    uint8_t registrar_nonce[16];
    uint8_t enrollee_pubkey[192];
    uint8_t registrar_pubkey[192];
    uint8_t e_hash1[32];
    uint8_t e_hash2[32];
    uint8_t auth_key[32];
    bool has_m1;
    bool has_m2;
    bool has_m3;
    bool complete;
};

/**
 * @brief Configuração do ataque WPS
 */
struct WPSAttackConfig {
    uint8_t target_bssid[6];
    char target_ssid[33];
    uint8_t channel;
    uint32_t timeout_ms = 30000;
    uint16_t delay_between_attempts_ms = 1000;
    bool verbose = true;
};

//================================================================================
// 1. WPS PIN BRUTE FORCE
//================================================================================
/**
 * @brief WPS PIN Brute Force (Native) - Testa PINs WPS com a pilha ESP-IDF
 */
void wpsPinBruteForceNative();

/**
 * @brief WPS PIN Brute Force (Raw) - Testa PINs WPS com injecao de pacotes
 */
void wpsPinBruteForceRaw();

//================================================================================
// 2. WPS PIXIE DUST
//================================================================================
/**
 * @brief WPS Pixie Dust (Native) - Captura M1-M3 real com a pilha ESP-IDF
 */
void wpsPixieDustNative();

/**
 * @brief WPS Pixie Dust (Raw) - Captura M1-M3 provocando com injecao de pacotes
 */
void wpsPixieDustRaw();

//================================================================================
// 3. WPS NULL PIN ATTACK
//================================================================================
/**
 * @brief WPS Null PIN Attack - Testa bypass com PIN vazio
 *
 * O que faz: Envia PIN vazio/nulo para testar implementações vulneráveis.
 * Como faz: Tenta variações de PIN nulo (vazio, zeros, espaços).
 * Para que serve: Teste de bypass em firmwares com validação fraca.
 */
void wpsNullPinAttack();

//================================================================================
// 4. WPS PBC (Push Button) FLOOD
//================================================================================
/**
 * @brief WPS PBC Flood - Confunde APs em modo Push Button
 *
 * O que faz: Injeta múltiplos frames de sessão PBC simultaneamente.
 * Como faz: Broadcast de Probe Request com WPS IE + PBC flag.
 * Para que serve: Teste de comportamento do AP sob múltiplas sessões PBC.
 */
void wpsPBCFlood();

//================================================================================
// 5. WPS LOCKOUT TESTER
//================================================================================
/**
 * @brief WPS Lockout Tester - Detecta mecanismo de lockout
 *
 * O que faz: Envia PINs inválidos e detecta quando AP faz lockout.
 * Como faz: Tenta PINs falsos medindo tempo de resposta e erros.
 * Para que serve: Teste de implementação de proteção anti-brute-force.
 */
void wpsLockoutTester();

//================================================================================
// 6. WPS SCAN
//================================================================================
/**
 * @brief WPS Scan - Escaneia redes com WPS habilitado
 *
 * O que faz: Lista todos os APs com WPS, mostrando versão e métodos.
 * Como faz: WiFi scan + parse de WPS IE nos beacons.
 * Para que serve: Reconhecimento de redes vulneráveis a ataques WPS.
 */
void wpsScan();

//================================================================================
// 7. WPS AP SPOOF
//================================================================================
/**
 * @brief WPS AP Spoof - Cria AP falso com WPS para capturar PINs
 *
 * O que faz: Cria um AP com WPS habilitado para capturar PINs de clientes.
 * Como faz: SoftAP + WPS registrar mode + log de tentativas.
 * Para que serve: Teste de clientes que tentam WPS automaticamente.
 */
void wpsAPSpoof();

//================================================================================
// 8. WPS NACK FLOOD
//================================================================================
/**
 * @brief WPS NACK Flood - Derruba sessões WPS ativas
 *
 * O que faz: Injeta frames WSC_NACK em massa para interromper handshakes WPS.
 * Como faz: Constrói EAP-Failure/NACK frames e injeta via 80211_tx.
 * Para que serve: DoS em sessões WPS ativas, impedindo conexões.
 */
void wpsNACKFlood();

//================================================================================
// 9. WPS AUTO ALL
//================================================================================
/**
 * @brief WPS Auto All - Executa todos os ataques no alvo em sequência
 */
void wpsAutoAll();

//================================================================================
// MENU E FUNÇÕES AUXILIARES
//================================================================================

/**
 * @brief Menu principal de ataques WPS
 */
void wpsAttacksMenu();

/**
 * @brief Escaneia redes e retorna lista de APs com WPS
 */
std::vector<WPSNetworkInfo> scanWPSNetworks();

/**
 * @brief Salva rede quebrada no SD card (/WPS/cracked.txt)
 */
void wpsSaveCracked(const WPSNetworkInfo& net, const char* pin);

/**
 * @brief Verifica se BSSID já foi quebrado (lê do SD)
 */
bool wpsIsCracked(const uint8_t* bssid, char* pin_out = nullptr);

/**
 * @brief Exibe lista de redes quebradas salvas no SD
 */
void wpsViewCracked();

/**
 * @brief Calcula checksum do PIN WPS (último dígito)
 */
uint8_t wpsChecksumPin(uint32_t pin);

/**
 * @brief Gera PIN WPS válido a partir de 7 dígitos
 */
void wpsGeneratePin(uint32_t base, char* pin_out);

/**
 * @brief Seleciona AP WPS alvo via menu
 * @return índice do AP selecionado, -1 se cancelado
 */
int wpsSelectTarget(const std::vector<WPSNetworkInfo>& networks);

#endif // __WPS_ATKS_H__
