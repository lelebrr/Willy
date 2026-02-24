# 📶 WiFi Resources

Recursos para análise e testes de segurança WiFi.

---

## ⚠️ Aviso Legal

**USE APENAS EM REDES AUTORIZADAS!** Interferir em redes WiFi sem autorização é crime.

---

## 📁 Estrutura

```
wifi/
├── exploits/              # Banco de dados de CVEs
│   ├── router_exploits.json
│   ├── camera_exploits.json
│   ├── wpa3_exploits.json
│   ├── smart_home_exploits.json
│   ├── printer_exploits.json
│   ├── nas_exploits.json
│   ├── wps_exploits.json
│   └── cve_2025_2026.json
├── evil portal/           # Templates de evil portal
└── portals/               # Portais captive
```

---

## 📖 O Que Cada Pasta Contém

### 📁 exploits/

Arquivos JSON com bancos de dados de vulnerabilidades organizados por tipo de dispositivo.

#### Como Usar

1. **Consulta Manual**
   ```bash
   cat router_exploits.json | jq '.exploits[] | select(.vendor=="TP-Link")'
   ```

2. **Integração com Ferramentas**
   - Use os dados para automatizar scans
   - Identifique CVEs por vendor/modelo
   - Referência para exploits

#### Arquivos Disponíveis

| Arquivo | Conteúdo | CVEs |
|---------|----------|------|
| `router_exploits.json` | TP-Link, D-Link, Netgear, ASUS, MikroTik | 6+ |
| `camera_exploits.json` | Hikvision, Dahua, Foscam, Reolink | 6+ |
| `wpa3_exploits.json` | Dragonblood, SAE, OWE attacks | 5+ |
| `smart_home_exploits.json` | Philips Hue, Nest, Echo, Ring | 8+ |
| `printer_exploits.json` | HP, Canon, Epson, Brother | 8+ |
| `nas_exploits.json` | Synology, QNAP, WD, Buffalo | 8+ |
| `wps_exploits.json` | WPS PIN, Pixie Dust | 4+ |
| `cve_2025_2026.json` | CVEs mais recentes 2025-2026 | 37+ |

---

### 📁 evil portal/

Templates HTML para evil twin attacks e captive portals.

#### Como Usar

1. Inicie o Evil Portal no Willy
2. Selecione o template HTML
3. Aguarde vítimas conectarem
4. Capture credenciais

#### Templates Disponíveis

| Arquivo | Uso |
|---------|-----|
| `wifi_connect.html` | Portal genérico de WiFi |
| `windows_update.html` | Fake atualização Windows |

---

### 📁 portals/

Portais captive para diferentes cenários.

---

## 💡 Cenários de Uso

### 1. Pentest Corporativo

```bash
# 1. Consultar CVEs do roteador
cat exploits/router_exploits.json

# 2. Verificar vulnerabilidades
nmap --script vuln <target>

# 3. Explorar se vulnerável
# Use exploit apropriado
```

### 2. Red Team

```bash
# 1. Configurar Evil Portal
# 2. Usar template apropriado
# 3. Aguardar conexões
# 4. Documentar descobertas
```

### 3. Bug Bounty

```bash
# Consultar CVEs conhecidos
# Verificar se alvo é vulnerável
# Reportar se encontrado
```

---

## 🔧 Formato dos Arquivos JSON

```json
{
  "exploits": [
    {
      "name": "Nome do Exploit",
      "cve": "CVE-XXXX-XXXXX",
      "date": "YYYY-MM",
      "vendor": "Fabricante",
      "device": "Modelo",
      "type": "RCE/Auth Bypass/DoS",
      "port": 80,
      "path": "/vulnerable/path",
      "severity": "Critical/High/Medium",
      "description": "Descrição da vulnerabilidade"
    }
  ]
}
```

---

## 🛡️ Mitigação

### Para Roteadores
- Atualizar firmware regularmente
- Desabilitar WPS
- Usar WPA3
- Alterar credenciais padrão

### Para IoT
- Segmentar rede IoT
- Atualizar dispositivos
- Usar firewall

### Para NAS
- Não expor à internet
- Atualizar DSM/QTS
- Usar VPN para acesso remoto

---

## 📚 Referências

- [CVE Details](https://www.cvedetails.com/)
- [NIST NVD](https://nvd.nist.gov/)
- [Exploit-DB](https://www.exploit-db.com/)
- [RouterPasswords](https://routerpasswords.com/)

---

*Use com responsabilidade! 🛡️*