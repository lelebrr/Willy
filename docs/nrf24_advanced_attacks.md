# NRF24 Attacks - Documentação Completa

## ⚠️ AVISO LEGAL

**Este código é fornecido EXCLUSIVAMENTE para fins educacionais e de pesquisa em segurança.**

O uso não autorizado destas ferramentas pode:
- Violar leis federais e estaduais
- Interferir em comunicações legítimas (jamming é ilegal em muitas jurisdições)
- Resultar em penalidades civis e criminais

**Use apenas em:**
- Dispositivos próprios e ambientes de laboratório
- Testes de penetração com autorização por escrito

---

## Índice

### Funcionalidades NRF24 Básicas
1. [Espectro (Spectrum Analyzer)](#1-espectro-spectrum-analyzer)
2. [Jammer NRF (Carrier Jammer)](#2-jammer-nrf-carrier-jammer)
3. [Salto de Canal (Channel Hopper)](#3-salto-de-canal-channel-hopper)

### Ataques NRF24 Avançados
4. [MouseJack Keystroke Injector](#4-mousejack-keystroke-injector)
5. [NRF Protocol Smart Fuzzer](#5-nrf-protocol-smart-fuzzer)
6. [Multi-Pipe Device Fingerprint](#6-multi-pipe-device-fingerprint)
7. [Adaptive Targeted Jammer](#7-adaptive-targeted-jammer)
8. [Keystroke Sequence Replay](#8-keystroke-sequence-replay)
9. [Device Impersonation MITM](#9-device-impersonation-mitm)
10. [Logitech Unifying Exploit Suite](#10-logitech-unifying-exploit-suite)
11. [NRF Data Stealth Exfil](#11-nrf-data-stealth-exfil)
12. [Mass Pairing Brute](#12-mass-pairing-brute)
13. [Signal Strength Geo-Locator](#13-signal-strength-geo-locator)

---

# FUNCIONALIDADES NRF24 BÁSICAS

## 1. Espectro (Spectrum Analyzer)

### O que faz
Transforma o Willy em um analisador de espectro para a banda de 2.4GHz, visualizando a atividade de rádio em tempo real.

### Como funciona
O NRF24L01 possui um detector de portadora (RPD - Received Power Detector). O Willy varre rapidamente os 126 canais possíveis e desenha um gráfico de barras no TFT representando a intensidade do sinal em cada frequência.

### Como Usar
1. **Menu -> NRF24 -> Espectro**
2. O gráfico começará a atualizar.
3. Picos constantes indicam Wi-Fi ou dispositivos Bluetooth.
4. Pressione **ESC** para sair.

---

## 2. Jammer NRF (Carrier Jammer)

### O que faz
Bloqueia comunicações NRF24 em um ou mais canais transmitindo uma portadora constante (Continuous Carrier).

### Como funciona
Coloca o rádio em modo de transmissão contínua sem modulação. Isso causa interferência destrutiva, impedindo que outros dispositivos NRF24 (como mouses sem fio ou sensores) consigam "ouvir" seus próprios pacotes.

### Como Usar
1. **Menu -> NRF24 -> Jammer NRF** (ou Jammer Canal para um canal fixo)
2. O rádio comecará a transmitir.
3. Pressione **ESC** para parar.

---

# ATAQUES NRF24 AVANÇADOS

## 4. MouseJack Keystroke Injector

### O que faz
Injeta teclas (keystrokes) em computadores através de receptores de mouses sem fio vulneráveis.

### Como funciona
Explora a falta de criptografia ou falhas na verificação de sequência em protocolos proprietários baseados em NRF24 (Enhanced ShockBurst). O Willy envia pacotes formatados como se fossem de um teclado legítimo.

### Como Usar
1. **Ataques NRF24 -> MouseJack Inject**
2. O Willy varre endereços comuns de fabricantes conhecidos.
3. Se um receptor responder, ele tenta injetar um payload (ex: abrir terminal e executar comando).

---

## 5. NRF Protocol Smart Fuzzer

### O que faz
Envia pacotes propositalmente malformados ou com campos aleatórios para travar ou causar comportamento inesperado em dispositivos NRF24.

### Como funciona
Varre canais e endereços, enviando payloads com tamanhos e conteúdos randômicos, ignorando regras do protocolo ESB para testar a robustez do parsing do alvo.

---

## 6. Multi-Pipe Device Fingerprint

### O que faz
Identifica a presença e o tipo de dispositivos NRF24 próximos sem a necessidade de pareamento.

### Como funciona
Coloca o rádio em modo de monitoramento com largura de endereço curta (2 bytes) e desabilita o CRC. Isso permite capturar o "ruído" de pacotes legítimos e identificar padrões de endereçamento e timing que revelam o tipo de dispositivo (ex: Microsoft Mouse, Logitech, etc).

---

## 7. Adaptive Targeted Jammer

### O que faz
Um jammer inteligente que só ataca quando detecta atividade em um canal específico.

### Como funciona
O Willy monitora silenciosamente os canais. Assim que a energia do sinal (RPD) sobe acima de um limite (indicando transmissão ativa), ele ativa o jammer instantaneamente naquele canal exato por alguns milissegundos, economizando energia e sendo mais difícil de detectar.

---

## 8. Keystroke Sequence Replay

### O que faz
Captura uma sequência de comandos (ex: uma senha sendo digitada) e permite reproduzi-la (replay) posteriormente.

### Como funciona
Grava os pacotes brutos recebidos em um buffer. Ao acionar o replay, o Willy retransmite exatamente os mesmos pacotes com os mesmos intervalos de tempo entre eles.

---

## 9. Device Impersonation MITM

### O que faz
Assume a identidade de um dispositivo confiável para enganar o receptor/host.

### Como funciona
Clona o endereço MAC de um dispositivo alvo e responde a solicitações do receptor antes que o dispositivo real consiga, podendo enviar dados falsos ou comandos de controle.

---

## 10. Logitech Unifying Exploit Suite

### O que faz
Conjunto de ataques específicos para receptores Logitech Unifying (largamente utilizados em mouses e teclados corporativos).

### Como funciona
Explora vulnerabilidades conhecidas (como a falta de criptografia em frames de pareamento ou injeção de teclas em canais de mouse) específicas da stack Logitech.

---

## 11. NRF Data Stealth Exfil

### O que faz
Exfiltra dados do Willy para outro dispositivo de forma oculta.

### Como funciona
Transmite dados em canais de alta frequência raramente usados e com potência de transmissão reduzida, dificultando a detecção por sistemas de monitoramento de rádio (WIDS).

---

## 12. Mass Pairing Brute

### O que faz
Tenta forçar o pareamento com diversos receptores próximos simultaneamente.

### Como funciona
Inunda os canais de pareamento padrão com solicitações contínuas usando IDs de dispositivos aleatórios, explorando receptores que entram em modo de pareamento automático ou têm janelas de pareamento abertas.

---

## 13. Signal Strength Geo-Locator

### O que faz
Ajuda a localizar fisicamente a fonte de um sinal NRF24 (como um rastreador ou um mouse escondido).

### Como funciona
Exibe um gráfico de barras de alta sensibilidade baseado no RSSI/RPD. À medida que você se aproxima da fonte, as barras sobem, permitindo triangulação visual.
