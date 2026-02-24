# 🐙 Pwnagotchi Resources

Arquivos de configuração e recursos para Pwnagotchi.

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

### Integração com Bruce

O Bruce pode:
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
- [Discord](https://discord.gg/pwnagotchi)

---

*Happy Hacking! 🐙*