# Módulos I2C: Sensor de Gesto (PAJ7620) e RTC (DS3231)

## 1. Sensor de Gesto PAJ7620

O sensor **PAJ7620** (integrado no módulo GY-PAJ7620U2) é incrivelmente versátil, capaz de reconhecer até 15 gestos complexos e rápidos através de sua interface I2C com alta precisão para a interface Cyber Menu e ativação de módulos do Willy.

**Referências:**

- [GitHub: Biblioteca RevEng PAJ7620](https://github.com/acrandal/RevEng_PAJ7620)
- [CircuitDigest: Introdução ao PAJ7620](https://circuitdigest.com/microcontroller-projects/introduction-to-paj7620-gesture-recognition-module)
- [ESP32Learning: Usando PAJ7620 com ESP32](https://www.esp32learning.com/code/paj7620-gesture-sensor-and-esp32.php)

**Características:**

- **Interface**: I2C (Endereço padrão: `0x73`)
- **Gestos Reconhecidos**: Cima, Baixo, Esquerda, Direita, Frente, Traz, Horário, Anti-Horário, Aceno (Wave), etc.
- **Velocidade**: Processamento interno dedicado de alta velocidade para resposta imediata.

## 2. Relógio de Tempo Real (RTC) DS3231 IIC de Alta Precisão

O **DS3231** fornece um relógio de tempo real (RTC) IIC de alta precisão com cristal (TCXO) compensado por temperatura. Essencial para logs forenses, wardriving e timestamps precisos durante a interceptação de RF e WiFi.

**Referências:**

- [Tutorial RandomNerdTutorials: ESP32 com DS3231 RTC](https://randomnerdtutorials.com/esp32-ds3231-real-time-clock-arduino/)
- [Referência Visual (Módulo RTC)](https://esp32io.com/images/cover/esp32-ds3231-rtc-module.jpg)

**Características:**

- **Precisão**: Cristal interno compensado por variação térmica.
- **Backup**: Mantém a contagem de data/hora precisa sem energia via bateria dedicada de backup CR2032/LIR2032.
- **Recursos**: Alarmes configuráveis e saída de onda quadrada (interrupções temporais).

## Mapeamento de Pinos - Willy ESP32-S3 (Barramento Compartilhado)

Ambos os módulos devem ser ligados em paralelo no mesmo barramento I2C, conforme abaixo:

| Pinos do Módulo | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VCC` / `VIN` | `3.3V` | Utilize STRICTAMENTE `3.3V` do ESP32 para evitar destruição de lógica I2C e danos ao S3. |
| `GND` | `GND` | Terra Comum. |
| `SDA` | `8` | I2C Data (Dados I2C). |
| `SCL` | `17` | I2C Clock (Relógio I2C). |

> [!NOTE]
> Como ambos os módulos compartilham fisicamente o mesmo barramento I2C, não haverá conflito pois seus endereços I2C de hardware são diferentes na raiz da placa (PAJ7620: `0x73`, DS3231: `0x68`). O firmware do Willy inicializa dinamicamente ambos usando a configuração padronizada via `Wire.begin(8, 17)`.
