# Display LCD TFT SPI de 2.4 polegadas (ILI9341) + Touch

**Referências:**

- [Mischianti: Guia Completo ILI9341 com TFT_eSPI](https://mischianti.org/complete-guide-using-an-ili9341-display-with-the-tft_espi-library/)
- [Mischianti: Diagrama de Fiação (ESP32)](https://mischianti.org/wp-content/uploads/2024/11/TFT-ili9341-esp32-wiring-677x1024.jpg.webp)
- [Fórum Arduino: Discussão sobre Touchscreen no ESP32](https://forum.arduino.cc/t/esp32-touchscreen-tft_espi-ili9341/607951/16)

## Especificações

- **Controlador**: ILI9341 (Display) / XPT2046 (Touch).
- **Resolução**: 320x240 pixels (2.4").
- **Interface**: SPI (4-fios).
- **Profundidade de Cor**: RGB 65k Cores.
- **Tensão Lógica**: 3.3V (VCC alimentado diretamente em 3.3V. Sinais SPI/Lógica também em 3.3V).

## Mapeamento de Pinos - Willy ESP32-S3

| Pino do Display | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VCC` | `3.3V` | Alimentação Principal (3.3V direto, sem regulador LDO). |
| `GND` | `GND` | Terra comum. |
| `CS` | `10` | Seleção de Chip (Display). |
| `RESET / RST` | `14` | Reset de Hardware. |
| `DC / RS` | `9` | Data/Command (Seleção de Registro). |
| `MOSI` | `11` | SPI MOSI. |
| `SCK` | `12` | SPI Clock. |
| `LED / BL` | `3` | Backlight (Controlado via código/PWM no pino 3). |
| `MISO` | `13` | SPI MISO. |

### Pinos do Touch (SPI Compartilhado XPT2046)

| Pino do Touch | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `T_CLK` | `12` | SPI Clock (Compartilhado). |
| `T_CS` | `15` | Seleção de Chip do Touch. |
| `T_DIN` | `11` | SPI MOSI (Compartilhado). |
| `T_DO` | `13` | SPI MISO (Compartilhado). |
| `T_IRQ` | `Não conectado`| **Interrupção do Touch** (Confirmado como -1 no User_Setup.h e platformio.ini). |

### Observações de Hardware (05/03/2026)
- **T_IRQ**: Removido e configurado como `-1`. Não conecte este pino para evitar conflitos de interrupção no ESP32-S3.
- **Modo Headless**: O firmware agora é capaz de rodar sem o display conectado. Se o `tft.init()` falhar ou a tela não for detectada, o sistema entra em modo headless automaticamente, mantendo Serial CLI e Web Dashboard ativos.

## Configuração da Biblioteca (`platformio.ini`)

Para a ESP32-S3 no firmware Willy, utilizamos as seguintes flags de build:

- `ILI9341_DRIVER=1`
- `TFT_WIDTH=240`, `TFT_HEIGHT=320`
- `TFT_CS=10`, `TFT_DC=9`, `TFT_RST=14`
- `TFT_BL=3`
- `TOUCH_CS=15`
- `TOUCH_IRQ=-1` (Nós utilizamos polling em vez de interrupção para evitar conflitos no core S3)
- `SPI_FREQUENCY=20000000` (20MHz)

## Pino TIRQ - Interrupção do Touch

No projeto original, o pino `T_IRQ` era recomendado. No entanto, por questões de conflito de interrupção e estabilidade de polling na arquitetura ESP32-S3 multi-core (com uso massivo de RF e WiFi em background), nós configuramos o touch para rodar no **modo polling contínuo**.

O XPT2046 é consultado ativamente apenas quando a camada da GUI (LVGL) requer entradas:

### Observações

- **Conexão Física**: Não é necessário conectar/soldar o pino TIRQ do display/touch à placa do ESP32-S3. Deixe-o desconectado.
- **Configuração**: Nosso build environment crava `TOUCH_IRQ=-1`.
- **Performance**: Nossas loops estão otimizadas para processar o polling do SPI Touch eficientemente na thread principal (core 1) enquanto serviços críticos atuam no core 0 e backgrounds independentes.
