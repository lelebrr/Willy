# Documentação de Hardware: CYD ESP32-3248S035 (Cheap Yellow Display 3.5")

## Visão Geral

A **ESP32-3248S035**, popularmente conhecida como **CYD 3.5"**, é uma placa de desenvolvimento baseada no ESP32-WROOM-32 que integra um display TFT de 3.5" com resolução de 480x320 (driver ST7796), slot para cartão SD, LED RGB, speaker, sensor de luz (LDR) e touchscreen.

Existem duas variantes:

- **ESP32-3248S035R** — Touchscreen **Resistivo** (XPT2046)
- **ESP32-3248S035C** — Touchscreen **Capacitivo** (GT911)

---

## ⚙️ Especificações Técnicas

| Componente | Detalhes |
| :--- | :--- |
| **MCU** | ESP32-WROOM-32 (Dual Core, 240MHz, Wi-Fi/BT 4.2) |
| **Flash** | 4MB |
| **Display** | 3.5" TFT SPI (480x320 px) |
| **Driver de Vídeo** | ST7796 |
| **Touchscreen (R)** | Resistivo XPT2046 (SPI) |
| **Touchscreen (C)** | Capacitivo GT911 (I2C) |
| **Armazenamento** | Slot MicroSD (SPI) |
| **USB** | 1x Micro-USB (UART/Power) |
| **Extras** | LED RGB, Speaker (8002A amp), Sensor de Luz (LDR/CdS), Botões Boot/Reset |

---

## 📍 Mapeamento de Pinos (Pinout)

### 1. Display TFT (SPI — HSPI)

O display utiliza o barramento SPI do ESP32 com driver ST7796.

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **TFT_MISO** | 12 | Dados do Display (Out) |
| **TFT_MOSI** | 13 | Dados do Display (In) |
| **TFT_SCLK** | 14 | Clock do Display |
| **TFT_CS** | 15 | Chip Select |
| **TFT_DC** | 2 | Data/Command |
| **TFT_RST** | -1 | Reset via software (ligado ao RST global) |
| **TFT_BL** | 27 | Backlight (PWM) |

### 2. Touchscreen Resistivo — XPT2046 (Modelo "R")

Compartilha o barramento SPI com o display.

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **TOUCH_CS** | 33 | Chip Select do Touch |
| **TOUCH_CLK** | 14 | Compartilhado com TFT_SCLK |
| **TOUCH_DIN** | 13 | Compartilhado com TFT_MOSI |
| **TOUCH_DOUT** | 12 | Compartilhado com TFT_MISO |
| **TOUCH_IRQ** | 36 | Interrupção (SENSOR_VP, input-only) |

### 3. Touchscreen Capacitivo — GT911 (Modelo "C")

Utiliza barramento I2C dedicado.

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **GT911_SDA** | 33 | I2C Data |
| **GT911_SCL** | 32 | I2C Clock |
| **GT911_RST** | 25 | Reset |
| **GT911_INT** | 21 | Interrupção |
| **Endereço I2C** | 0x5D | Slave address (baixo) |

### 4. Cartão SD (SPI)

Utiliza barramento SPI separado (VSPI).

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **SD_CS** | 5 | Chip Select |
| **SD_SCK** | 18 | Clock |
| **SD_MISO** | 19 | MISO |
| **SD_MOSI** | 23 | MOSI |

### 5. Periféricos Integrados

| Componente | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **LED RGB - Vermelho** | 4 | Ativo Baixo (LOW = Ligado) |
| **LED RGB - Verde** | 16 | Ativo Baixo |
| **LED RGB - Azul** | 17 | Ativo Baixo |
| **Sensor de Luz (LDR)** | 34 | Entrada ADC (input-only) |
| **Speaker (Áudio)** | 26 | Saída PWM/DAC para amplificador 8002A |
| **Botão BOOT** | 0 | Usado para modo Flash / Wake |
| **Botão RESET** | - | Reset de Hardware |

---

## 🔌 Conectores e Portas

A placa possui conectores Micro JST 1.25mm para expansão:

- **P1 (UART)**: 5V, GND, TX (GPIO1), RX (GPIO3). Ideal para GPS ou Serial Externo.
- **P3 (Extended IO)**: GND, GPIO35 (Input Only), GPIO22, GPIO21.
- **CN1 (I2C/IO)**: 3V3, GND, GPIO22 (SCL), GPIO27 (SDA/IO).
- **P4 (Speaker)**: Conector 2-pin para falante externo de 8Ω (GPIO26).
- **Conector de Bateria (4P)**: Entrada direta de energia/LiPo. Não possui divisor de tensão nativo ligado ao ADC.

---

## 🔧 Diferenças entre Modelos R e C

| Característica | Modelo R (Resistivo) | Modelo C (Capacitivo) |
| :--- | :--- | :--- |
| **Controlador de Touch** | XPT2046 (SPI) | GT911 (I2C) |
| **Touch CS (GPIO 33)** | Chip Select do Touch | Usado como I2C SDA |
| **Pinos ocupados** | GPIO 33 (CS), 36 (IRQ) | GPIO 32 (SCL), 33 (SDA), 25 (RST), 21 (INT) |
| **Calibração** | Necessária (salva em LittleFS) | Não necessária |
| **Inversão de Cores** | Não | Sim (`-DTFT_INVERSION_ON`) |
| **Ambiente PlatformIO** | `CYD-3248S035R` | `CYD-3248S035C` |
| **Launcher** | `LAUNCHER_CYD-3248S035R` | `LAUNCHER_CYD-3248S035C` |

---

## 🛠️ Compilação no PlatformIO

### Resistivo (XPT2046)

No `platformio.ini`, descomente:

```ini
CYD-3248S035R
```

### Capacitivo (GT911)

No `platformio.ini`, descomente:

```ini
CYD-3248S035C
```

### Compilar via CLI

```bash
# Resistivo
pio run -e CYD-3248S035R

# Capacitivo
pio run -e CYD-3248S035C

# Upload direto
pio run -e CYD-3248S035R -t upload
```

---

## 🛠️ Mods e Melhorias Comuns

1. **Fix TX/RX (Swap R5/R6)**:
   Algumas revisões possuem as trilhas de TX/RX invertidas no conector P1.

2. **Precisão do LDR**:
   O sensor de luz nativo (GPIO 34) é pouco preciso. Trocar o resistor do divisor por 100K-500K melhora a curva de leitura.

3. **Speaker Upgrade**:
   O amplificador 8002A onboard suporta speakers de até 2W. Um falante de 4 ohm 3W (com cuidado no volume) provê alertas sonoros muito melhores.

4. **GT911 INT Pin (Modelo C)**:
   Em alguns boards, GPIO 21 (INT) está conectado ao GND. Se o touch não responder, pode ser necessário cortar a trilha INT-GND e adicionar um pull-up de 10K.

---

## ⚡ Comparativo com CYD-2432S028

| Característica | CYD-2432S028 | CYD-3248S035 |
| :--- | :--- | :--- |
| **Tela** | 2.8" (320x240) | 3.5" (480x320) |
| **Driver LCD** | ILI9341 | ST7796 |
| **Resolução** | 76,800 px | 153,600 px |
| **Backlight** | GPIO 21 (2USB) / 27 | GPIO 27 |
| **Touch** | XPT2046 / CST816D | XPT2046 / GT911 |
| **Speaker** | GPIO 26 | GPIO 26 |
| **RGB LED** | R=4, G=16, B=17 | R=4, G=16, B=17 |
| **LDR** | GPIO 34 | GPIO 34 |
| **Firmware** | `CYD-2432S028` / `CYD-2USB` | `CYD-3248S035R` / `CYD-3248S035C` |
