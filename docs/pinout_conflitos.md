# Mapa de pinos e conflitos (ESP32-S3 N8R2 + regras gerais)

## N8R2 — mapa resumido (`boards/ESP32-S3-WROOM-1-N8R2/`)

| Função | GPIOs |
|---|---|
| SPI compartilhado | SCK 12, MOSI 11, MISO 13 |
| TFT ILI9341 / Touch XPT2046 | CS 10, DC 9, RST 14, BL 3 / TOUCH_CS 15 |
| SD (SPI) | CS 38 |
| NRF24 #1 / #2 | CS 16, CE 21 / CS2 35, CE2 37 (36 no N8R2, 37 no `.ini`) |
| CC1101 | CS 18, GDO0 33, GDO2 34 |
| LoRa | CS 19, RST 20, DIO0 22 |
| W5500 | CS 23, INT 24, RST 25 |
| GPS (UART) | TX 40, RX 39 @9600 |
| I2S mic | SCK 41, WS 42, SD 2 |
| Joystick | X 4, Y 5, BTN 6 |
| IR | TX 1, RX 47 |
| I2C Grove | SDA 8, SCL 17 |
| LED RGB | 48 |

Auditado: nenhum conflito real — aliases (`SPI_SS_PIN`/`TFT_CS`, `NRF24_SS`/`NRF24_CS`, `CC1101_SS`/`CC1101_CS`, `BACKLIGHT`/`TFT_BL`) são consistentes; barramento SPI compartilhado por projeto (CSs distintos + instâncias `SPI`/`sdcardSPI`/`CC_NRF_SPI`).

## ⚠️ Zonas proibidas no S3 octal (N16R8; N8R2 conforme flash)

- **GPIO 33–37**: flash/PSRAM octal. Padrões `CC1101_GDO0=33`, `GDO2=34`, `NRF24_CS2=35`, `CE2=37` **só valem com módulo externo e barramento não-octal** — em placas octal, remapeie em Config → Modo Dev → Pinos antes de ligar o módulo.
- **GPIO 19/20**: USB nativo (CDC). Padrões `LORA_CS=19`, `LORA_RST=20` conflitam com USB-serial ativo — use LoRa **ou** USB-serial, ou remapeie.
- `SPIPins::checkConflict()` agora cobre também `io0`/`io2` (GDOs).

## Regras gerais (todas as placas)

- Um pino, uma função — exceto barramento SPI/I2C compartilhado (CSs/endereços distintos).
- `sys_i2c` (touch/PMIC/RTC) nunca é desligado; `i2c_bus` (Grove) é reconfigurável.
- CYD-2432S028: Grove SCL 22 compartilhado com CC1101_SS/NRF24_SS/W5500_INT — periféricos mutuamente exclusivos.
- SD em barramento próprio usa `sdcardSPI`; no TFT usa a instância do display; 4 MHz → 1 MHz de fallback.
