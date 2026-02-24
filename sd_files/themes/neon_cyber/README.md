# 🌃 Neon Cyber Theme

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

Tema futurista cyberpunk para o Willy Firmware com efeitos neon e animações - Proprietário

## ✨ Características

### Efeitos Visuais

- **Scanline Effect**: Linha de varredura animada
- **Neon Glow**: Brilho neon pulsante
- **Glitch Effect**: Efeito de interferência
- **Matrix Rain**: Chuva de caracteres estilo Matrix
- **Border Glow**: Bordas com brilho colorido alternado
- **Glassmorphism**: Efeito de vidro fosco

### Animações

- Background animado com gradiente pulsante
- Botões com efeito de luz deslizante
- Menu items com hover animado
- Progress bars com gradiente móvel
- Loading spinners

### Cores

| Variável | Cor | Uso |
|----------|-----|-----|
| `--primary` | #00ffff (Ciano) | Elementos principais |
| `--secondary` | #ff00ff (Magenta) | Elementos secundários |
| `--accent` | #00ff00 (Verde) | Status online |
| `--warning` | #ffff00 (Amarelo) | Alertas |
| `--danger` | #ff0000 (Vermelho) | Erros |

## 📁 Arquivos

```
neon_cyber/
├── theme.css      # Estilos CSS principais
├── theme.json     # Configuração do tema
└── README.md      # Esta documentação
```

## 🚀 Instalação

1. Copie a pasta `neon_cyber` para o SD card em `/themes/`
2. Acesse Menu → Config → Themes
3. Selecione "Neon Cyber"

## 💻 Classes CSS Disponíveis

### Container

```css
.container       # Container principal com borda neon
.card            # Card com efeito glass
.menu-item       # Item de menu animado
```

### Botões

```css
button, .btn     # Botão neon padrão
```

### Status

```css
.status-online   # Indicador online (verde pulsante)
.status-offline  # Indicador offline (vermelho)
.status-warning  # Indicador de alerta (amarelo piscando)
```

### Componentes

```css
.progress-bar    # Barra de progresso
.progress-fill   # Preenchimento animado
.loader          # Spinner de carregamento
.data-grid       # Grid de dados
.data-cell       # Célula de dados
```

### Efeitos Especiais

```css
.glitch          # Efeito glitch no texto
.matrix-bg       # Background Matrix
.tooltip         # Tooltip animado
```

## 🎨 Personalização

### Alterar Cores

Edite as variáveis CSS no início do arquivo:

```css
:root {
    --primary: #00ffff;    /* Mude para sua cor preferida */
    --secondary: #ff00ff;
    --accent: #00ff00;
}
```

### Desabilitar Animações

Para melhor performance, comente as animações:

```css
/* animation: bgPulse 15s ease infinite; */
/* animation: borderGlow 6s ease infinite; */
```

## 📱 Responsividade

O tema é responsivo e se adapta a:

- Desktop (>768px)
- Tablet (768px)
- Mobile (<768px)

## 🔧 Requisitos

- Willy Firmware 1.0+
- Display TFT ou similar
- Navegador com suporte a CSS3

---

## 📜 Licença

**⚠️ SOFTWARE PROPRIETÁRIO - TODOS OS DIREITOS RESERVADOS**

Este tema e recursos de interface são propriedade exclusiva da Willy Firmware e são fornecidos sob licença estritamente controlada.

### Termos de Uso

- **Uso Exclusivo**: Este tema pode ser utilizado apenas pelo proprietário autorizado
- **Modificação Proibida**: Qualquer modificação ou distribuição é estritamente proibida
- **Responsabilidade**: O usuário é o único responsável pelo uso do tema
- **Restrições**: Uso permitido apenas em sistemas autorizado por escrito

### Limitações de Responsabilidade

A Willy Firmware não se responsabiliza por:

- Qualquer dano resultante do uso não autorizado do tema
- Consequências legais de atividades realizadas com os recursos de interface
- Uso indevido ou violação dos termos de licença

### Direitos Autorais

© 2026 Willy Firmware. Todos os direitos reservados.

### Contato

Para informações sobre licenciamento ou suporte, entre em contato através dos canais oficiais da Willy Firmware.

---

**⚠️ AVISO LEGAL**: O uso deste tema implica concordância total com os termos de licença proprietária. Qualquer violação resultará em ações legais cabíveis.

---
*Cyberpunk nunca morre! 🤖*
