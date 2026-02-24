# IR Attack Files

Este diretório contém arquivos de suporte para os ataques IR avançados do Willy.

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

## Protocolos Suportados

| Protocolo | Bits | Frequência | Fabricantes |
|-----------|------|------------|-------------|
| NEC | 32 | 38 kHz | Samsung, LG, Toshiba |
| Samsung | 32 | 38 kHz | Samsung |
| Sony | 12-20 | 40 kHz | Sony |
| RC5 | 12 | 36 kHz | Philips |
| RC6 | 20 | 36 kHz | Philips, Microsoft |
| LG | 28 | 38 kHz | LG |
| Sharp | 15 | 38 kHz | Sharp |
| Panasonic | 48 | 38 kHz | Panasonic |
| JVC | 16 | 38 kHz | JVC |

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