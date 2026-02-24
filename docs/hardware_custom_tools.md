# Ferramentas Customizadas para Offensive Security (CYD & Bruce)

Este guia detalha a integração e uso de módulos externos de GPS e Infravermelho (IR) no firmware Bruce, focando no setup de UART compartilhada via conector P1 do CYD.

---

## 🛰️ Módulo GPS: NEO-6M (GY-GPS6MV2)

O NEO-6M é um receptor GPS de alta performance, ideal para geotagging em operações de wardriving e auditorias de segurança física.

### Especificações Técnicas

- **Receptor**: 50 canais, GPS L1 C/A, SBAS (WAAS, EGNOS, MSAS).
- **Precisão**: ~2.5 metros (Horizontal).
- **Update Rate**: 1Hz a 5Hz.
- **Sensibilidade**: -161 dBm (Tracking).
- **Alimentação**: 3.0V - 5.0V (Possui regulador LDO 3.3V onboard).
- **Interface**: UART (Default 9600 baud).

### Pinagem e Wiring (CYD P1)

| Fio | Sinal GPS | Pino CYD (P1) | Função |
| :--- | :--- | :--- | :--- |
| **Vermelho** | VCC | 5V / VIN | Alimentação (usar interruptor) |
| **Amarelo** | TX | RX (GPIO 3) | Envio de dados NMEA |
| **Azul** | RX | TX (GPIO 1) | Recebimento de comandos |
| **Preto** | GND | GND | Ground comum |

![Pinout NEO-6M](file:///c:/Users/leleb/OneDrive/Documentos/Projetos/Bruce/media/pictures/neo6m_pinout.png)
*Diagrama de pinagem do módulo NEO-6M.*

### Dicas de Uso

- **Céu Aberto**: Requer visibilidade do céu para o primeiro "Fix" (LED PPS pisca quando fixado).
- **Bateria Backup**: Possui bateria para manter dados de órbita e permitir "Hot Start" rápido (~1s).

![Arquitetura Interna NEO-6M](file:///c:/Users/leleb/OneDrive/Documentos/Projetos/Bruce/media/pictures/neo6m_internal.png)

---

## 📡 Módulo IR: Y-SIRTM (UART Transceiver)

Módulo transceiver via UART para envio e recepção de sinais no protocolo NEC (38kHz).

### Especificações Técnicas

- **Protocolo**: NEC 32-bit.
- **Frequência**: 38kHz.
- **Interface**: UART (9600-115200 baud).
- **Alimentação**: 5V DC.

### Upgrade Recomendado: TSOP38238

Para máxima performance em ataques de replay e jamming, recomenda-se substituir o receptor original pelo **TSOP38238 (Vishay)**.

- **Melhoria**: Rejeição de ruído óptico e maior range (até 45m).

![TSOP38238 Pinout](file:///c:/Users/leleb/OneDrive/Documentos/Projetos/Bruce/media/pictures/tsop38238_pinout.png)

### Pinagem e Wiring (CYD P1)

| Fio | Sinal IR | Pino CYD (P1) | Função |
| :--- | :--- | :--- | :--- |
| **Vermelho** | 5V | 5V / VIN | Alimentação (usar interruptor) |
| **Amarelo** | TXD | RX (GPIO 3) | Recebe dados IR decodificados |
| **Azul** | RXD | TX (GPIO 1) | Envia comando IR para transmitir |
| **Preto** | GND | GND | Ground comum |

![YS-IRTM Close-up](file:///c:/Users/leleb/OneDrive/Documentos/Projetos/Bruce/media/pictures/ys_irtm_closeup.jpg)

---

## ⚠️ Estratégia de Compartilhamento UART (P1)

Como o GPS e o IR compartilham os mesmos pinos de UART (GPIO 1 e 3) no conector P1:

1. **Interruptores**: Use um interruptor físico no fio de alimentação (VCC) de cada módulo.
2. **Uso Exclusivo**: Ligue apenas um módulo por vez para evitar colisão de dados no barramento Serial.
3. **Level Shifter**: Como o CYD usa lógica 3.3V e o IR opera a 5V, recomenda-se o uso de um divisor de tensão ou level shifter no pino RX do IR para evitar danos ao ESP32.

---

## 📚 Bibliotecas no Bruce

O firmware já integra suporte para estes módulos via:

- **TinyGPS++**: Para processamento de coordenadas e tempo.
- **IRremoteESP8266**: Para manipulação de protocolos IR avançados.
