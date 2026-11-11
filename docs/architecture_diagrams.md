# Diagramas de Arquitetura - Willy Firmware

## Arquitetura Geral (MVC + Módulos)

```mermaid
graph TB
    subgraph "Sistema Principal"
        A[main.cpp] --> B[SystemController]
        B --> C[SystemModel]
        B --> D[SystemView]
        B --> E[SystemManager]
    end

    subgraph "Padrão MVC"
        C --> F[Dados do Sistema]
        D --> G[Interface Gráfica LVGL]
        D --> H[Display TFT]
    end

    subgraph "Gerenciamento de Módulos"
        E --> I[WiFiModule]
        E --> J[RFModule]
        E --> K[RFIDModule]
        E --> L[Outros Módulos]
    end

    subgraph "Interface de Módulos"
        I --> M[IModule]
        J --> M
        K --> M
        L --> M
    end

    subgraph "Hardware ESP32-S3"
        N[ESP32-S3] --> O[WiFi/Bluetooth]
        N --> P[GPIO/Pinos]
        N --> Q[SPI/I2C/UART]
        N --> R[USB/CDC]
    end

    B --> N
    I --> O
    J --> P
    K --> Q
```

## Fluxos Principais

### Setup e Inicialização

```mermaid
flowchart TD
    A[main.cpp - setup()] --> B[SystemController::init()]
    B --> C[SystemManager::initAllModules()]
    C --> D[WiFiModule::init()]
    C --> E[RFModule::init()]
    C --> F[RFIDModule::init()]
    D --> G[Inicialização WiFi OK]
    E --> H[Inicialização RF OK]
    F --> I[Inicialização RFID OK]
    G --> J[SystemController::startStartupApp()]
    H --> J
    I --> J
    J --> K[Menu Principal Pronto]
```

### Loop Principal

```mermaid
flowchart TD
    A[main.cpp - loop()] --> B[SystemController::runMainLoop()]
    B --> C[SystemManager::processAllModules()]
    C --> D[WiFiModule::process()]
    C --> E[RFModule::process()]
    C --> F[RFIDModule::process()]
    D --> G[Processamento WiFi]
    E --> H[Processamento RF]
    F --> I[Processamento RFID]
    G --> J[SystemController::processMenuInput()]
    H --> J
    I --> J
    J --> K[Atualização Interface]
    K --> L[Loop Continua]
```

### Navegação de Menus

```mermaid
stateDiagram-v2
    [*] --> MainMenu
    MainMenu --> WiFiMenu: Selecionar WiFi
    MainMenu --> RFMenu: Selecionar RF
    MainMenu --> RFIDMenu: Selecionar RFID
    MainMenu --> SettingsMenu: Selecionar Configurações

    WiFiMenu --> WiFiScan: Escanear Redes
    WiFiMenu --> WiFiAttack: Ataques WiFi
    WiFiScan --> MainMenu: Voltar
    WiFiAttack --> MainMenu: Voltar

    RFMenu --> RFScan: Escanear RF
    RFMenu --> RFReplay: Replay RF
    RFScan --> MainMenu: Voltar
    RFReplay --> MainMenu: Voltar

    RFIDMenu --> RFIDRead: Ler Tags
    RFIDMenu --> RFIDWrite: Escrever Tags
    RFIDRead --> MainMenu: Voltar
    RFIDWrite --> MainMenu: Voltar

    SettingsMenu --> MainMenu: Voltar
```

## Integrações de Hardware

```mermaid
graph TB
    subgraph "ESP32-S3 Core"
        A[ESP32-S3] --> B[CPU Dual Core]
        A --> C[WiFi 802.11ax]
        A --> D[Bluetooth 5.0]
        A --> E[USB OTG]
        A --> F[PSRAM 8MB]
        A --> G[Flash 8MB]
    end

    subgraph "Interfaces de Comunicação"
        H[SPI] --> I[TFT ILI9341]
        H --> J[SD Card]
        H --> K[RF Modules CC1101]
        L[I2C] --> M[RTC DS3231]
        L --> N[Gesture Sensor]
        O[UART] --> P[GPS NEO-6M]
        O --> Q[NFC PN532]
        R[GPIO] --> S[LED RGB WS2812]
        R --> T[IR Receiver/Transmitter]
        R --> U[Buttons/Joystick]
    end

    subgraph "Módulos de Ataque"
        V[WiFi Module] --> C
        W[RF Module] --> K
        X[RFID Module] --> Q
        Y[IR Module] --> T
        Z[BLE Module] --> D
    end

    B --> V
    B --> W
    B --> X
    B --> Y
    B --> Z
```

## Dependências de Bibliotecas

```mermaid
graph LR
    subgraph "Core Libraries"
        A[TFT_eSPI] --> B[Display]
        C[LVGL] --> B
        D[FastLED] --> E[LED Control]
        F[Adafruit NeoPixel] --> E
    end

    subgraph "Communication Libraries"
        G[WiFi] --> H[WiFi Attacks]
        I[Bluetooth] --> J[BLE Operations]
        K[NimBLE-Arduino] --> J
        L[RF24] --> M[NRF24 Operations]
        N[rc-switch] --> O[RF 433MHz]
        P[IRremoteESP8266] --> Q[IR Control]
    end

    subgraph "Security Libraries"
        R[RFID_MFRC522v2] --> S[RFID/NFC]
        T[Adafruit PN532] --> S
        U[ESP Chameleon Ultra] --> S
        V[LibSSH-ESP32] --> W[SSH Operations]
        X[WireGuard-ESP32] --> Y[VPN]
    end

    subgraph "Utility Libraries"
        Z[ArduinoJson] --> AA[Configuration]
        BB[ESPAsyncWebServer] --> CC[Web Interface]
        DD[TinyGPSPlus] --> EE[GPS Processing]
        FF[FFT] --> GG[Audio Processing]
    end

## 🗺️ Mapa de Pinagem Otimizada

### Barramentos Compartilhados

```mermaid
graph TD
    subgraph "SPI Compartilhado (3 pinos → 6 dispositivos)"
        A[SPI MOSI - GPIO 11] --> B[TFT ILI9341]
        A --> C[Touch XPT2046]
        A --> D[SD Card]
        A --> E[NRF24L01 #1]
        A --> F[NRF24L01 #2]
        A --> G[CC1101 Sub-GHz]

        H[SPI SCK - GPIO 12] --> B
        H --> C
        H --> D
        H --> E
        H --> F
        H --> G

        I[SPI MISO - GPIO 13] --> B
        I --> C
        I --> D
        I --> E
        I --> F
        I --> G
    end

    subgraph "I2C Compartilhado (2 pinos → 3 dispositivos)"
        J[I2C SDA - GPIO 8] --> K[PN532 NFC]
        J --> L[DS3231 RTC]
        J --> M[PAJ7620 Gesture]

        N[I2C SCL - GPIO 17] --> K
        N --> L
        N --> M
    end

    subgraph "UARTs Dedicados"
        O[UART1 TX - GPIO 39] --> P[GPS NEO-6M]
        Q[UART1 RX - GPIO 40] --> P

        R[UART2 TX - GPIO 1] --> S[IR YS-IRTM]
        T[UART2 RX - GPIO 47] --> S
    end
```

### Eficiência de Pinagem

| Barramento | Pinos Utilizados | Dispositivos | Eficiência |
|------------|------------------|--------------|------------|
| SPI | 3 | 6 | 80% redução |
| I2C | 2 | 3 | 67% redução |
| UART | 4 | 2 | 100% dedicado |
| **Total** | **9** | **11** | **60% economia** |

## ⚡ Diagrama de Consumo de Energia

```mermaid
graph LR
    subgraph "Fonte de Alimentação 5V/3.3V"
        A[5V Input] --> B[Regulador ESP32-S3]
        A --> C[Regulador TFT ILI9341]
        A --> D[Regulador GPS NEO-6M]

        E[3.3V Rail] --> F[ESP32-S3 Core]
        E --> G[NRF24L01 PA+LNA]
        E --> H[CC1101 Sub-GHz]
        E --> I[PN532 NFC]
    end

    subgraph "Consumo por Módulo (mA)"
        F --> J[ESP32: 150mA médio]
        C --> K[TFT: 60mA médio]
        G --> L[NRF24: 26mA médio]
        H --> M[CC1101: 17mA médio]
        I --> N[PN532: 60mA médio]
        D --> O[GPS: 45mA médio]
    end

    subgraph "Picos de Corrente"
        J --> P[Pico: 500mA]
        K --> Q[Pico: 80mA]
        L --> R[Pico: 115mA]
        M --> S[Pico: 30mA]
        N --> T[Pico: 150mA]
        O --> U[Pico: 67mA]
    end

    P --> V[Total Pico: 942mA]
    Q --> V
    R --> V
    S --> V
    T --> V
    U --> V
```

### Otimizações de Energia

- **Modo Sleep Automático**: PN532 e GPS entram em sleep quando inativos
- **PWM Backlight Control**: TFT ajusta brilho automaticamente
- **LDO Dedicados**: NRF24 e CC1101 têm reguladores próprios
- **Eficiência Total**: 85%+ em modo ativo

## 🔄 Fluxo de Benchmarking

```mermaid
flowchart TD
    A[BenchmarkManager] --> B[CPU Monitor]
    A --> C[Memory Monitor]
    A --> D[Latency Timer]

    B --> E[Sample CPU Usage]
    C --> F[Sample Heap Usage]
    D --> G[Start Timer]

    E --> H[WiFi Scan Test]
    F --> H
    G --> H

    H --> I[RF Transmit Test]
    H --> J[RFID Read Test]
    H --> K[ML Inference Test]

    I --> L[Stop Timer]
    J --> L
    K --> L

    L --> M[Calculate Metrics]
    M --> N[Generate Report]
    N --> O[Log to SD Card]
    N --> P[Display Dashboard]

    O --> Q[Automated Analysis]
    P --> Q
    Q --> R[Performance Insights]
```