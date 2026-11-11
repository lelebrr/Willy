# Integração: Módulo NFC PN532 V3 — ESP32-S3

Este documento detalha como configurar e conectar o módulo **PN532 NFC V3** ao ESP32-S3-WROOM-1-N8R2 rodando o firmware Willy.

---

## 📋 Especificações

| Parâmetro | Valor |
|-----------|-------|
| Chip | NXP PN532 |
| Frequência | 13.56 MHz |
| Protocolos | ISO/IEC 14443A/B, FeliCa, ISO 18092 (P2P), NDEF |
| Interface | **I2C** (recomendado), HSU (UART), SPI |
| Endereço I2C | `0x24` |
| Tensão | 3.3V – 5V (possui regulador LDO onboard) |
| Corrente | ~150mA (pico durante transmissão) |

---

## ⚙️ Configuração dos DIP Switches

O módulo PN532 V3 possui **duas chaves DIP** (mini-switches brancos) que selecionam o modo de comunicação.

**Para o Willy ESP32-S3, use o modo I2C:**

| Chave 1 | Chave 2 | Modo | Status |
|:-------:|:-------:|:----:|:------:|
| **ON** (↑ Cima) | **OFF** (↓ Baixo) | **I2C** | ✅ **USE ESTE** |
| OFF | OFF | HSU (Serial) | ⚠️ Alternativo |
| OFF | ON | SPI | ❌ Não recomendado |

> [!IMPORTANT]
> As chaves DIP **devem** ser configuradas **ANTES** de energizar o módulo. Mudar as chaves com o módulo ligado não surte efeito.

---

## 🔌 Diagrama de Conexão (I2C)

O PN532 é conectado ao **barramento I2C compartilhado** do ESP32-S3, junto com o DS3231 RTC e o PAJ7620 Gesture Sensor.

```
   ESP32-S3                    PN532 V3
   ─────────                   ─────────
   3.3V           ──→  VCC
   GND            ──→  GND
   GPIO 8  (SDA)  ──→  SDA
   GPIO 17 (SCL)  ──→  SCL
```

### Tabela de Fiação

| Pino do PN532 | Pino ESP32-S3 | Cor do Fio Sugerida | Descrição |
|:-------------:|:-------------:|:-------------------:|-----------|
| VCC | `3.3V` | 🔴 Vermelho | Alimentação |
| GND | `GND` | ⚫ Preto | Terra |
| SDA | `GPIO 8` | 🟢 Verde | I2C Data |
| SCL | `GPIO 17` | 🟡 Amarelo | I2C Clock |

> [!NOTE]
> O PN532 convive **sem conflito** com o DS3231 (endereço `0x68`) e o PAJ7620 (endereço `0x73`) no mesmo barramento I2C, pois seu endereço é `0x24`.

---

## 💻 Código de Inicialização

```cpp
#include <Wire.h>
#include <Adafruit_PN532.h>

// Modo I2C — sem pinos de reset
Adafruit_PN532 nfc(-1, -1, &Wire);

void setup() {
    Serial.begin(115200);
    Wire.begin(8, 17);  // SDA=8, SCL=17

    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("PN532 não encontrado!");
        while (1);
    }

    Serial.print("Firmware: ");
    Serial.println((versiondata >> 16) & 0xFF);

    nfc.SAMConfig();
    Serial.println("PN532 pronto! Aproxime um cartão...");
}

void loop() {
    uint8_t uid[7];
    uint8_t uidLength;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        Serial.print("UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}
```

---

## 🛠️ Teste de Funcionamento

1. No menu principal do Willy, vá em **NFC** → **Scan**.
2. Aproxime um cartão ou tag NFC (13.56MHz) do módulo.
3. Se o **UID do cartão** aparecer na tela, está funcionando!

### Se mostrar "PN532 Not Found"

- [ ] Verifique os DIP switches: Chave 1 = **ON**, Chave 2 = **OFF**
- [ ] Execute um scan I2C no Serial Monitor — deve mostrar endereço `0x24`
- [ ] Verifique se SDA está no GPIO **8** e SCL no GPIO **17**
- [ ] Se tiver 3+ módulos I2C com pull-ups, remova os pull-ups extras

---

## 📚 Referências

- [Adafruit PN532 Library](https://github.com/adafruit/Adafruit-PN532)
- [NXP PN532 Datasheet](https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf)
- [Ataques NFC Avançados](nfc_advanced_attacks.md)
- [Bíblia do Hardware (Guia Mestre)](willy_hardware_bible.md)
