# ⌨️ BadUSB / Ducky Script Payloads

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![USB](https://img.shields.io/badge/USB-2.0/3.0-blue.svg)
![HID](https://img.shields.io/badge/HID-Attack-orange.svg)

Payloads HID para automação e testes de segurança - Proprietário

---

## ⚠️ Aviso Legal

**USE APENAS EM SISTEMAS AUTORIZADOS!** O uso indevido pode ser crime.

---

## 📖 O Que é BadUSB?

BadUSB é uma técnica onde o dispositivo se apresenta como teclado/mouse USB, permitindo executar comandos automaticamente no computador alvo.

---

## 🚀 Como Usar

### 1. Copie o payload para o SD card

### 2. Conecte o Willy ao computador via USB

### 3. Execute o payload

```
Menu → BadUSB → Selecione arquivo
```

---

## 📁 Payloads Disponíveis

### Windows

| Arquivo | Função |
|---------|--------|
| `windows_recon.txt` | Reconhecimento do sistema |
| `windows_wifi_grab.txt` | Captura senhas WiFi salvas |
| `windows_reverse_shell.txt` | Abre reverse shell |
| `windows_download_exec.txt` | Download e executa payload |
| `windows_privilege_check.txt` | Verifica privilégios |
| `windows_clipboard_grab.txt` | Captura clipboard |
| `windows_add_admin.txt` | Cria usuário admin |
| `windows_message_box.txt` | Exibe mensagem |

### Linux

| Arquivo | Função |
|---------|--------|
| `linux_recon.txt` | Reconhecimento do sistema |

### macOS

| Arquivo | Função |
|---------|--------|
| `macos_recon.txt` | Reconhecimento do sistema |

### Universal

| Arquivo | Função |
|---------|--------|
| `rickroll.txt` | Rickroll clássico |

---

## 📝 Sintaxe Ducky Script

| Comando | Descrição |
|---------|-----------|
| `DELAY n` | Aguarda n milissegundos |
| `STRING texto` | Digita texto |
| `ENTER` | Tecla Enter |
| `TAB` | Tecla Tab |
| `GUI` | Tecla Windows/Command |
| `CTRL` | Tecla Control |
| `ALT` | Tecla Alt |
| `SHIFT` | Tecla Shift |
| `DOWN/UP/LEFT/RIGHT` | Setas |
| `REM` | Comentário |

---

## 📚 Referências

- [Hak5 Ducky Script](https://docs.hak5.org/)
- [USB Rubber Ducky](https://shop.hak5.org/products/usb-rubber-ducky)

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes payloads e scripts são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes scripts podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos scripts
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos scripts
- Consequências legais de atividades realizadas com os payloads
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes scripts implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.
