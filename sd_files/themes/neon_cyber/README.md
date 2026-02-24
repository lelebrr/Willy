# 🌃 Neon Cyber Theme

Tema futurista cyberpunk para o Willy Firmware com efeitos neon e animações.

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

*Cyberpunk nunca morre! 🤖*