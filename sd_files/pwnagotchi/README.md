# 🐙 Pwnagotchi Resources

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.9+-yellow.svg)
![WiFi](https://img.shields.io/badge/WiFi-802.11%20a/b/g/n/ac/ax-green.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![Wardriving](https://img.shields.io/badge/Wardriving-Map-green.svg)
![AI](https://img.shields.io/badge/AI-Machine%20Learning-blue.svg)

Arquivos de configuração e recursos para Pwnagotchi - Proprietário

---

## 📖 O Que é Pwnagotchi?

Pwnagotchi é um "pet" cibernético baseado em IA que aprende a capturar handshakes WiFi. Ele usa reinforcement learning para otimizar ataques de captura de handshakes.

---

## 📁 Arquivos Disponíveis

| Arquivo | Descrição |
|---------|-----------|
| `config.toml` | Configuração principal do Pwnagotchi |
| `custom_faces.txt` | Emoticons customizados para display |
| `pwngridspam.txt` | Spam para PwnGRID |

---

## 🚀 Como Usar

### 1. Configuração Básica

```bash
# Copiar configuração para o Pwnagotchi
sudo cp config.toml /etc/pwnagotchi/config.toml

# Reiniciar serviço
sudo systemctl restart pwnagotchi
```

### 2. Faces Customizadas

As faces são exibidas no display do Pwnagotchi dependendo do estado:

```
(◕‿◕)  - Happy (capturou handshake)
ಠ_ಠ    - Bored (nada acontecendo)
(⌐■_■) - Cool (alguma conquista)
```

---

## ⚙️ Configuração Principal

### config.toml

```toml
[main]
name = "seu_pwnagotchi"
lang = "en"
autoupdate = true

[ui]
display = true
fps = 0
rotation = 180

[ai]
enabled = true
path = "/root/brain.nn"

[bettercap]
handshakes_dir = "/root/handshakes"
interface = "wlan0"

[wifi]
channels = [1, 6, 11]

[personality]
deauth = true
associate = true
```

---

## 📊 Estatísticas

### Ver Status

```bash
# Status do serviço
sudo systemctl status pwnagotchi

# Handshakes capturados
ls /root/handshakes/

# Logs
tail -f /var/log/pwnagotchi.log
```

### Acessar Web UI

```
http://pwnagotchi.local:8080/
```

---

## 🔧 Plugins Úteis

### Instalar Plugins

```bash
# Lista de plugins disponíveis
ls /usr/local/share/pwnagotchi/available-plugins/

# Ativar plugin
sudo pwnagotchi plugins enable <plugin>
```

### Plugins Recomendados

| Plugin | Função |
|--------|--------|
| `grid` | Compartilha com PwnGRID |
| `bt-tether` | Tethering Bluetooth |
| `gps` | Adiciona GPS |
| `webcfg` | Configuração web |
| `memtemp` | Temperatura CPU |
| `wpa-sec` | Upload para wpa-sec |

---

## 💡 Dicas

### Melhorar Capturas

1. **Posicionamento**: Áreas com muito tráfego
2. **Antena**: Use antena externa de 5dBi+
3. **Canais**: Configure canais do seu país
4. **Bateria**: Use power bank de 10.000mAh+

### Integração com Willy

O Willy pode:

- Detectar Pwnagotchis próximos
- Analisar handshakes capturados
- Usar como extensão de captura

---

## 🛠️ Troubleshooting

### Problemas Comuns

| Problema | Solução |
|----------|---------|
| Display não funciona | Verifique conexão SPI |
| Não captura handshakes | Verifique modo monitor |
| IA não aprende | Reset brain: `rm /root/brain.nn` |
| WiFi não funciona | Verifique interface wlan0 |

---

## 📚 Referências

- [Pwnagotchi Oficial](https://pwnagotchi.ai/)
- [Documentação](https://github.com/evilsocket/pwnagotchi)
- [Plugins](https://github.com/evilsocket/pwnagotchi-plugins)

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes arquivos de configuração e recursos Pwnagotchi são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes recursos podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos recursos
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos recursos
- Consequências legais de atividades realizadas com Pwnagotchi
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes recursos implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Happy Hacking! 🐙*
