<div align="center">

# 🔧 Willy Firmware - Guia de Configuração de Placas

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compat%C3%ADvel-brightgreen.svg)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-Suportado-green.svg)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-Framework-orange.svg)](https://www.arduino.cc/)

**Guia completo para adicionar e configurar novas placas**

</div>

---

## 📋 Índice

- [Visão Geral](#-visão-geral)
- [Estrutura de Diretórios](#-estrutura-de-diretórios)
- [Adicionando Nova Placa](#-adicionando-nova-placa)
- [Arquivos de Configuração](#-arquivos-de-configuração)
- [Placas Suportadas](#-placas-suportadas)
- [Solução de Problemas](#-solução-de-problemas)

---

## 🗺️ Visão Geral

Este diretório contém todas as configurações específicas de hardware para as placas suportadas pelo Willy Firmware. Cada placa possui seus próprios arquivos de configuração que definem:

- 📍 **Mapeamento de pinos** (GPIO, SPI, I2C, UART)
- ⚙️ **Configurações de build** (flags, partições)
- 🖥️ **Setup específico** (display, touch, periféricos)
- 📦 **Definições de hardware** (LEDs, botões, cartão SD)

---

## 📂 Estrutura de Diretórios

```
boards/
│
├── 📁 _boards_json/              # JSONs de configuração PlatformIO
│   ├── 📄 CYD-2432S028.json
│   ├── 📄 ESP-General.json
│   ├── 📄 esp32dev.json
│   └── ... outros JSONs
│
├── 📁 pinouts/                   # Pinos genéricos compartilhados
│   ├── 📄 pins_arduino.h         # Header principal de pinos
│   └── 📄 variant.cpp            # Variante do Arduino
│
├── 📁 CYD-2432S028/              # Placa CYD-2432S028
│   ├── 📄 CYD-2432S028.ini       # Configuração PlatformIO
│   ├── 📄 interface.cpp          # Código de inicialização
│   └── 📄 pins_arduino.h         # Mapeamento de pinos
│
├── 📁 ESP-General/               # Configuração genérica ESP32
│   ├── 📄 ESP-General.ini
│   ├── 📄 interface.cpp
│   └── 📄 pins_arduino.h
│
└── 📄 README.md                  # Este arquivo
```

---

## 🚀 Adicionando Nova Placa

### Passo a Passo

#### 1. Criar Diretório da Placa

```bash
mkdir boards/minha_placa
```

#### 2. Criar JSON de Configuração

Crie `boards/_boards_json/minha_placa.json`:

```json
{
  "build": {
    "arduino": {
      "ldscript": "esp32.ld"
    },
    "core": "esp32",
    "extra_flags": [
      "-DARDUINO_ESP32_DEV",
      "-DBOARD_HAS_I2C",
      "-DBOARD_HAS_SPI"
    ],
    "f_cpu": "240000000L",
    "f_flash": "80000000L",
    "flash_mode": "qio",
    "mcu": "esp32",
    "variant": "pinouts"
  },
  "connectivity": [
    "wifi",
    "bluetooth",
    "ethereum",
    "can"
  ],
  "debug": {
    "openocd_board": "esp-wroom-32.cfg"
  },
  "frameworks": [
    "arduino",
    "espidf"
  ],
  "name": "Minha Placa ESP32",
  "upload": {
    "flash_size": "4MB",
    "maximum_ram_size": 327680,
    "maximum_size": 4194304,
    "require_upload_port": true,
    "speed": 921600
  },
  "url": "https://minhaplaca.com",
  "vendor": "Meu Fabricante"
}
```

#### 3. Criar Header de Pinos

Crie `boards/minha_placa/pins_arduino.h`:

```cpp
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// ================================
// PINOS GPIO
// ================================
#define PIN_WIRE_SDA        21
#define PIN_WIRE_SCL        22

#define PIN_SPI_MISO        19
#define PIN_SPI_MOSI        23
#define PIN_SPI_SCK         18
#define PIN_SPI_SS          5

// ================================
// DISPLAY TFT
// ================================
#define TFT_MISO            19
#define TFT_MOSI            23
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              2
#define TFT_RST             4
#define TFT_BL              21

// ================================
// TELA TOUCH
// ================================
#define TOUCH_CS            33
#define TOUCH_IRQ           36

// ================================
// CARTÃO SD
// ================================
#define SDCARD_CS           5
#define SDCARD_SCK          18
#define SDCARD_MISO         19
#define SDCARD_MOSI         23

// ================================
// INFRVERMELHO
// ================================
#define IR_TX_PIN           1
#define IR_RX_PIN           3

// ================================
// MÓDULOS RF
// ================================
#define RF_TX_PIN           22
#define RF_RX_PIN           27

// ================================
// LED E BOTÕES
// ================================
#define RGB_LED             -1
#define BTN_PIN             0

// ================================
// SERIAL
// ================================
#define SERIAL_TX           1
#define SERIAL_RX           3
#define GPS_SERIAL_TX       SERIAL_TX
#define GPS_SERIAL_RX       SERIAL_RX

#endif /* Pins_Arduino_h */
```

#### 4. Criar Interface de Inicialização

Crie `boards/minha_placa/interface.cpp`:

```cpp
#include "../core/mykeyboard.h"
#include "../core/display.h"
#include "../core/sd_functions.h"

// Inicialização específica da placa
void initBoard() {
    // Inicializar GPIO
    pinMode(BTN_PIN, INPUT_PULLUP);

    // Inicializar LED RGB se existir
    #ifdef HAS_RGB_LED
    // Código de inicialização do LED
    #endif

    // Outras inicializações específicas
}

// Ler bateria
float readBattery() {
    #ifdef BAT_PIN
    return analogRead(BAT_PIN) * 3.3 / 4095.0;
    #else
    return 0.0;
    #endif
}

// Verificar botão
bool checkBtnPress() {
    return digitalRead(BTN_PIN) == LOW;
}
```

#### 5. Criar Configuração PlatformIO

Crie `boards/minha_placa/minha_placa.ini`:

```ini
[env:minha_placa]
board = minha_placa
monitor_speed = 115200
board_build.partitions = custom_4Mb_full.csv

build_src_filter = ${env.build_src_filter} +<../boards/minha_placa>

build_flags =
    ${env.build_flags}
    -Iboards/minha_placa
    -Os

    # Debug
    -DCORE_DEBUG_LEVEL=0
    -DCONFIG_ESP32_JTAG_SUPPORT_DISABLE=1

    # Identificação da placa
    -DMINHA_PLACA=1
    -DDEVICE_NAME='"Minha Placa"'

    # Recursos
    -DWilly_IR_SERIAL=1
    -DHAS_SCREEN=1
    -DHAS_TOUCH=1

    # Display TFT
    -DUSER_SETUP_LOADED=1
    -DILI9341_2_DRIVER=1
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_BL=21
    -DTFT_BACKLIGHT_ON=HIGH

    # Cartão SD
    -DSDCARD_CS=5
    -DSDCARD_SCK=18
    -DSDCARD_MISO=19
    -DSDCARD_MOSI=23

    # IR
    -DIR_TX_PINS='{{"Pino 1", 1}}'
    -DIR_RX_PINS='{{"Pino 3", 3}}'

    # RF
    -DRF_TX_PINS='{{"Pino 22", 22}}'
    -DRF_RX_PINS='{{"Pino 27", 27}}'

    # CC1101 via SPI
    -DUSE_CC1101_VIA_SPI
    -DCC1101_GDO0_PIN=27
    -DCC1101_SS_PIN=22
    -DCC1101_MOSI_PIN=SPI_MOSI_PIN
    -DCC1101_SCK_PIN=SPI_SCK_PIN
    -DCC1101_MISO_PIN=SPI_MISO_PIN

    # NRF24 via SPI
    -DUSE_NRF24_VIA_SPI
    -DNRF24_CE_PIN=27
    -DNRF24_SS_PIN=22
    -DNRF24_MOSI_PIN=SPI_MOSI_PIN
    -DNRF24_SCK_PIN=SPI_SCK_PIN
    -DNRF24_MISO_PIN=SPI_MISO_PIN

lib_deps = ${env.lib_deps}
```

#### 6. Atualizar Header de Pinouts

Adicione em `boards/pinouts/pins_arduino.h`:

```cpp
// Adicionar condição para sua placa
#ifdef MINHA_PLACA
#include "../minha_placa/pins_arduino.h"
#endif
```

#### 7. Atualizar platformio.ini Principal

Adicione em `platformio.ini`:

```ini
[platformio]
default_envs = minha_placa

extra_configs =
    boards/*.ini
    boards/*/*.ini
```

---

## 📁 Arquivos de Configuração

### 📄 JSON de Configuração (`_boards_json/[placa].json`)

| Campo | Descrição |
|-------|-----------|
| `build.mcu` | Tipo de MCU (esp32, esp32s3, esp32c3) |
| `build.f_cpu` | Frequência da CPU em Hz |
| `build.f_flash` | Frequência do flash |
| `build.flash_mode` | Modo flash (qio, dio, qout) |
| `build.variant` | Deve apontar para "pinouts" |
| `upload.flash_size` | Tamanho do flash |
| `upload.maximum_size` | Tamanho máximo do firmware |

**Referência oficial:** [PlatformIO ESP32 Boards](https://github.com/platformio/platform-espressif32/blob/master/boards/)

### 📄 Header de Pinos (`[placa]/pins_arduino.h`)

Define todos os pinos GPIO da placa:

| Categoria | Pinos Típicos |
|-----------|---------------|
| **I2C** | SDA, SCL |
| **SPI** | MOSI, MISO, SCK, SS |
| **Display** | TFT_CS, TFT_DC, TFT_RST, TFT_BL |
| **Touch** | TOUCH_CS, TOUCH_IRQ |
| **Cartão SD** | SDCARD_CS, SDCARD_SCK, etc |
| **IR** | IR_TX, IR_RX |
| **RF** | RF_TX, RF_RX, GDO0 |
| **GPS** | GPS_TX, GPS_RX |
| **LED** | RGB_LED, LED_PIN |
| **Botões** | BTN_PIN |

**Referência oficial:** [Arduino ESP32 Variants](https://github.com/espressif/arduino-esp32/blob/master/variants/)

### 📄 Interface (`[placa]/interface.cpp`)

Contém código de inicialização específico:

```cpp
// Funções obrigatórias
void initBoard();        // Inicialização
float readBattery();     // Leitura de bateria
bool checkBtnPress();    // Verificar botão
```

### 📄 Configuração PlatformIO (`[placa]/[placa].ini`)

Herda de `[env]` e define configurações específicas:

| Seção | Descrição |
|-------|-----------|
| `build_flags` | Flags de compilação |
| `build_src_filter` | Filtros de código fonte |
| `board_build.partitions` | Tabela de partições |
| `lib_deps` | Dependências específicas |
| `lib_ignore` | Bibliotecas a ignorar |

---

## 🖥️ Placas Suportadas

### 📱 CYD-2432S028 (Cheap Yellow Display)

| Especificação | Valor |
|---------------|-------|
| **MCU** | ESP32-WROOM-32 |
| **Display** | 3.5" TFT ILI9341 |
| **Touch** | Resistivo XPT2046 |
| **Cartão SD** | Slot embutido |
| **USB** | 1x USB-C |
| **Preço** | ~R$100 |

#### Pinagem CYD

```
TFT:
  MOSI → GPIO 13
  MISO → GPIO 12
  SCK  → GPIO 14
  CS   → GPIO 15
  DC   → GPIO 2
  BL   → GPIO 21

Touch:
  MOSI → GPIO 13
  MISO → GPIO 12
  SCK  → GPIO 14
  CS   → GPIO 33
  IRQ  → GPIO 36

Cartão SD:
  MOSI → GPIO 23
  MISO → GPIO 19
  SCK  → GPIO 18
  CS   → GPIO 5
```

### 📱 CYD-2USB

Igual ao CYD-2432S028, mas com 2 portas USB-C.

| Diferença | Valor |
|-----------|-------|
| **USB** | 2x USB-C |
| **Display** | ILI9341 com inversão |

### 📱 CYD-3248S035 (Cheap Yellow Display 3.5")

| Especificação | Valor |
|---------------|-------|
| **MCU** | ESP32-WROOM-32 |
| **Display** | 3.5" TFT ST7796 (480x320) |
| **Touch (R)** | Resistivo XPT2046 |
| **Touch (C)** | Capacitivo GT911 |
| **Cartão SD** | Slot embutido |
| **USB** | 1x Micro-USB |
| **Extras** | RGB LED, Speaker, LDR |
| **Preço** | ~$25 |

#### Pinagem CYD-3248S035

```
TFT (ST7796):
  MOSI → GPIO 13
  MISO → GPIO 12
  SCK  → GPIO 14
  CS   → GPIO 15
  DC   → GPIO 2
  BL   → GPIO 27

Touch Resistivo (XPT2046):
  CS   → GPIO 33
  IRQ  → GPIO 36
  (CLK/DIN/DOUT compartilhados com TFT)

Touch Capacitivo (GT911):
  SDA  → GPIO 33
  SCL  → GPIO 32
  RST  → GPIO 25
  INT  → GPIO 21

Cartão SD:
  MOSI → GPIO 23
  MISO → GPIO 19
  SCK  → GPIO 18
  CS   → GPIO 5

Periféricos & Expansão:
  Speaker     → GPIO 26 (P4 Conector)
  LED R       → GPIO 4  (ativo LOW)
  LED G       → GPIO 16 (ativo LOW)
  LED B       → GPIO 17 (ativo LOW)
  LDR (Luz)   → GPIO 34 (ADC, input-only)
  Bateria     → Conector 4P (1.25mm) Direct Power (Sem Mod de ADC nativo)
  P1 (UART)   → TX (1), RX (3)
  P3 (IO)     → GPIO 35, 22, 21
  CN1 (I2C)   → SCL (22), SDA (27)
```

📖 **Documentação completa:** [hardware_cyd_3248s035.md](../docs/hardware_cyd_3248s035.md)

### 📱 ESP-General

Configuração genérica para qualquer ESP32:

- Use como modelo para novas placas
- Funciona com ESP32, ESP32-S3, ESP32-C3
- Pinagem configurável via defines

---

## 🔧 Solução de Problemas

### Problemas Comuns

| Problema | Solução |
|----------|---------|
| **Placa não compila** | Verifique os defines e includes |
| **Tela branca** | Verifique pinos do display |
| **Touch não funciona** | Verifique pinos do touch |
| **SD não monta** | Verifique pinos do cartão SD |
| **IR não envia** | Verifique pino IR_TX |

### Debug

```bash
# Compilar com debug
pio run -e minha_placa -v

# Ver erros
pio run -e minha_placa 2>&1 | grep error
```

### Validação

```bash
# Verificar configuração
pio boards | grep minha_placa

# Listar ambientes
pio run --list-targets
```

---

## 📚 Referências

### Documentação Oficial

| Recurso | Link |
|---------|------|
| PlatformIO ESP32 | [docs.platformio.org](https://docs.platformio.org/pt/latest/platforms/espressif32.html) |
| Arduino ESP32 | [espressif.com](https://docs.espressif.com/projects/arduino-esp32/) |
| Datasheet ESP32 | [espressif.com](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf) |
| TFT_eSPI | [github.com/Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) |

### Modelos

| Modelo | Uso |
|--------|-----|
| `ESP-General` | Ponto de partida para novas placas |
| `CYD-2432S028` | Referência completa com display |

---

<div align="center">

### 🛡️ Willy Firmware

**[⬆ Voltar ao Topo](#-Willy-firmware---guia-de-configuração-de-placas)**

*Documentação mantida pela comunidade Willy*

</div>
