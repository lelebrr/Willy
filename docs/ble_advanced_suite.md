# BLE Advanced Suite (Willy)

Suite avançada de 53 funções BLE para pesquisa de segurança em `src/modules/ble/ble_advanced.{h,cpp}`.

**Menu:** Bluetooth → BLE Suite Avancada (builds completos, fora do `LITE_VERSION`).

## Estrutura

```
bleAdvancedSuiteMenu()
├── Reconhecimento (bleReconMenu)
│   ├── Filtro por Nome
│   ├── Filtro por RSSI
│   ├── Filtro por Servico
│   ├── Filtro por Fabricante
│   ├── Exportar CSV
│   ├── Scan Continuo
│   ├── Comparar Sinal
│   ├── Rastreador
│   ├── Dispositivos Ocultos
│   └── Watchdog
├── Ataques (bleAttackMenu)
│   ├── Relay Attack
│   ├── EATT Flood
│   ├── Conn Flood DoS
│   ├── Adv Flood DoS
│   ├── Param Abuse Drain
│   ├── Firmware Crash
│   ├── Char Overflow
│   ├── PIN Brute Force
│   ├── Pairing Storm
│   ├── Replay Attack
│   ├── MITM Pairing
│   ├── BLE Jammer
│   ├── GATT Confusion
│   ├── L2CAP Flood
│   └── Cross-Protocol
├── Utilidades (bleUtilityMenu)
│   ├── Packet Crafter
│   ├── Raw Advertising
│   ├── Decoder Mfg Data
│   ├── Monitor de Sinal
│   ├── MAC Randomizer
│   ├── Badge Creator
│   ├── Device Clone
│   ├── Estimador Distancia
│   ├── Packet Logger
│   └── Firmware Detector
├── Exploits Device (bleDeviceExploitMenu)
│   ├── AirPods Drain
│   ├── AirPods Crash
│   ├── Tile Manipulate
│   ├── Fitbit Inject
│   ├── Smart Lock Bypass
│   ├── Keyboard Inject
│   ├── HRM Data Spoof
│   ├── ESP32 Crash
│   ├── Smart Home Flood
│   └── GATT Exploiter
└── Monitoramento (bleMonitorMenu)
    ├── Tracker Detector
    ├── Device Classifier
    ├── Alerta Novos Disp.
    ├── Mapa de Sinal
    ├── Security Analyzer
    ├── Vuln Scanner
    ├── Service Enumerator
    └── Connection Quality
```

## Estruturas de Dados

| Estrutura | Descricao |
|---|---|
| `BLEScanResult` | Resultado de scan: nome, endereco, RSSI, appearance, servicos, manufacturer data |
| `BLETrackEntry` | Entrada de rastreamento: endereco, nome, RSSI, timestamps, contagem, media RSSI |
| `BLEPacketLog` | Log de pacote: timestamp, RSSI, tipo, dados, endereco fonte |
| `BLESignalSample` | Amostra de sinal: timestamp, RSSI |
| `BLEVulnCheck` | Verificacao de vulnerabilidade: nome, descricao, status, detalhes |

## Enums

| Enum | Valores |
|---|---|
| `BLESimpleFilterType` | `FILTER_NAME`, `FILTER_RSSI`, `FILTER_SERVICE`, `FILTER_MANUFACTURER`, `FILTER_APPEARANCE`, `FILTER_CONNECTABLE` |
| `BLEAttackMode` | `ATTACK_RELAY`, `ATTACK_FLOOD_EATT`, `ATTACK_CONN_FLOOD`, `ATTACK_ADV_FLOOD`, `ATTACK_PARAM_DRAIN`, `ATTACK_FIRMWARE_CRASH`, `ATTACK_CHAR_OVERFLOW`, `ATTACK_PAIRING_BRUTE`, `ATTACK_PAIRING_SCREAM`, `ATTACK_REPLAY`, `ATTACK_MITM_PAIR`, `ATTACK_JAMMER` |

## 1. Reconhecimento & Scanning (10 funcoes)

### 1.1 `bleScanFilterByName()`
Scan com filtro por padrao de nome. Permite busca parcial ou regex em nomes de dispositivos BLE.

### 1.2 `bleScanFilterByRSSI()`
Scan com filtro por faixa de RSSI. Filtra dispositivos por potencia de sinal (ex: -40 a -70 dBm).

### 1.3 `bleScanFilterByService()`
Scan com filtro por UUID de servico. Encontra dispositivos que anunciam servicos especificos (ex: 0x180F = Battery).

### 1.4 `bleScanFilterByManufacturer()`
Scan com filtro por dados do fabricante. Analisa manufacturer data nos advertisements.

### 1.5 `bleScanExportCSV()`
Exporta resultados de scan para CSV no SD card. Inclui nome, endereco, RSSI, servicos, timestamp.

### 1.6 `bleScanContinuous()`
Scan continuo com auto-save. Escaneia periodicamente e salva novos dispositivos encontrados.

### 1.7 `bleScanCompareRSSI()`
Compara RSSI entre multiplos dispositivos. Util para mapear distancias relativas.

### 1.8 `bleDeviceTracker()`
Rastreia movimento de dispositivo ao longo do tempo. Registra primeSeen, lastSeen, hitCount, mediaRSSI.

### 1.9 `bleScanHiddenDevices()`
Scan por dispositivos ocultos/non-broadcasting. Tenta descobrir dispositivos que nao anunciam ativamente.

### 1.10 `bleScanWatchdog()`
Watchdog: detecta novos dispositivos ao longo do tempo. Alerta quando um novo dispositivo entra no alcance.

## 2. Ataques & Exploits (15 funcoes)

### 2.1 `bleRelayAttack()`
Relay attack BLE: reencaminha dados entre dois dispositivos. Util para testes de proximidade.

### 2.2 `bleEATTFlood()`
EATT Channel Flood: esgota canais Enhanced ATT. Ataque de negacao de servico em camada de transporte.

### 2.3 `bleConnectionFloodDoS()`
Connection Flood DoS: inundacao de conexoes. Exaure recursos do alvo com conexoes simultaneas.

### 2.4 `bleAdvertisingFloodDoS()`
Advertising Flood DoS: inundacao de advertisements. Sobrecarrega receptores com pacotes de advertising.

### 2.5 `bleParamAbuseDrain()`
Connection Parameter Abuse: drena bateria. Forca parametros de conexao ineficientes para aumentar consumo.

### 2.6 `bleFirmwareCrash()`
Firmware Crash via pacotes malformados. Explora bugs no parsing de pacotes BLE.

### 2.7 `bleCharOverflow()`
Characteristic Overflow Attack: overflow de caracteristicas GATT. Testa limites de buffer.

### 2.8 `blePINBruteForce()`
PIN Brute Force: forca bruta de PIN numerico. Testa todos os PINs possiveis em pairing.

### 2.9 `blePairingStorm()`
Pairing Request Storm: tempestade de requisicoes de pairing. Sobrecarrega o servidor.

### 2.10 `bleReplayAttack()`
Replay Attack: captura e reproduz pacotes. Testa implementacoes de anti-replay.

### 2.11 `bleMITMPair()`
MITM Pairing Interceptor: intercepta processo de pairing. Simula ataque man-in-the-middle.

### 2.12 `bleJammerAll()`
BLE Jammer: jamma todos os canais. Bloqueia comunicacoes BLE na area (37, 38, 39).

### 2.13 `bleGATTConfusion()`
GATT Service Confusion: confusao de servicos GATT. Envia servicos/caracteristicas invalidas.

### 2.14 `bleL2CAPFlood()`
L2CAP Channel Flood: inundacao de canais L2CAP. Sobrecarrega camada de transporte.

### 2.15 `bleCrossProtocol()`
Cross-Protocol Interference: interferencia entre protocolos. Testa coexistencia BLE+WiFi.

## 3. Utilidades & Tools (10 funcoes)

### 3.1 `blePacketCrafter()`
BLE Packet Crafter: cria pacotes de advertising customizados. Permite construir payloads arbitrary.

### 3.2 `bleRawAdvertising()`
Raw Advertising Injection: injeta advertising raw. Envia pacotes sem validacao do stack.

### 3.3 `bleManufacturerDecoder()`
Manufacturer Data Decoder: decodifica dados do fabricante. Analisa campos de manufacturer data.

### 3.4 `bleSignalMonitor()`
Signal Strength Monitor: grafico de sinal em tempo real. Exibe RSSI em grafico continuo.

### 3.5 `bleMACRandomizer()`
MAC Address Randomizer: randomiza endereco MAC. Gera enderecos aleatorios para anonymidade.

### 3.6 `bleBadgeCreator()`
BLE Badge Creator: cria badge customizado. Define nome e icone para advertising.

### 3.7 `bleDeviceClone()`
Device Clone: clona dados de advertising. Replica advertising de outro dispositivo.

### 3.8 `bleDistanceEstimator()`
Distance Estimator: estima distancia baseado em RSSI. Usa modelo de path loss.

### 3.9 `blePacketLogger()`
Packet Logger: salva pacotes no SD card. Registra todos os pacotes capturados.

### 3.10 `bleFirmwareDetector()`
Firmware Version Detector: detecta versao de firmware. Identifica firmwares por patterns conhecidos.

## 4. Exploits por Dispositivo (10 funcoes)

### 4.1 `bleAirPodsDrain()`
AirPods Battery Drain: drena bateria de AirPods via BLE. Mantem conexao ativa.

### 4.2 `bleAirPodsCrash()`
AirPods Firmware Crash: crasha firmware de AirPods via pacotes malformados.

### 4.3 `bleTileManipulate()`
Tile Tracker Track Manipulation: manipula dados de Tile Trackers.

### 4.4 `bleFitbitInject()`
Fitbit Command Injection: injeta comandos em dispositivos Fitbit.

### 4.5 `bleSmartLockBypass()`
Smart Lock PIN Bypass: bypass de PIN em cadeados inteligentes.

### 4.6 `bleKeyboardInject()`
BLE Keyboard Keystroke Injection: injeta keystrokes em teclados BLE.

### 4.7 `bleHRMSpoof()`
Heart Rate Monitor Data Spoof: falsifica dados de monitores cardiacos.

### 4.8 `bleESP32Crash()`
ESP32 Crash Exploit: explora vulnerabilidades em ESP32s.

### 4.9 `bleSmartHomeFlood()`
Smart Home Device Flood: inundacao de dispositivos smart home.

### 4.10 `bleGATTExploiter()`
Generic GATT Exploiter: explorador generico de servicos GATT.

## 5. Monitoramento & Deteccao (8 funcoes)

### 5.1 `bleTrackerDetector()`
Tracker Detector: detecta AirTag, Tile, SmartTag. Identifica rastreadores na area.

### 5.2 `bleDeviceClassifier()`
Device Classifier: categoriza dispositivos por tipo. Classifica automaticamente.

### 5.3 `bleNewDeviceAlert()`
New Device Alert: alerta de entrada/saida de alcance. Monitora dispositivos novos.

### 5.4 `bleRSSIHeatmap()`
RSSI Heatmap: mapeamento de distancia. Gera mapa de calor baseado em sinal.

### 5.5 `bleSecurityAnalyzer()`
BLE Security Analyzer: analisa seguranca de dispositivos BLE. Verifica vulnerabilidades.

### 5.6 `bleVulnScanner()`
Vulnerability Scanner: scanner de vulnerabilidades. Verifica CVEs conhecidos.

### 5.7 `bleServiceEnumerator()`
Service Enumerator: enumera todos os servicos GATT. Leitura completa de atributos.

### 5.8 `bleConnectionQuality()`
Connection Quality Monitor: monitora qualidade de conexao. Metricas de latencia/perda.

## Funcoes Auxiliares

| Funcao | Descricao |
|---|---|
| `blePerformScan(duration, active)` | Realiza scan BLE e retorna vetor de resultados |
| `bleDisplayScanResult(result)` | Exibe resultado de scan no display |
| `bleDisplayProgress(msg, percent)` | Exibe barra de progresso |
| `bleConfirmAction(msg)` | Confirma acao com o usuario (Sim/Nao) |
| `bleSelectTarget()` | Seleciona alvo para ataque |
| `bleShowStats(packets, duration, rssi)` | Exibe estatisticas de execucao |
| `bleCleanup()` | Limpa recursos BLE apos uso |

## Uso

1. Navegue ate **Bluetooth → BLE Suite Avancada**
2. Selecione a categoria desejada
3. Escolha a funcao
4. Siga as instrucoes no display

## Dependencias

- NimBLE-Arduino (scan, conexao, GATT)
- ArduinoJson (serializacao de dados)
- SD card (exportacao de logs)
- Display TFT (interface grafica)
- MyKeyboard (input do usuario)
