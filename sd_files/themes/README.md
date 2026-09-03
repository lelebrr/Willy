<div align="center">

# 🎨 Temas

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-red.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![JavaScript](https://img.shields.io/badge/JavaScript-ES6+-yellow.svg)
![Security](https://img.shields.io/badge/Security-Pentest-red.svg)
![Hacking](https://img.shields.io/badge/Hacking-Tools-red.svg)
![Red Team](https://img.shields.io/badge/Red%20Team-Offensive-red.svg)
![UI](https://img.shields.io/badge/UI-UX%20Design-purple.svg)
![CSS](https://img.shields.io/badge/CSS3-Style-blue.svg)
![JSON](https://img.shields.io/badge/JSON-Data-green.svg)

[![SD Card](https://img.shields.io/badge/SD%20Card-Necess%C3%A1rio-orange.svg)]()
[![Formato](https://img.shields.io/badge/Formato-JSON%2FCSS-blue.svg)]()

**Temas customizados para a interface do Willy Firmware - Proprietário**

</div>

---

## 📋 Índice

- [Visão Geral](#-visão-geral)
- [Estrutura de Arquivos](#-estrutura-de-arquivos)
- [Como Usar](#-como-usar)
- [Temas Disponíveis](#-temas-disponíveis)
- [Criar Novo Tema](#-criar-novo-tema)
- [Referências](#-referências)

---

## 🗺️ Visão Geral

Os temas permitem personalizar completamente a aparência do Willy Firmware, incluindo:

- 🎨 **Cores** - Fundo, primária, secundária, destaque
- 🖼️ **Imagens** - Backgrounds, logos, ícones
- ✨ **Efeitos** - Animações, transições, filtros
- 🔤 **Fontes** - Tamanho e estilo do texto

---

## 📁 Estrutura de Arquivos

```
themes/
├── 📄 Theme_Builder.html    # Criador de temas visual
├── 📄 README.md             # Esta documentação
│
├── 📁 neon_cyber/           # Tema futurista cyberpunk
│   ├── 📄 theme.css         # Estilos CSS
│   ├── 📄 theme.json        # Metadados e configurações
│   ├── 📄 background.png    # Imagem de fundo
│   └── 📄 README.md
│
└── 📁 example/              # Temas de exemplo
    ├── 📄 Pirata_png.zip
    └── 📄 Sharky.zip
```

---

## 🚀 Como Usar

### Método 1: Theme Builder (Recomendado)

1. **Abra o criador de temas**:
   - Localize `Theme_Builder.html` na pasta `themes/`
   - Abra no navegador de sua preferência

2. **Configure as cores**:
   - Escolha as cores principais
   - Ajuste brilho e contraste
   - Visualize em tempo real

3. **Adicione imagens**:
   - Carregue backgrounds personalizados
   - Adicione logos e ícones

4. **Exporte o tema**:
   - Clique em "Exportar"
   - Salve o arquivo ZIP

5. **Instale no SD Card**:
   - Extraia o ZIP para o SD card
   - Coloque na pasta `/themes/nome_do_tema/`

### Método 2: Instalação Manual

1. **Copie a pasta do tema** para o cartão SD:

   ```
   /themes/seu_tema/
   ├── theme.css
   ├── theme.json
   └── background.png (opcional)
   ```

2. **Acesse o menu de temas**:
   - Menu → Configurações → Temas

3. **Selecione o tema** desejado

4. **Aplique e reinicie** se necessário

---

## 📋 Temas Disponíveis

### 🌙 Neon Cyber

Tema futurista estilo cyberpunk com efeitos visuais avançados.

| Recurso | Descrição |
|---------|-----------|
| **Estilo** | Cyberpunk / Neon |
| **Cores** | Verde neon, magenta, ciano |
| **Efeitos** | Scanline, glow, glitch |

#### Características

- ✨ **Scanline Animation** - Efeito de linhas de varredura
- 💡 **Neon Glow** - Brilho neon em elementos
- 📺 **Glitch Effect** - Efeito de distorção
- 🌧️ **Matrix Rain** - Chuva de caracteres estilo Matrix
- 🔲 **Border Glow** - Bordas com brilho animado

#### Preview

```
┌──────────────────────────────┐
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │
│  ██  Willy FIRMWARE  ████    │
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │
│                              │
│  > WiFi        [████████]   │
│  > Bluetooth   [████████]   │
│  > IR          [████████]   │
│  > NFC         [████████]   │
│                              │
│  ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀  │
└──────────────────────────────┘
```

---

### 🏴‍☠️ Tema Pirata

Tema inspirado em piratas com estética de mapa do tesouro.

| Recurso | Descrição |
|---------|-----------|
| **Estilo** | Pirata / Vintage |
| **Cores** | Pergaminho, marrom, dourado |
| **Fontes** | Estilo manuscrito |

---

### 🦈 Tema Sharky

Tema com estética aquática e tubarões.

| Recurso | Descrição |
|---------|-----------|
| **Estilo** | Aquático / Oceano |
| **Cores** | Azul, turquesa, branco |
| **Elementos** | Tubarões, ondas |

---

## 🎨 Criar Novo Tema

### Passo a Passo Completo

#### 1. Estrutura Mínima

Crie uma pasta com os arquivos necessários:

```
meu_tema/
├── theme.css      # Estilos CSS (obrigatório)
├── theme.json     # Metadados (obrigatório)
├── background.png # Imagem de fundo (opcional)
├── logo.png       # Logo personalizado (opcional)
└── icons/         # Ícones customizados (opcional)
    ├── wifi.png
    ├── bt.png
    └── ...
```

#### 2. Arquivo theme.json

Configure os metadados e cores:

```json
{
  "name": "Meu Tema",
  "version": "1.0.0",
  "author": "Seu Nome",
  "description": "Descrição do tema",
  "colors": {
    "primary": "#00ff00",
    "secondary": "#00ffff",
    "background": "#000000",
    "text": "#ffffff",
    "highlight": "#ff00ff",
    "border": "#333333"
  },
  "fonts": {
    "normal": "default",
    "large": "default"
  },
  "images": {
    "background": "background.png",
    "logo": "logo.png"
  },
  "effects": {
    "scanlines": true,
    "glow": true,
    "animation": "fade"
  }
}
```

#### 3. Arquivo theme.css

Defina os estilos personalizados:

```css
/* Tema: Meu Tema */
/* Autor: Seu Nome */

/* Fundo principal */
.background {
    background-color: #000000;
    background-image: url('background.png');
    background-size: cover;
}

/* Texto principal */
.text-primary {
    color: #00ff00;
    text-shadow: 0 0 10px #00ff00;
}

/* Elementos de destaque */
.highlight {
    background-color: #ff00ff;
    border: 2px solid #00ffff;
    box-shadow: 0 0 20px #ff00ff;
}

/* Efeito scanline */
.scanline::after {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: repeating-linear-gradient(
        0deg,
        rgba(0,0,0,0.1),
        rgba(0,0,0,0.1) 1px,
        transparent 1px,
        transparent 2px
    );
    pointer-events: none;
}

/* Efeito glow neon */
.neon-glow {
    text-shadow:
        0 0 5px #00ff00,
        0 0 10px #00ff00,
        0 0 20px #00ff00,
        0 0 40px #00ff00;
}

/* Animação de fade */
@keyframes fadeIn {
    from { opacity: 0; }
    to { opacity: 1; }
}

.fade-animation {
    animation: fadeIn 0.5s ease-in-out;
}
```

#### 4. Tabela de Cores Comuns

| Elemento | Cor Padrão | Descrição |
|----------|------------|-----------|
| `primary` | `#00ff00` | Cor principal (verde neon) |
| `secondary` | `#00ffff` | Cor secundária (ciano) |
| `background` | `#000000` | Fundo (preto) |
| `text` | `#ffffff` | Texto (branco) |
| `highlight` | `#ff00ff` | Destaque (magenta) |
| `warning` | `#ffff00` | Aviso (amarelo) |
| `error` | `#ff0000` | Erro (vermelho) |
| `success` | `#00ff00` | Sucesso (verde) |

#### 5. Efeitos Disponíveis

```css
/* Efeito Glitch */
.glitch {
    animation: glitch 0.5s infinite;
}

@keyframes glitch {
    0% { transform: translate(0); }
    20% { transform: translate(-2px, 2px); }
    40% { transform: translate(-2px, -2px); }
    60% { transform: translate(2px, 2px); }
    80% { transform: translate(2px, -2px); }
    100% { transform: translate(0); }
}

/* Efeito Matrix Rain */
.matrix-bg {
    background: linear-gradient(180deg,
        transparent 0%,
        rgba(0, 255, 0, 0.1) 50%,
        transparent 100%
    );
    animation: matrix 2s linear infinite;
}

/* Efeito Border Glow */
.border-glow {
    border: 2px solid #00ff00;
    box-shadow:
        0 0 5px #00ff00,
        0 0 10px #00ff00,
        inset 0 0 5px rgba(0, 255, 0, 0.5);
}
```

---

## 📚 Referências

### Documentação CSS

| Recurso | Link |
|---------|------|
| CSS Animations | [MDN Web Docs](https://developer.mozilla.org/pt-BR/docs/Web/CSS/animation) |
| CSS Gradients | [MDN Web Docs](https://developer.mozilla.org/pt-BR/docs/Web/CSS/gradient) |
| CSS Filters | [MDN Web Docs](https://developer.mozilla.org/pt-BR/docs/Web/CSS/filter) |

### Inspiração de Temas

| Estilo | Descrição |
|--------|-----------|
| **Cyberpunk** | Neon, escuro, futurista |
| **Retro** | Pixel art, cores 8-bit |
| **Minimalista** | Limpo, preto e branco |
| **Natureza** | Verde, azul, orgânico |

---

## 🤝 Contribuir com Temas

Quer compartilhar seu tema com a comunidade?

1. **Crie seu tema** seguindo as diretrizes acima
2. **Teste** no Willy Firmware
3. **Faça um fork** do repositório
4. **Adicione** na pasta `themes/seu_tema/`
5. **Envie um Pull Request**

---

<div align="center">

### 🎨 Willy Themes

**[⬆ Voltar ao Topo](#-temas)**

*Personalize seu Willy do seu jeito - Proprietário*

</div>

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Estes temas e recursos de interface são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Estes temas podem ser utilizados apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso dos temas
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado dos temas
- Consequências legais de atividades realizadas com os recursos de interface
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso destes temas implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---

## Pacotes de cor do firmware (theme.json)

Temas só-cor no schema que o firmware lê (`priColor`/`secColor`/`bgColor` hex RGB565 sem `#`, mais `border`, `label`, `gifDuration`). Copie a pasta para o SD e ative em **Config → Tema UI**:

| Pasta | Nome | Cores |
|---|---|---|
| `cyberpunk/` | Cyberpunk | magenta F81F / roxo 7819 |
| `matrix/` | Matrix | verde 07E0 sobre preto 0000 |
| `ocean/` | Ocean | ciano-azul 04FF / 0210 |
| `sunset/` | Sunset | laranja FBE0 / A800 |
| `blood/` | Blood | vermelho E004 / 9802 |
| `gold/` | Gold | dourado FE60 / A500 |
| `ice/` | Ice | gelo DFFF / 841F |
| `dracula/` | Dracula | roxo-rosa C8FF / 601F sobre 0842 |
| `neon_cyber/` | Neon Cyber | agora também com chaves de firmware (07FF/F81F/0841) |

Os mesmos 8 nomes existem embutidos em **Config → Cor UI** (sem precisar de SD).
