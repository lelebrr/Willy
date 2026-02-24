# CC1101 e NRF24L01 — Módulos de Rádio SPI

**Referências**:
- [CC1101 Guide](https://zaitronics.com.au/blogs/guides/cc1101-guide-instruction-manual)
- [NRF24L01 Guide](https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/)

Ambos os módulos compartilham a **mesma fiação e barramento SPI**, além de compartilharem pinos com o **NFC PN532**. Cada módulo tem **2 interruptores dedicados** para ligar/desligar individualmente.

> [!CAUTION]
> Ambos os módulos operam a **3.3V apenas**. Nunca conectar a 5V!
> Apenas **um módulo de rádio pode estar ativo por vez** (CC1101 OU NRF24), pois compartilham CSN e barramento SPI.

---

## Pinagem dos Módulos

| Pino | CC1101 | NRF24L01 | Cor do Fio | Destino na CYD | Interruptor? |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | GND | GND | Preto | GND do NFC (CN1) | Não |
| **2** | VCC (3.3V) | VCC (3.3V) | Vermelho | VCC do NFC (3.3V CN1) | **Sim** (1 por módulo) |
| **3** | GDO0 | CE | Amarelo | SDA do NFC (**GPIO 27**) | **Sim** (1 por módulo) |
| **4** | CSN | CSN | Azul | SCL do NFC (**GPIO 22**) | Não |
| **5** | SCK | SCK | Laranja | Sniffer CLK (**GPIO 18**) | Não |
| **6** | MOSI | MOSI | Amarelo | Sniffer CMD (**GPIO 23**) | Não |
| **7** | MISO | MISO | Roxo | Sniffer DAT0/DO (**GPIO 19**) | Não |
| **8** | GDO2 | IRQ | — | Não conectado | — |

---

## Diagrama de Interruptores

Cada módulo possui **2 interruptores**: um no **VCC** (liga/desliga energia) e um no **pino 3** (habilita sinal GDO0/CE).

```
3.3V (CN1) ─── NFC Switch ─── VCC NFC (PN532)
            ├── CC1101 Switch 1 ─── VCC CC1101 (Pino 2)
            └── NRF24 Switch 1 ──── VCC NRF24 (Pino 2)

GPIO 27 ─────── NFC (SDA - direto, sem interruptor)
            ├── CC1101 Switch 2 ─── GDO0 CC1101 (Pino 3)
            └── NRF24 Switch 2 ──── CE NRF24 (Pino 3)
```

**Total de interruptores dos rádios**: 4 (2 por módulo)

---

## Conexão via MicroSD Sniffer

Os pinos SPI (5, 6, 7) são acessados através do **SparkFun MicroSD Sniffer** instalado no slot SD:

| Pino Módulo | Pino Sniffer | GPIO |
| :--- | :--- | :--- |
| Pino 5 (SCK) | CLK | 18 |
| Pino 6 (MOSI) | CMD/DI | 23 |
| Pino 7 (MISO) | DAT0/DO | 19 |

---

## Configuração no Willy

- **Core → Pins → CC1101**: GDO0=27, CSN=22
- **Core → Pins → NRF24**: CE=27, CSN=22
- **Documentação de Ataques**: Veja o guia de [Ataques NRF24](nrf24_advanced_attacks.md) para detalhes de funcionamento.

> [!IMPORTANT]
> Antes de usar um módulo de rádio:
> 1. **Desligue** o interruptor do NFC (compartilham GPIO 22/27)
> 2. **Ligue** apenas os 2 interruptores do módulo desejado (CC1101 **ou** NRF24)
> 3. Nunca ligue CC1101 e NRF24 ao mesmo tempo

---

## Resumo de Pinos Compartilhados

| GPIO | Usado por | Função |
| :--- | :--- | :--- |
| **22** | NFC SCL / CC1101 CSN / NRF24 CSN | I2C Clock ou SPI Chip Select |
| **27** | NFC SDA / CC1101 GDO0 / NRF24 CE | I2C Data ou Controle de rádio |
| **18** | SD SCK / CC1101 SCK / NRF24 SCK | SPI Clock (via Sniffer) |
| **23** | SD MOSI / CC1101 MOSI / NRF24 MOSI | SPI Data In (via Sniffer) |
| **19** | SD MISO / CC1101 MISO / NRF24 MISO | SPI Data Out (via Sniffer) |
