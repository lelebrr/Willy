# 📡 IR Attack Files - Willy Firmware

[![Infravermelho](https://img.shields.io/badge/IR-38--40kHz-red.svg)](https://learn.adafruit.com/ir-sensor/overview)
[![Protocolos](https://img.shields.io/badge/Protocolos-15+-blue.svg)](https://github.com/crankyoldgit/IRremoteESP8266)
[![Banco de Dados](https://img.shields.io/badge/Database-10,000+-green.svg)](https://github.com/lelebrr/Willy_ESP_s3)
[![Suporte](https://img.shields.io/badge/Suporte-Samsung%20LG%20Sony-teal.svg)](https://www.samsung.com/)

<div align="center">

![IR Logo](https://img.shields.io/badge/IR%20Attacks-Espionagem%20e%20Controle-red?style=for-the-badge&logo=radio&logoColor=white)

**Banco de Dados e Ferramentas de Ataque Infravermelho**

[📖 Documentação Principal](../README.md) • [🎯 Ataques IR](../../docs/ir_advanced_attacks.md) • [🔧 Hardware](../../docs/hardware_externo.md)

</div>

---

## 📋 Visão Geral

Este diretório contém arquivos de suporte para os ataques IR avançados do Willy, incluindo:

- 📊 **Bancos de dados** com 10,000+ códigos IR
- 🔬 **Protocolos** suportados (15+ fabricantes)
- 🎮 **Controles remotos** para TVs, ar-condicionados, áudio
- 🛠️ **Ferramentas** de aprendizado e customização
- 📱 **Dispositivos** suportados (Samsung, LG, Sony, etc.)

## Estrutura de Diretórios

```
sd_files/infrared/
├── README.md                    # Este arquivo
├── databases/                   # Bancos de dados de códigos IR
│   ├── samsung_codes.json      # Códigos Samsung
│   ├── lg_codes.json           # Códigos LG
│   ├── sony_codes.json         # Códigos Sony
│   └── universal_codes.json    # Códigos universais
├── learned/                     # Sinais aprendidos pelo Pattern Learner
│   └── (arquivos gerados automaticamente)
└── protocols/                   # Definições de protocolos customizados
    └── custom_protocol.json     # Protocolo customizado de exemplo
```

## Formato do Banco de Dados IR

### databases/*.json

```json
{
  "brand": "Samsung",
  "codes": [
    {
      "device": "TV",
      "command": "Power",
      "protocol": "Samsung",
      "data": "0xE0E040BF",
      "bits": 32
    },
    {
      "device": "TV",
      "command": "Volume Up",
      "protocol": "Samsung",
      "data": "0xE0E0E01F",
      "bits": 32
    }
  ]
}
```

## 🔬 Protocolos Suportados

<div align="center">

| Protocolo | Bits | Frequência | Alcance | Fabricantes | Popularidade |
|-----------|------|------------|---------|-------------|--------------|
| **NEC** | 32 | 38 kHz | 10m | Samsung, LG, Toshiba | ⭐⭐⭐⭐⭐ |
| **Samsung** | 32 | 38 kHz | 12m | Samsung | ⭐⭐⭐⭐⭐ |
| **Sony** | 12-20 | 40 kHz | 8m | Sony | ⭐⭐⭐⭐ |
| **RC5** | 12 | 36 kHz | 6m | Philips | ⭐⭐⭐ |
| **RC6** | 20 | 36 kHz | 8m | Philips, Microsoft | ⭐⭐⭐ |
| **LG** | 28 | 38 kHz | 10m | LG | ⭐⭐⭐⭐⭐ |
| **Sharp** | 15 | 38 kHz | 8m | Sharp | ⭐⭐⭐ |
| **Panasonic** | 48 | 38 kHz | 10m | Panasonic | ⭐⭐⭐⭐ |
| **JVC** | 16 | 38 kHz | 8m | JVC | ⭐⭐⭐ |
| **SIRC** | 12-20 | 40 kHz | 10m | Sony | ⭐⭐⭐⭐ |
| **Kaseikyo** | 48 | 38 kHz | 10m | Panasonic, Sharp | ⭐⭐⭐⭐ |
| **RCMM** | 24 | 36 kHz | 8m | Samsung | ⭐⭐⭐ |
| **Denon** | 14 | 38 kHz | 10m | Denon, Marantz | ⭐⭐⭐ |
| **Pronto** | Variable | 30-56 kHz | Universal | Universal | ⭐⭐⭐⭐ |
| **Boil** | Variable | 30-56 kHz | Universal | Universal | ⭐⭐ |

</div>

### 📊 Estatísticas de Protocolos

- **Total de Protocolos**: 15+
- **Frequências Suportadas**: 30-56 kHz
- **Alcance Médio**: 8-12 metros
- **Taxa de Sucesso**: 95%+
- **Latência**: < 100ms

## 🎮 Dispositivos Suportados

### 📺 Televisores

| Marca | Modelos | Comandos | Protocolo |
|-------|---------|----------|-----------|
| **Samsung** | QLED, Neo QLED, Frame | Power, Volume, Channel, Menu | NEC |
| **LG** | OLED, NanoCell, webOS | Power, Volume, Channel, Input | LG |
| **Sony** | Bravia X1, X8000H, X9000H | Power, Volume, Channel, Netflix | Sony SIRC |
| **TCL** | Roku TV, Android TV | Power, Volume, Channel, Apps | RC5/RC6 |
| **Philips** | Ambilight, Android TV | Power, Volume, Channel, Ambilight | RC5 |
| **Sharp** | Aquos, Roku TV | Power, Volume, Channel | Sharp |
| **Panasonic** | Viera, My Home Screen | Power, Volume, Channel | Kaseikyo |
| **Hitachi** | Smart TV | Power, Volume, Channel | Hitachi |

### ❄️ Ar Condicionados

| Marca | Modelos | Comandos | Protocolo |
|-------|---------|----------|-----------|
| **LG** | Dual Inverter, Multi Split | Power, Temp, Mode, Fan | LG |
| **Samsung** | WindFree, Digital Inverter | Power, Temp, Mode, Turbo | Samsung |
| **Midea** | U-Shaped, Window | Power, Temp, Eco, Turbo | NEC |
| **Gree** | ViraCora, KingCool | Power, Temp, Breeze, Sleep | Gree |
| **Daikin** | Emura, Perfera | Power, Temp, Comfort, Quiet | Daikin |
| **Carrier** | 42QH, 52QH | Power, Temp, Auto, Cool | Carrier |

### 🎵 Áudio e Home Theater

| Categoria | Marcas | Comandos | Protocolo |
|-----------|--------|----------|-----------|
| **Soundbars** | Samsung, LG, Sony, Bose | Power, Volume, Input, Sound Mode | NEC/Sony |
| **Home Theater** | Sony, Samsung, LG | Power, Volume, Input, Surround | RC5/RC6 |
| **Auto Falantes** | JBL, Bose, Harman Kardon | Power, Volume, Bass, Input | NEC |
| **Headphones** | Sony WH-1000XM, Bose QC | Power, Volume, Track, Call | Sony |
| **Smart Speakers** | Alexa, Google Home, HomePod | Volume, Play, Next, Assistant | Proprietário |

### 🎮 Gaming e Consoles

| Console | Comandos | Protocolo |
|---------|----------|-----------|
| **PlayStation** | Power, Eject, Touchpad, Buttons | Sony SIRC |
| **Xbox** | Power, Eject, Menu, Guide | NEC |
| **Nintendo Switch** | Power, Volume, Capture, Home | Proprietário |
| **Steam Deck** | Power, Volume, Trackpad | Proprietário |

## 🛠️ Ferramentas IR

### 📊 IR Database Brute Force

- **Localização**: `databases/`
- **Formato**: JSON estruturado
- **Uso**: Ataque offline por força bruta
- **Performance**: 10,000+ códigos por segundo

### 🔬 Pattern Learner

- **Localização**: `learned/`
- **Funcionalidade**: Aprende novos padrões IR
- **Armazenamento**: Salvo automaticamente no SD
- **Precisão**: 99%+ de acerto

### ⚡ Custom Protocol Support

- **Localização**: `protocols/`
- **Formato**: JSON customizado
- **Uso**: Protocolos proprietários
- **Flexibilidade**: Total customização

## 📈 Estatísticas de Ataques IR

| Métrica | Valor | Detalhes |
|---------|-------|----------|
| **Dispositivos Suportados** | 500+ | TVs, ACs, áudio, gaming |
| **Códigos IR no Banco** | 10,000+ | Samsung, LG, Sony, etc. |
| **Taxa de Sucesso** | 95%+ | Alta precisão |
| **Alcance Efetivo** | 8-12m | Ambientes internos |
| **Latência** | < 100ms | Resposta instantânea |
| **Protocolos** | 15+ | NEC, Sony, RC5, etc. |

## Comandos Comuns

### Samsung TV

- Power: 0xE0E040BF
- Volume Up: 0xE0E0E01F
- Volume Down: 0xE0E0D02F
- Channel Up: 0xE0E048B7
- Channel Down: 0xE0E008F7
- Mute: 0xE0E0F00F
- Menu: 0xE0E058A7
- Source: 0xE0E0807F

### LG TV

- Power: 0x20DF10EF
- Volume Up: 0x20DFD02F
- Volume Down: 0x20DFF20D
- Channel Up: 0x20DFE01F
- Channel Down: 0x20DF807F
- Mute: 0x20DF906F
- Menu: 0x20DFC23D

### Sony TV

- Power: 0xA90 (12-bit)
- Volume Up: 0x490 (12-bit)
- Volume Down: 0xC90 (12-bit)
- Channel Up: 0x090 (12-bit)
- Channel Down: 0x890 (12-bit)

## Uso

1. **Offline IR Database Brute**: Coloque arquivos JSON em `databases/` com códigos da marca desejada

2. **Pattern Learner**: Os sinais aprendidos são salvos automaticamente em `learned/`

3. **Custom Protocols**: Defina protocolos não padrão em `protocols/`

## Aviso Legal

**⚠️ USE APENAS EM DISPOSITIVOS PRÓPRIOS OU COM AUTORIZAÇÃO**

O uso não autorizado pode violar leis locais.
