<div align="center">

# 🔌 PCBs do Bruce

[![PCBWAY](https://img.shields.io/badge/PCBWAY-Patrocinador-orange.svg)](https://www.pcbway.com/)
[![Open Source](https://img.shields.io/badge/Open%20Source-Hardware-green.svg)]()
[![Licença](https://img.shields.io/badge/Licen%C3%A7a-CERN%20OHL-blue.svg)]()

<img src="https://www.pcbway.com/project/img/images/frompcbway-1220.png" alt="Banner PCBWay" width="500"/>

**PCBs desenvolvidos pela comunidade para o Bruce Firmware**

[🛒 Pedir no PCBWay](https://www.pcbway.com/project/shareproject/Bruce_PCB_Smoochiee_d6a0284b.html) • [📋 Arquivos de Design](#-arquivos-de-design) • [🔧 Guia de Montagem](#-guia-de-montagem)

</div>

---

## 📋 Índice

- [Visão Geral](#-visão-geral)
- [PCBs Disponíveis](#-pcbs-disponíveis)
- [Pedir no PCBWay](#-pedir-no-pcbway)
- [Arquivos de Design](#-arquivos-de-design)
- [Guia de Montagem](#-guia-de-montagem)
- [Contribuindo](#-contribuindo)
- [Patrocinadores](#-patrocinadores)

---

## 🗺️ Visão Geral

Este diretório contém designs de PCB open-source criados pela comunidade Bruce. Esses PCBs são projetados para funcionar perfeitamente com o Bruce Firmware e aprimorar suas capacidades de pesquisa em segurança.

### Por que usar nossos PCBs?

| Benefício | Descrição |
|-----------|-----------|
| ✅ **Prontos para Uso** | Projetados especificamente para o Bruce Firmware |
| 📐 **Design Compacto** | Fator de forma otimizado para portabilidade |
| 💰 **Custo Efetivo** | Fabricação acessível através do PCBWay |
| 🔧 **Fácil Montagem** | Silkscreen claro e guias de montagem |
| 📖 **Open Source** | Modifique e melhore os designs |

---

## 🖥️ PCBs Disponíveis

### 1. Bruce PCB v2 por Smoochiee

<div align="center">

[![Bruce PCB v2](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/smoochie/Bruce_PCB_full.png)](https://www.pcbway.com/project/shareproject/Bruce_PCB_Smoochiee_d6a0284b.html)

</div>

#### Recursos

| Recurso | Especificação |
|---------|---------------|
| **Display** | 3.5" TFT Touchscreen |
| **Módulo RF** | CC1101 Sub-GHz integrado |
| **Cartão SD** | Slot MicroSD |
| **IR** | LEDs TX/RX |
| **USB** | Conector USB-C |
| **Bateria** | Circuito de carregamento Li-Po |
| **GPIO** | Headers de expansão |

#### Imagens

<div align="center">

| Frente | Verso |
|:------:|:-----:|
| ![Frente](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/smoochie/front.png) | ![Verso](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/smoochie/back.png) |

</div>

#### Especificações

```
Dimensões: 85mm x 55mm
Camadas: 4
Espessura: 1.6mm
Acabamento: ENIG
Máscara de Solda: Preto
Silkscreen: Branco
```

#### Lista de Componentes

| Componente | Encapsulamento | Qtd | Observações |
|------------|----------------|-----|-------------|
| ESP32-WROOM-32 | Módulo | 1 | MCU Principal |
| CC1101 | Módulo | 1 | RF Sub-GHz |
| ILI9341 TFT | FPC | 1 | Display 3.5" |
| XPT2046 | TSOP | 1 | Controlador Touch |
| TP4056 | SOP-8 | 1 | Carregador de Bateria |
| AMS1117-3.3 | SOT-223 | 1 | Regulador de Tensão |
| Slot MicroSD | SMD | 1 | Tipo push-push |
| Conector USB-C | SMD | 1 | 16 pinos |

---

### 2. StickCPlus PCB por Pirata

<div align="center">

![StickCPlus PCB](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/Pirata/front.png)

</div>

#### Recursos

| Recurso | Especificação |
|---------|---------------|
| **Compatibilidade** | M5Stack StickC Plus 1.1 & 2 |
| **Módulo RF** | Header para CC1101 externo |
| **IR** | LED IR integrado |
| **Expansão** | Conector Grove |
| **Bateria** | Usa bateria interna do StickC |

#### Imagens

<div align="center">

| Frente | Verso |
|:------:|:-----:|
| ![Frente](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/Pirata/front.png) | ![Verso](https://raw.githubusercontent.com/pr3y/Bruce/refs/heads/main/media/pcbs/Pirata/back.png) |

</div>

#### Especificações

```
Dimensões: 55mm x 25mm
Camadas: 2
Espessura: 1.0mm
Acabamento: HASL
Máscara de Solda: Preto
```

---

## 🛒 Pedir no PCBWay

<div align="center">

[![Pedir no PCBWay](https://www.pcbway.com/project/img/images/frompcbway-1220.png)](https://www.pcbway.com/project/shareproject/Bruce_PCB_Smoochiee_d6a0284b.html)

### [👉 Clique Aqui para Pedir o Bruce PCB](https://www.pcbway.com/project/shareproject/Bruce_PCB_Smoochiee_d6a0284b.html)

</div>

### Por que PCBWay?

| Benefício | Descrição |
|-----------|-----------|
| 🏭 **Qualidade** | Certificado ISO 9001:2015 |
| 💰 **Acessível** | A partir de $5 por 10 PCBs |
| 🚚 **Envio Rápido** | Produção em 24 horas disponível |
| 🎨 **Opções** | Múltiplas cores e acabamentos |
| ✅ **PCBA** | Serviço de montagem disponível |

### Processo de Pedido

1. **Visite a página do projeto**: [Bruce PCB no PCBWay](https://www.pcbway.com/project/shareproject/Bruce_PCB_Smoochiee_d6a0284b.html)
2. **Selecione especificações**:
   - Quantidade (10, 20, 50, 100+)
   - Espessura (1.6mm recomendado)
   - Peso do cobre (1oz padrão)
   - Acabamento (ENIG para melhor qualidade)
3. **Adicione ao carrinho** e finalize
4. **Receba os PCBs** em 5-15 dias

---

## 📁 Arquivos de Design

### Estrutura de Arquivos

```
pcbs/
├── Bruce_PCB_smoochie/
│   ├── BOM_Bruce_PCB_v2_Smoochiee.xlsx    # Lista de Materiais
│   ├── Bruce_PCB_Smoochiee_Pick_and_Plack.xlsx
│   ├── Bruce_PCB_v2_Schematic.pdf          # Esquemático
│   ├── gerber/
│   │   ├── BRUCE_MANUAL_BUILD.zip          # Arquivos Gerber
│   │   └── Bruce_PCB_v2_Smoochiee.zip      # Arquivos de produção
│   └── 3d/
│       ├── Bruce_PCB_V2_3dCase.zip         # Caixa 3D impressa
│       └── README.md
│
├── M5Stick_Intermidiate_ultramarines/
│   ├── Gerber_m5module_PCB_m5module_2024-11-18.zip
│   └── README.md
│
├── StickCPlus_PCB_Pirata/
│   ├── BOM of Pirata Board.xlsx
│   ├── Pirata Board Schematics.pdf
│   └── Pirata_StickC_Board.zip
│
└── README.md
```

### Baixar Arquivos

| PCB | Esquemático | Gerber | BOM |
|-----|-------------|--------|-----|
| **Smoochiee v2** | [PDF](Bruce_PCB_smoochie/Bruce_PCB_v2_Schematic.pdf) | [ZIP](Bruce_PCB_smoochie/gerber/Bruce_PCB_v2_Smoochiee.zip) | [XLSX](Bruce_PCB_smoochie/BOM_Bruce_PCB_v2_Smoochiee.xlsx) |
| **StickCPlus Pirata** | [PDF](StickCPlus_PCB_Pirata/Pirata%20Board%20Schematics.pdf) | [ZIP](StickCPlus_PCB_Pirata/Pirata_StickC_Board.zip) | [XLSX](StickCPlus_PCB_Pirata/BOM%20of%20Pirata%20Board.xlsx) |

---

## 🔧 Guia de Montagem

### Ferramentas Necessárias

| Ferramenta | Propósito |
|------------|-----------|
| 🔧 **Ferro de Solda** | Soldagem de componentes SMD |
| 🔍 **Microscópio** | Inspeção de componentes pequenos |
| 💨 **Estação de Ar Quente** | Reflow/reparo |
| 📏 **Multímetro** | Teste de conexões |
| 🔩 **Pinças** | Manuseio de peças SMD |
| ⚗️ **Fluxo** | Melhor fluxo de solda |

### Passos de Montagem

#### 1. Preparar Componentes

```
□ Organize todos os componentes por valor
□ Verifique a BOM contra as peças recebidas
□ Prepare pasta de solda (para reflow)
□ Limpe o PCB com álcool isopropílico
```

#### 2. Soldar Componentes SMD

```
Ordem de montagem (menor para maior):
1. □ Resistores e capacitores (0603/0805)
2. □ ICs (encapsulamentos SOIC/TQFP)
3. □ Conectores e headers
4. □ Módulos (ESP32, CC1101)
5. □ Conector do display
```

#### 3. Instalar Componentes Through-Hole

```
1. □ Headers de pinos
2. □ Botões/switches
3. □ Suportes de LED
```

#### 4. Instalar Módulos

```
1. □ Módulo ESP32-WROOM-32
2. □ Módulo RF CC1101
3. □ Display (via conector FPC)
```

#### 5. Testes

```
□ Verifique curtos com multímetro
□ Verifique trilho de 3.3V
□ Conecte USB e verifique LED de energia
□ Flash do Bruce Firmware
□ Teste todas as funções
```

### Vídeo de Montagem

🎥 Em breve! Confira nosso canal no YouTube para tutoriais de montagem.

---

## 🤝 Contribuindo

### Envie Seu Design de PCB

Agradecemos novos designs de PCB da comunidade!

#### Requisitos

1. **Arquivos de Design**: KiCad, EasyEDA ou Altium
2. **Arquivos Gerber**: Prontos para produção
3. **BOM**: Lista de materiais completa
4. **Esquemático**: Formato PDF
5. **Fotos**: Frente, verso e montado
6. **README**: Descrição e especificações

#### Processo de Envio

1. Faça um fork do repositório
2. Crie uma pasta em `pcbs/nome_do_seu_pcb/`
3. Adicione todos os arquivos necessários
4. Crie um pull request
5. Nós revisaremos e mesclaremos!

### Diretrizes de Design

```
✅ Use encapsulamentos de componentes padrão
✅ Inclua pontos de teste
✅ Adicione rótulos claros no silkscreen
✅ Design para fabricação (DFM)
✅ Inclua furos de montagem
✅ Adicione LEDs indicadores
```

---

## 🏆 Patrocinadores

<div align="center">

### Agradecimento Especial aos Nossos Patrocinadores

[![PCBWay](https://www.pcbway.com/project/img/images/frompcbway-1220.png)](https://www.pcbway.com/)

**PCBWay** - Fabricação de PCB de Qualidade

[Visite o PCBWay](https://www.pcbway.com/) para todas as suas necessidades de PCB:
- Fabricação de PCB
- Montagem PCBA
- Impressão 3D
- Usinagem CNC

</div>

---

## 📜 Licença

Estes designs de PCB são liberados sob a **CERN Open Hardware Licence v2**.

Você é livre para:
- ✅ Compartilhar — copiar e redistribuir o material
- ✅ Adaptar — remixar, transformar e construir sobre o material

Sob os seguintes termos:
- 📝 Atribuição — Você deve dar crédito apropriado
- 📝 CompartilhaIgual — Se você remixar, transformar ou construir sobre o material, você deve distribuir suas contribuições sob a mesma licença

---

## 📚 Recursos

### Recursos de Design de PCB

| Recurso | Link |
|---------|------|
| KiCad EDA | [kicad.org](https://www.kicad.org/) |
| EasyEDA | [easyeda.com](https://easyeda.com/) |
| SnapEDA | [snapeda.com](https://www.snapeda.com/) |
| UltraLibrarian | [ultralibrarian.com](https://www.ultralibrarian.com/) |

### Recursos de Aprendizado

| Recurso | Descrição |
|---------|-----------|
| [EEVblog](https://www.youtube.com/user/EEVblog) | Tutoriais de eletrônica |
| [Altium Academy](https://www.youtube.com/c/AltiumAcademy) | Cursos de design de PCB |
| [Contextual Electronics](https://contextualelectronics.com/) | Design prático de PCB |

---

<div align="center">

### 🔌 Comunidade PCB do Bruce

**[⬆ Voltar ao Topo](#-pcbs-do-bruce)**

*Hardware Aberto para Segurança Aberta*

</div>