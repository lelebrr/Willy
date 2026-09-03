# Placa de Expansão de Armazenamento Micro SD (SPI)

**Referências:**

- [InstantPanel: Micro TF Card Memory Shield](https://instantpanel.co.nz/product/micro-tf-card-memory-shield-module-spi-micro-storage-card-adapter-for-arduino.html)
- [RandomNerdTutorials: Cartão MicroSD no ESP32](https://randomnerdtutorials.com/esp32-microsd-card-arduino/)
- [MicrocontrollersLab: MicroSD com ESP32](https://microcontrollerslab.com/microsd-card-esp32-arduino-ide/)
- [LastMinuteEngineers: Diagrama de Pinos](https://lastminuteengineers.com/wp-content/uploads/arduino/Micro-SD-TF-Card-Module-Pinout-SPI.png)

## Especificações

- **Formato**: Cartão Micro SD / Cartão TF
- **Interface**: SPI
- **Tensão Lógica**: 3.3V (O pino VCC pode ser alimentado com 5V/VIN se o módulo tiver regulador LDO embutido, caso contrário alimente direto com 3.3V)

## Mapeamento de Pinos - Willy ESP32-S3

A interface SPI é compartilhada com a Tela TFT, Controlador de Touch, Módulos NRF24L01+ e Módulo CC1101.

| Pino do Módulo SD | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VCC` | `5V / VIN` | Fonte de Alimentação Principal (Verifique se seu módulo tem LDO interno) |
| `GND` | `GND` | Terra comum |
| `CS`  | `38` | SPI CS (Chip Select para o Cartão SD) |
| `MOSI` | `11` | SPI MOSI (Entrada de Dados) |
| `MISO` | `13` | SPI MISO (Saída de Dados) |
| `SCK` | `12` | SPI Clock (Relógio Geração) |

## Notas de Configuração Críticas

- **Pino CS**: No código do sistema operacional Willy, o cartão SD é inicializado utilizando o pino pré-definido via chamada `SD.begin(38)`.
- **Anomalia de MISO Paralelo**: Muitos módulos Micro SD genéricos ou baratos de Arduino POSSUEM UMA FALHA DE DESIGN onde eles **não liberam o pino MISO** no barramento SPI quando não estão sendo lidos (O sinal não entra em Hi-Z, alta impedância).
  - **O Problema**: Se o módulo MISO não ficar em Hi-Z, os leitores de Tela, CC1101 ou NRF vão travar permanentemente e falharão ao serem inicializados junto ao SD.
  - **A Solução**: Utilize módulos Micro SD de qualidade que apresentem o chip Buffer Logic `74LVC125A` ou similar, que corte a corrente adequadamente. Como alternativa, você pode soldar seu próprio resistor na linha do módulo.
