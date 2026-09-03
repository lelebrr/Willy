# 2x NRF24L01+PA+LNA 2.4GHz Transceiver (Configuração Dupla)

A arquitetura do Willy foi projetada para utilizar **dois módulos NRF24L01+ PA+LNA** simultaneamente. Esse design exclusivo permite operações full-duplex, escaneamento simultâneo e aumento drástico das capacidades ofensivas/defensivas em 2.4GHz.

**Referências:**

- [AliExpress: NRF24L01+PA+LNA](https://pt.aliexpress.com/item/1005007038556192.html)
- [Cytron: NRF24L01+PA+LNA](https://my.my.cytron.io/p-wireless-nrf24l01-plus-pa-lna-with-antenna)
- [Guia NRF24L01](https://blog.eletrogate.com/guia-definitivo-do-modulo-wireless-nrf24l01-2/)

## Especificações

- **Banda de Frequência**: 2.400 - 2.525 GHz (Banda ISM)
- **Alcance**: Até 1100 metros (com a antena externa em cada módulo)
- **Interface**: SPI (4-fios), compartilhada entre os dois módulos
- **Tensão Lógica**: 3.3V (Entradas tolerantes a 5V, mas a alimentação DEVE ser 3.3V)

## Mapeamento de Pinos - ESP32-S3

A interface SPI (MOSI, MISO, SCK) é compartilhada, requerendo apenas pinos separados para `CSN` e `CE` de cada módulo.

| Pino NRF (Módulo 1) | Pino NRF (Módulo 2) | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- | :--- |
| `VCC` | `VCC` | `3.3V` | Alimentação. Extremamente sensível a ruído (Exige capacitor 10uF - 100uF). |
| `GND` | `GND` | `GND` | Terra comum |
| `MOSI` | `MOSI` | `11` | SPI Master Out (Compartilhado) |
| `MISO` | `MISO` | `13` | SPI Master In (Compartilhado) |
| `SCK` | `SCK` | `12` | SPI Clock (Compartilhado) |
| `CSN` | - | `16` | SPI Chip Select (Módulo 1) |
| `CE`  | - | `21` | Chip Enable (Rx/Tx Módulo 1) |
| - | `CSN` | `35` | SPI Chip Select (Módulo 2) |
| - | `CE`  | `36` | Chip Enable (Rx/Tx Módulo 2) |

## Considerações Críticas de Energia

- **Consumo do PA+LNA (Duplo)**: O uso de **dois** módulos de longo alcance (PA+LNA) aumenta o pico de consumo absurdamente (podendo chegar a >230mA simultaneamente ao transmitir).
- **Regulador Dedicado (LDO)**: O regulador interno de 3.3V do ESP32 pode ser insuficiente para suportar a tela TFT (Luz de fundo), Cartão SD e DOIS NRF24L01+ em modo de transmissão. É **altamente recomendável** utilizar um LDO dedicado (ex: AMS1117-3.3) apenas para a linha de rádio.
- **Antenas**: **Nunca** energize nenhum dos módulos sem as antenas (SMA) rigorosamente conectadas. A potência refletida destruirá os estágios amplificadores (PA) imediatamente.
- **Isolamento**: Utilize capacitores cerâmicos e eletrolíticos nos pinos VCC e GND de **ambos** os módulos, soldado o mais próximo possível dos pinos, para evitar interrupções de hardware (brownout) durante surtos de corrente transientes.
