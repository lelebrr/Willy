# ⚠️ DOCUMENTAÇÃO LEGADA - Apenas para CYD-2432S028R

**Este guia é específico para a placa CYD-2432S028R (M5Stack CYD).**

**Para a placa principal ESP32-S3-WROOM-1-N8R2, consulte:**
- [`willy_hardware_bible.md`](./willy_hardware_bible.md)
- [`pinout_master_s3_n8r2.md`](./pinout_master_s3_n8r2.md)

---

# Hardware Externo & Conexões no CYD (Willy Project)

Este documento detalha como conectar módulos externos de hardware (RF, IR, CC1101, etc.) à placa **CYD-2432S028R (Variante 2USB)**, conforme suportado pelo firmware Willy.

## ⚠️ Aviso Importante sobre a Variante "2USB"
Nesta variante, o pino **GPIO 21** é usado para o **Backlight do Display**.
*   **NÃO use o GPIO 21** para módulos externos, pois isso interferirá na tela.
*   Os pinos seguros para uso nos conectores laterais são **GPIO 22**, **GPIO 27** e **GPIO 35** (apenas entrada).

---

## 📡 Módulos Suportados e Conexões

### 1. Transmissor/Receptor RF (433MHz / 315MHz)
Módulos simples de RF (ASK/OOK) de 1 pino de dados.

| Módulo | Pino Recomendado | Conector CYD | Configuração Willy |
| :--- | :--- | :--- | :--- |
| **RF TX (Enviar)** | **GPIO 22** ou **27** | CN1 / P1 | `-DRF_TX_PINS` |
| **RF RX (Receber)** | **GPIO 22**, **27** ou **35** | CN1 / P1 / P3 | `-DRF_RX_PINS` |

> **Nota:** GPIO 35 é somente leitura (Input Only), ideal para o pino de DADOS do receptor RF.

### 2. Infravermelho (IR)
LEDs IR para controle remoto ou receptores IR (ex: VS1838B).

| Componente | Pino Recomendado | Conector CYD | Configuração Willy |
| :--- | :--- | :--- | :--- |
| **LED IR (TX)** | **GPIO 22** ou **27** | CN1 / P1 | `-DIR_TX_PINS` |
| **Receptor IR (RX)** | **GPIO 22**, **27** ou **35** | CN1 / P1 / P3 | `-DIR_RX_PINS` |

### 3. Módulos SPI (CC1101, NRF24L01, W5500)
Estes módulos requerem comunicação SPI (4 pinos) + Alimentação.
**DESAFIO:** Os pinos SPI (SCK, MISO, MOSI) **NÃO** estão disponíveis nos conectores laterais JST desta placa. Eles estão conectados internamente ao slot SD Card e Display.

**Como conectar:**
Você precisará soldar fios diretamente nos pinos do módulo ESP32 ou nos pads do leitor de cartão SD.

| Sinal SPI | Pino ESP32 | Ponto de Solda Alternativo |
| :--- | :--- | :--- |
| **MOSI** | GPIO 23 | Pino do Slot SD |
| **MISO** | GPIO 19 | Pino do Slot SD |
| **SCK** | GPIO 18 | Pino do Slot SD |
| **CSN / SS** | **GPIO 27** | **Disponível no Conector CN1/P1** |
| **GDO0 / IRQ** | **GPIO 22** | **Disponível no Conector CN1/P1** |
| **VCC** | 3.3V | Conector CN1/P1 |
| **GND** | GND | Conector CN1/P1 |
| **Ataques** | - | Veja o guia de [Ataques NRF24](nrf24_advanced_attacks.md) |

> **Configuração Padrão do Willy (`CYD-2USB`):**
> *   `CC1101_SS_PIN`: 27
> *   `CC1101_GDO0_PIN`: 22

### 4. GPS (NMEA Serial) e IR Compartilhado
O firmware usa o **Conector Serial (P1)** para o GPS, compartilhando os mesmos pinos com o módulo IR Serial.

| Sinal | Conexão CYD (P1) | Notas |
| :--- | :--- | :--- |
| **GPS TX** / **IR TXD** | **RXD (GPIO 3)** | ESP32 Recebe Dados |
| **GPS RX** / **IR RXD** | **TXD (GPIO 1)** | ESP32 Envia Dados |

> **Importante**: Use interruptores no VCC (5V) para ligar apenas um módulo por vez.

### 5. Alto-Falante (Speaker)
Conectado ao plug dedicado de 2 pinos na placa.

| Componente | Pino ESP32 | Notas |
| :--- | :--- | :--- |
| **Speaker** | **GPIO 26** | Controlado via PWM/Tone |

---

## 🔌 Pinout dos Conectores JST (Para Referência)

Se você olhar para os conectores laterais da placa (com a tela para cima):

### Conector CN1 (Próximo ao USB/SD)
1.  **GND**
2.  **VCC** (Cuidado com a voltagem, verifique se é 5V ou 3.3V)
3.  **GPIO 27** (I/O, CS para SPI)
4.  **GPIO 22** (I/O, GDO0 para CC1101)

### Conector P3 (Outro lado)
1.  **GPIO 35** (Input Only - Bom para RX RF ou IR)
2.  **GPIO 22** (Repetido/Compartilhado)
3.  **GND**

---

## 🛠️ Resumo de Instalação para Hardware Externo

1.  **Apenas RF/IR simples?** Use os conectores laterais (JST 1.25mm 4-pin). Compre cabos compatíveis ("JST 1.25mm 4 pin connector").
2.  **CC1101/NRF24?** Requer soldagem ou uma placa adaptadora que intercepte os pinos do cartão SD (SPI extender).
3.  **Cuidado com GPIO 21:** Na placa "2USB", ele controla o brilho da tela. Se você conectar algo nele (comum no conector P3 de outras placas), a tela pode desligar ou piscar.

### 📚 Guias Relacionados
*   [Ataques WiFi](wifi_advanced_attacks.md)
*   [Ataques Bluetooth (BLE)](ble_advanced_attacks.md)
*   [Ataques NRF24](nrf24_advanced_attacks.md)
*   [Ataques BadUSB & HID (Nativo)](badusb_hid_attacks.md)
*   [NFC/RFID](nfc_pn532.md)
