# 📱 Bluetooth Low Energy Avançado - Documentação Completa

> **AVISO LEGAL:** Este documento é fornecido exclusivamente para fins educacionais e de pesquisa em segurança. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.

---

## 📖 Índice

1. [Sniffers BLE](#sniffers-ble)
2. [Ataques BLE](#ataques-ble)
3. [Ataques Avançados](#ataques-avançados)
4. [Casos de Uso](#casos-de-uso)

---

## 🛡️ Sniffers BLE

### Bluetooth Analyzer

**Descrição:** Análise detalhada de pacotes BLE no ambiente.

**O que faz:**
- Escaneia dispositivos BLE continuamente
- Captura advertising data
- Identifica serviços e características
- Analisa manufacturer specific data

**Informações capturadas:**
- MAC Address
- Nome do dispositivo
- RSSI (sinal)
- Appearance
- Service UUIDs
- TX Power
- Manufacturer Data

**Uso:**
1. Menu Bluetooth → BLE Sniffers → Bluetooth Analyzer
2. Visualize dispositivos em tempo real
3. ESC para sair

**Saída:**
```
Dispositivos: 15

AA:BB:CC:DD:EE:FF
  Nome: Samsung TV
  RSSI: -65 dBm
  Services: 0x1800, 0x1801
  Appearance: 0x0080 (TV)

11:22:33:44:55:66
  Nome: JBL Flip 5
  RSSI: -78 dBm
  Services: 0x1812
```

**Aplicação:**
- Reconhecimento de ambiente BLE
- Inventário de dispositivos
- Análise de segurança

---

### Flipper Sniff

**Descrição:** Detecta especificamente dispositivos Flipper Zero.

**O que faz:**
- Identifica padrões de MAC de Flipper
- Detecta firmware versão
- Monitora RSSI
- Alerta em tempo real

**Padrões detectados:**
- Nome: "Flipper", "flip_", etc.
- MAC patterns característicos
- Comportamento de advertising

**Uso:**
1. Menu Bluetooth → BLE Sniffers → Flipper Sniff
2. Aguarde detecção automática

**Saída:**
```
Flippers detectados: 2

Flipper_1337
  MAC: 80:E1:26:XX:XX:XX
  RSSI: -55 dBm
  Firmware: 0.92.3
  Último visto: 2s atrás

flip_DarkSide
  MAC: 80:E1:26:YY:YY:YY
  RSSI: -72 dBm
  Firmware: 0.93.1
```

**Aplicação:**
- Detecção de ferramentas de pentest
- Awareness de segurança
- Identificação de ameaças

---

### Airtag Monitor

**Descrição:** Monitora AirTags e dispositivos FindMy.

**O que faz:**
- Detecta AirTags próximas
- Identifica AirTags desconhecidas (potencial stalking)
- Monitora bateria
- Rastreia movimento

**Informações capturadas:**
- Public Key (para tracking)
- Hash Code
- Status
- Battery Level
- Movement detect

**Uso:**
1. Menu Bluetooth → BLE Sniffers → Airtag Monitor
2. Deixe rodando para monitoramento contínuo

**Saída:**
```
AirTags detectadas: 3

AirTag 1
  Status: Conectada ao dono
  Battery: 85%
  Movimento: Não

AirTag 2 [ALERTA]
  Status: Separada do dono
  Battery: 45%
  Movimento: Sim
  ⚠️ Possível stalking!
```

**Aplicação:**
- Proteção contra stalking
- Localização de AirTags ocultas
- Pesquisa de segurança FindMy

---

### BT Wardrive Continuous

**Descrição:** Wardriving BLE dedicado com GPS.

**O que faz:**
- Escaneia dispositivos BLE continuamente
- Registra coordenadas GPS
- Salva em arquivo para análise
- Export para Wigle.net

**Formato de saída:**
- CSV compatível com Wigle.net
- JSON para análise customizada
- Log completo com timestamps

**Uso:**
1. Conecte módulo GPS
2. Menu Bluetooth → BLE Sniffers → BT Wardrive
3. Dirija/caminhe pelo área
4. Dados salvos automaticamente

**Saída no SD:**
```
wardrive_20260221.csv

MAC,Name,RSSI,Latitude,Longitude,Time,ServiceUUIDs
AA:BB:CC:DD:EE:FF,Samsung TV,-65,-23.5505,-46.6333,2026-02-21T14:30:00,0x1800
11:22:33:44:55:66,JBL Flip,-78,-23.5510,-46.6338,2026-02-21T14:31:00,0x1812
```

**Aplicação:**
- Mapeamento de dispositivos BLE
- Pesquisa de segurança urbana
- Análise de penetração BLE

---

### Flock Sniff

**Descrição:** Detecta dispositivos IoT em flock (grupos).

**O que faz:**
- Identifica dispositivos IoT por tipo
- Agrupa por fabricante
- Detecta redes mesh IoT
- Mapeia topologia

**Dispositivos detectados:**
- Smart bulbs (Philips Hue, LIFX, etc.)
- Smart plugs
- Sensores (temperatura, movimento)
- Smart locks
- Termostatos

**Uso:**
1. Menu Bluetooth → BLE Sniffers → Flock Sniff
2. Visualize dispositivos agrupados

**Saída:**
```
Flocks detectados: 3

Flock 1: Philips Hue
  5 dispositivos
  Tipos: Bulb (3), Motion (1), Dimmer (1)
  RSSI médio: -58 dBm

Flock 2: Xiaomi Mi Home
  8 dispositivos
  Tipos: Temp (4), Motion (2), Plug (2)
  RSSI médio: -72 dBm
```

**Aplicação:**
- Inventário IoT
- Análise de redes smart home
- Identificação de vulnerabilidades

---

### Detect Card Skimmers

**Descrição:** Identifica skimmers de cartão BLE.

**O que faz:**
- Escaneia dispositivos BLE suspeitos
- Identifica padrões de skimmers conhecidos
- Avalia nível de risco
- Alerta em tempo real

**Padrões detectados:**
- Nomes suspeitos ("HC-05", "BT", etc.)
- Comportamento anômalo
- MACs conhecidos de skimmers

**Uso:**
1. Menu Bluetooth → BLE Sniffers → Detect Card Skimmers
2. Escaneie ATMs, bombas de gasolina, etc.
3. Alertas automáticos se detectado

**Saída:**
```
Escaneando por skimmers...

✅ Nenhum skimmer detectado

[Se detectado]
⚠️ ALERTA: Possível skimmer!
  MAC: XX:XX:XX:XX:XX:XX
  Nome: HC-05
  Risco: 85%
  Razão: Nome conhecido de skimmer
```

**Aplicação:**
- Proteção contra fraudes
- Inspeção de ATMs
- Segurança em postos de gasolina

---

## ⚔️ Ataques BLE

### Google BLE Spam

**Descrição:** Spam específico para dispositivos Google.

**Dispositivos afetados:**
- Chromebooks
- Pixel phones
- Android tablets
- Nest devices

**Como funciona:**
- Emula dispositivos Google
- Envia fast pair packets
- Cria notificações falsas
- Sobrecarrega pairing queue

**Uso:**
1. Menu Bluetooth → BLE Spam Avançado → Google BLE Spam
2. ESC para parar

**Saída:**
```
Google BLE Spam ativo
Dispositivos: 50+
Packets/s: 100

Alvos:
- Chromebooks
- Pixel phones
- Android devices
```

**Aplicação:**
- Teste de resistência BLE
- Pesquisa de segurança
- Teste de estabilidade

---

### Flipper BLE Spam

**Descrição:** Spam para dispositivos Flipper Zero.

**Como funciona:**
- Emula múltiplos Flippers
- Cria entradas na lista de dispositivos
- Spam de notificações

**Uso:**
1. Menu Bluetooth → BLE Spam Avançado → Flipper BLE Spam
2. ESC para parar

**Aplicação:**
- Teste de estabilidade Flipper
- Demonstração de vulnerabilidades

---

### Samsung BLE Spam

**Descrição:** Spam para dispositivos Samsung.

**Dispositivos afetados:**
- Galaxy phones
- Galaxy Watch
- Galaxy tablets
- Galaxy Buds

**Como funciona:**
- Emula dispositivos Samsung
- Smart Switch spam
- Quick Share spoofing

**Uso:**
1. Menu Bluetooth → BLE Spam Avançado → Samsung BLE Spam
2. ESC para parar

**Aplicação:**
- Teste de segurança Samsung
- Pesquisa de comportamento BLE

---

### Swift Pair Spam

**Descrição:** Spam via Microsoft Swift Pair.

**Dispositivos afetados:**
- Windows 10/11 PCs
- Surface devices
- Xbox controllers

**Como funciona:**
- Emula dispositivos Swift Pair
- Cria popups de pairing
- Sobrecarrega fila de pairing

**Uso:**
1. Menu Bluetooth → BLE Spam Avançado → Swift Pair Spam
2. ESC para parar

**Aplicação:**
- Teste de segurança Windows
- Demonstração de UX vulnerabilities

---

### BLE Spam All

**Descrição:** Combina todos os tipos de spam BLE.

**Tipos incluídos:**
- Google Fast Pair
- Samsung Smart Switch
- Apple Continuity
- Microsoft Swift Pair
- Flipper detection

**Uso:**
1. Menu Bluetooth → BLE Spam Avançado → BLE Spam All
2. ESC para parar

**Saída:**
```
BLE Spam All ativo

Tipos ativos:
✓ Google Fast Pair
✓ Samsung Smart Switch
✓ Apple Continuity
✓ Microsoft Swift Pair
✓ Flipper Spam

Packets/s: 500
Alvos: Todos dispositivos BLE
```

**Aplicação:**
- Teste de estresse BLE
- Pesquisa abrangente
- Demonstração de ataque

---

## 🎯 Ataques Avançados

### 1. Malformed ATT Fuzzer

**Descrição:** Envia pacotes ATT malformados para crashar stacks BLE.

**O que faz:**
- Gera opcodes ATT inválidos
- Handles inexistents
- Overflow de atributos
- Testa robustez da stack

**Parâmetros:**
```
packets_per_second: 100
randomize_opcode: true
invalid_handles: true
overflow_attributes: true
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Malformed ATT Fuzzer
2. Conecte ao dispositivo alvo
3. Inicie fuzzing

**Aplicação:**
- Fuzzing de stacks BLE
- Pesquisa de vulnerabilidades
- Teste de robustez

---

### 2. GATT Phishing Server

**Descrição:** Emula serviços GATT para phishing.

**Serviços emulados:**
- Keyboard HID
- Printer
- Headset
- Smart Lock
- Fitness Tracker
- Heart Rate Monitor

**Como funciona:**
- Anuncia serviço falso
- Aguarda conexão
- Captura credenciais digitadas
- Registra interações

**Uso:**
1. Menu Bluetooth → Ataques Avançados → GATT Phishing Server
2. Selecione tipo de serviço
3. Aguarde vítimas conectarem

**Saída:**
```
GATT Phishing Server ativo
Tipo: Keyboard HID

Conexões: 3
Dados capturados:
- admin@empresa.com
- senha123
```

**Aplicação:**
- Teste de phishing BLE
- Treinamento de segurança
- Pesquisa de comportamento

---

### 3. Advertising Channel Jammer

**Descrição:** Bloqueia todos os dispositivos BLE na área.

**Como funciona:**
- Flood nos canais 37, 38, 39
- Pacotes de advertising spoofados
- Blackout BLE total

**Configuração:**
```
channel_37: true
channel_38: true
channel_39: true
packets_per_second: 500
randomize_payload: true
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Advertising Channel Jammer
2. Configure canais
3. Inicie jamming

**Aviso:** Afeta TODOS dispositivos BLE na área!

**Aplicação:**
- Teste de resiliência BLE
- Pesquisa de interferência
- Demonstração de DoS

---

### 4. Connection Parameter Drain

**Descrição:** Drena bateria de dispositivos BLE.

**Como funciona:**
- Força intervalos de conexão mínimos
- Solicitações de parâmetro constantes
- Mantém dispositivo ativo
- Drena bateria rapidamente

**Configuração:**
```
min_interval: 6 (7.5ms)
max_interval: 6 (7.5ms)
latency: 0
timeout: 100
aggressive_mode: true
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Connection Parameter Drain
2. Conecte ao dispositivo alvo
3. Inicie ataque

**Dispositivos vulneráveis:**
- AirPods
- Galaxy Buds
- Smartwatches
- Fitness trackers

**Aplicação:**
- Teste de autonomia
- Pesquisa de power management
- Teste de resistência

---

### 5. Mass AirTag Spoof

**Descrição:** Emula centenas de AirTags falsos.

**Como funciona:**
- Gera IDs aleatórios
- Advertising no formato Apple
- FindMy network spam
- Cria confusão no ecossistema Apple

**Configuração:**
```
num_tags: 100
randomize_ids: true
rotate_ids: true
rotation_interval_ms: 1000
use_offline_finding: true
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Mass AirTag Spoof
2. Configure número de tags
3. Inicie spoofing

**Efeito:**
- iPhone detecta múltiplas AirTags
- Alertas constantes de "AirTag desconhecida"
- Sobrecarrega FindMy network

**Aplicação:**
- Teste de escalabilidade FindMy
- Pesquisa de segurança Apple
- Demonstração de DoS

---

### 6. Long-Range Extended Attack

**Descrição:** Ataques BLE em alcance estendido (até 200m).

**Como funciona:**
- Usa PHY codificado (Long Range)
- 125kbps ou 500kbps
- Extended advertising
- Alcance até 4x maior

**Configuração:**
```
use_coded_phy: true
extended_advertising: true
primary_phy: 3 (BLE_HCI_LE_CODED_PHY)
secondary_phy: 3
primary_adv_interval: 1000
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Long-Range Extended Attack
2. Configure parâmetros
3. Inicie ataque

**Aplicação:**
- Ataques à distância
- Teste de segurança perimetral
- Pesquisa de range BLE

---

### 7. Pairing MITM Simulator

**Descrição:** Intercepta processo de pairing BLE.

**Como funciona:**
- Relay em tempo real
- Captura chaves de pairing
- JustWorks exploitation
- MITM entre dispositivos

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Pairing MITM Simulator
2. Posicione entre vítima e dispositivo
3. Inicie interceptação

**Dados capturados:**
- Long Term Key (LTK)
- Identity Address
- IRK (Identity Resolving Key)
- CSRK (Connection Signature Resolving Key)

**Aplicação:**
- Pesquisa de segurança pairing
- Teste de JustWorks
- Demonstração de MITM

---

### 8. Advertisement Stealth Exfil

**Descrição:** Exfiltração de dados via advertising BLE.

**Como funciona:**
- Codifica dados em manufacturer data
- Envia via advertising packets
- Receptor externo decodifica
- Indetectável por firewalls

**Tipos de codificação:**
- Raw (sem codificação)
- Base64
- XOR simples
- Custom encoding

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Advertisement Stealth Exfil
2. Selecione dados para exfiltrar
3. Escolha codificação
4. Inicie transmissão

**Aplicação:**
- Teste de data exfil
- Pesquisa de covert channels
- Demonstração de DLP bypass

---

### 9. Mesh Worm Propagator

**Descrição:** Propaga malware em redes mesh BLE.

**Como funciona:**
- Detecta nós mesh
- Infecta primeiro nó
- Propaga automaticamente
- Cria botnet BLE

**Configuração:**
```
scan_duration_ms: 5000
auto_spread: true
spread_interval_ms: 30000
max_hops: 10
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Mesh Worm Propagator
2. Inicie propagação
3. Monitore infecção

**Aplicação:**
- Pesquisa de segurança mesh
- Teste de resiliência IoT
- Demonstração de worm

---

### 10. Directed Disconnect Flood

**Descrição:** Desconecta dispositivos específicos.

**Como funciona:**
- Conecta ao dispositivo alvo
- Disconnect imediato
- Loop contínuo
- Dispositivo fica inutilizável

**Configuração:**
```
target_addresses: [lista de MACs]
disconnect_per_second: 10
scan_for_targets: true
target_appearance: 0 (qualquer)
```

**Uso:**
1. Menu Bluetooth → Ataques Avançados → Directed Disconnect Flood
2. Selecione alvos ou escaneie
3. Inicie ataque

**Aplicação:**
- Teste de resiliência BLE
- DoS direcionado
- Pesquisa de segurança

---

## 📋 Casos de Uso

### Auditoria de Smart Office

1. **Reconhecimento**
   - Bluetooth Analyzer
   - Flock Sniff
   - BT Wardrive

2. **Análise**
   - Identificar dispositivos IoT
   - Mapear topologia
   - Detectar vulnerabilidades

3. **Teste**
   - GATT Phishing Server
   - Connection Parameter Drain

### Proteção Pessoal

1. **Anti-Stalking**
   - Airtag Monitor
   - Detectar AirTags desconhecidas

2. **Anti-Fraude**
   - Detect Card Skimmers
   - Escanear ATMs e bombas

### Pesquisa de Segurança

1. **Fuzzing**
   - Malformed ATT Fuzzer
   - Advertising Channel Jammer

2. **Análise de Protocolo**
   - Pairing MITM Simulator
   - Long-Range Extended Attack

---

## 🔧 Troubleshooting

### BLE não detecta dispositivos
- Verifique se Bluetooth está ativo
- Reinicie o stack BLE
- Teste em outro ambiente

### Ataques não funcionam
- Verifique compatibilidade do alvo
- Confirme versão do firmware
- Teste conexão manual primeiro

### Performance baixa
- Reduza taxa de pacotes
- Use filtro de dispositivos
- Desative serviços não utilizados

---

## 📚 Referências

- Bluetooth Core Specification 5.0+
- NimBLE Documentation
- BLE Security Guide
- OWASP IoT Top 10