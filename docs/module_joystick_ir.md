# Shield de Joystick (Funduino / KY-023) e Infravermelho (IR)

Este capítulo detalha os componentes de entrada analógica e a infraestrutura de comunicação infravermelha (IR).

## 1. Shield de Joystick (Funduino / KY-023)

O "Joystick Shield para Arduino" (baseado no módulo KY-023) fornece controle analógico de dois eixos (X/Y) e um botão de clique (Z/SW). Este shield é amplamente utilizado em projetos Arduino e ESP32 devido à sua compatibilidade e facilidade de uso.

**Referências:**

- [Reddit: ESP32 com Funduino Joystick Shield](https://www.reddit.com/r/esp32/comments/1ck5hb7/esp32wroom32_with_funduino_joystick_shield/)
- [Arduino Forum: Funduino Joystick Shield e Nokia LCD 5110](https://forum.arduino.cc/t/funduino-joystick-shield-and-nokia-lcd-5110/311226)
- [GitHub: Joystick ESP32-S2](https://github.com/schnoog/Joystick_ESP32S2)
- [Vida de Silício: Entendendo Shield Joystick](https://portal.vidadesilicio.com.br/entendendo-shield-joystick/)

### 1.1 Especificações Técnicas

**KY-023 Joystick Module:**

- **Resolução ADC:** 12 bits (0-4095) na ESP32-S3
- **Ponto Central (Repouso):** ~2048
- **Faixa de Operação:** 0-4095
- **Tensão de Operação:** 3.3V - 5V (compatível com ESP32-S3)
- **Debounce:** Implementado no firmware
- **Consumo:** Baixo (< 5mA)

### 1.2 Pinagem Correta - ESP32-S3-WROOM-1-N8R2

| Pino do Joystick | Pino ESP32-S3 | Canal ADC | Tipo | Descrição |
| :--- | :--- | :--- | :--- | :--- |
| `VCC` | `3.3V` | - | Alimentação | Fonte de alimentação |
| `GND` | `GND` | - | Terra | Terra comum |
| `VRx` (X-axis) | `4` | `ADC1_CH3` | Analógico | Eixo X (esquerda/direita) |
| `VRy` (Y-axis) | `5` | `ADC1_CH4` | Analógico | Eixo Y (cima/baixo) |
| `SW` (Button) | `6` | - | Digital | Botão de seleção (pullup) |

### 1.3 Configuração do Firmware

**Setup Inicial:**

```cpp
// Configuração dos pinos do joystick
#define JOY_X_PIN  4    // ADC1_CH3
#define JOY_Y_PIN  5    // ADC1_CH4
#define JOY_BTN_PIN 6   // Digital com INPUT_PULLUP

void setup() {
    // Configuração do botão do joystick
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);

    // Inicialização dos ADCs (opcional, já configurado por padrão)
    analogReadResolution(12); // 12 bits (0-4095)
}
```

**Leitura e Calibração:**

```cpp
void readJoystick() {
    // Leitura dos eixos analógicos
    int x = analogRead(JOY_X_PIN);  // 0-4095
    int y = analogRead(JOY_Y_PIN);  // 0-4095

    // Valores calibrados para KY-023 Funduino Shield
    // Centro: ~2048, Faixa de ativação: ~1500-3500
    if (x < 1500) {
        // Movimento para a esquerda
        PrevPress = true;
    } else if (x > 3500) {
        // Movimento para a direita
        NextPress = true;
    }

    if (y < 1500) {
        // Movimento para cima
        UpPress = true;
    } else if (y > 3500) {
        // Movimento para baixo
        DownPress = true;
    }

    // Leitura do botão (ativação em LOW devido ao INPUT_PULLUP)
    if (digitalRead(JOY_BTN_PIN) == LOW) {
        SelPress = true;
    }
}
```

### 1.4 Valores Esperados e Calibração

**Tabela de Valores Típicos:**

| Posição | Eixo X | Eixo Y | Direção |
| :--- | :--- | :--- | :--- |
| Centro | ~2048 | ~2048 | Repouso |
| Esquerda | 0-1000 | ~2048 | PrevPress |
| Direita | 3000-4095 | ~2048 | NextPress |
| Cima | ~2048 | 0-1000 | UpPress |
| Baixo | ~2048 | 3000-4095 | DownPress |

**Faixas de Ativação (Configuradas):**

- **Ativação:** < 1500 ou > 3500
- **Central:** 1500-3500 (zona morte para evitar falsos positivos)
- **Debounce:** 20ms entre leituras

### 1.5 Integração com o Sistema de Input

O joystick é integrado ao sistema de input através da função `InputHandler()` em `boards/ESP32-S3-WROOM-1-N8R2/interface.cpp`:

```cpp
void InputHandler(void) {
    // ... código existente ...

    // Joystick Navigation - KY-023 Funduino Shield Configuration
    // Valores calibrados para KY-023: ponto central ~2048, faixa de ativação ~1500-3500
#ifdef JOY_X_PIN
    if (JOY_X_PIN >= 0) {
        int x = analogRead(JOY_X_PIN);
        // Centro: ~2048, Limite inferior: < 1500 (esquerda), Limite superior: > 3500 (direita)
        if (x < 1500) { PrevPress = true; AnyKeyPress = true; }
        else if (x > 3500) { NextPress = true; AnyKeyPress = true; }
    }
#endif

#ifdef JOY_Y_PIN
    if (JOY_Y_PIN >= 0) {
        int y = analogRead(JOY_Y_PIN);
        // Centro: ~2048, Limite inferior: < 1500 (cima), Limite superior: > 3500 (baixo)
        if (y < 1500) { UpPress = true; AnyKeyPress = true; }
        else if (y > 3500) { DownPress = true; AnyKeyPress = true; }
    }
#endif
}
```

### 1.6 Troubleshooting

**Problemas Comuns:**

1. **Joystick não responde:**
   - Verificar conexões dos pinos
   - Checar alimentação (3.3V e GND)
   - Testar com multímetro ou serial monitor

2. **Valores incorretos:**
   - Calibrar usando serial monitor para verificar valores reais
   - Ajustar faixas de ativação conforme necessário
   - Verificar resolução ADC (12 bits na ESP32-S3)

3. **Botão não funciona:**
   - Verificar se está configurado como `INPUT_PULLUP`
   - Testar com pullup externo se necessário
   - Verificar debouncing

**Código de Teste:**

```cpp
void setup() {
    Serial.begin(115200);
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
}

void loop() {
    int x = analogRead(JOY_X_PIN);
    int y = analogRead(JOY_Y_PIN);
    bool btn = digitalRead(JOY_BTN_PIN);

    Serial.printf("X: %d, Y: %d, BTN: %s\n", x, y, btn ? "HIGH" : "LOW");
    delay(100);
}
```

---

## 2. Infravermelho (IR) — Serial YS-IRTM v3.08

Para detalhes técnicos sobre o módulo IR serial, consulte o documento dedicado: **[docs/ys_irtm_ir_serial.md](ys_irtm_ir_serial.md)**

### 2.1 Especificações Técnicas

**Módulo YS-IRTM v3.08:**

- **Comunicação:** Serial UART (TTL)
- **Baud Rate:** 9600 bps (padrão)
- **Tensão de Operação:** 3.3V - 5V
- **Protocolos:** Suporta múltiplos protocolos IR
- **Alcance:** Até 10 metros (dependente do emissor)
- **Consumo:** Baixo em standby

### 2.2 Pinagem - ESP32-S3-WROOM-1-N8R2

| Componente | Pino ESP32-S3 | UART | Protocolo |
| :--- | :--- | :--- | :--- |
| **TX** (Transmissão) | `1` | UART2 | Serial IR TX |
| **RX** (Recepção) | `47` | UART2 | Serial IR RX |
| **VCC** | `3.3V` | - | Alimentação |
| **GND** | `GND` | - | Terra |

### 2.3 Configuração do Firmware

**Setup Inicial:**

```cpp
// Configuração dos pinos IR
#define IR_SERIAL_TX_PIN 1    // ESP TX → YS-IRTM RXD
#define IR_SERIAL_RX_PIN 47   // ESP RX ← YS-IRTM TXD
#define IR_SERIAL_BAUD  9600

void setup() {
    // Inicialização da serial IR
    Serial2.begin(IR_SERIAL_BAUD, SERIAL_8N1, IR_SERIAL_RX_PIN, IR_SERIAL_TX_PIN);
}
```

### 2.4 Integração com o Sistema

O módulo IR é integrado ao sistema através das classes em `src/modules/ir/`:

- **IR Transmissão:** `ir_send.cpp`, `custom_ir.cpp`
- **IR Recepção:** `ir_read.cpp`
- **Ataques IR:** `advanced_ir_atks.cpp`, `TV-B-Gone.cpp`
- **Configuração:** `ir_config.cpp`

### 2.5 Protocolos Suportados

O sistema suporta múltiplos protocolos IR através do módulo YS-IRTM:

- **NEC:** Protocolo comum para TVs e dispositivos
- **RC5:** Philips Pronto Code
- **RC6:** Versão melhorada do RC5
- **SONY:** SIRC Protocol
- **PANASONIC:** Panasonic Protocol
- **JVC:** JVC Protocol
- **Samsung:** Samsung Protocol
- **LG:** LG Protocol
- **Custom:** Protocolos personalizados

### 2.6 Exemplo de Uso

**Transmissão IR:**

```cpp
#include "modules/ir/custom_ir.h"

void sendIRCommand() {
    // Envia comando NEC (Power TV)
    uint32_t address = 0x00;
    uint32_t command = 0x45;
    sendNECCommand(address, command);
}
```

**Recepção IR:**

```cpp
#include "modules/ir/ir_read.h"

void setup() {
    IRrecv irrecv(IR_SERIAL_RX_PIN);
    irrecv.enableIRIn();
}

void loop() {
    decode_results results;
    if (irrecv.decode(&results)) {
        // Processa comando recebido
        Serial.printf("Protocolo: %d, Valor: %lx\n", results.decode_type, results.value);
        irrecv.resume();
    }
}
```
