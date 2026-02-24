# Integração: Módulo GPS u-blox NEO-6M V2

Este documento detalha como conectar e configurar o módulo GPS **NEO-6M V2** com a placa **CYD-2432S028R**, conforme suportado pelo firmware Bruce.

## ⚙️ Especificações do Módulo
*   **Modelo**: u-blox NEO-6M (GY-NEO6MV2)
*   **Tensão de Operação**: 3.3V a 5V (O módulo possui regulador, mas a lógica é 3.3V).
*   **Comunicação**: UART (Serial)
*   **Baud Rate Padrão**: 9600 bps

---

O firmware foi configurado para usar o **Conector Serial (P1)** da placa CYD, facilitando a conexão e permitindo o compartilhamento de pinos com o módulo IR.

### Conexões Pin-a-Pin

| Pino NEO-6M | Fio (Sugst.) | Pino CYD (P1) | Função ESP32 |
| :--- | :--- | :--- | :--- |
| **VCC** | Vermelho | **VIN (5V)** | Alimentação (via Chave Liga/Desliga) |
| **GND** | Preto | **GND** | Terra |
| **TX** | Azul | **RXD (GPIO 3)** | ESP32 Recebe Dados do GPS |
| **RX** | Amarelo | **TXD (GPIO 1)** | ESP32 Envia Dados para o GPS |

> [!IMPORTANT]
> **Cruzamento de Dados**: Note que o **TX** do GPS deve ser ligado no **RX** da placa (GPIO 3), e o **RX** do GPS no **TX** da placa (GPIO 1).

### Compartilhamento com Módulo IR
Este setup permite que o GPS compartilhe os mesmos pinos de dados (1 e 3) com um transmissor/receptor IR.
- **Chave de Alimentação**: Use interruptores independentes no fio VCC (5V) de cada módulo.
- **Regra**: Ligue apenas um por vez (ou o GPS ou o IR) para evitar conflitos na comunicação serial.

---

## ✅ Vantagens desta Configuração
1.  **Sem Conflito USB:** Você pode usar o Monitor Serial e fazer Upload via USB normalmente, pois o GPS não está mais na UART0.
2.  **Facilidade:** Usa os conectores laterais, sem necessidade de solda delicada no chip.

---

## 🛠️ Verificação e Uso no Bruce

1.  Conecte o GPS conforme acima (idealmente em área externa para pegar sinal).
2.  Ligue o Bruce.
3.  Vá para o menu **GPS**.
4.  Certifique-se de que a taxa de transmissão (Baud Rate) do GPS seja 9600 (padrão do módulo e do firmware).

### LED de Status
*   O módulo NEO-6M geralmente tem um LED azul que pisca quando consegue "fixar" satélites (3D Fix). Se não piscar, vá para um local aberto.

---

## 🔧 Configurações Avançadas do GPS

O firmware Bruce oferece configurações avançadas para otimizar o desempenho do módulo GPS NEO-6M para diferentes cenários de uso.

### Acessando as Configurações

1.  Vá para **GPS → Config → Modo de Operação**
2.  Escolha entre **Padrão** ou **Avançado**

### Modo Padrão
- **Taxa de Update**: 1 Hz (1 atualização por segundo)
- **Protocolo**: NMEA (texto)
- **Modo de Energia**: Máximo desempenho
- **Indicado para**: Uso geral, economia de bateria

### Modo Avançado

Ao selecionar o modo avançado, você tem acesso às seguintes configurações:

#### 📡 Taxa de Update (Frequência de Atualização)

| Opção | Descrição | Indicação |
|-------|-----------|-----------|
| **1 Hz** | Padrão, 1 atualização por segundo | Uso geral, economia de bateria |
| **2 Hz** | 2 atualizações por segundo | Navegação pedestre |
| **5 Hz** | 5 atualizações por segundo | **Wardriving** (recomendado) |
| **10 Hz** | 10 atualizações por segundo | Alta velocidade (pode instabilizar) |

> [!TIP]
> Para wardriving, use **5 Hz** para melhor precisão em movimento. Taxas acima de 5Hz podem causar instabilidade em módulos baratos.

#### 🔋 Modo de Energia

| Opção | Consumo | Descrição |
|-------|---------|-----------|
| **Max Desempenho** | ~45mA | Melhor performance, maior consumo |
| **Eco (Economia)** | ~20mA | Balanceado entre performance e consumo |
| **Power Save (Mínimo)** | ~10-200µA | Mínimo consumo, atualizações lentas |

> [!WARNING]
> O modo Power Save é ideal para uso prolongado com bateria, mas pode aumentar o tempo para obter fix inicial.

#### 📶 Protocolo de Comunicação

| Opção | Descrição | Vantagens |
|-------|-----------|-----------|
| **NMEA (Texto)** | Padrão da indústria | Compatibilidade universal |
| **UBX (Binário)** | Protocolo proprietário u-blox | Mais eficiente, menor latência |
| **NMEA + UBX** | Ambos ativos | Máxima compatibilidade |

#### 🧭 Modo de Navegação

| Opção | Descrição | Precisão |
|-------|-----------|----------|
| **Portátil** | Padrão, uso geral | ~2.5m |
| **Estacionário** | Objeto parado | ~2.0m (melhor precisão) |
| **Pedestre** | Caminhando | ~2.5m |
| **Automotivo** | Veículo em movimento | ~3.0m |
| **Marítimo** | Embarcações | ~3.0m |
| **Aéreo** | Aeronaves | ~5.0m |

> [!TIP]
> Use **Estacionário** para aplicações que requerem máxima precisão e o dispositivo ficará fixo.

#### 🛰️ Filtro de Satélites

| Opção | Descrição | Indicação |
|-------|-----------|-----------|
| **Desativado** | Aceita todos os satélites | Máxima disponibilidade |
| **25 dB-Hz** | Filtra sinais muito fracos | Áreas com algum ruído |
| **30 dB-Hz** | Filtra sinais fracos | Balanceado |
| **35 dB-Hz** | Recomendado | Melhor relação sinal/ruído |
| **40 dB-Hz** | Apenas sinais fortes | Áreas com muito ruído/interferência |

> [!TIP]
> O filtro de **35 dB-Hz** é recomendado para ambientes urbanos com interferência. Sinais abaixo deste threshold são geralmente refletidos ou com ruído.

### Aplicando as Configurações

1. Configure todas as opções desejadas no menu **Avançado**
2. Selecione **"Aplicar Config"**
3. As configurações serão enviadas ao GPS e salvas na EEPROM do módulo

> [!NOTE]
> As configurações são aplicadas automaticamente ao iniciar Wardriving ou GPS Tracker quando o modo avançado está ativo.

### 📋 Resumo de Otimizações por Cenário

| Cenário | Taxa | Energia | Navegação | Filtro |
|---------|------|---------|-----------|--------|
| **Wardriving** | 5 Hz | Max | Automotivo | 35 dB-Hz |
| **GPS Tracker** | 1 Hz | Eco | Portátil | 30 dB-Hz |
| **Estacionário** | 1 Hz | Power Save | Estacionário | 35 dB-Hz |
| **Pedestre** | 2 Hz | Eco | Pedestre | 30 dB-Hz |
| **Bateria Fraca** | 1 Hz | Power Save | Portátil | Desativado |

---

## 🔄 Restaurar Configurações Padrão

Para voltar às configurações de fábrica:

1. Vá para **GPS → Config → Modo de Operação**
2. Selecione **"Padrão (1Hz, NMEA)"**
3. As configurações padrão serão restauradas

---

## 📁 Arquivos Gerados

O módulo GPS gera os seguintes arquivos no cartão SD ou LittleFS:

### Wardriving
- **Local**: `/BruceWardriving/`
- **Formato**: CSV (compatível com WiGLE)
- **Nome**: `AAAMMDD_HHMMSS_wardriving.csv`

### GPS Tracker
- **Local**: `/BruceGPS/`
- **Formato**: GPX
- **Nome**: `AAAMMDD_HHMMSS_gps_tracker.gpx`

---

## 🔗 Veja Também

- [Documentação WiGLE](https://wigle.net/)
- [u-blox NEO-6M Datasheet](https://www.u-blox.com/en/product/neo-6-series)