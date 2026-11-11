# Cyber Menu UI (Willy v2.1 Evolution)

O Cyber Menu do Willy é uma interface de usuário avançada e de alto desempenho, projetada para telas embarcadas (como a do CYD 2.8"). Ele integra animações LVGL avançadas e monitoramento do sistema em tempo real.

## Recursos

### 1. Animações Avançadas

- **Entrada em Cascata**: Os ícones surgem e deslizam para cima com um atraso incremental, criando um efeito de onda.
- **Pulsação Neon**: As bordas dos botões pulsam sutilmente com suavização de transparência.
- **Hover Interativo**:
  - **Zoom**: Ampliação suave de 1.3x.
  - **Rotação 3D**: Inclinação de 8 graus para percepção de profundidade.
  - **Brilho/Sombra**: Expansão de sombra com a cor de destaque (accent).

### 2. Integração & Temas

- **Temas Dinâmicos**: Utiliza `WillyConfig.priColor` e `WillyConfig.secColor` para combinar com o tema atual do sistema (Escuro, Claro ou Cyber).
- **Integração Core**: Chama perfeitamente os callbacks dos módulos existentes do Willy (Wi-Fi, BLE, IR, etc.).

### 3. Barra de Notificações

- **Status em Tempo Real**:
  - **Tempo**: Sincroniza com o relógio do sistema (suporta NTP).
  - **Bateria**: Porcentagem em tempo real com alertas codificados por cores (Verde > 50%, Amarelo > 20%, Vermelho < 20%).
- **Símbolos Interativos**: Indicadores visuais para conectividade WiFi e Bluetooth.

## Detalhes Técnicos

- **Impacto na Memória (Footprint)**: < 10KB.
- **Engine Gráfica**: LVGL 8.3.11.
- **Otimização**: Reutilização de funções e número mínimo de pontos em vetores para evitar travamentos (lag) na CPU do ESP32.

## Como Acessar

Vá para o **Menu Principal (Main Menu)** e selecione **Cyber Menu (Exp)**.
