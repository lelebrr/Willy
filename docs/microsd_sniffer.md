# SparkFun MicroSD Sniffer

**Referência**: [SparkFun MicroSD Sniffer](https://opencircuit.pt/product/sparkfun-microsd-sniffer)

O MicroSD Sniffer é uma placa de breakout que permite interceptar e analisar o tráfego SPI entre o ESP32 e o cartão microSD, ou utilizar o slot SD como uma porta de expansão para conectar outros dispositivos SPI (como CC1101, NRF24, etc) sem soldas.

## Pinagem (SPI)

Quando inserido no slot MicroSD da placa CYD-2432S028R, os pinos expostos correspondem ao barramento SPI VSPI do ESP32:

| Pino Sniffer | Função SPI | Pino CYD (GPIO) | Descrição |
| :--- | :--- | :--- | :--- |
| **CLK** | SCK | **18** | Clock do SPI |
| **DO** | MISO | **19** | Master In Slave Out (Dados do cartão p/ ESP) |
| **DI** | MOSI | **23** | Master Out Slave In (Dados do ESP p/ cartão) |
| **CS** | CS | **5** | Chip Select (Seleciona o cartão SD) |
| **VCC** | VCC | **3.3V** | Alimentação |
| **GND** | GND | **GND** | Terra |
| **CD** | - | - | Card Detect (não conectado padrão na CYD) |
| **RSV** | - | - | Reservado |

## Notas de Uso

1.  **Conflitos SPI**: O barramento SPI (18/19/23) é compartilhado. Se conectar outro dispositivo Slave nestes pinos, você deve garantir que o pino **CS** (Chip Select) do slot SD (GPIO 5) esteja HIGH (desativado) enquanto fala com o outro dispositivo, e vice-versa.
2.  **CS Adicional**: Para usar um módulo externo (ex: CC1101) simultaneamente com o SD, você precisará de um pino GPIO livre extra para usar como CS do novo módulo.
3.  **Análise Lógica**: Você pode conectar um analisador lógico nestes pinos para debugar a comunicação com o cartão SD.
