# Módulo GPS Série NEO (6M / 7M / 8M)

**Referências:**

- [RandomNerdTutorials: ESP32 NEO-6M GPS](https://randomnerdtutorials.com/esp32-neo-6m-gps-module-arduino/)
- [CircuitDigest: Interfacing NEO-6M with ESP32](https://circuitdigest.com/microcontroller-projects/interfacing-neo6m-gps-module-with-esp32)

## Visão Geral

Este é um módulo GPS versátil da série NEO (comumente v2 ou v3 com conector Micro USB para depuração/alimentação direta), que se comunica via UART. Ele gera sentenças NMEA padrão contendo latitude, longitude, altitude e timestamp.

## Especificações

- **Tensão de Operação**: 3.3V a 5V (A maioria possui regulador LDO interno)
- **Porta USB**: Variante com **Micro USB** permite teste direto no PC ou alimentação externa estável.
- **Nível Lógico**: 3.3V TTL (Seguro para conexão direta nos GPIOs da ESP32-S3)
- **Taxa de Baud Padrão**: 9600 bps
- **Antena**: Antena cerâmica ativa inclusa.

## Mapeamento de Pinagem Willy ESP32-S3

No projeto Willy, o GPS utiliza a UART paralela aos pinos de debug se necessário, ou pinos dedicados mapeados abaixo:

| Pino do Módulo GPS | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VCC` | `5V / VIN` | Recomendado carregar via 5V para melhor estabilidade do sinal. |
| `GND` | `GND` | Terra |
| `TX` | `39` (RX) | Transmitir dados (GPS out -> ESP32 in) |
| `RX` | `40` (TX) | Receber dados (ESP32 out -> GPS in) |

## Configuração

No código do Willy:

```cpp
HardwareSerial gpsSerial(1);
gpsSerial.begin(9600, SERIAL_8N1, 39, 40); // RX=39, TX=40
```

> [!TIP]
> Use a biblioteca `TinyGPS++` para parsear os dados. O primeiro "lock" (Cold Start) pode levar até 5 minutos em ambiente aberto.
