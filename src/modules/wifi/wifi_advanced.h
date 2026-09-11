/**
 * @file wifi_advanced.h
 * @brief Willy WiFi Advanced Suite - 55+ WiFi Attack/Analysis Functions
 * @author Willy Team
 * @date 2026
 */

#ifndef __WIFI_ADVANCED_H__
#define __WIFI_ADVANCED_H__

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

// ============================================================================
// Main entry point
// ============================================================================
void wifiAdvancedSuiteMenu();

// ============================================================================
// 1. NETWORK RECONNAISSANCE (10 functions)
// ============================================================================
void wifiNetworkScanner();       // Escaneamento completo de redes
void wifiHiddenNetworkFinder();  // Encontra redes ocultas
void wifiVendorLookup();         // OUI/Vendor lookup por MAC
void wifiSignalMapper();         // Mapeamento de sinal por localizacao
void wifiChannelDistributor();   // Distribuicao de canais
void wifiSecurityAnalyzer();     // Analise de seguranca (WPA2/WPA3/OWE)
void wifiBeaconCollector();      // Coleta de beacons
void wifiProbeSniffer();         // Sniffer de probe requests
void wifiClientTracker();        // Rastreador de clientes
void wifiNetworkProfiler();      // Perfil completo da rede

// ============================================================================
// 2. ATTACK VECTORS (10 functions)
// ============================================================================
void wifiDeauthAllNetworks();    // Deauth em todas as redes
void wifiBeaconFlood();          // Flood de beacons malformados
void wifiProbeFlood();           // Flood de probe requests
void wifiAuthFlood();            // Flood de autenticacao
void wifiAssocFlood();           // Flood de associacao
void wifiDisassocFlood();        // Flood de desassociacao
void wifiEapolFlood();           // Flood de EAPOL
void wifiDnsAmplification();     // Amplificacao DNS
void wifiTcpSynFlood();          // SYN flood
void wifiUdpFlood();             // UDP flood

// ============================================================================
// 3. WPA/WPA2 CRACKING (8 functions)
// ============================================================================
void wifiHandshakeCapture();     // Captura de handshake WPA
void wifiPmkidCapture();         // Captura de PMKID
void wifiDeauthForHandshake();   // Deauth para capturar handshake
void wifiBeaconSniff();          // Sniff de beacons para cracking
void wifiEapolAnalyzer();        // Analise de pacotes EAPOL
void wifiWordlistManager();      // Gerenciador de wordlists
void wifiPasswordRecovery();     // Recuperacao de senha
void wifiKeyConverter();         // Conversor de chaves (PMK/PTK)

// ============================================================================
// 4. EVIL TWIN & SOCIAL (8 functions)
// ============================================================================
void wifiEvilTwinCreator();      // Cria evil twin automatico
void wifiCaptivePortal();        // Portal cativo personalizado
void wifiPhishingGenerator();    // Gerador de phishing pages
void wifiSslStrip();             // SSL strip attack
void wifiMitmProxy();            // Proxy MITM
void wifiCredentialSniffer();    // Sniffer de credenciais
void wifiSessionHijacker();      // Sequestro de sessao
void wifiCookieStealer();        // Roubo de cookies

// ============================================================================
// 5. SNIFFING & ANALYSIS (8 functions)
// ============================================================================
void wifiPacketSniffer();        // Sniffer de pacotes
void wifiDnsSniffer();           // Sniffer DNS
void wifiHttpSniffer();          // Sniffer HTTP
void wifiArpSniffer();           // Sniffer ARP
void wifiTlsSniffer();           // Sniffer TLS/SSL
void wifiTrafficAnalyzer();      // Analise de trafego
void wifiBandwidthMonitor();     // Monitor de largura de banda
void wifiLatencyTester();        // Teste de latencia

// ============================================================================
// 6. NETWORK UTILITIES (7 functions)
// ============================================================================
void wifiPortScanner();          // Scanner de portas
void wifiServiceDiscovery();     // Descoberta de servicos
void wifiHostnameResolver();     // Resolucao de hostname
void wifiWhoisLookup();          // Whois lookup
void wifiTraceroute();           // Traceroute
void wifiNmapLite();             // Nmap simplificado
void wifiNetbiosScanner();       // Scanner NetBIOS

// ============================================================================
// 7. DEFENSIVE TOOLS (7 functions)
// ============================================================================
void wifiIntrusionDetector();    // Detector de intrusao
void wifiRogueApDetector();      // Detector de APs falsos
void wifiJammingDetector();      // Detector de jamming
void wifiPacketFilter();         // Filtro de pacotes
void wifiMacFilter();            // Filtro MAC
void wifiNetworkMonitor();       // Monitor de rede
void wifiAlertSystem();          // Sistema de alertas

// ============================================================================
// 8. CONFIGURATION & TOOLS (5 functions)
// ============================================================================
void wifiMacSpoofer();           // MAC spoofing
void wifiChannelHop();           // Hopping de canais
void wifiPowerAdjust();          // Ajuste de potencia
void wifiConfigManager();        // Gerenciador de configuracoes
void wifiExportToSD();           // Export para SD

// ============================================================================
// Menu functions
// ============================================================================
void wifiAdvReconMenu();         // Submenu de reconhecimento
void wifiAdvAttackMenu();        // Submenu de ataques
void wifiAdvCrackMenu();         // Submenu de cracking
void wifiAdvEvilMenu();          // Submenu de evil twin
void wifiAdvSniffMenu();         // Submenu de sniffing
void wifiAdvUtilMenu();          // Submenu de utilitarios
void wifiAdvDefenseMenu();       // Submenu de defesa
void wifiAdvConfigMenu();        // Submenu de configuracoes

// ============================================================================
// Configuration
// ============================================================================
struct WiFiAdvConfig {
    int scanChannel;              // Canal para escaneamento (0=todos)
    int attackDuration;           // Duracao do ataque (segundos)
    int deauthPackets;            // Pacotes deauth por burst
    int beaconInterval;           // Intervalo de beacons (ms)
    bool randomizeMac;            // Randomizar MAC durante ataque
    bool stealthMode;             // Modo stealth
    int outputPower;              // Potencia de transmissao (dBm)
    bool autoDeauth;              // Auto-deauth durante captura
    int targetChannel;            // Canal alvo
    String targetSSID;            // SSID alvo
    String targetBSSID;           // BSSID alvo
    bool showFeedback;            // Mostrar feedback visual
    bool exportLogs;              // Exportar logs

    WiFiAdvConfig() :
        scanChannel(0),
        attackDuration(60),
        deauthPackets(5),
        beaconInterval(100),
        randomizeMac(false),
        stealthMode(false),
        outputPower(20),
        autoDeauth(true),
        targetChannel(0),
        targetSSID(""),
        targetBSSID(""),
        showFeedback(true),
        exportLogs(false) {}
};

extern WiFiAdvConfig wifiAdvConfig;

#endif // __WIFI_ADVANCED_H__
