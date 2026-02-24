# Módulo IR Serial YS-IRTM — Codificador/Decodificador

O **YS-IRTM** é um módulo de comunicação infravermelha via Serial (UART). Ao contrário de LEDs IR comuns, este módulo possui um microcontrolador próprio que gerencia a codificação e decodificação dos sinais.

**Características:**
- **Tensão**: 5V (recomendado)
- **Comunicação**: Serial TTL (9600 baud por padrão)
- **Função**: Envia e recebe comandos infravermelhos via comandos hexadecimais ou AT.

---

## Pinagem Recomendada (Compartilhada com GPS)

Como a placa CYD tem poucos pinos UART disponíveis, a melhor forma de conectar sem soldagem é compartilhar os pinos do **Port 1 (P1)** com o GPS, usando um **interruptor dedicado para o VCC**.

| Pino YS-IRTM | Função | Destino na CYD (P1) | Cor do Fio | Interruptor? |
| :--- | :--- | :--- | :--- | :--- |
| **GND** | Terra | Pino GND (P1) | Preto | Não |
| **VCC** | 5V | Pino VIN (P1) | Vermelho | **Sim** (Dedicado) |
| **RXD** | Recebe Dados | **TXD (GPIO 1)** | Azul | Não (Via VCC) |
| **TXD** | Envia Dados | **RXD (GPIO 3)** | Amarelo | Não (Via VCC) |

> [!IMPORTANT]
> **Cruzamento Serial**: Assim como no GPS, o **RXD** do módulo IR deve ser ligado no **TXD** da placa (GPIO 1), e o **TXD** do módulo no **RXD** da placa (GPIO 3).

### Funcionamento com Interruptores

Para evitar conflitos, você deve garantir que apenas **um** dispositivo esteja alimentado por vez:

- **Para usar GPS**: Ligue o interruptor do GPS e desligue o do IR.
- **Para usar IR**: Ligue o interruptor do IR e desligue o do GPS.

Como ambos operam a 9600 baud, a comunicação serial funcionará normalmente com o dispositivo que estiver ligado.

---

## Protocolo de Comunicação (Breve Resumo)

O módulo trabalha com pacotes de dados. Por exemplo, para enviar um código IR, você envia uma sequência de bytes via serial.

- **Configuração Serial**: 9600 bps, 8 bits, Sem paridade, 1 Stop bit.
- **Exemplo de Envio**: `0xFA` + `Endereço` + `Comando` + `...`

> [!NOTE]
> O firmware **Willy** atual utiliza bibliotecas que controlam LEDs IR diretamente via GPIO (toggling). Para usar este módulo YS-IRTM, seria necessário implementar um driver serial específico no firmware. No momento, a conexão física é documentada para uso futuro ou scripts customizados.

---

## Comparativo de Pinos

| Recurso | GPS | YS-IRTM | Pino CYD |
| :--- | :--- | :--- | :--- |
| **Alimentação** | 5V (VIN) | 5V (VIN) | VIN (P1) |
| **Dados In (RX)** | RX (Blue) | RXD (Blue) | **GPIO 1 (TX)** |
| **Dados Out (TX)**| TX (Yellow)| TXD (Yellow)| **GPIO 3 (RX)** |
