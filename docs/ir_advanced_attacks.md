# 📡 Infravermelho Avançado - Documentação Completa

> **AVISO LEGAL:** Este documento é fornecido exclusivamente para fins educacionais e de pesquisa em segurança. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.

---

## 📖 Índice

1. [Introdução](#introdução)
2. [Protocolos Suportados](#protocolos-suportados)
3. [Bancos de Dados IR](#bancos-de-dados-ir)
4. [Ataques Avançados](#ataques-avançados)
5. [Casos de Uso](#casos-de-uso)

---

## 🚀 Introdução

O módulo de Infravermelho do Bruce permite controle e análise de dispositivos IR, incluindo TVs, ar-condicionado, projetores, sistemas de som e muito mais.

### Hardware Necessário
- LED IR (940nm recomendado)
- Receptor IR (TSOP1838 ou similar)
- Resistor 100Ω para LED

### Wiring
```
LED IR:
  - Anodo → GPIO (via resistor 100Ω)
  - Catodo → GND

Receptor IR:
  - VCC → 3.3V
  - GND → GND
  - OUT → GPIO
```

---

## 📋 Protocolos Suportados

### Protocolos Comuns

| Protocolo | Marcas | Frequência | Bits |
|-----------|--------|------------|------|
| NEC | Samsung, Yamaha, Onkyo | 38kHz | 32 |
| NECext | Samsung estendido | 38kHz | 32+ |
| Samsung36 | Samsung AC | 36kHz | 36 |
| LG | LG TVs, Soundbars | 38kHz | 28 |
| LG2 | LG estendido | 38kHz | 28+ |
| Sony | Sony TVs, Blu-ray | 40kHz | 12/15/20 |
| RC5 | Philips, Marantz | 36kHz | 12/13 |
| RC6 | Philips, Microsoft | 36kHz | 20/24/32 |
| Panasonic | Panasonic TVs | 37kHz | 48 |
| JVC | JVC devices | 52.7kHz | 16 |
| Sanyo | Sanyo, Canon | 38kHz | 12+ |
| Sharp | Sharp TVs | 38kHz | 15 |
| Denon | Denon, Marantz | 38kHz | 48 |

### Protocolos de Ar-Condicionado

| Protocolo | Marcas | Características |
|-----------|--------|-----------------|
| Kelvinator | Kelvinator | 16 bytes |
| Midea | Midea, Toshiba AC | 17 bytes |
| Gree | Gree ACs | 16 bytes |
| Haier | Haier ACs | 17 bytes |
| Mitsubishi | Mitsubishi Electric | 18 bytes |
| Daikin | Daikin ACs | 35 bytes |
| Trotec | Trotec ACs | 16 bytes |
| Whirlpool | Whirlpool ACs | 21 bytes |

### Protocolos Especiais

| Protocolo | Uso | Bits |
|-----------|-----|------|
| Dish | Dish Network | 16 |
| Whynter | Whynter ACs | 32 |
| Bose | Bose systems | variável |
| Magicar | Car alarms | variável |
| Vegas | Casino games | variável |

---

## 📚 Bancos de Dados IR

### Samsung (38+ códigos)

#### TV
```
POWER:      0xE0E040BF
SOURCE:     0xE0E0807F
HDMI:       0xE0E0D12E
VOL_UP:     0xE0E0E01F
VOL_DOWN:   0xE0E0D02F
CH_UP:      0xE0E048B7
CH_DOWN:    0xE0E008F7
MENU:       0xE0E058A7
TOOLS:      0xE0E0D22D
INFO:       0xE0E0F807
RETURN:     0xE0E01AE5
EXIT:       0xE0E0B44B
```

#### Soundbar
```
POWER:      0xE0E0E01F
VOL_UP:     0xE0E0E01F
VOL_DOWN:   0xE0E0D02F
MUTE:       0xE0E0F00F
BASS_UP:    0xE0E050AF
BASS_DOWN:  0xE0E0D02F
SURROUND:   0xE0E0807F
```

#### Ar-Condicionado
```
POWER:      Samsung36
MODE_COOL:  Samsung36
MODE_HEAT:  Samsung36
TEMP_16-30: Samsung36
FAN_LOW:    Samsung36
FAN_HIGH:   Samsung36
```

---

### LG (36+ códigos)

#### TV
```
POWER:      0x20DF10EF
INPUT:      0x20DFD02F
VOL_UP:     0x20DF40BF
VOL_DOWN:   0x20DFC03F
CH_UP:      0x20DF00FF
CH_DOWN:    0x20DF807F
MENU:       0x20DFC23D
Q_MENU:     0x20DF02FD
INFO:       0x20DF55AA
RETURN:     0x20DF14EB
```

#### Soundbar
```
POWER:      0x20DF10EF
VOL_UP:     0x20DF40BF
VOL_DOWN:   0x20DFC03F
FUNCTION:   0x20DFD02F
BASS:       0x20DF4EB1
```

---

### Sony (42+ códigos)

#### TV
```
POWER:      0xA90, 15 bits
STANDBY:    0xA90, 15 bits
VOL_UP:     0x490, 15 bits
VOL_DOWN:   0xC90, 15 bits
CH_UP:      0x090, 15 bits
CH_DOWN:    0x890, 15 bits
INPUT:      0xA50, 15 bits
HDMI1:      0x46B, 15 bits
HDMI2:      0xC6B, 15 bits
MENU:       0x070, 15 bits
```

#### Blu-ray
```
POWER:      0xA90, 15 bits
PLAY:       0x4CB, 15 bits
PAUSE:      0xCCB, 15 bits
STOP:       0x0CB, 15 bits
EJECT:      0x24D, 15 bits
```

---

### Universal (30+ códigos)

#### Códigos para múltiplas marcas
```
TV_POWER_UNIVERSAL:
  - NEC: 0x00FF02FD
  - RC5: 0x0C
  - Sony: 0x15

TV_VOL_UP_UNIVERSAL:
  - NEC: 0x00FF40BF
  - RC5: 0x16
  - Sony: 0x12
```

---

## ⚔️ Ataques Avançados

### 1. IR Protocol Fuzzer

**Descrição:** Fuzzing em protocolos IR para descobrir códigos ocultos.

**Como funciona:**
- Gera códigos aleatórios para cada protocolo
- Testa ranges de endereço/comando
- Identifica códigos que causam comportamento anômalo

**Configuração:**
```
protocol: NEC, Samsung36, LG, Sony, RC5, RC6
address_range: 0x0000-0xFFFF
command_range: 0x00-0xFF
packets_per_second: 10
```

**Uso:**
1. Menu IR → Advanced → IR Protocol Fuzzer
2. Selecione protocolo
3. Configure ranges
4. Inicie fuzzing

**Saída:**
```
Protocolo: NEC
Enviados: 1,234
Códigos válidos: 23
Comportamentos anômalos:
  - 0x1234: TV reiniciou
  - 0x5678: Menu secreto abriu
```

**Aplicação:**
- Descoberta de códigos ocultos
- Pesquisa de vulnerabilidades
- Teste de robustez

---

### 2. Pattern Learner

**Descrição:** Aprende protocolos IR desconhecidos.

**Como funciona:**
- Captura múltiplos sinais
- Analisa timing e estrutura
- Identifica padrões
- Gera template de protocolo

**Uso:**
1. Menu IR → Advanced → Pattern Learner
2. Pressione botões do controle
3. Aguarde análise
4. Salve protocolo aprendido

**Saída:**
```
Protocolo aprendido:
  Nome: Custom_AC
  Frequência: 38kHz
  Header: 9000, 4500
  BitMark: 560
  OneSpace: 1690
  ZeroSpace: 560
  Bits: 48
  
Códigos capturados:
  POWER: 0x28C7...
  TEMP_UP: 0x28C7...
```

**Aplicação:**
- Controles não suportados
- Dispositivos customizados
- Engenharia reversa

---

### 3. Anti-Replay Variable Timing

**Descrição:** Replay de códigos com timing variável para evitar detecção.

**Como funciona:**
- Captura código válido
- Adiciona jitter aleatório
- Varia intervalos entre envios
- Bypass de sistemas anti-replay

**Configuração:**
```
base_interval_ms: 100
jitter_percent: 10
repeat_count: 3
gap_variance_ms: 50
```

**Uso:**
1. Menu IR → Advanced → Anti-Replay Variable Timing
2. Capture código alvo
3. Configure timing
4. Envie com variação

**Aplicação:**
- Sistemas com proteção replay
- Teste de segurança
- Pesquisa de protocolos

---

### 4. IR Burst Overload Jammer

**Descrição:** Flood multi-protocolo para sobrecarregar receptores.

**Como funciona:**
- Envia múltiplos protocolos simultaneamente
- Flood de códigos POWER
- Sobrecarrega receptor IR
- Torna dispositivo inutilizável

**Configuração:**
```
protocols: [NEC, Samsung36, LG, Sony]
codes: [POWER, VOL_UP, VOL_DOWN]
burst_duration_ms: 1000
burst_interval_ms: 100
```

**Uso:**
1. Menu IR → Advanced → IR Burst Overload Jammer
2. Selecione protocolos
3. Inicie jamming

**Aviso:** Afeta todos os dispositivos IR no range!

**Aplicação:**
- Teste de imunidade IR
- Pesquisa de interferência
- Demonstração de DoS

---

### 5. WiFi Remote Bridge

**Descrição:** Controle IR via interface WiFi.

**Como funciona:**
- Cria servidor web
- Interface HTML para controle
- Envia códigos IR via requisição
- Controle remoto completo

**Uso:**
1. Menu IR → Advanced → WiFi Remote Bridge
2. Conecte ao WiFi
3. Acesse IP no navegador
4. Controle dispositivos

**Interface Web:**
```
http://192.168.4.1/ir

POST /send
{
  "protocol": "NEC",
  "code": "0xE0E040BF",
  "repeat": 3
}
```

**Aplicação:**
- Controle remoto
- Automação residencial
- Integração com outros sistemas

---

### 6. Stealth Low-Duty Blaster

**Descrição:** Envio discreto de códigos IR.

**Como funciona:**
- Pulso de baixa potência
- Timing preciso
- Múltiplas repetições fracas
- Difícil de detectar visualmente

**Configuração:**
```
duty_cycle: 25%
pulse_duration_us: 5
inter_pulse_delay_us: 20
total_repeats: 10
```

**Uso:**
1. Menu IR → Advanced → Stealth Low-Duty Blaster
2. Configure duty cycle
3. Envie código

**Aplicação:**
- Operações discretas
- Teste de sensibilidade
- Pesquisa de receptores

---

### 7. Device Predictor

**Descrição:** Identifica marca/modelo por análise de código IR.

**Como funciona:**
- Captura código IR
- Compara com banco de dados
- Identifica protocolo
- Sugere marca e modelo

**Uso:**
1. Menu IR → Advanced → Device Predictor
2. Aponte controle remoto
3. Pressione qualquer botão
4. Visualize identificação

**Saída:**
```
Código capturado: 0xE0E040BF

Identificação:
  Protocolo: NEC (Samsung)
  Fabricante: Samsung
  Dispositivo: TV/Soundbar
  Modelo provável: Samsung Smart TV
  
Outros códigos do mesmo dispositivo:
  POWER: 0xE0E040BF ✓
  VOL_UP: 0xE0E0E01F (probável)
  VOL_DOWN: 0xE0E0D02F (probável)
```

**Aplicação:**
- Identificação de dispositivos
- Inventário de equipamentos
- Pesquisa de protocolos

---

### 8. Offline DB Brute Force

**Descrição:** Testa banco de dados offline de códigos IR.

**Como funciona:**
- Carrega banco de dados do SD
- Testa códigos sequencialmente
- Identifica códigos que funcionam
- Salva resultados

**Banco de dados incluído:**
- 10.000+ códigos
- 50+ marcas
- TVs, ACs, Soundbars, Projetores

**Uso:**
1. Menu IR → Advanced → Offline DB Brute Force
2. Selecione categoria
3. Inicie brute force
4. Visualize códigos que funcionaram

**Saída:**
```
Testados: 2,345
Funcionaram: 12

Códigos válidos:
  Samsung TV POWER: 0xE0E040BF
  LG TV POWER: 0x20DF10EF
  Sony TV POWER: 0xA90
  ...
```

**Aplicação:**
- Descoberta de códigos
- Controle de dispositivos desconhecidos
- Pentest físico

---

### 9. Multi-IR Attack

**Descrição:** Ataque sincronizado em múltiplos dispositivos.

**Como funciona:**
- Configura múltiplos códigos
- Executa em sequência
- Timing sincronizado
- Ataque coordenado

**Configuração:**
```
devices:
  - name: "Samsung TV"
    protocol: NEC
    code: 0xE0E040BF
    delay: 0
  - name: "LG Soundbar"
    protocol: LG
    code: 0x20DF10EF
    delay: 100
  - name: "Sony Blu-ray"
    protocol: Sony
    code: 0xA90
    delay: 200
```

**Uso:**
1. Menu IR → Advanced → Multi-IR Attack
2. Configure dispositivos
3. Execute ataque

**Aplicação:**
- Desligar múltiplos dispositivos
- Teste de ambiente
- Demonstração coordenada

---

### 10. Waveform Visualizer

**Descrição:** Visualiza forma de onda IR em tempo real.

**Como funciona:**
- Captura sinal IR
- Mostra timing no display
- Identifica estrutura
- Análise visual

**Uso:**
1. Menu IR → Advanced → Waveform Visualizer
2. Aponte controle remoto
3. Pressione botão
4. Visualize waveform

**Saída:**
```
Waveform capturado:

│█▀▀▀▀▀█│▄▀█│▄▀█│█▀█│█▀▀█│
│ Header │ 1  │ 0  │ 1 │ ... │

Timing (μs):
  Header Mark:   9,000
  Header Space:  4,500
  Bit Mark:      560
  One Space:     1,690
  Zero Space:    560
  Total bits:    32
```

**Aplicação:**
- Análise de protocolos
- Debug de códigos
- Engenharia reversa

---

## 📋 Casos de Uso

### Pentest Físico

1. **Reconhecimento**
   - Device Predictor
   - Waveform Visualizer

2. **Análise**
   - Pattern Learner
   - Identificar dispositivos

3. **Exploração**
   - IR Protocol Fuzzer
   - Offline DB Brute Force

4. **Execução**
   - Multi-IR Attack
   - Desligar todos dispositivos

### Controle Residencial

1. **Setup**
   - WiFi Remote Bridge
   - Configure códigos

2. **Uso diário**
   - Interface web
   - Automação

### Pesquisa de Protocolos

1. **Captura**
   - Waveform Visualizer
   - Pattern Learner

2. **Análise**
   - Identificar estrutura
   - Documentar timing

3. **Implementação**
   - Criar suporte
   - Testar códigos

---

## 🔧 Troubleshooting

### IR não funciona
- Verifique conexões do LED
- Teste com câmera de celular (LED deve aparecer roxo)
- Ajuste resistor se necessário

### Códigos não são reconhecidos
- Verifique se protocolo é suportado
- Use Pattern Learner
- Verifique frequência do receptor

### Alcance curto
- Aumente corrente do LED (máx 100mA)
- Use transistor para maior potência
- Direcione LED corretamente

---

## 📚 Referências

- IRremoteESP8266 Library
- IRDB Database
- SB-Projects IR Protocol Documentation