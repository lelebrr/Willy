# Sistema de Logging Centralizado - Willy

## Visão Geral

O **Willy Logger** é um sistema de logging forense completo que registra todas as atividades do sistema no cartão SD para análise pós-operação. Projetado para uso em Red Team, Offensive Security e Forense Digital.

## Funcionalidades

### Logging Completo
- **GPS**: Coordenadas, altitude, satélites, tempo de fix
- **IR**: Códigos capturados, protocolos, bits
- **WiFi**: Scans, RSSI, SSIDs detectados
- **BLE**: Dispositivos encontrados, scans
- **CC1101 (Sub-GHz)**: Frequência, RSSI, modo de operação
- **NRF24**: Canal, pacotes, modo de operação
- **NFC/RFID**: UIDs, tipos de tags, status de leitura
- **RF**: Códigos Sub-GHz capturados

### Recursos Avançados
- **Timestamps precisos** (Unix time ou millis)
- **Registro de heap livre** em cada entrada
- **Códigos de erro específicos** por componente
- **Buffer em RAM** para menor desgaste do SD
- **Rotação automática** de arquivos de log
- **Thread-safe** com mutex FreeRTOS
- **Configuração persistente** em arquivo

## Arquitetura

```
┌─────────────────────────────────────────────────────────┐
│                    Willy Logger                         │
├─────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │    GPS       │  │     IR       │  │    WiFi      │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
│         │                 │                  │          │
│  ┌──────┴───────┐  ┌──────┴───────┐  ┌──────┴───────┐  │
│  │    BLE       │  │   CC1101     │  │    NRF24     │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
│         │                 │                  │          │
│  ┌──────┴───────┐  ┌──────┴───────┐                  │
│  │    NFC       │  │     RF       │                  │
│  └──────┬───────┘  └──────┬───────┘                  │
│         │                 │                           │
│         └────────┬────────┘                           │
│                  ▼                                    │
│  ┌───────────────────────────────────────────────┐   │
│  │            Buffer em RAM (10 entradas)        │   │
│  └───────────────────────┬───────────────────────┘   │
│                          ▼                            │
│  ┌───────────────────────────────────────────────┐   │
│  │         Arquivo CSV no SD (/WILLY_LOGS/)      │   │
│  └───────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

## Instalação

### Requisitos
- Cartão SD montado (128GB recomendado)
- Sistema de arquivos LittleFS para configuração
- ESP32 com PSRAM recomendado

### Dependências
O sistema utiliza bibliotecas já presentes no projeto:
- `ArduinoJson` - Para configuração
- `FS` e `SD` - Para armazenamento
- `FreeRTOS` - Para mutex

## Uso

### Inicialização Automática
O logger é inicializado automaticamente no `setup()` do main.cpp:

```cpp
if (sdcardMounted) {
    willyLogger.begin();
    willyLogger.logSystemStatus();
    willyLogger.info(COMP_SYSTEM, "Willy iniciado - versao " Willy_VERSION);
    willyLogger.showLogWarning();  // Mostra aviso no display
}
```

### Logging Básico

```cpp
#include "willy_logger.h"

// Informação geral
willyLogger.info(COMP_GPS, "GPS conectado");

// Com valores
willyLogger.info(COMP_WIFI, "Scan completo", apCount, rssi);

// Erro com código
willyLogger.error(COMP_CC1101, "Falha na inicialização", 0, 0, 0, 1001);

// Usando macros
LOG_GPS("Satélites: %d", 8);
LOG_WIFI("AP detectado: %s", ssid);
```

### Níveis de Log

| Nível | Valor | Uso |
|-------|-------|-----|
| TRACE | 0 | Debug detalhado |
| DEBUG | 1 | Debug normal |
| INFO | 2 | Informações gerais (padrão) |
| NOTICE | 3 | Avisos importantes |
| WARNING | 4 | Alertas |
| ERROR | 5 | Erros |
| CRITICAL | 6 | Erros críticos |
| OFF | 7 | Log desativado |

```cpp
// Definir nível mínimo
willyLogger.setMinLevel(LOG_LEVEL_DEBUG);

// Ativar/desativar
willyLogger.setEnabled(true);
```

### Métodos Específicos por Componente

#### GPS
```cpp
willyLogger.logGPS(lat, lon, alt, satellites, age, valid);
```

#### IR
```cpp
willyLogger.logIR(code, protocol, bits, raw);
```

#### WiFi
```cpp
willyLogger.logWiFiScan(apCount, rssi, ssid);
```

#### BLE
```cpp
willyLogger.logBLEScan(deviceCount, name);
```

#### CC1101
```cpp
willyLogger.logCC1101(freq, rssi, mode);
```

#### NRF24
```cpp
willyLogger.logNRF24(channel, packetCount, mode);
```

#### NFC
```cpp
willyLogger.logNFC(uid, type, readSuccess);
```

#### RF
```cpp
willyLogger.logRF(code, freq, protocol, raw);
```

### Gerenciamento de Arquivos

```cpp
// Forçar flush do buffer
willyLogger.flush();

// Listar arquivos de log
std::vector<String> files = willyLogger.listLogFiles();

// Obter tamanho total
uint64_t size = willyLogger.getTotalLogSize();

// Limpar logs antigos (manter 10)
willyLogger.cleanupOldLogs(10);
```

## Formato do Arquivo de Log

### CSV Format
```csv
Timestamp,Level,Component,Message,Value1,Value2,Value3,HeapFree,ErrorCode
12345,INFO,GPS,"GPS Fix: lat=-23.5505 lon=-46.6333",0,0,0,125000,0
12350,WARN,CC1101,"Sem sinal",-95,0,0,124800,0
12400,ERROR,NFC,"Falha na leitura",0,0,0,124500,1001
```

### Saída Serial
```
[12s] INFO [GPS]: GPS Fix: lat=-23.5505 lon=-46.6333 | heap=125000
[12s] WARN [CC1101]: Sem sinal | heap=124800
[12s] ERROR [NFC]: Falha na leitura | heap=124500 | err=1001
```

## Configuração

### Arquivo de Configuração (`/willy_log.conf`)
```json
{
  "enabled": true,
  "minLevel": 2,
  "logToSerial": true,
  "logToSD": true,
  "includeHeap": true,
  "flushInterval": 5000,
  "maxLogSize": 1048576,
  "bufferEntries": 10
}
```

### Estrutura de Configuração
```cpp
struct WillyLoggerConfig {
    bool enabled = true;                    // Log ativo
    WillyLogLevel minLevel = LOG_LEVEL_INFO;// Nível mínimo
    bool logToSerial = true;                // Enviar para Serial
    bool logToSD = true;                    // Salvar no SD
    bool includeHeap = true;                // Incluir heap livre
    uint32_t flushInterval = 5000;          // Intervalo de flush (ms)
    uint32_t maxLogSize = 1048576;          // 1MB por arquivo
    uint8_t bufferEntries = 10;             // Entradas no buffer
    String logDirectory = "/WILLY_LOGS";    // Diretório
};
```

## Componentes Suportados

| Componente | Enum | Descrição |
|------------|------|-----------|
| SYSTEM | COMP_SYSTEM | Sistema geral |
| GPS | COMP_GPS | Módulo GPS NEO-6M |
| IR | COMP_IR | Infravermelho |
| WIFI | COMP_WIFI | WiFi |
| BLE | COMP_BLE | Bluetooth Low Energy |
| CC1101 | COMP_CC1101 | Rádio Sub-GHz |
| NRF24 | COMP_NRF24 | Rádio 2.4GHz |
| NFC | COMP_NFC | NFC/RFID |
| RF | COMP_RF | RF genérico |
| BADUSB | COMP_BADUSB | BadUSB/HID |
| ETHERNET | COMP_ETHERNET | Ethernet |
| SDCARD | COMP_SDCARD | Cartão SD |
| DISPLAY | COMP_DISPLAY | Display TFT |
| POWER | COMP_POWER | Energia/Bateria |
| WEBUI | COMP_WEBUI | Interface Web |
| INTERPRETER | COMP_INTERPRETER | Interpretador JS/Lua |
| LORA | COMP_LORA | LoRa |
| FM | COMP_FM | FM Radio |

## Integração em Módulos

### Exemplo: Wardriving
```cpp
#include "willy_logger.h"

void wardrivingLoop() {
    if (gps.location.isValid()) {
        willyLogger.logGPS(
            gps.location.lat(),
            gps.location.lng(),
            gps.altitude.meters(),
            gps.satellites.value(),
            gps.location.age(),
            true
        );
    }
}
```

### Exemplo: IR Receiver
```cpp
#include "willy_logger.h"

void irReceiveLoop() {
    if (irrecv.decode(&results)) {
        willyLogger.logIR(
            results.value,
            results.decode_type,
            results.bits,
            false
        );
        irrecv.resume();
    }
}
```

### Exemplo: NFC Read
```cpp
#include "willy_logger.h"

void nfcReadLoop() {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        String uidStr = bytesToHexString(uid, uidLength);
        willyLogger.logNFC(uidStr.c_str(), "MIFARE", true);
    }
}
```

## Análise de Logs

### Importar no Excel
1. Copie o arquivo `.csv` do SD para o PC
2. Abra no Excel/Google Sheets
3. Use filtros por Componente, Level ou ErrorCode

### Análise via Python
```python
import pandas as pd

# Carregar log
df = pd.read_csv('willy_20260223_205530.csv')

# Filtrar erros
erros = df[df['Level'] == 'ERROR']

# Agrupar por componente
erros_por_componente = erros.groupby('Component').size()

# GPS fixes
gps = df[df['Component'] == 'GPS']
gps_validos = gps[gps['Message'].str.contains('Fix')]
```

### Scripts de Análise
Crie scripts em `sd_files/interpreter/` para análise direta no dispositivo:
```javascript
// analyze_logs.js
let logs = SD.readCSV("/WILLY_LOGS/willy_latest.csv");
let errors = logs.filter(r => r.Level === "ERROR");
console.log("Total de erros:", errors.length);
```

## Aviso de Log Ativo

Ao iniciar o sistema, se o logging estiver ativo, um aviso é exibido:

```
┌──────────────────────┐
│     LOG ATIVO        │
│                      │
│   Gravando em:       │
│ /WILLY_LOGS/xxx.csv  │
│                      │
│   Entradas: 42       │
│   Erros: 3           │
│                      │
│ Heap: 125000 bytes   │
└──────────────────────┘
```

## Troubleshooting

### SD não montado
```
[WillyLogger] SD não montado - logging limitado
```
**Solução**: Verifique se o SD está inserido e formatado corretamente.

### Erro ao criar diretório
```
[WillyLogger] Erro ao criar diretório de logs
```
**Solução**: Verifique permissões e espaço no SD.

### Erro ao criar arquivo
```
[WillyLogger] Erro ao criar arquivo de log
```
**Solução**: Verifique se o SD não está cheio ou corrompido.

## Estatísticas

```cpp
uint32_t total, errors, warnings;
willyLogger.getStats(total, errors, warnings);

Serial.printf("Total: %lu, Erros: %lu, Warnings: %lu\n", 
              total, errors, warnings);
```

## Boas Práticas

1. **Use níveis apropriados**: TRACE para debug, INFO para operações normais, ERROR para falhas
2. **Inclua contexto**: Sempre adicione informações relevantes na mensagem
3. **Códigos de erro**: Use códigos específicos para facilitar análise
4. **Flush regular**: Chame `flush()` antes de operações críticas
5. **Limpeza periódica**: Configure `cleanupOldLogs()` para evitar SD cheio

## Arquivos do Sistema

| Arquivo | Descrição |
|---------|-----------|
| `include/willy_logger.h` | Header com definições e API |
| `src/core/willy_logger.cpp` | Implementação |
| `/WILLY_LOGS/*.csv` | Arquivos de log no SD |
| `/willy_log.conf` | Configuração no LittleFS |

## Licença

MIT License - Uso livre para projetos open source e comerciais.

---

**Autor**: Willy Firmware Team  
**Versão**: 1.0  
**Data**: 2026-02-23