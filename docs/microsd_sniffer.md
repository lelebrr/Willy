# Sniffer de Cartão MicroSD / Placa de Breakout

**Referência Visual**: [Micro SD Sniffer Compatível com Adaptador de Cartão TF](https://ae01.alicdn.com/kf/S37124ee5ec4b4084a7733237931efaaby.jpg)

O **MicroSD Sniffer** é uma placa de breakout passiva indispensável que permite interceptar, monitorar e analisar o tráfego SPI entre o ESP32 e o cartão microSD. Além disso, no projeto Willy, ele serve como uma porta de expansão externa para conectar outros dispositivos SPI (como CC1101 ou NRF24L01+) sem a necessidade de soldas diretas nos pinos internos do barramento.

## Mapeamento de Pinos (SPI ESP32-S3)

Quando utilizado com a arquitetura ESP32-S3 customizada para o firmware Willy, os pinos expostos na placa correspondem ao barramento SPI principal compartilhado:

| Pino do Sniffer | Função SPI | Pino ESP32-S3 (GPIO) | Descrição |
| :--- | :--- | :--- | :--- |
| **CLK** | SCK | **12** | Relógio do SPI (Clock) |
| **DO** | MISO | **13** | Master In Slave Out (Dados do cartão para o ESP) |
| **DI** | MOSI | **11** | Master Out Slave In (Dados do ESP para o cartão) |
| **CS** | CS | **38** | Chip Select (Seleciona o cartão SD) |
| **VCC** | VCC | **3.3V** | Alimentação Lógica |
| **GND** | GND | **GND** | Terra Comum |
| **CD** | - | - | Card Detect (Detecção de Cartão - Opcional) |
| **RSV** | - | - | Reservado |

## Notas de Uso e Aplicações

1. **Compartilhamento do Barramento**: O barramento SPI (GPIOs 11, 12 e 13) é compartilhado simultaneamente entre a Tela TFT, o Controlador de Touch, e os módulos NRF24/CC1101. O Sniffer fornece acesso físico imediato a este barramento sem abrir o dispositivo.
2. **Chip Select Adicional (CS)**: Para utilizar um módulo externo (ex: CC1101) conectado via Sniffer simultaneamente com o cartão SD, você deve gerenciar os pinos de **CS** (Chip Select) individualmente no firmware para evitar colisões de dados.
3. **Análise de Protocolo**: Esta ferramenta é ideal para conectar analisadores lógicos (como Saleae ou PulseView) e monitorar a integridade da comunicação e do sistema de arquivos FAT32 em tempo real.
