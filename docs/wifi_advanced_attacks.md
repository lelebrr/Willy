# 📡 WiFi Avançado - Documentação Completa

> **AVISO LEGAL:** Este documento é fornecido exclusivamente para fins educacionais e de pesquisa em segurança. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.

---

## 📖 Índice

1. [Sniffers WiFi](#sniffers-wifi)
2. [Ataques WiFi](#ataques-wifi)
3. [Ataques Avançados](#ataques-avançados)
4. [Configuração](#configuração)
5. [Casos de Uso](#casos-de-uso)

---

## 🛡️ Sniffers WiFi

### Channel Analyzer

**Descrição:** Analisa todos os canais WiFi (1-11 ou 1-14) para identificar interferência e congestionamento.

**O que faz:**
- Varre todos os canais sequencialmente
- Conta APs por canal
- Mede nível de ruído/interferência
- Conta pacotes por canal

**Uso:**
1. Menu WiFi → WiFi Sniffers → Channel Analyzer
2. Aguarde a análise completa
3. Visualize resultados em tempo real

**Saída:**
```
Ch1: 3 APs | -65dBm
Ch6: 8 APs | -45dBm  ← Mais congestionado
Ch11: 5 APs | -55dBm
```

**Aplicação:**
- Identificar melhor canal para AP próprio
- Detectar fontes de interferência
- Planejar posicionamento de equipamentos

---

### Packet Counter

**Descrição:** Contagem em tempo real de pacotes WiFi capturados.

**O que faz:**
- Conta pacotes totais
- Classifica por tipo (Beacon, Probe, Data, Management)
- Calcula taxa de pacotes por segundo

**Uso:**
1. Menu WiFi → WiFi Sniffers → Packet Counter
2. Visualize estatísticas em tempo real
3. ESC para sair

**Saída:**
```
Total: 15,432

Beacons: 3,245
Probes: 1,567
Data: 9,876
Mgmt: 744

Taxa: ~1,543 pkt/s
```

**Aplicação:**
- Monitorar atividade de rede
- Detectar anomalias de tráfego
- Baseline de comportamento

---

### Detect Pwnagotchi

**Descrição:** Identifica dispositivos Pwnagotchi ativos no range.

**O que faz:**
- Monitora todos os canais
- Identifica padrões MAC de Pwnagotchi
- Detecta comportamento característico
- Exibe identidade e RSSI

**Uso:**
1. Menu WiFi → WiFi Sniffers → Detect Pwnagotchi
2. Aguarde detecção automática
3. Visualize Pwnagotchis encontrados

**Saída:**
```
Pwnagotchis detectados: 2

pwnagotchi_1337
  MAC: DE:AD:BE:EF:13:37
  Ch: 6 RSSI: -67

pwnagotchi_42
  MAC: DE:AD:BE:CA:FE:42
  Ch: 1 RSSI: -78
```

**Aplicação:**
- Detectar pentesters usando Pwnagotchi
- Identificar redes em teste
- Awareness de segurança

---

### Scan All Networks

**Descrição:** Escaneamento completo de redes WiFi com informações detalhadas.

**O que faz:**
- Scan ativo + passivo
- Detecta redes ocultas
- Identifica tipo de criptografia
- Destaca redes abertas e WPA3

**Uso:**
1. Menu WiFi → WiFi Sniffers → Scan All
2. Aguarde escaneamento
3. Navegue pelos resultados

**Saída:**
```
Redes encontradas: 12

[OPEN] Hotel_Guest     -65dBm Ch6
[WPA3] Corporate_WiFi  -58dBm Ch36
[WPA2] Home_Network    -72dBm Ch1
[HIDDEN] <Hidden>      -80dBm Ch11
```

**Aplicação:**
- Reconhecimento de redes
- Identificação de alvos
- Detecção de rogue APs

---

### SAE Commit Capture

**Descrição:** Captura commits SAE (Simultaneous Authentication of Equals) para redes WPA3.

**O que faz:**
- Monitora handshakes WPA3
- Captura commits de autenticação
- Armazena client/AP MAC
- Registra timestamp

**Uso:**
1. Menu WiFi → WiFi Sniffers → SAE Commit Capture
2. Deixe rodando enquanto dispositivos conectam
3. Commits são capturados automaticamente

**Saída:**
```
Commits capturados: 3

Último commit:
Cliente: AA:BB:CC:DD:EE:FF
AP: 11:22:33:44:55:66
Len: 64 bytes
```

**Aplicação:**
- Pesquisa de segurança WPA3
- Captura para análise offline
- Teste de implementação SAE

---

### Ping Scan

**Descrição:** Descoberta de hosts ativos via ICMP ping.

**O que faz:**
- Varre range completo da rede local
- Envia ICMP echo requests
- Lista hosts que respondem
- Mostra progresso em tempo real

**Uso:**
1. Conecte-se a uma rede WiFi
2. Menu WiFi → WiFi Sniffers → Ping Scan
3. Aguarde varredura completa

**Saída:**
```
Gateway: 192.168.1.1
Varrendo: 254/254
Hosts ativos: 23

192.168.1.1
192.168.1.10
192.168.1.15
192.168.1.100
...
```

**Aplicação:**
- Mapeamento de rede
- Inventário de dispositivos
- Detecção de hosts ativos

---

### ARP Scan

**Descrição:** Descoberta de hosts via ARP requests.

**O que faz:**
- Envia ARP requests para todos os IPs
- Captura respostas ARP
- Mais rápido que ping
- Detecta hosts com firewall

**Uso:**
1. Conecte-se a uma rede WiFi
2. Menu WiFi → WiFi Sniffers → ARP Scan
3. Aguarde varredura

**Saída:**
```
Progresso: 254/254
Hosts: 28

192.168.1.1
192.168.1.5
192.168.1.12
...
```

**Aplicação:**
- Descoberta rápida de hosts
- Detecção de hosts silenciosos
- Mapeamento de rede

---

### Port Scan All

**Descrição:** Varredura de portas comuns em um host específico.

**O que faz:**
- Varre 22 portas comuns
- Identifica serviços
- Mostra portas abertas
- Classifica por serviço

**Portas varridas:**
| Porta | Serviço | Porta | Serviço |
|-------|---------|-------|---------|
| 21 | FTP | 443 | HTTPS |
| 22 | SSH | 445 | SMB |
| 23 | Telnet | 1433 | MSSQL |
| 25 | SMTP | 1521 | Oracle |
| 53 | DNS | 3306 | MySQL |
| 80 | HTTP | 3389 | RDP |
| 110 | POP3 | 5432 | PostgreSQL |
| 143 | IMAP | 5900 | VNC |
| 993 | IMAPS | 6379 | Redis |
| 995 | POP3S | 8080 | HTTP-Alt |
| - | - | 27017 | MongoDB |

**Uso:**
1. Conecte-se a uma rede WiFi
2. Menu WiFi → WiFi Sniffers → Port Scan
3. Digite o IP alvo
4. Aguarde varredura

**Saída:**
```
Alvo: 192.168.1.100
Progresso: 22/22
Portas abertas: 4

22 SSH
80 HTTP
443 HTTPS
3306 MySQL
```

**Aplicação:**
- Identificação de serviços
- Avaliação de superfície de ataque
- Pentest inicial

---

### SSH Scan

**Descrição:** Varredura específica para servidores SSH (porta 22).

**O que faz:**
- Varre toda a rede na porta 22
- Lista servidores SSH ativos
- Identifica hosts com SSH exposto

**Uso:**
1. Conecte-se a uma rede WiFi
2. Menu WiFi → WiFi Sniffers → SSH Scan
3. Aguarde varredura

**Saída:**
```
Progresso: 254/254
SSH servers: 3

192.168.1.10
192.168.1.50
192.168.1.100
```

**Aplicação:**
- Identificar servidores SSH
- Auditoria de segurança
- Inventário de infraestrutura

---

### Telnet Scan

**Descrição:** Varredura específica para servidores Telnet (porta 23).

**O que faz:**
- Varre toda a rede na porta 23
- Identifica dispositivos com Telnet exposto
- Potenciais dispositivos IoT vulneráveis

**Uso:**
1. Conecte-se a uma rede WiFi
2. Menu WiFi → WiFi Sniffers → Telnet Scan
3. Aguarde varredura

**Aplicação:**
- Identificar dispositivos IoT
- Detectar configurações inseguras
- Auditoria de segurança

---

## ⚔️ Ataques WiFi

### Probe Request Flood

**Descrição:** Envia probe requests aleatórios em massa.

**O que faz:**
- Gera SSIDs aleatórios
- Randomiza MAC addresses
- Flood de probe requests
- Testa capacidade de processamento de APs

**Uso:**
1. Menu WiFi → WiFi Attacks → Probe Req Flood
2. ESC para parar

**Saída:**
```
Probe requests: 45,678
Taxa: alta

Redes visíveis no ambiente
```

**Aplicação:**
- Teste de estabilidade de APs
- Pesquisa de segurança
- Detecção de WIDS

---

### AP Clone Spam

**Descrição:** Clona SSIDs de redes existentes e envia beacons falsos.

**O que faz:**
- Escaneia redes próximas
- Clona até 10 SSIDs
- Envia beacons spoofados
- Rota entre SSIDs clonados

**Uso:**
1. Menu WiFi → WiFi Attacks → AP Clone Spam
2. ESC para parar

**Saída:**
```
Beacons: 123,456
Clones ativos: 8

Redes clonadas:
  Home_Network
  Corporate_WiFi
  Guest_WiFi
  ...
```

**Aplicação:**
- Teste de rogue AP detection
- Pesquisa de segurança
- Treinamento de equipe de segurança

---

### Karma Attack

**Descrição:** Responde a probe requests com SSIDs atrativos.

**O que faz:**
- Monitora probe requests
- Responde com probe responses
- Usa SSIDs populares (Starbucks, Free WiFi)
- Atrai dispositivos a conectar

**SSIDs atrativos padrão:**
- Starbucks
- Airport WiFi
- Hotel Guest
- Free WiFi
- xfinitywifi
- attwifi

**Uso:**
1. Menu WiFi → WiFi Attacks → Karma Attack
2. ESC para parar

**Aplicação:**
- Teste de segurança de clientes
- Pesquisa de comportamento de dispositivos
- Detecção de configurações vulneráveis

---

### Bad Message Attack

**Descrição:** Envia frames WiFi malformados.

**O que faz:**
- Gera frames com tipos inválidos
- Payloads corrompidos
- Testa robustez de implementações

**Aviso:** Pode causar crash em dispositivos vulneráveis!

**Uso:**
1. Menu WiFi → WiFi Attacks → Bad Message
2. ESC para parar

**Aplicação:**
- Fuzzing de stacks WiFi
- Teste de robustez
- Pesquisa de vulnerabilidades

---

### Sleep Attack

**Descrição:** Impede dispositivos de entrar em modo sleep.

**O que faz:**
- Envia frames com flag MoreData
- Mantém clientes acordados
- Drena bateria de dispositivos móveis

**Uso:**
1. Menu WiFi → WiFi Attacks → Sleep Attack
2. ESC para parar

**Aplicação:**
- Teste de autonomia de bateria
- Pesquisa de power management
- Teste de resistência

---

### SAE Commit Flood

**Descrição:** Flood de commits SAE para redes WPA3.

**O que faz:**
- Envia authentication frames SAE
- Testa capacidade de processamento
- Foco em WPA3-Enterprise

**Uso:**
1. Menu WiFi → WiFi Attacks → SAE Commit Flood
2. ESC para parar

**Aplicação:**
- Teste de implementação WPA3
- Pesquisa de segurança
- Fuzzing de handshake SAE

---

## 🎯 Ataques Avançados

### 1. Dynamic Beacon Fuzzer

**Descrição:** Envia beacons malformados para testar crash de roteadores.

**Parâmetros:**
- Packets por segundo: 1000
- Randomize SSID: Sim/Não
- Randomize MAC: Sim/Não
- Malformed IE: Sim/Não
- Canal alvo: 0 (todos) ou específico

**Funcionamento:**
- Gera beacons com IE fields inválidos
- Testa parsing de frames
- Identifica implementações vulneráveis

---

### 2. Client Battery Drain

**Descrição:** PS-Poll spoofing para drenar bateria de clientes.

**Como funciona:**
- Monitora clientes conectados
- Envia PS-Polls spoofados
- Força cliente a permanecer ativo
- Drena bateria rapidamente

**Uso:**
1. Menu WiFi → Ataques Avançados → Client Battery Drain
2. Selecione alvos ou deixe em branco para todos

---

### 3. Visual WiFi Heatmap

**Descrição:** Mapa de calor visual de redes WiFi.

**Funcionalidades:**
- Visualização em tempo real
- RSSI por posição
- Múltiplos APs
- Export de dados

**Uso:**
1. Menu WiFi → Ataques Avançados → Visual WiFi Heatmap
2. Mova pelo ambiente
3. Visualize intensidade de sinal

---

### 4. WPA3 Downgrade + MITM

**Descrição:** Força downgrade de WPA3 para WPA2 e executa MITM.

**Como funciona:**
- Spoofa beacon sem PMF/SAE
- Cliente conecta em WPA2
- Evil Portal integrado
- Captura handshake

**Uso:**
1. Menu WiFi → Ataques Avançados → WPA3 Downgrade
2. Configure SSID alvo
3. Aguarde clientes conectarem

---

### 5. IoT Exploit Injector

**Descrição:** Detecta IoT por MAC e injeta exploits conhecidos.

**Funcionamento:**
- Identifica fabricante por OUI
- Carrega exploits do SD
- Injeta payloads específicos
- Testa vulnerabilidades conhecidas

**Exploits incluídos:**
- Câmeras IP
- Smart bulbs
- Smart plugs
- Roteadores

---

### 6. Mesh Disruptor

**Descrição:** Quebra redes mesh 802.11s.

**Como funciona:**
- Detecta redes mesh
- Flood de path-request frames
- Sobrecarrega roteamento mesh
- Quebra conectividade

---

### 7. Smart Deauth Scheduler

**Descrição:** Deauth programado por horário ou condição.

**Configuração:**
- Horário de início/fim
- Intervalo entre bursts
- Pacotes por burst
- Script JS personalizado

**Uso:**
1. Menu WiFi → Ataques Avançados → Smart Deauth Scheduler
2. Configure horários e condições
3. Inicie agendamento

---

### 8. Encrypted Traffic Fingerprint

**Descrição:** Identifica OS/dispositivo por padrões de tráfego criptografado.

**O que detecta:**
- Windows, macOS, Linux
- iOS, Android
- Dispositivos IoT
- Aplicações específicas

**Métricas:**
- Tamanho médio de pacotes
- Intervalo entre pacotes
- Variância
- Padrões característicos

---

### 9. Session Hijack Portal

**Descrição:** Portal que rouba sessões ativas.

**Como funciona:**
- Evil Portal ativo
- JS injection no tráfego
- Captura cookies
- Captura tokens de sessão

**Uso:**
1. Menu WiFi → Ataques Avançados → Session Hijack Portal
2. Vítima conecta ao portal
3. Sessões são capturadas

---

### 10. Management Frame Bypass Flood

**Descrição:** Flood de management frames abaixo do radar WIDS.

**Configuração:**
- Frames por segundo: 50 (abaixo do threshold)
- Randomize MAC: Sim
- Rotate channels: Sim
- Frame types: Bitmask

**Tipos de frames:**
- Beacon (bit 0)
- Probe (bit 1)
- Auth (bit 2)
- Assoc (bit 3)

---

## ⚙️ Configuração

### MAC Spoofing

**Como usar:**
1. Menu WiFi → Config → Mudar MAC
2. Digite novo MAC ou use random
3. Reinicie WiFi

### Evil WiFi Config

**Opções:**
- Modo Senha: Aberto/Fechado
- Endpoint /creds: Personalizável
- Endpoint /ssid: Personalizável
- Mostrar endpoints: Sim/Não

### Redes Ocultas

**Toggle:**
1. Menu WiFi → Config → Redes Ocultas
2. Ativa/Desativa inclusão de redes ocultas no scan

---

## 📋 Casos de Uso

### Pentest Corporativo

1. **Reconhecimento**
   - Scan All Networks
   - Detect Pwnagotchi
   - Channel Analyzer

2. **Mapeamento**
   - Ping Scan
   - ARP Scan
   - Port Scan

3. **Ataques**
   - WPA3 Downgrade
   - Session Hijack Portal

### Auditoria WiFi

1. **Análise de Ambiente**
   - Channel Analyzer
   - Packet Counter
   - Visual Heatmap

2. **Identificação de Problemas**
   - Canais congestionados
   - Interferência
   - Rogue APs

### Pesquisa de Segurança

1. **Fuzzing**
   - Dynamic Beacon Fuzzer
   - Bad Message Attack
   - Malformed ATT Fuzzer

2. **Análise de Tráfego**
   - Encrypted Traffic Fingerprint
   - SAE Commit Capture
   - Packet Counter

---

## 🔧 Troubleshooting

### WiFi não conecta
- Verifique se senha está correta
- Tente canal específico
- Reinicie o dispositivo

### Ataques não funcionam
- Verifique se WiFi está em modo correto
- Confirme suporte do hardware
- Teste em outro ambiente

### Performance baixa
- Reduza taxa de pacotes
- Use canal específico
- Desative serviços não utilizados

---

## 📚 Referências

- IEEE 802.11 Standard
- WPA3 Specification
- WiFi Alliance Documentation
- Aircrack-ng Documentation