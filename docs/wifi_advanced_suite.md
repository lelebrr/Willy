# WiFi Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 55+ funções WiFi para o Willy Firmware. Acesse via menu **WiFi → WiFi Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/wifi/wifi_advanced.h`
- **Implementação**: `src/modules/wifi/wifi_advanced.cpp` (~1500 linhas)
- **Integração**: `src/core/menu_items/WifiMenu.cpp`

## Categorias (8 menus)

### 1. Reconhecimento (10 funções)
| Função | Descrição |
|--------|-----------|
| `wifiNetworkScanner` | Escaneamento completo de redes WiFi |
| `wifiHiddenNetworkFinder` | Encontra redes ocultas |
| `wifiVendorLookup` | OUI/Vendor lookup por MAC |
| `wifiSignalMapper` | Mapeamento de sinal por localizacao |
| `wifiChannelDistributor` | Distribuicao de canais |
| `wifiSecurityAnalyzer` | Analise de seguranca (WPA2/WPA3/OWE) |
| `wifiBeaconCollector` | Coleta de beacons |
| `wifiProbeSniffer` | Sniffer de probe requests |
| `wifiClientTracker` | Rastreador de clientes |
| `wifiNetworkProfiler` | Perfil completo da rede |

### 2. Attack Vectors (10 funções)
| Função | Descrição |
|--------|-----------|
| `wifiDeauthAllNetworks` | Deauth em todas as redes |
| `wifiBeaconFlood` | Flood de beacons malformados |
| `wifiProbeFlood` | Flood de probe requests |
| `wifiAuthFlood` | Flood de autenticacao |
| `wifiAssocFlood` | Flood de associacao |
| `wifiDisassocFlood` | Flood de desassociacao |
| `wifiEapolFlood` | Flood de EAPOL |
| `wifiDnsAmplification` | Amplificacao DNS |
| `wifiTcpSynFlood` | SYN flood |
| `wifiUdpFlood` | UDP flood |

### 3. WPA/WPA2 Cracking (8 funções)
| Função | Descrição |
|--------|-----------|
| `wifiHandshakeCapture` | Captura de handshake WPA |
| `wifiPmkidCapture` | Captura de PMKID |
| `wifiDeauthForHandshake` | Deauth para capturar handshake |
| `wifiBeaconSniff` | Sniff de beacons para cracking |
| `wifiEapolAnalyzer` | Analise de pacotes EAPOL |
| `wifiWordlistManager` | Gerenciador de wordlists |
| `wifiPasswordRecovery` | Recuperacao de senha |
| `wifiKeyConverter` | Conversor de chaves (PMK/PTK) |

### 4. Evil Twin & Social (8 funções)
| Função | Descrição |
|--------|-----------|
| `wifiEvilTwinCreator` | Cria evil twin automatico |
| `wifiCaptivePortal` | Portal cativo personalizado |
| `wifiPhishingGenerator` | Gerador de phishing pages |
| `wifiSslStrip` | SSL strip attack |
| `wifiMitmProxy` | Proxy MITM |
| `wifiCredentialSniffer` | Sniffer de credenciais |
| `wifiSessionHijacker` | Sequestro de sessao |
| `wifiCookieStealer` | Roubo de cookies |

### 5. Sniffing & Analysis (8 funções)
| Função | Descrição |
|--------|-----------|
| `wifiPacketSniffer` | Sniffer de pacotes |
| `wifiDnsSniffer` | Sniffer DNS |
| `wifiHttpSniffer` | Sniffer HTTP |
| `wifiArpSniffer` | Sniffer ARP |
| `wifiTlsSniffer` | Sniffer TLS/SSL |
| `wifiTrafficAnalyzer` | Analise de trafego |
| `wifiBandwidthMonitor` | Monitor de largura de banda |
| `wifiLatencyTester` | Teste de latencia |

### 6. Network Utilities (7 funções)
| Função | Descrição |
|--------|-----------|
| `wifiPortScanner` | Scanner de portas |
| `wifiServiceDiscovery` | Descoberta de servicos |
| `wifiHostnameResolver` | Resolucao de hostname |
| `wifiWhoisLookup` | Whois lookup |
| `wifiTraceroute` | Traceroute |
| `wifiNmapLite` | Nmap simplificado |
| `wifiNetbiosScanner` | Scanner NetBIOS |

### 7. Defensive Tools (7 funções)
| Função | Descrição |
|--------|-----------|
| `wifiIntrusionDetector` | Detector de intrusao |
| `wifiRogueApDetector` | Detector de APs falsos |
| `wifiJammingDetector` | Detector de jamming |
| `wifiPacketFilter` | Filtro de pacotes |
| `wifiMacFilter` | Filtro MAC |
| `wifiNetworkMonitor` | Monitor de rede |
| `wifiAlertSystem` | Sistema de alertas |

### 8. Configuration & Tools (5 funções)
| Função | Descrição |
|--------|-----------|
| `wifiMacSpoofer` | MAC spoofing |
| `wifiChannelHop` | Hopping de canais |
| `wifiPowerAdjust` | Ajuste de potencia |
| `wifiConfigManager` | Gerenciador de configuracoes |
| `wifiExportToSD` | Export para SD |

## Estruturas de Dados

### WiFiAdvConfig
```cpp
struct WiFiAdvConfig {
    int scanChannel;        // Canal para escaneamento (0=todos)
    int attackDuration;     // Duracao do ataque (segundos)
    int deauthPackets;      // Pacotes deauth por burst
    int beaconInterval;     // Intervalo de beacons (ms)
    bool randomizeMac;      // Randomizar MAC durante ataque
    bool stealthMode;       // Modo stealth
    int outputPower;        // Potencia de transmissao (dBm)
    bool autoDeauth;        // Auto-deauth durante captura
    int targetChannel;      // Canal alvo
    String targetSSID;      // SSID alvo
    String targetBSSID;     // BSSID alvo
    bool showFeedback;      // Mostrar feedback visual
    bool exportLogs;        // Exportar logs
};
```

## Visuais
- Cards coloridos por categoria (Cyan, Red, Orange, Magenta, Green, White, Yellow)
- Header animado com titulo
- Footer com instrucoes
- Barras de progresso
- Tela de boas-vindas com categorias

## Para Acessar
```
Menu → WiFi → WiFi Suite Avancada
```

## Notas de Segurança
- Todas as funcoes sao para fins **educacionais e de pesquisa em seguranca**
- Use apenas em ambientes controlados e autorizados
- Muitos ataques usam `esp_wifi_80211_tx` para injecao de frames brutos
