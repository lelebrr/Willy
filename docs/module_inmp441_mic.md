# Microfone Omnidirecional INMP441 I2S

## Visão Geral

Um microfone MEMS de alto desempenho, baixo consumo, saída digital, omnidirecional com porta inferior. Utiliza a interface I2S (Inter-IC Sound) e elimina a necessidade de um ADC externo no ESP32-S3.

## Especificações

- **Interface**: I2S
- **Faixa de Tensão**: 1.8V a 3.3V
- **SNR**: 61 dBA

## Mapeamento de Pinagem Willy ESP32-S3

O ESP32-S3 lida com dados de áudio nativamente em seu barramento `I2S_NUM_0` ou `I2S_NUM_1`.

| Pino do Microfone | Pino ESP32-S3 | Descrição |
| :--- | :--- | :--- |
| `VDD` | `3.3V` | Alimentação. Deve ser 3.3V. |
| `GND` | `GND` | Terra |
| `SCK` | `41` | Clock Serial I2S (BCLK) |
| `WS`  | `42` | Seleção de Palavra I2S (LRCK) |
| `SD`  | `2`  | Dados Seriais I2S (DIN) |
| `L/R` | `GND`| Seleção de canal. Conectado a GND significa Canal Esquerdo. |

## Suporte a Bibliotecas

Configure `driver/i2s.h` do ESP-IDF para uma taxa de amostragem alta (por exemplo, 16000Hz ou 44100Hz) usando o Modo I2S Justificado à Esquerda / Padrão. Como o pino `L/R` está em GND, leia dados do multiplex do canal esquerdo.
