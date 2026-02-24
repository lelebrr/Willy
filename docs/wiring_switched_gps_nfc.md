# Guia Completo de Hardware: Willy (CYD-2432S028R)

Este documento detalha a instalação de todos os módulos suportados no projeto, incluindo pinagem, cores dos fios e diagrama de alimentação.

## Índice
1. [GPS NEO-6M](#1-gps-neo-6m)
2. [NFC PN532](#2-nfc-pn532-i2c)
3. [Módulos de Rádio (CC1101 / NRF24L01)](#3-módulos-de-rádio-cc1101--nrf24l01)
4. [MicroSD Sniffer](#4-expansão-microsd-sniffer-spi)
5. [Módulo IR Serial (YS-IRTM)](#5-módulo-ir-serial-ys-irtm)
6. [Interruptores de Alimentação](#6-sistema-de-interruptores)
7. [Resumo Geral de Pinos](#7-resumo-geral)

---

## 1. GPS NEO-6M

Conectado ao **Port 1 (P1)** da placa CYD (Serial).

| Pino GPS | Função | Pino CYD (P1) | Cor Fio | Interruptor? |
| :--- | :--- | :--- | :--- | :--- |
| **VCC** | 5V/3.3V | **VIN** | Vermelho | **Sim** (Chave GPS) |
| **RX** | Dados In | **TX (GPIO 1)** | Amarelo | Não |
| **TX** | Dados Out | **RX (GPIO 3)** | Azul | Não |
| **GND** | Terra | **GND** | Preto | Não |

> **Configuração Willy:** `Core > Pins > GPS`: TX=1, RX=3. Baudrate: 9600.

---

## 2. NFC PN532 (I2C)

Conectado aos pinos I2C nos conectores **P3** e **CN1**.
*Modo DIP Switch:* I2C (1=ON, 2=OFF).

| Pino PN532 | Função | Pino CYD | Cor Fio | Interruptor? |
| :--- | :--- | :--- | :--- | :--- |
| **VCC** | 3.3V | **CN1 3.3V** | Vermelho | **Sim** (Chave NFC) |
| **GND** | Terra | **CN1 GND** | Preto | Não |
| **SDA** | Dados | **IO 27** | Azul | Não |
| **SCL** | Clock | **P3 IO 22** | Amarelo | Não |

> **Configuração Willy:** `Core > Pins > I2C`: SDA=27, SCL=22.

---

## 3. Módulos de Rádio (CC1101 / NRF24L01)

Compartilham a fiação com o **NFC** (SDA/SCL) e **Sniffer** (SPI).

> [!WARNING]
> Nunca ligue o CC1101 e o NRF24 ao mesmo tempo. Apenas um interruptor de rádio deve estar ligado.
> Ao usar rádio, desligue o NFC para evitar interferência no barramento I2C/GPIO.

**Pinagem Compartilhada:**

| Pino Rádio (1-8) | Função | Conectado em | Cor Fio |
| :--- | :--- | :--- | :--- |
| **1 (GND)** | Terra | **NFC GND** | Preto |
| **2 (VCC)** | 3.3V | **NFC VCC** | Vermelho (c/ Switch) |
| **3 (GDO0/CE)** | Seleção | **NFC SDA (IO 27)** | Amarelo (c/ Switch) |
| **4 (CSN)** | Chip Sel | **NFC SCL (IO 22)** | Azul |
| **5 (SCK)** | Clock | **Sniffer CLK** | Laranja |
| **6 (MOSI)** | Dados In | **Sniffer CMD** | Amarelo |
| **7 (MISO)** | Dados Out| **Sniffer DAT0** | Roxo |

📄 **Detalhes Completos:** Veja [cc1101_nrf24_wiring.md](cc1101_nrf24_wiring.md)

---

## 4. Expansão: MicroSD Sniffer (SPI)

Adaptador SparkFun instalado no slot SD para expor o barramento VSPI.

| Pino Sniffer | Função SPI | Pino CYD (GPIO) | Usado por |
| :--- | :--- | :--- | :--- |
| **CLK** (Pino 5) | SCK | **18** | Rádios (Pino 5) |
| **CMD** (Pino 3) | MOSI | **23** | Rádios (Pino 6) |
| **DAT0** (Pino 7)| MISO | **19** | Rádios (Pino 7) |
| **CD/DAT3** | CS (SD) | **5** | Cartão SD |

📄 **Detalhes:** Veja [microsd_sniffer.md](microsd_sniffer.md)

---

## 5. Sistema de Interruptores

O sistema possui múltiplos interruptores para gerenciar a energia e evitar conflitos de pinos compartilhados:

1.  **Chave GPS** (no fio VIN do P1) → Liga/Desliga GPS.
2.  **Chave NFC** (no fio 3.3V do CN1) → Liga/Desliga PN532.
3.  **Chaves Rádio** (no fio 3.3V compartilhado):
    *   **Interruptor CC1101** (VCC + GDO0)
    *   **Interruptor NRF24** (VCC + CE)

9. **Chave IR Serial** (no fio VCC do IR) → Liga/Desliga YS-IRTM.

**Regras de Uso:**
*   **GPS + NFC**: Podem ficar ligados juntos.
*   **GPS + Rádio**: Podem ficar ligados juntos.
*   **NFC + Rádio**: ❌ **NÃO**. Desligue NFC antes de ligar Rádio.
*   **CC1101 + NRF24**: ❌ **NÃO**. Use apenas um rádio por vez.
*   **GPS + IR Serial**: ❌ **NÃO**. Compartilham a mesma Serial (P1) — use apenas um por vez via interruptor.

---

## 6. Resumo Geral

| GPIO | Função Principal | Compartilhado com |
| :--- | :--- | :--- |
| **1** | GPS TX / IR RXD | - |
| **3** | GPS RX / IR TXD | - |
| **22** | NFC SCL | Rádio CSN |
| **27** | NFC SDA | Rádio GDO0/CE |
| **18** | SD SCK | Rádio SCK |
| **19** | SD MISO | Rádio MISO |
| **23** | SD MOSI | Rádio MOSI |
| **5** | SD CS | - |

---

## 8. Módulo IR Serial: YS-IRTM

Instalado compartilhando os pinos serial do GPS (Port 1 - GPIO 1/3). Possui interruptor dedicado no VCC (5V).
Para detalhes do protocolo e fiação: **[ys_irtm_ir_serial.md](ys_irtm_ir_serial.md)**
