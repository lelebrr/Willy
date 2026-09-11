# NFC Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 50+ funções NFC/RFID para o Willy Firmware. Acesse via menu **RFID → NFC Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/rfid/nfc_advanced.h`
- **Implementação**: `src/modules/rfid/nfc_advanced.cpp` (~1300 linhas)
- **Integração**: `src/core/menu_items/RFIDMenu.cpp`

## Categorias (6 menus)

### 1. Analise & Leitura (10 funções)
| Função | Descrição |
|--------|-----------|
| `nfcTagInfoReader` | Leitura completa com UID, SAK, ATQA, tipo |
| `nfcUidAnalyzer` | Análise detalhada do UID |
| `nfcProtocolDetector` | Detecção automática de protocolo |
| `nfcNdefParser` | Parse de mensagens NDEF |
| `nfcMemoryDumper` | Dump completo da memória da tag |
| `nfcAuthAnalyzer` | Análise de autenticação |
| `nfcAntiCollisionAnalyzer` | Análise de anti-collision |
| `nfcTagFingerprint` | Fingerprint único da tag |
| `nfcSignalQualityChecker` | Verificação de qualidade do sinal |
| `nfcBatchDecoder` | Decodificação em lote |

### 2. Geracao & Clonagem (10 funções)
| Função | Descrição |
|--------|-----------|
| `nfcUidCloner` | Clonagem de UID |
| `nfcNdefBuilder` | Construção de mensagens NDEF (texto, URL, WiFi) |
| `nfcCustomDataWriter` | Escrita de dados customizados |
| `nfcMifareKeyManager` | Gerenciamento de chaves MIFARE |
| `nfcMagicTagWriter` | Escrita em Magic Tags |
| `nfcTagEmulator` | Emulação de tags |
| `nfcMultiTagCloner` | Clonagem múltipla de tags |
| `nfcDataConverter` | Conversão de dados (HEX, ASCII) |
| `nfcTagEditor` | Edição de dados da tag |
| `nfcFlipperConverter` | Conversão para formato Flipper |

### 3. Device Control & Auto (10 funções)
| Função | Descrição |
|--------|-----------|
| `nfcTagMapper` | Mapeamento de tags de dispositivos |
| `nfcMacroRecorder` | Gravação de macros NFC |
| `nfcMacroPlayer` | Reprodução de macros |
| `nfcSceneController` | Controle de cenas (portão, alarme, luz) |
| `nfcTimerScheduler` | Agendamento por timer |
| `nfcMultiDeviceCloner` | Clone multi-dispositivo |
| `nfcTagEmulatorAuto` | Emulação contínua |
| `nfcUidSearcher` | Busca de UIDs |
| `nfcProtocolExplorer` | Explorador de protocolos |
| `nfcUniversalReader` | Leitor universal |

### 4. Manipulacao Tag (10 funções)
| Função | Descrição |
|--------|-----------|
| `nfcTagRepeater` | Repetição de sinal da tag |
| `nfcProtocolConverter` | Conversão entre protocolos |
| `nfcTimingModifier` | Modificação de timing |
| `nfcDataObfuscator` | Ofuscação de dados (XOR) |
| `nfcTagAmplifier` | Amplificação de sinal |
| `nfcTagFilter` | Filtragem de tags |
| `nfcNoiseGenerator` | Geração de ruído RF |
| `nfcDataSplitter` | Divisão de dados |
| `nfcDataMerger` | Fusão de dados |
| `nfcProtocolBridge` | Ponte entre protocolos |

### 5. Testes & Diagnosticos (10 funções)
| Função | Descrição |
|--------|-----------|
| `nfcTagValidator` | Validação de tags |
| `nfcProtocolTester` | Teste de protocolos |
| `nfcReaderDiagnostics` | Diagnóstico do leitor |
| `nfcWriterDiagnostics` | Diagnóstico do escritor |
| `nfcRangeTest` | Teste de alcance |
| `nfcInterferenceDetector` | Detecção de interferência |
| `nfcTagBenchmark` | Benchmark de performance |
| `nfcProtocolStressTest` | Teste de estresse |
| `nfcHardwareDiagnostics` | Diagnóstico de hardware |
| `nfcTagPlayback` | Playback de tags |

## Estruturas de Dados

### NFCTagInfo
```cpp
struct NFCTagInfo {
    String uid;
    String sak;
    String atqa;
    String typeName;
    int totalPages;
    String dump;
    bool authSuccess;
};
```

## Visuais
- Cards coloridos por categoria (Cyan, Green, Orange, Magenta, Red)
- Header animado com título
- Footer com instruções
- Barras de progresso
- Spinners de carregamento
- Confirmação visual com SIM/NAO

## Módulos Suportados
- PN532 (I2C/SPI)
- RFID2 (RC522)
- ST25R3916
- Chameleon Ultra
- PN532 BLE

## Dicionário de Chaves
15 chaves padrão MIFARE pré-configuradas em `RFIDInterface::keys[]`.
