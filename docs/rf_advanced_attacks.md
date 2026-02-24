# 📻 RF / Sub-GHz Avançado - Documentação Completa

> **AVISO LEGAL:** Este documento é fornecido exclusivamente para fins educacionais e de pesquisa em segurança. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.

---

## 📖 Índice

1. [Introdução](#introdução)
2. [Hardware CC1101](#hardware-cc1101)
3. [Frequências Suportadas](#frequências-suportadas)
4. [Rolling Code Analysis](#rolling-code-analysis)
5. [Ataques Avançados](#ataques-avançados)
6. [Casos de Uso](#casos-de-uso)

---

## 🚀 Introdução

O módulo RF do Bruce utiliza o transceptor CC1101 para operações em frequências Sub-GHz, permitindo análise e transmissão de sinais de controles remotos, sensores, fechaduras e outros dispositivos.

### Aplicações
- Controles de garage
- Fechaduras eletrônicas
- Sensores de alarme
- Controles de portão
- Termômetros sem fio
- Dispositivos IoT Sub-GHz

---

## 🔧 Hardware CC1101

### Especificações
```
Frequência: 300-348 MHz, 387-464 MHz, 779-928 MHz
Modulação: 2-FSK, GFSK, MSK, OOK, ASK
Taxa: 0.6-500 kbps
Potência: +10 dBm (máx)
Sensibilidade: -116 dBm
```

### Wiring
```
CC1101 → ESP32
────────────────
VCC   → 3.3V
GND   → GND
CS    → GPIO (SPI CS)
SCK   → GPIO (SPI CLK)
MOSI  → GPIO (SPI MOSI)
MISO  → GPIO (SPI MISO)
GDO0  → GPIO (TX/RX Data)
GDO2  → GPIO (opcional)
```

### Configuração de Pinos
```
Menu Config → RF Config → Pin Setup
- CS Pin: 5
- GDO0 Pin: 4
```

---

## 📻 Frequências Suportadas

### Região USA (FCC)
| Frequência | Uso Comum |
|------------|-----------|
| 315 MHz | Controles de garage, alarmes |
| 390 MHz | Controles automotivos |
| 433.92 MHz | ISM, IoT, sensores |
| 915 MHz | ISM, LoRa |

### Região Europa (ETSI)
| Frequência | Uso Comum |
|------------|-----------|
| 433.92 MHz | ISM, controles |
| 868 MHz | ISM, IoT |
| 868.35 MHz | Smart Home |

### Região Brasil (ANATEL)
| Frequência | Uso Comum |
|------------|-----------|
| 315 MHz | Alarmes, controles |
| 433.92 MHz | ISM, IoT |
| 915 MHz | ISM (limitado) |

---

## 🔐 Rolling Code Analysis

### O que é Rolling Code?

Rolling Code (ou hopping code) é um mecanismo de segurança onde o código transmitido muda a cada ativação. Usado em:
- Fechaduras de carros
- Controles de garage
- Alarmes veiculares
- Sistemas de segurança

### Protocolos Rolling Code Suportados

#### KeeLoq
```
Fabricantes: Microchip
Bits: 32 (hop) + 28 (serial) + 4 (button)
Criptografia: KeeLoq cipher
```

#### Chamberlain/Craftmaster
```
Fabricantes: Chamberlain, LiftMaster
Frequência: 315 MHz, 390 MHz
Bits: 32 hopping + 28 serial
```

#### Sommer
```
Fabricantes: Sommer, Sommer Antriebs
Frequência: 868 MHz
Protocolo: Sommer rolling code
```

#### Nice Flo
```
Fabricantes: Nice
Frequência: 433.92 MHz
Protocolo: Nice Flo/R-S
```

#### BFT
```
Fabricantes: BFT
Frequência: 433.92 MHz
Protocolo: BFT Mitto
```

---

## ⚔️ Ataques Avançados

### 1. Rolling Code Capture

**Descrição:** Captura códigos rolling para análise.

**Como funciona:**
- Sintoniza frequência alvo
- Captura múltiplas transmissões
- Armazena códigos sequenciais
- Analisa padrões

**Uso:**
1. Menu RF → Rolling Code → Capture
2. Selecione frequência
3. Pressione controle remoto
4. Códigos são capturados automaticamente

**Saída:**
```
Rolling Code Capture ativo
Frequência: 433.92 MHz
Modulação: ASK

Códigos capturados:
#1: 0x12345678 (button: LOCK)
#2: 0x12345679 (button: LOCK)
#3: 0x1234567A (button: UNLOCK)
#4: 0x1234567B (button: LOCK)

Padrão: Sequencial (+1 por transmissão)
```

**Aplicação:**
- Análise de protocolos
- Pesquisa de segurança
- Identificação de vulnerabilidades

---

### 2. Rolling Code Replay

**Descrição:** Reproduz código capturado.

**Limitações:**
- Código já usado não funciona novamente (sync counter)
- Requer captura de código não usado

**Uso:**
1. Capture código do controle original
2. Menu RF → Rolling Code → Replay
3. Selecione código
4. Envie

**Aplicação:**
- Teste de vulnerabilidade
- Demonstrações de segurança
- Pesquisa acadêmica

---

### 3. Rolling Code Prediction

**Descrição:** Tenta prever próximos códigos.

**Como funciona:**
- Captura múltiplos códigos
- Analisa algoritmo de geração
- Tenta predizer sequência
- Funciona apenas em implementações fracas

**Uso:**
1. Menu RF → Rolling Code → Prediction
2. Capture 10+ códigos do mesmo controle
3. Sistema analisa padrões
4. Tenta prever próximos códigos

**Saída:**
```
Análise de Rolling Code

Códigos analisados: 15
Padrão detectado: Sequencial simples
Diferença média: +1

Próximos códigos prováveis:
#16: 0x1234567C
#17: 0x1234567D
#18: 0x1234567E

⚠️ Confiança: Baixa
(maioria dos sistemas usa crypto forte)
```

**Aplicação:**
- Teste de qualidade de implementação
- Pesquisa de vulnerabilidades
- Auditoria de segurança

---

### 4. Signal Jammer

**Descrição:** Bloqueia sinais RF em frequência específica.

**Como funciona:**
- Transmite ruído na frequência
- Sobrepondo sinais legítimos
- Impede recepção

**Aviso:** Ilegal na maioria dos países!

**Configuração:**
```
frequência: 433.92 MHz
modulação: ASK
potência: máxima
duração: contínua
```

**Uso:**
1. Menu RF → Jammer
2. Configure frequência
3. Inicie (cuidado!)

**Aplicação:**
- Teste de imunidade RF
- Pesquisa de interferência
- Laboratório controlado

---

### 5. Signal Analyzer

**Descrição:** Análise detalhada de sinais RF.

**Informações exibidas:**
- Frequência central
- Largura de banda
- Modulação detectada
- Duração do pulso
- Intervalos
- Raw data

**Uso:**
1. Menu RF → Signal Analyzer
2. Sintonize frequência
3. Aguarde transmissão
4. Visualize análise

**Saída:**
```
Signal Analyzer

Frequência: 433.92 MHz
Modulação: ASK/OOK
RSSI: -45 dBm

Pulse Analysis:
┌────────────────────────────────┐
│ ▄▄▄   ▄▄   ▄▄▄   ▄▄   ▄▄▄    │
│                                │
└────────────────────────────────┘

Pulse Width: 200-800 μs
Gap Width: 400-1600 μs
Total Bits: 24

Raw Data: 0xABC123
```

**Aplicação:**
- Engenharia reversa
- Análise de protocolos
- Debug de transmissões

---

### 6. Key Fob Cloner

**Descrição:** Clona controles remotos simples (fixed code).

**Como funciona:**
- Captura código do controle original
- Armazena na memória
- Permite retransmissão

**Protocolos clonáveis (fixed code):**
- PT2262 / PT2272
- EV1527
- RT1527
- FP527
- HT12E / HT12D

**Uso:**
1. Menu RF → Key Fob Cloner
2. Capture código original
3. Salve na memória
4. Envie quando necessário

**Saída:**
```
Key Fob Cloner

Capturando...
✅ Código capturado!

Protocolo: EV1527
Bits: 24
Código: 0xABC123

Opções:
[1] Enviar agora
[2] Salvar na memória
[3] Descartar
```

**Aplicação:**
- Backup de controles
- Teste de segurança
- Controle de acesso autorizado

---

### 7. Frequency Hopper

**Descrição:** Varre múltiplas frequências em busca de sinais.

**Configuração:**
```
start_freq: 300 MHz
end_freq: 928 MHz
step: 100 kHz
dwell_time: 100 ms
threshold: -80 dBm
```

**Uso:**
1. Menu RF → Frequency Hopper
2. Configure range
3. Inicie varredura
4. Sinais são detectados automaticamente

**Saída:**
```
Frequency Hopper

Range: 300-928 MHz
Step: 100 kHz

Sinais detectados:
315.00 MHz: -55 dBm (ASK)
390.00 MHz: -67 dBm (ASK)
433.92 MHz: -45 dBm (ASK)
868.35 MHz: -78 dBm (FSK)
```

**Aplicação:**
- Descoberta de dispositivos
- Inventário RF
- Pesquisa de ambiente

---

### 8. De Bruijn Attack

**Descrição:** Ataque de força bruta otimizado para fechaduras.

**Como funciona:**
- Gera sequência De Bruijn
- Testa todas combinações de N bits
- Cada código compartilha bits com anterior
- Otimiza tempo de transmissão

**Configuração:**
```
bit_length: 12
protocol: Chamberlain
repeats: 5
timing: padrão
```

**Uso:**
1. Menu RF → De Bruijn Attack
2. Configure bit length
3. Inicie ataque

**Aviso:** Pode bloquear o receptor!

**Aplicação:**
- Teste de vulnerabilidade
- Pesquisa de segurança
- Demonstrações

---

### 9. Raw Transmit

**Descrição:** Transmissão de dados brutos.

**Como funciona:**
- Define dados hexadecimais
- Configura parâmetros de transmissão
- Envia pulso a pulso

**Uso:**
1. Menu RF → Raw Transmit
2. Digite dados hex
3. Configure modulação
4. Envie

**Exemplo:**
```
Raw Transmit

Dados: ABC123456789
Modulação: ASK
Frequência: 433.92 MHz
Bit rate: 2000 bps

[ENVIAR]
```

**Aplicação:**
- Testes customizados
- Protocolos não suportados
- Pesquisa avançada

---

### 10. Spectrum Scanner

**Descrição:** Visualização espectral do ambiente RF.

**Como funciona:**
- Varre frequência continuamente
- Mede RSSI
- Plota gráfico no display
- Identifica fontes de interferência

**Uso:**
1. Menu RF → Spectrum Scanner
2. Configure range
3. Visualize em tempo real

**Saída:**
```
Spectrum Scanner

    RSSI
     ▲
-40dB│    ████
-50dB│   ██████
-60dB│  ████████
-70dB│ ██████████
-80dB│████████████
     └────────────►
      433.0  434.0 MHz

Pico: 433.92 MHz (-42 dBm)
```

**Aplicação:**
- Análise de ambiente
- Detecção de interferência
- Planejamento de frequências

---

## 🚀 Novos Ataques Avançados (v2.0)

Esta seção documenta os 10 novos ataques de rádio implementados na suíte Bruce.

### 11. Predictive Garage/Portão Brute
**Uso:** Menu RF → Garage Brute P2262
- **Descrição:** Ataque de força bruta otimizado para receptores P2262/PT2262.
- **Funcionamento:** Em vez de tentar todas as combinações aleatoriamente, utiliza padrões binários predictores comumente usados em controles de garagem brasileiros.

### 12. Stealth Low-Power Exfil Beacon
**Uso:** Menu RF → Stealth L.P. Beacon
- **Descrição:** Transmissão de "batimento cardíaco" (beacon) em baixa potência.
- **Funcionamento:** Emite pulsos curtos e espaçados para exfiltração de dados mínima sem saturar o espectro, dificultando a detecção por analisadores de espectro RF convencionais.

### 13. Multi-Device Coordinated Replay
**Uso:** Menu RF → Coord. Replay Seq
- **Descrição:** Sequenciador de sinais para ataques em cadeia.
- **Funcionamento:** Permite configurar uma lista de sinais salvos (ex: #1 Abrir Portão, #2 Desarmar Alarme) e reproduzi-los em sequência com temporização precisa.

### 14. Protocol Fuzzer Smart
**Uso:** Menu RF → Smart Fuzzer
- **Descrição:** Fuzzing inteligente baseado em protocolos.
- **Funcionamento:** Identifica a estrutura do protocolo alvo (ex: Princeton) e gera variações válidas (bitflips, mutações) para testar falhas de lógica em decodificadores.

### 15. False Alarm Trigger Loop
**Uso:** Menu RF → Alarm Trigger
- **Descrição:** Loop de disparo persistente para sensores/alarmes.
- **Funcionamento:** Carrega um sinal capturado e o repete infinitamente com um "jitter" (atraso variável entre 500ms e 2s) para confundir o receptor.

### 16. TPMS Spoof Chaos
**Uso:** Menu RF → TPMS Spoof
- **Descrição:** Emulação de sensores de pressão de pneus falsos.
- **Funcionamento:** Transmite IDs e dados de telemetria TPMS aleatórios em 433.92MHz para acionar alertas falsos em painéis de veículos próximos.

### 17. Rolling Code Learner + Replay Delayed
**Uso:** Menu RF → Rolling Learner
- **Descrição:** Interceptador de códigos variáveis.
- **Funcionamento:** Captura um sinal de Rolling Code, bloqueia visualmente a tela para o operador e retransmite o sinal após um atraso programado.

### 18. Spectrum Visualizer + Target Lock
**Uso:** Menu RF → Spectrum Lock
- **Descrição:** Scanner com trava automática no sinal mais forte.
- **Funcionamento:** Analisa o espectro e desenha uma mira de "LOCK" na frequência com maior RSSI, permitindo sintonizar automaticamente transmissores próximos.

### 19. Sensor Battery Drain Flood
**Uso:** Menu RF → Sensor Drain Flood
- **Descrição:** Ataque de exaustão de bateria.
- **Funcionamento:** Inunda o canal RF com tráfego RAW, forçando receptores de sensores de baixo consumo a permanecerem em estado de "wake" constante, drenando a bateria.

### 20. Jammer Hopper Adaptativo
**Uso:** Menu RF → Jammer Hopper
- **Descrição:** Bloqueador que salta frequências.
- **Funcionamento:** Alterna rapidamente entre as bandas de 315, 433 e 868MHz para tentar neutralizar múltiplos tipos de dispositivos simultaneamente.

---

## 📋 Casos de Uso

### Pentest de Fechaduras

1. **Reconhecimento**
   - Frequency Hopper
   - Spectrum Scanner

2. **Análise**
   - Signal Analyzer
   - Rolling Code Capture

3. **Exploração**
   - Rolling Code Prediction
   - Key Fob Cloner (se fixed code)

### Auditoria de Alarmes

1. **Identificação**
   - Detectar frequências usadas
   - Identificar protocolos

2. **Teste**
   - Signal Jammer (laboratório)
   - Replay attacks

### Pesquisa de Protocolos

1. **Captura**
   - Signal Analyzer
   - Raw data extraction

2. **Análise**
   - Decodificação
   - Documentação

3. **Implementação**
   - Suporte ao protocolo
   - Testes

---

## 🔧 Troubleshooting

### CC1101 não detectado
- Verifique conexões SPI
- Confirme alimentação 3.3V
- Teste com outro módulo

### Alcance curto
- Adicione antena adequada
- Aumente potência (se legal)
- Verifique frequência correta

### Sinais não detectados
- Confirme frequência
- Ajuste modulação
- Verifique threshold RSSI

---

## 📚 Referências

- CC1101 Datasheet
- KeeLoq Protocol Documentation
- Sub-GHz RF Analysis
- RFCat Project
