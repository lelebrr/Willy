# Configurações Avançadas do Módulo IR (Infravermelho)

Este documento detalha as configurações avançadas disponíveis para o módulo de infravermelho no firmware Willy, permitindo otimizar a transmissão e recepção de sinais IR para diferentes cenários.

---

## 📋 Visão Geral

O módulo IR do Willy suporta diversas configurações avançadas que permitem:
- Ajustar a frequência de modulação para melhor compatibilidade
- Otimizar o duty cycle para maior alcance
- Configurar preamble para transmissões de longa distância
- Filtrar ruído em ambientes com interferência
- Controlar o modo de potência do LED IR

---

## 🔧 Acessando as Configurações

1. Vá para **IR → Config → Modo de Operação**
2. Escolha entre **Padrão** ou **Avançado**

---

## ⚙️ Modo Padrão

O modo padrão é recomendado para a maioria dos usuários:
- **Frequência**: 38 kHz (padrão da indústria)
- **Duty Cycle**: 33% (menor consumo, menos calor)
- **Preamble**: Padrão
- **Filtro de Ruído**: Desativado
- **Potência**: Normal

---

## 🚀 Modo Avançado

### 📡 Frequência de Modulação

A frequência de modulação determina a velocidade de oscilação do sinal IR. Diferentes fabricantes usam frequências diferentes.

| Opção | Descrição | Dispositivos Compatíveis |
|-------|-----------|--------------------------|
| **36 kHz** | Frequência mais baixa | Alguns dispositivos antigos, Philips RC-5 |
| **38 kHz** | Padrão da indústria | **Maioria dos dispositivos** (NEC, Samsung, etc.) |
| **40 kHz** | Frequência mais alta | Sony SIRC, alguns controles de ar condicionado |
| **Customizado** | Valor personalizado | Casos especiais (1000-100000 Hz) |

> [!TIP]
> Se um controle remoto não funcionar, tente frequências próximas (36, 38, 40 kHz). A maioria dos dispositivos tolera pequenas variações.

### 📊 Duty Cycle

O duty cycle determina a proporção de tempo que o LED IR fica ligado durante cada ciclo de modulação.

| Opção | Descrição | Características |
|-------|-----------|-----------------|
| **33% (Padrão)** | Menor consumo | Menor aquecimento do LED, menor alcance |
| **50% (Max Pot)** | Máxima potência | Maior alcance, maior aquecimento do LED |
| **Customizado** | Valor personalizado | Entre 10% e 90% |

> [!WARNING]
> Duty cycle de 50% gera mais calor no LED. Para transmissões prolongadas, monitore a temperatura do LED para evitar danos.

#### Comparação de Alcance

```
Duty Cycle 33%: ████████░░░░░░░░  ~3-5 metros
Duty Cycle 50%: ████████████████  ~5-10 metros
```

### 📶 Preamble (Preâmbulo)

O preamble é um burst inicial que ajuda o receptor a sincronizar (AGC lock). Preambles mais longos melhoram a recepção em distâncias maiores.

| Opção | Multiplicador | Indicação |
|-------|---------------|-----------|
| **Padrão** | 1x | Uso normal, funciona na maioria dos casos |
| **Longo (+dist)** | 2x | Maior distância, ambientes com ruído |
| **Extra Longo** | 3x | Máxima distância, ambientes muito ruidosos |

> [!TIP]
> Se os comandos IR não são reconhecidos à distância, tente aumentar o preamble. Isso dá mais tempo para o receptor ajustar o ganho.

### 🔇 Filtro de Ruído

O filtro de ruído ajuda a eliminar sinais espúrios em ambientes com muita interferência (luz solar, lâmpadas fluorescentes, etc.).

| Opção | Nível | Descrição |
|-------|-------|-----------|
| **Desativado** | 0% | Sem filtragem, aceita todos os sinais |
| **Leve** | 25% | Filtra apenas ruído óbvio |
| **Médio** | 50% | Balanceado, recomendado para uso geral |
| **Agressivo** | 75% | Máxima filtragem, pode rejeitar sinais válidos fracos |

> [!WARNING]
> Filtros muito agressivos podem rejeitar sinais legítimos de controles remotos fracos ou distantes.

### ⚡ Modo de Potência

Controla a potência de saída do LED IR.

| Opção | Descrição | Consumo | Alcance |
|-------|-----------|---------|---------|
| **Normal** | Uso padrão | Baixo | Normal |
| **Alto (+alcance)** | Maior potência | Médio | +50% |
| **Boost (Max)** | Máxima potência | Alto | +100% |

> [!DANGER]
> O modo **Boost** pode superaquecer o LED IR em uso prolongado. Use apenas para transmissões curtas e monitore a temperatura.

---

## 📋 Resumo de Configurações por Cenário

| Cenário | Frequência | Duty Cycle | Preamble | Filtro | Potência |
|---------|------------|------------|----------|--------|----------|
| **Uso Geral** | 38 kHz | 33% | Padrão | Desativado | Normal |
| **Alcance Máximo** | 38 kHz | 50% | Longo | Leve | Alto |
| **Controle Sony** | 40 kHz | 33% | Padrão | Desativado | Normal |
| **Ambiente Ruído** | 38 kHz | 50% | Longo | Médio | Alto |
| **Bateria Fraca** | 38 kHz | 33% | Padrão | Desativado | Normal |
| **Dispositivo Antigo** | 36 kHz | 50% | Longo | Leve | Alto |

---

## 🔧 Integração com Biblioteca IRremoteESP8266

As configurações avançadas são aplicadas através da biblioteca IRremoteESP8266:

```cpp
// Frequência de modulação
irsend.sendRaw(data, len, frequency);

// Duty cycle (aplicado na calibração)
irsend.calibrate();
```

### Protocolos Suportados

| Protocolo | Frequência Padrão | Bits |
|-----------|-------------------|------|
| NEC | 38 kHz | 32 |
| NECext | 38 kHz | 32 |
| RC5 | 36 kHz | 13 |
| RC6 | 36 kHz | 20 |
| Samsung32 | 38 kHz | 32 |
| Sony (SIRC) | 40 kHz | 12/15/20 |
| Kaseikyo | 38 kHz | 48 |

---

## 📁 Formato de Arquivos IR

Os arquivos de código IR usam o formato do Flipper Zero:

### Formato Raw
```
type: raw
frequency: 38000
data: 9000 4500 500 500 500 1500 ...
#
```

### Formato Parsed
```
type: parsed
protocol: NEC
address: 00 00
command: FF 00
#
```

---

## 🛠️ Solução de Problemas

### O controle remoto não funciona

1. **Verifique a frequência**: Tente 36, 38 e 40 kHz
2. **Aumente o duty cycle**: Mude para 50%
3. **Aumente o preamble**: Use "Longo" ou "Extra Longo"
4. **Desative o filtro**: Certifique-se de que o filtro está desativado

### O LED esquenta demais

1. **Reduza o duty cycle**: Use 33%
2. **Reduza a potência**: Use modo "Normal"
3. **Evite transmissões contínuas**: Dê pausas entre comandos

### Alcance muito curto

1. **Aumente o duty cycle**: Use 50%
2. **Aumente o preamble**: Use "Longo"
3. **Aumente a potência**: Use "Alto" ou "Boost"
4. **Verifique o LED**: Certifique-se de que o LED IR está funcionando corretamente

### Interferência em ambientes claros

1. **Ative o filtro**: Use "Médio" ou "Agressivo"
2. **Aumente a potência**: Use "Alto"
3. **Aproxime-se do dispositivo**: Reduza a distância

---

## 🔗 Veja Também

- [IRremoteESP8266 Library](https://github.com/crankyoldgit/IRremoteESP8266)
- [Flipper Zero IR Format](https://developer.flipper.net/flipperzero/doxygen/infrared_file_format.html)
- [Conexão do Módulo IR](./hardware_cyd_2432s028r.md)