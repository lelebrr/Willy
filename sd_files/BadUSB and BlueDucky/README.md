# ⌨️ BadUSB / Ducky Script Payloads

Payloads HID para automação e testes de segurança.

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