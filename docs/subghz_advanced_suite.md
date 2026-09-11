# SubGHz Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 55+ funções CC1101 Sub-GHz para o Willy Firmware. Acesse via menu **RF → SubGHz Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/rf/subghz_advanced.h`
- **Implementação**: `src/modules/rf/subghz_advanced.cpp` (~1500 linhas)
- **Integração**: `src/core/menu_items/RFMenu.cpp`
- **Depende de**: ELECHOUSE_CC1101_SRC_DRV, RCSwitch

## Categorias (7 menus)

### 1. Espectro (10 funções)
| Função | Descrição |
|--------|-----------|
| `subghzSpectrumAnalyzer` | Analisador de espectro completo 300-930MHz |
| `subghzFreqScanner` | Scanner de frequencias com RSSI |
| `subghzSignalDetector` | Detector de sinais em tempo real |
| `subghzNoiseFloor` | Medicao de piso de ruido |
| `subghzPeakFinder` | Encontra picos de sinal |
| `subghzBandwidthAnalyzer` | Analise de largura de banda |
| `subghzModulationDetector` | Detector de modulacao (ASK/FSK/OOK) |
| `subghzHarmonicFinder` | Encontra harmonicas |
| `subghzInterferenceMap` | Mapa de interferencia colorido |
| `subghzSignalStrengthLog` | Log de intensidade de sinal |

### 2. Captura & Decode (10 funções)
| Função | Descrição |
|--------|-----------|
| `subghzSignalCapture` | Captura de sinal bruto via RMT |
| `subghzProtocolDecoder` | Decodificador de protocolo RCSwitch |
| `subghzBitAnalyzer` | Analisador de bits (BIN/HEX) |
| `subghzTimingDecoder` | Decodificador de timing |
| `subghzPulseAnalyzer` | Analisador de pulsos |
| `subghzMultiProtocolCapture` | Captura multi-protocolo (315/390/433/868/915) |
| `subghzSignalComparison` | Comparacao de sinais |
| `subghzSignalFingerprint` | Fingerprint de sinal |
| `subghzSignalExporter` | Export para formato Flipper .sub |
| `subghzSignalValidator` | Validacao de sinal |

### 3. Transmissao (10 funções)
| Função | Descrição |
|--------|-----------|
| `subghzSignalCloner` | Clonador de sinal com replay |
| `subghzProtocolGenerator` | Gerador de protocolo (Princeton/Nice/CAME) |
| `subghzCodeBuilder` | Construtor de codigo custom |
| `subghzRollingCodeGen` | Gerador de rolling code |
| `subghzBruteForce` | Brute force de codigo |
| `subghzSignalReplay` | Replay de sinal capturado |
| `subghzMultiFreqTransmit` | Transmissao multi-frequencia |
| `subghzTimedTransmit` | Transmissao temporizada |
| `subghzBulkTransmit` | Transmissao em lote (100 codigos) |
| `subghzSignalModulator` | Modulador de sinal |

### 4. Emulacao (8 funções)
| Função | Descrição |
|--------|-----------|
| `subghzRemoteEmulator` | Emulador de controle remoto |
| `subghzGarageOpener` | Abridor de garagem |
| `subghzGateOpener` | Abridor de portao |
| `subghzDoorbellEmulator` | Emulador de campainha |
| `subghzAlarmEmulator` | Emulador de alarme |
| `subghzSensorEmulator` | Emulador de sensor (TPMS) |
| `subghzLightSwitch` | Interruptor de luz |
| `subghzUniversalRemote` | Controle universal (6 dispositivos) |

### 5. Ataques (8 funções)
| Função | Descrição |
|--------|-----------|
| `subghzReplayAttack` | Ataque de replay |
| `subghzRollingCodeAttack` | Ataque de rolling code |
| `subghzBruteForceAttack` | Ataque de brute force (1000 codigos) |
| `subghzProtocolFuzzing` | Fuzzing de protocolo (500 tentativas) |
| `subghzSignalJamming` | Jamming de sinal |
| `subghzRollingCodeSniff` | Sniff de rolling code |
| `subghzKeeLoqAttack` | Ataque KeeLoq |
| `subghzFixedCodeAttack` | Ataque de codigo fixo |

### 6. Monitoramento (6 funções)
| Função | Descrição |
|--------|-----------|
| `subghzWeatherStation` | Estacao meteorologica |
| `subghzTPMSMonitor` | Monitor TPMS (pneus) |
| `subghzMotionDetector` | Detector de movimento |
| `subghzDoorWindowSensor` | Sensor porta/janela |
| `subghzTemperatureSensor` | Sensor de temperatura |
| `subghzEnergyMonitor` | Monitor de energia |

### 7. Utilitarios (7 funções)
| Função | Descrição |
|--------|-----------|
| `subghzFrequencyConverter` | Conversor de frequencia |
| `subghzCodeDatabase` | Banco de codigos |
| `subghzSignalLibrary` | Biblioteca de sinais |
| `subghzRecordingManager` | Gerenciador de gravacoes |
| `subghzProtocolInfo` | Info de protocolos |
| `subghzHardwareTest` | Teste de hardware CC1101 |
| `subghzExportToSD` | Export para SD |

## Estruturas de Dados

### SubGHzAdvConfig
```cpp
struct SubGHzAdvConfig {
    float baseFrequency;     // 315/433/868/915 MHz
    int modulationType;      // 0=ASK, 1=FSK, 2=OOK
    float dataRate;          // kbps
    float deviation;         // kHz
    float rxBW;              // kHz
    int txPower;             // dBm
    bool randomizeAddress;   // Randomizar endereco
    int repeatCount;         // Repeticoes por TX
    int delayBetween;        // Delay entre TX (ms)
    bool autoDetect;         // Auto-detectar protocolo
    bool saveToSD;           // Salvar no SD
    bool showRawData;        // Mostrar dados brutos
    int protocolIndex;       // Protocolo selecionado
};
```

## Visuais
- Cards coloridos por categoria (Cyan, Green, Orange, Magenta, Red, Yellow, White)
- Header animado com titulo
- Footer com instrucoes
- Barras de progresso
- Tela de boas-vindas com categorias

## Dependencias
- `ELECHOUSE_CC1101_SRC_DRV` - Driver CC1101
- `RCSwitch` - Protocolos OOK/ASK
- `rf_utils.h` - Utilitarios RF
- `rf_send.h` - Transmissao RF

## Para Acessar
```
Menu → RF → SubGHz Suite Avancada
```

## Notas de Segurança
- Todas as funcoes sao para fins **educacionais e de pesquisa em seguranca**
- Use apenas em ambientes controlados e autorizados
- Ataques e jamming podem ser ilegais em alguns paises
