# Documentação de Hardware: CYD-2432S028R (Cheap Yellow Display)

## Visão Geral

A **ESP32-2432S028R**, popularmente conhecida como **Cheap Yellow Display (CYD)**, é uma placa de desenvolvimento baseada no ESP32-WROOM-32 que integra um display TFT de 2.8" com touchscreen resistivo, slot para cartão SD, LED RGB e sensores básicos.

Esta documentação foca na **Variante "2USB"** (também conhecida como "Inverted Colors"), que possui características específicas de pinagem e display.

---

## ⚙️ Especificações Técnicas

| Componente | Detalhes |
| :--- | :--- |
| **MCU** | ESP32-WROOM-32 (Dual Core, 240MHz, Wi-Fi/BT) |
| **Flash** | 4MB |
| **Display** | 2.8" TFT SPI (320x240 px) |
| **Driver de Vídeo** | ILI9341 ou ST7789V (Configuração `TFT_INVERSION_ON` necessária) |
| **Touchscreen** | Resistivo (XPT2046) |
| **Armazenamento** | Slot MicroSD (SPI) |
| **USB** | 2x Portas MicroUSB (1x UART/Power, 1x USB-OTG/Power) |
| **Extras** | LED RGB, Sensor de Luz (LDR), Botões de Boot/Reset |

---

## 📍 Mapeamento de Pinos (Pinout)

### 1. Display TFT (SPI)

O display utiliza o barramento SPI (VSPI) do ESP32.

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **TFT_MISO** | 12 | Dados do Display (Out) |
| **TFT_MOSI** | 13 | Dados do Display (In) |
| **TFT_SCK** | 14 | Clock do Display |
| **TFT_CS** | 15 | Chip Select |
| **TFT_DC** | 2 | Data/Command |
| **TFT_RST** | -1 | Reset via software (Ligado ao RST global) |
| **TFT_BL** | **21** | Backlight (**GPIO 21 na versão 2USB**, GPIO 27 na standard) |

### 2. Touchscreen (XPT2046)

Compartilha o barramento SPI com o display.

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **TOUCH_CS** | 33 | Chip Select do Touch |
| **TOUCH_IRQ** | 36 | Interrupção do Touch |
| **CLK** | 25 | Clock dedicado (em algumas revisões) |
| **MOSI** | 32 | MOSI dedicado |
| **MISO** | 39 | MISO dedicado |

### 3. Cartão SD

Utiliza o barramento SPI (HSPI).

| Função | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **SD_CS** | 5 | Chip Select |
| **SD_SCK** | 18 | Clock |
| **SD_MISO** | 19 | MISO |
| **SD_MOSI** | 23 | MOSI |

### 4. Periféricos Integrados e Botões

| Componente | Pino (GPIO) | Notas |
| :--- | :--- | :--- |
| **LED RGB - Vermelho** | 4 | Ativo Baixo (LOW = Ligado) |
| **LED RGB - Verde** | 16 | Ativo Baixo |
| **LED RGB - Azul** | 17 | Ativo Baixo |
| **Sensor de Luz (LDR)** | 34 | Entrada ADC (GPIO34) |
| **Speaker (Áudio)** | 26 | Saída PWM/DAC para amplificador 8002A |
| **Botão BOOT** | 0 | Usado para modo Flash |
| **Botão RESET** | - | Reset de Hardware |

---

## 🔌 Conectores e Portas

A placa possui conectores Micro JST 1.25mm para expansão:

- **P1 (UART)**: 5V, GND, TX (GPIO1), RX (GPIO3). Ideal para GPS ou Serial Externo.
- **P3 (Extended IO)**: GND, GPIO35 (Input Only), GPIO22, GPIO21.
- **CN1 (I2C/IO)**: 3V3, GND, GPIO22 (SCL), GPIO27 (SDA/IO).
- **Speaker Port**: Conector 2-pin para falante externo de 8Ω (GPIO26).

---

## 🛠️ Mods e Melhorias Comuns

1. **Fix TX/RX (Swap R5/R6)**:
    Algumas revisões possuem as trilhas de TX/RX invertidas no conector P1. Se o seu módulo serial não funcionar, verifique se é necessário trocar os resistores R5 e R6 de lugar.
2. **Precisão do LDR**:
    O sensor de luz nativo é pouco preciso. Trocar o resistor R15 por um de 100K-500K melhora significativamente a curva de leitura.
3. **Liberar GPIO 21**:
    Se precisar de mais um pino de I/O, você pode cortar a trilha do Backlight e ligá-la diretamente ao 3.3V (sempre on), liberando o GPIO 21 no conector P3.
4. **Speaker Upgrade**:
    O amplificador onboard suporta speakers de até 2W. Instalar um falante de 4 ohm 3W (com cuidado no volume) provê alertas sonoros muito melhores para o Willy.

---

## ⚡ Variante "2USB" (CYD2USB)

Esta versão (usada no Willy) possui:

- **USB-C Principal**: Para programação e alimentação.
- **Micro USB Secundário**: Para alimentação extra ou serial secundário.
- **Inversão de Cores**: Obrigatório usar `-DTFT_INVERSION_ON` no PlatformIO.
- **Configuração no Willy**: Selecione o ambiente `[env:CYD-2USB]` para aplicar todas as correções automaticamente.
