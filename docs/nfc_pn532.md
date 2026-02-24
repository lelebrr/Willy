# Integração: Módulo NFC PN532 V3

Este documento detalha como configurar e conectar o módulo **PN532 NFC V3 (Elechouse/Genérico)** na placa CYD-2432S028R rodando Bruce.

## ⚠️ Aviso de Conflito de Pinos
Se você configurou o **GPS nos pinos 22 e 27** (conforme o guia anterior), **você não tem pinos livres nos conectores** para ligar o PN532 simultaneamente.

**Soluções:**
1.  **Alternar (Swap):** Desconecte o GPS e conecte o PN532 nos mesmos pinos quando quiser usar NFC.
2.  **Solda (Avançado):** Soldar fios nos pinos do LED RGB (GPIO 4 e 16) para criar uma segunda porta.

> [!NOTE]
> Estes pinos (22 e 27) também são compartilhados com o **NRF24**. Veja [Ataques NRF24](nrf24_advanced_attacks.md) para mais detalhes. Para conhecer as ferramentas de exploração, veja [Ataques NFC Avançados](nfc_advanced_attacks.md).

---

## 1. Configuração dos DIP Switches (Modo de Operação)
O módulo PN532 V3 possui duas chavinhas brancas (DIP Switches) para selecionar o modo de comunicação. O Bruce suporta **I2C** e **HSU (High Speed UART)**.

| Modo | Chave 1 (__1__) | Chave 2 (__2__) | Recomendado? |
| :--- | :--- | :--- | :--- |
| **HSU (Serial)** | **OFF** (Baixo) | **OFF** (Baixo) | ✅ **Sim** (Mais fácil se trocar com GPS) |
| **I2C** | **ON** (Cima) | **OFF** (Baixo) | ✅ **Sim** (Padrão para NFC) |
| **SPI** | **OFF** (Baixo) | **ON** (Cima) | ❌ Não (Requer muitos fios) |

---

## 2. Diagrama de Conexão

### Opção A: Usando Conectores Laterais (Desconectando GPS)
Use esta opção se for alternar entre GPS e NFC.

**Configuração do Módulo:** Chaves em **HSU (Serial)** (0, 0).

| Pino PN532 | Fio | Pino CYD (CN1/P1) | Configuração no Menu Bruce |
| :--- | :--- | :--- | :--- |
| **VCC** | Vermelho | **VCC (5V/3.3V)** | - |
| **GND** | Preto | **GND** | - |
| **TX** | Verde | **GPIO 27** | RFID RX |
| **RX** | Azul | **GPIO 22** | RFID TX |

**Configuração no Menu:**
1.  Vá em **Core** -> **Pins**.
2.  Selecione **RFID Pins** (ou Serial Pins se usar HSU).
3.  Defina **RX = 27** e **TX = 22**.
4.  Vá em **Core** -> **NFC**.
5.  Selecione **Module: PN532 UART** (ou HSU).

---

### Opção B: Soldando no LED RGB (Uso Simultâneo)
Use esta opção para manter GPS e NFC conectados juntos. Requer solda.

**Configuração do Módulo:** Chaves em **I2C** (1, 0).

| Pino PN532 | Pino ESP32 (Solda) | Função |
| :--- | :--- | :--- |
| **VCC** | **5V/3.3V** | Alimentação |
| **GND** | **GND** | Terra |
| **SDA** | **GPIO 4** (LED Vermelho) | Dados I2C |
| **SCL** | **GPIO 16** (LED Verde) | Clock I2C |

**Configuração no Menu:**
1.  Vá em **Core** -> **Pins**.
2.  Selecione **I2C Pins**.
3.  Defina **SDA = 4** e **SCL = 16**.
4.  Vá em **Core** -> **NFC**.
5.  Selecione **Module: PN532 I2C**.

---

## 🛠️ Teste de Funcionamento
1.  No menu principal, vá em **NFC** -> **Scan**.
2.  Aproxime um cartão/tag NFC (13.56MHz).
3.  Se a tela mostrar o ID do cartão (UID), está funcionando!
4.  Se mostrar "PN532 Not Found", verifique:
    *   DIP Switches corretos?
    *   RX/TX invertidos? (Tente trocar).
    *   Pinos configurados corretamente no menu "Pins"?
