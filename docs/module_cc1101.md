# Módulo Wireless CC1101 com Antena SMA (433MHz)

**Referências:**

- [Ai-ChipLink: Blog & Datasheet do CC1101](https://aichiplink.com/blog/CC1101-Datasheet-Pinout-Circuit-Equivalent-and-Applications_447)
- [Ai-ChipLink: Imagem do Módulo](https://file.aichiplink.com/static/2025-07-29/102bc3cf-8308-4608-93ad-82850420cc38.webp)
- [GitHub: mfurga/cc1101](https://github.com/mfurga/cc1101)
- [GitHub: SmartRC-CC1101-Driver-Lib](https://github.com/LSatan/SmartRC-CC1101-Driver-Lib)

## Especificações

- **Bandas de Frequência**: 300-348 MHz, 387-464 MHz (Otimizado para 433MHz)
- **Modulação**: 2-FSK, GFSK, MSK, OOK, ASK
- **Interface**: SPI (4-fios)
- **Tensão Lógica**: 3.3V (Obrigatório, não é tolerante a 5V de forma confiável na alimentação)

## Mapeamento de Pinos - Willy ESP32-S3

| Pino CC1101 | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VCC` | `3.3V` | Tensão de alimentação. |
| `GND` | `GND` | Terra comum |
| `CSN` | `18` | SPI CS (Chip Select) |
| `SCK` | `12` | SPI Clock |
| `MOSI` | `11` | SPI MOSI |
| `MISO` | `13` | SPI MISO / GDO1 |
| `GDO0` | `33` | Captura assíncrona / Interrupções |
| `GDO2` | `34` | Saída Digital / Status do sinal |

## Uso e Desempenho

- **Antena SMA**: A antena externa SMA é fundamental para o alcance e confiabilidade em ataques sub-GHz e captura de RF.
- **Bibliotecas**: O firmware Willy utiliza a biblioteca `SmartRC-CC1101` para configuração avançada de registradores e a `mfurga` para rotinas de captura de sinais brutos (raw).
- **Conexões (Wiring)**: Certifique-se de que o pino `GDO0` esteja conectado ao `GPIO 33` para permitir interrupções rápidas no processamento de sinais OOK.
