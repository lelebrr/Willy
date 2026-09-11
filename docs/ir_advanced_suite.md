# IR Advanced Suite - Willy

## Visão Geral

O IR Advanced Suite é um toolkit completo de infravermelho com **50+ funções** organizadas em 5 categorias para pesquisa de segurança e automação.

**Localização:** `src/modules/ir/ir_advanced.h` + `src/modules/ir/ir_advanced.cpp`  
**Acesso:** Menu IR → "IR Suite Avancada"

## Interface Visual

- Cards coloridos por categoria
- Header/footer com navegação
- Barras de progresso para operações longas
- Spinner animado durante captura
- Tela de boas-vindas com logo WILLY

## Categorias

### 1. Análise & Decodificação (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Analisador de Sinal** | Captura e analisa sinal IR em tempo real |
| 2 | **ID de Protocolo** | Identifica protocolo, bits, address e command |
| 3 | **Comparação de Sinais** | Compara dois sinais capturados |
| 4 | **Scanner de Freqüência** | Testa múltiplas frequências de modulação |
| 5 | **Medidor de Força** | Mede intensidade do sinal IR (RSSI) |
| 6 | **Análise de Timing** | Exibe tempos de header, bit, space e gap |
| 7 | **Qualidade do Sinal** | Verifica taxa de sucesso de decodificação |
| 8 | **Decoder em Lote** | Captura múltiplos sinais sequencialmente |
| 9 | **Dicionário Protoc.** | Lista protocolos suportados com detalhes |
| 10 | **Exportar Sinal** | Salva sinal capturado em arquivo SD |

### 2. Geração & Clonagem (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Clonador de Sinal** | Captura e reenvia sinal (clonagem direta) |
| 2 | **Gerador de Protocolo** | Gera código NEC/RC5/RC6 customizado |
| 3 | **Construtor Custom** | Monta sinal byte a byte (addr + cmd) |
| 4 | **Transmissão em Lote** | Envia múltiplos sinais do buffer |
| 5 | **Transmissão Agendada** | Agenda envio após delay configurável |
| 6 | **Repetir Sinal** | Repete sinal N vezes com intervalo |
| 7 | **Multi-Protocolo** | Captura e envia em diferentes protocolos |
| 8 | **Modulador de Sinal** | Modifica bits do sinal capturado |
| 9 | **Conversor de Código** | Exibe código em HEX, BIN e DEC |
| 10 | **Editor de Sinal** | Edita bits individuais visualmente |

### 3. Device Control & Automação (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Mapeador de Devices** | Mapeia sinais de múltiplos dispositivos |
| 2 | **Gravador de Macro** | Grava sequência de sinais como macro |
| 3 | **Reprodutor de Macro** | Executa macro gravada |
| 4 | **Controlador de Cena** | Cenas pré-definidas (Luz, TV, Volume) |
| 5 | **Agendador Timer** | Agenda envio após minutos configuráveis |
| 6 | **Clonador de Remoto** | Clona até 6 teclas de um controle |
| 7 | **Emulador de Device** | Emula dispositivos (Samsung, Daikin, etc.) |
| 8 | **Busca de Códigos** | Busca variações de código alvo |
| 9 | **Explorar Marcas** | Explora marcas e protocolos conhecidos |
| 10 | **Controle Univers.** | Controle básico Power/Vol+/Vol- |

### 4. Manipulação de Sinal (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Repetidor de Sinal** | Repete sinal com contagem configurável |
| 2 | **Conversor Protoc.** | Converte entre protocolos (NEC→RC5→etc.) |
| 3 | **Modificador Timing** | Ajusta fator de timing (50%-200%) |
| 4 | **Ofuscador de Código** | Aplica XOR para ofuscar dados |
| 5 | **Amplificador de Sinal** | Envia sinal 10x para aumentar alcance |
| 6 | **Filtro de Sinal** | Filtra sinais por qualidade/bits |
| 7 | **Gerador de Ruido** | Gera ruído IR para testes de interferência |
| 8 | **Divisor de Sinal** | Divide sinal em partes alta/baixa |
| 9 | **Fusão de Sinais** | Combina dois sinais em um |
| 10 | **Ponte Protoc.** | Recebe e retransmite em tempo real |

### 5. Testes & Diagnósticos (10 funções)

| # | Função | Descrição |
|---|--------|-----------|
| 1 | **Validador de Sinal** | Verifica integridade do sinal |
| 2 | **Teste de Protocolo** | Testa todos protocolos suportados |
| 3 | **Diagnóstico Receptor** | Verifica funcionalidade do receptor IR |
| 4 | **Diagnóstico Transmissor** | Verifica funcionalidade do transmissor |
| 5 | **Teste de Alcance** | Mede intensidade máxima do sinal |
| 6 | **Detector de Interferência** | Detecta ruído no canal IR |
| 7 | **Benchmark de Sinal** | Mede taxa de transmissão (env/s) |
| 8 | **Stress Test Protocolo** | Teste de estresse com múltiplos protocolos |
| 9 | **Diagnóstico Hardware** | Exibe configuração de pinos e status |
| 10 | **Playback de Sinal** | Captura e reenvia continuamente |

## Estruturas de Dados

```cpp
struct IRSignalInfo {
    decode_type_t protocol;   // Protocolo detectado
    uint64_t data;            // Código decodificado
    uint32_t bits;            // Número de bits
    uint16_t address;         // Endereço do dispositivo
    uint16_t command;         // Comando recebido
    uint32_t frequency;       // Frequência de modulação
    float dutyCycle;          // Ciclo de trabalho
    String protocolName;      // Nome do protocolo
    String description;       // Descrição adicional
};

struct IRMacroStep {
    uint32_t delay;           // Delay após passo (ms)
    decode_type_t protocol;   // Protocolo do passo
    uint64_t data;            // Dados a enviar
    uint32_t bits;            // Bits do sinal
    String label;             // Label do passo
};
```

## Configuração

- **Pino TX:** `wilyConfigPins.irTx`
- **Pino RX:** `wilyConfigPins.irRx`
- **Frequência padrão:** 38kHz
- **Repetições:** `wilyConfigPins.irTxRepeats`

## Dependências

- IRremoteESP8266
- ArduinoJson
- SD (para exportação)
- TFT_eSPI (display)
