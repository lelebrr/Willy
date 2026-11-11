# Acessórios de Hardware: BMS & SD Sniffer

## 1. BMS 2S 5A (Sistema de Gerenciamento de Bateria)

Este módulo atua como o coração físico do sistema de alimentação sem fio do dispositivo Willy, permitindo que você execute o dispositivo com duas células de lítio 18650 de 3.7V em série (2S = 7.4V/8.4V).

### Aviso
>
> [!CAUTION]
> A saída do BMS é de 7.4V (nominal) a 8.4V (totalmente carregado). **VOCÊ NÃO PODE alimentar diretamente os pinos `3.3V` ou `5V` do ESP32-S3**. Você deve reduzir a 8.4V usando um conversor buck em massa (como um LM2596 ou um Mini-360 de alta eficiência) configurado para `5.0V` para alimentar o pino `VIN`/`5V` do ESP-S3.

### Conexão do BMS

- `B+`: Conectar ao terminal positivo da Bateria 1.
- `B-`: Conectar ao terminal negativo da Bateria 2.
- `BM`: Conecta-se à junção entre o terminal negativo da Bateria 1 e o terminal positivo da Bateria 2.
- `P+`: Saída de Energia Positiva (+) -> **Conversor Buck IN+**
- `P-`: Saída de Energia Negativa (-) -> **Conversor Buck IN- / Terra**

---

## 2. Sniffer de Cartão Micro SD (Adaptador TF Universal)

Esta placa não se conecta ao ESP32 algoritmicamente nem precisa de código. É um adaptador de passagem em linha.

- Insira o conector macho do Micro SD (TF) em um dispositivo de destino (por exemplo, uma câmera IP, um Raspberry Pi).
- Insira seu cartão Micro SD físico no slot fêmea do sniffer.
- Os vias ou headers expostos na placa do sniffer permitem que você conecte um analisador lógico externo (como um Saleae) diretamente nas linhas `CMD`, `CLK`, `D0-D3` para interceptar transferências de dados.
