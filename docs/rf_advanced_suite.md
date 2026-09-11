# RF Advanced Suite - Willy

## Visão Geral

O RF Advanced Suite é um toolkit completo de Sub-GHz com **50+ funções** organizadas em 5 categorias para pesquisa de segurança e automação via radiofrequência.

**Localização:** `src/modules/rf/rf_advanced.h` + `src/modules/rf/rf_advanced.cpp`  
**Acesso:** Menu RF → "RF Suite Avancada"

## Interface Visual

- Cards coloridos por categoria (igual IR/BLE Advanced Suites)
- Header/footer com navegação
- Barras de progresso para operações longas
- Spinner animado durante captura
- Tela de boas-vindas com logo WILLY

## Categorias

### 1. Análise & Scanning (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Analisador de Sinal** | Captura e analisa sinal RF via CC1101/RMT |
| 2 | **ID de Protocolo** | Identifica protocolo, bits, TE e frequência |
| 3 | **Comparação de Sinais** | Compara dois sinais RF capturados |
| 4 | **Scanner de Frequência** | Escaneia 20 frequências com RSSI |
| 5 | **Medidor de Força** | Mede intensidade RSSI em tempo real |
| 6 | **Análise de Timing** | Exibe TE, bitrate e duração total |
| 7 | **Qualidade do Sinal** | Verifica taxa de sucesso de decodificação |
| 8 | **Decoder em Lote** | Captura múltiplos sinais sequencialmente |
| 9 | **Dicionário Protoc.** | Lista protocolos Sub-GHz conhecidos |
| 10 | **Exportar Sinal** | Salva sinal em formato .sub (Flipper) |

### 2. Geração & Transmissão (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Clonador de Sinal** | Captura e reenvia sinal (clonagem direta) |
| 2 | **Gerador de Protocolo** | Gera código Princeton customizado |
| 3 | **Construtor Custom** | Monta sinal byte a byte (addr + cmd) |
| 4 | **Transmissão em Lote** | Envia múltiplos sinais do buffer |
| 5 | **Transmissão Agendada** | Agenda envio após delay configurável |
| 6 | **Repetir Sinal** | Repete sinal N vezes com intervalo |
| 7 | **Multi-Frequência** | Envia em 4 bandas (315/433/868/915 MHz) |
| 8 | **Modulador de Sinal** | Modifica bits do sinal capturado |
| 9 | **Conversor de Código** | Exibe código em HEX, BIN e DEC |
| 10 | **Editor de Sinal** | Edita bits individuais visualmente |

### 3. Device Control & Automação (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Mapeador de Devices** | Mapeia sinais de múltiplos dispositivos |
| 2 | **Gravador de Macro** | Grava sequência de sinais como macro |
| 3 | **Reprodutor de Macro** | Executa macro gravada |
| 4 | **Controlador de Cena** | Cenas pré-definidas (Garagem, Portão, Alarme) |
| 5 | **Agendador Timer** | Agenda envio após minutos configuráveis |
| 6 | **Clonador de Remoto** | Clona até 6 teclas de um controle |
| 7 | **Emulador de Device** | Emula dispositivos (Garagem, Portão, Alarme) |
| 8 | **Busca de Códigos** | Busca variações de código alvo |
| 9 | **Explorar Marcas** | Explora marcas Sub-GHz conhecidas |
| 10 | **Controle Univers.** | Controle básico Power/Abrir/Fechar |

### 4. Manipulação de Sinal (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Repetidor de Sinal** | Repete sinal com contagem configurável |
| 2 | **Conversor Protoc.** | Converte entre protocolos (Princeton→Nice→Came) |
| 3 | **Modificador Timing** | Ajusta fator de timing (50%-200%) |
| 4 | **Ofuscador de Código** | Aplica XOR para ofuscar dados |
| 5 | **Amplificador de Sinal** | Envia sinal 10x para aumentar alcance |
| 6 | **Filtro de Sinal** | Filtra sinais por qualidade/bits |
| 7 | **Gerador de Ruido** | Gera ruído RF para testes |
| 8 | **Divisor de Sinal** | Divide sinal em partes alta/baixa |
| 9 | **Fusão de Sinais** | Combina dois sinais em um |
| 10 | **Ponte Protoc.** | Recebe e retransmite em tempo real |

### 5. Testes & Diagnósticos (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Validador de Sinal** | Verifica integridade do sinal |
| 2 | **Teste de Protocolo** | Testa todos protocolos suportados |
| 3 | **Diagnóstico Receptor** | Verifica funcionalidade do receptor RF |
| 4 | **Diagnóstico Transmissor** | Verifica funcionalidade do transmissor |
| 5 | **Teste de Alcance** | Mede intensidade máxima do sinal |
| 6 | **Detector de Interferência** | Detecta ruído no canal RF |
| 7 | **Benchmark de Sinal** | Mede taxa de transmissão (env/s) |
| 8 | **Stress Test Protocolo** | Teste de estresse multi-frequência |
| 9 | **Diagnóstico Hardware** | Exibe configuração de pinos e módulo |
| 10 | **Playback de Sinal** | Captura e reenvia continuamente |

## Estruturas de Dados

```cpp
struct RFSignalInfo {
    float frequency;      // Frequência em MHz
    int rssi;             // Intensidade do sinal
    String protocol;      // Protocolo detectado
    uint64_t key;         // Código decodificado
    int bits;             // Número de bits/pulsos
    int te;               // Timing element (us)
    String preset;        // Preset CC1101
    String rawData;       // Dados brutos
    String description;   // Descrição adicional
};

struct RFMacroStep {
    uint32_t delay;       // Delay após passo (ms)
    float frequency;      // Frequência do passo
    String protocol;      // Protocolo do passo
    uint64_t key;         // Dados a enviar
    int bits;             // Bits do sinal
    int te;               // Timing base
    String label;         // Label do passo
};
```

## Configuração

- **Módulo RF:** `wilyConfigPins.rfModule` (M5_RF_MODULE ou CC1101_SPI_MODULE)
- **Pino TX:** `wilyConfigPins.rfTx`
- **Pino RX:** `wilyConfigPins.rfRx`
- **Frequência padrão:** `wilyConfigPins.rfFreq` (433.92 MHz)
- **Range:** 300-928 MHz (suportado pelo CC1101)

## Funções RF Existentes (não substituídas)

| Função | Arquivo | Descrição |
|--------|---------|-----------|
| RFScan | rf_scan.cpp | Escaneamento com RCSwitch |
| rf_raw_record | record.cpp | Gravação RAW |
| sendCustomRF | rf_send.cpp | Envio personalizado |
| rf_spectrum | rf_spectrum.cpp | Analisador de espectro |
| rf_waterfall | rf_waterfall.cpp | Espectrograma |
| rf_listen | rf_listen.cpp | Ouvir sinais RF |
| rf_bruteforce | rf_bruteforce.cpp | Força bruta |
| RFJammer | rf_jammer.cpp | Jammer (3 modos) |
| 9 Advanced Attacks | rf_advanced_atks.cpp | Ataques avançados |

## Dependências

- ELECHOUSE_CC1101_SRC_DRV
- ESP-IDF RMT (recepção de pulsos)
- ArduinoJson
- SD / LittleFS
- TFT_eSPI (display)
