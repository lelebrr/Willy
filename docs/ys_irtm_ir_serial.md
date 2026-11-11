# Módulo IR Serial YS-IRTM — Codificador e Decodificador

O **YS-IRTM v3.08** é um módulo de comunicação infravermelha de alto desempenho via Serial (UART). Ao contrário de LEDs IR comuns que exigem controle preciso de tempo (bit-banging), este módulo possui um MCU integrado que gerencia a codificação e decodificação dos sinais, simplificando radicalmente a integração e liberando o processamento do ESP32 para outras tarefas de pentest.

## Características Principais

- **Tensão de Operação**: 5V (Obrigatório para o correto funcionamento do decodificador).
- **Interface de Comunicação**: Serial TTL (9600 baud por padrão).
- **Funcionalidade**: Capacidade de enviar e receber comandos infravermelhos complexos via pacotes hexadecimais ou comandos AT.

---

## Mapeamento de Pinos - Willy (ESP32-S3)

Para o projeto Willy no ESP32-S3, utilizamos pinos UART dedicados para evitar qualquer conflito com os módulos SPI (TFT/SD) ou I2C (Sensores).

| Pino YS-IRTM | Função | ESP32-S3 (GPIO) | Cor do Fio (Sugerida) |
| :--- | :--- | :--- | :--- |
| **GND** | Terra (Ground) | **GND** | Preto |
| **VCC** | Alimentação 5V | **5V / VIN** | Vermelho |
| **RXD** | Recebe Dados | **1 (TX)** | Azul |
| **TXD** | Envia Dados | **47 (RX)** | Amarelo |

> [!IMPORTANT]
> **Conexão Cruzada (Serial RX/TX)**: Lembre-se que o pino **RXD** do módulo IR deve ser conectado ao **TX** da placa ESP32 (GPIO 1), e o pino **TXD** do módulo ao **RX** da placa ESP32 (GPIO 47).

---

## Protocolo de Comunicação

O módulo utiliza um protocolo baseado em pacotes. Para transmitir um código infravermelho, o firmware envia uma sequência específica de bytes via serial.

- **Configuração da Porta Serial**: 9600 bps, 8 bits de dados, Sem paridade, 1 Bit de parada (Stop bit).
- **Formato de Envio Típico**: `0xFA` + `Endereço` + `Comando` + `...`

> [!NOTE]
> O firmware **Willy** no ESP32-S3 está configurado para inicializar a `HardwareSerial(1)` especificamente nos pinos 1 e 47 para este módulo. Isto garante que o sistema possa farejar (sniffing) ou disparar sinais IR em background sem interromper o processamento da interface gráfica ou capturas de rede.

---

## Referências Técnicas

- [GitHub: Biblioteca Micropython YS-IRTM](https://github.com/mcauser/micropython-ys-irtm)
- [Datasheet do Receptor IR M1838](https://aitendo3.sakura.ne.jp/aitendo_data/product_img/sensor/infrared/M1838-NEC-4P/M1838-NEC-4P_aitendo.pdf)
