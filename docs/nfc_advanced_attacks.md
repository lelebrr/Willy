# 🔖 NFC / RFID Avançado - Documentação Completa

> **AVISO LEGAL:** Este documento é fornecido exclusivamente para fins educacionais e de pesquisa em segurança. O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.

---

## 📖 Índice

1. [Ataques Avançados (v2.0)](#-ataques-avançados-v20)
2. [Hardware PN532](#hardware-pn532)
3. [Protocolos Suportados](#protocolos-suportados)

---

## 🚀 Ataques Avançados (v2.0)

Esta seção detalha os 12 novos ataques NFC implementados na suíte Bruce para o módulo PN532.

### 1. Real-Time NFC Relay MITM
**Uso:** Menu RFID → NFC Relay MITM
- **Descrição:** Intercepta e encaminha comunicações entre uma tag e um leitor em tempo real.
- **Funcionamento:** O Bruce atua como um "proxy", lendo dados de uma tag legítima e repetindo-os para um leitor, ou vice-versa, permitindo ataques de distância e análise de tráfego (Man-in-the-Middle).

### 2. APDU Command Fuzzer
**Uso:** Menu RFID → APDU Fuzzer
- **Descrição:** Fuzzing de comandos APDU (Application Protocol Data Unit).
- **Funcionamento:** Envia variações e comandos corrompidos para dispositivos inteligentes (cartões de crédito, passaportes) para identificar comportamentos inesperados ou falhas de segurança no firmware do cartão.

### 3. Dynamic UID Collision Attack
**Uso:** Menu RFID → UID Collision
- **Descrição:** Simulação de múltiplos cartões simultâneos.
- **Funcionamento:** Alterna rapidamente entre diferentes UIDs durante a fase de anti-colisão do leitor, tentando causar falhas em sistemas de contagem ou validar múltiplas identidades em um curto espaço de tempo.

### 4. NDEF Stealth Exfil
**Uso:** Menu RFID → NDEF Exfil
- **Descrição:** Exfiltração de dados via emulação de registros NDEF.
- **Funcionamento:** Emula uma tag NFC com dados maliciosos ou links de exfiltração escondidos em registros NDEF que são processados automaticamente por smartphones (Android/iOS).

### 5. MIFARE Rainbow Table On-Device
**Uso:** Menu RFID → MIFARE Rainbow
- **Descrição:** Recuperação de chaves MIFARE via dicionário.
- **Funcionamento:** Utiliza tabelas de chaves pré-calculadas e dicionários de chaves comuns salvos no cartão SD para tentar desbloquear setores protegidos de cartões MIFARE Classic.

### 6. Live Data Injection in Emulation
**Uso:** Menu RFID → Live Injection
- **Descrição:** Manipulação de dados durante a emulação.
- **Funcionamento:** Enquanto emula uma tag conhecida, o Bruce permite injetar ou alterar bytes específicos do dump em tempo real antes que o leitor acesse a informação.

### 7. Multi-Slot Chameleon Ultra Mode
**Uso:** Menu RFID → Chameleon Ultra
- **Descrição:** Ciclo automático de cartões emulados.
- **Funcionamento:** Alterna periodicamente entre os 8 slots do dispositivo Chameleon (ou memória interna) para testar sequencialmente múltiplos cartões salvos contra um leitor.

### 8. RFID Skimmer Hunter Pro
**Uso:** Menu RFID → Skimmer Hunter
- **Descrição:** Detector de leitores/tags anômalos.
- **Funcionamento:** Realiza um scan passivo e ativo buscando por assinaturas de hardware típicas de skimmers de cartão ou dispositivos de interceptação NFC.

### 9. UID Obfuscation Randomizer
**Uso:** Menu RFID → UID Obfuscator
- **Descrição:** Anonimização de presença via UID variável.
- **Funcionamento:** Inicia a emulação de tag mas altera o UID randomicamente a cada poucos segundos, dificultando o rastreamento do dispositivo por sistemas de logs de presença.

### 10. NFC Worm Propagator
**Uso:** Menu RFID → NFC Worm
- **Descrição:** Simulação de propagação de payload malicioso.
- **Funcionamento:** Emula uma tag que, ao ser lida por um dispositivo vulnerável, tenta "entregar" um payload (como um URL de download de APK ou comando pareamento) simulando uma propagação viral.

### 11. Mobile Payment Sniffer (MSRP)
**Uso:** Menu RFID → MSRP Sniffer
- **Descrição:** Monitoramento passivo de transações móveis.
- **Funcionamento:** Tenta capturar as primeiras trocas de APDU (Select AID, etc) quando um smartphone (Google/Apple Pay) é aproximado, para identificar o tipo de cartão e aplicação.

### 12. MIFARE Master Key Brute-Force
**Uso:** Menu RFID → Mifare Brute
- **Descrição:** Ataque de força bruta contra autenticação MIFARE.
- **Funcionamento:** Testa as chaves de fábrica e chaves de mestre mais comuns em todos os setores do cartão de forma automatizada e multi-thread.

---

## 🔧 Hardware PN532

### Configuração Recomendada
Para o Bruce, recomenda-se o uso do PN532 via **I2C** ou **Serial (HSU)**.

**Pinagem I2C (Padrão):**
- **SDA:** GPIO 4
- **SCL:** GPIO 16
- **VCC:** 3.3V / 5V
- **GND:** GND

---

## 📚 Protocolos Suportados
- ISO/IEC 14443A (Mifare, NTAG)
- ISO/IEC 14443B
- FeliCa
- ISO/IEC 18092 (P2P, Target)
- NDEF (Software layer)
