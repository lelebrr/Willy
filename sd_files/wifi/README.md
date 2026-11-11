# 📶 WiFi Resources

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![WiFi](https://img.shields.io/badge/WiFi-802.11%20a/b/g/n/ac/ax-green.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![Blue Team](https://img.shields.io/badge/Blue%20Team-Defensive-blue.svg)
![Forensics](https://img.shields.io/badge/Forensics-Forensic-blue.svg)
![Wardriving](https://img.shields.io/badge/Wardriving-Map-green.svg)
![CVE](https://img.shields.io/badge/CVE-Vulnerabilities-orange.svg)

Recursos para análise e testes de segurança WiFi - Proprietário

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

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes recursos de segurança WiFi são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes recursos podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos recursos
- **Restrições**: Uso permitido apenas em redes autorizadas por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos recursos
- Consequências legais de atividades realizadas com segurança WiFi
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes recursos implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Use com responsabilidade! 🛡️*
