# Willy 2.1 - Interface Web

**Hub de Segurança Ofensiva para Pentest**

---

## 📋 Visão Geral

O Willy 2.1 é uma interface web moderna e futurista para dispositivos ESP32, projetada para ferramentas de segurança ofensiva e pentest. A interface é 100% responsiva, funciona em desktop, tablet e mobile, e é otimizada para rodar em dispositivos com recursos limitados.

### Características Principais

- **100% em Português Brasileiro** - Interface, menus, logs e avisos
- **Design Cyberpunk Neon** - Tema escuro com cores vibrantes
- **Gráficos Real-time** - Chart.js para visualização de dados em tempo real
- **PWA Ready** - Instalável como aplicativo no celular/desktop
- **Leve** - Menos de 100KB total (otimizado para ESP32)
- **Seguro** - Sistema de autenticação com criptografia

---

## 🚀 Funcionalidades

### 1. Sistema de Autenticação

No primeiro acesso, o sistema solicita a criação de um usuário e senha. As credenciais são criptografadas e salvas no dispositivo.

**Características:**
- Criptografia das credenciais
- Sessão com expiração (24 horas)
- Alteração de senha nas configurações
- Proteção contra acesso não autorizado

### 2. Painel (Dashboard)

Visão geral do sistema com:

- **Cards de Status**: RAM, SD Card, Flash, WiFi
- **Gráficos Real-time**: Uso de memória e sinal WiFi RSSI
- **Status dos Módulos**: WiFi, Bluetooth, RF, IR, GPS, BadUSB, NFC, NRF24
- **Controles Rápidos**: Reiniciar, Navegador, Brilho, Terminal
- **Informações do Sistema**: Chip, CPU, tela, versão

### 3. Navegador Remoto

Controle total do dispositivo como se estivesse tocando na tela:

- Visualização em tempo real da tela TFT
- Botões de navegação (cima, baixo, esquerda, direita, OK, voltar)
- Atalhos de teclado
- Auto-refresh configurável

### 4. Módulos

Controles detalhados para cada módulo:

#### WiFi
- Escanear redes
- Modo AP
- Ataques: Deauth, Beacon, Probe, PMKID

#### RF Sub-GHz
- Receber sinais
- Frequências: 315, 433, 868, 915 MHz
- Escanear

#### Infravermelho
- Receber códigos
- Biblioteca de códigos

#### BadUSB
- Scripts de ataque
- Criar novo script

#### Bluetooth
- Escanear dispositivos
- Ataque Spam

#### GPS
- Iniciar/Parar rastreamento
- Modo Wardrive
- Coordenadas em tempo real

#### NFC/RFID
- Ler tags
- Escrever tags
- Clonar

#### NRF24
- Escanear
- Receber sinais

### 5. Arquivos

Gerenciador de arquivos completo:

- Navegação entre SD Card e Flash interna
- Upload de arquivos
- Criar pastas e arquivos
- Editar arquivos com editor de código
- Executar scripts (.js, .bjs, .ir, .sub)
- Download de arquivos
- Excluir e renomear
- Drag & Drop para upload

### 6. Logs

Sistema de logs completo:

- Filtro por módulo (WiFi, BLE, RF, IR, GPS, NFC)
- Busca por texto
- Exportar para TXT
- Limpar logs

### 7. Terminal

Terminal serial integrado:

- Envio de comandos
- Histórico de comandos (setas ↑↓)
- Saída formatada
- Limpar terminal

### 8. Capturas

Visualização de capturas:

- PCAP (capturas de rede)
- IR Codes (códigos infravermelho)
- Sub-GHz (sinais RF)
- NFC Dumps
- GPS Logs

### 9. Scripts

Gerenciador de scripts:

- Listar scripts (.js, .bjs)
- Criar novo script
- Editar e executar

### 10. Configurações

#### Credenciais de Acesso
- Alterar usuário
- Alterar senha

#### Configurações de Tela
- Brilho (slider)
- Rotação (0°, 90°, 180°, 270°)

#### Logo (Configurações do Splash)
- Velocidade da animação
- Ativar/Desativar som
- Tipo de som (Rugido, Esguicho, Silêncio)
- Cor customizada (independente do tema)

#### Sistema
- Reiniciar dispositivo
- Restaurar padrões

---

## 🎨 Temas

O sistema suporta múltiplos temas. Para trocar o tema, edite o arquivo `theme.css`:

### Temas Disponíveis

| Tema | Cor Primária | Descrição |
|------|-------------|-----------|
| Padrão | `#00f0ff` | Ciano neon cyberpunk |
| Cyberpunk | `#ff00ff` | Magenta neon |
| Matrix | `#00ff00` | Verde hacker |
| Oceano | `#00bcd4` | Azul marinho |
| Púrpura | `#a855f7` | Roxo místico |
| Solar | `#ff6b35` | Laranja intenso |
| Sangue | `#ff3333` | Vermelho |

### Trocar Tema

1. Abra o arquivo `theme.css`
2. Descomente o bloco do tema desejado
3. Comente o tema atual (se necessário)

---

## 🔧 API Endpoints

A interface se comunica com o dispositivo através dos seguintes endpoints:

### Sistema
- `GET /systeminfo` - Informações do sistema (JSON)
- `POST /cm` - Executar comando

### Arquivos
- `GET /listfiles?fs=SD&folder=/` - Listar arquivos
- `GET /file?fs=SD&name=/caminho&action=download` - Baixar arquivo
- `GET /file?fs=SD&name=/caminho&action=edit` - Editar arquivo
- `GET /file?fs=SD&action=delete&name=/caminho` - Excluir
- `POST /edit` - Salvar arquivo editado
- `POST /rename` - Renomear arquivo

### Tela
- `GET /getscreen` - Obter tela atual (binário)

---

## 📁 Estrutura de Arquivos

```
embedded_resources/web_interface/
├── index.html        # Página principal
├── index.css         # Estilos
├── index.js          # JavaScript
├── theme.css         # Tema customizável
├── manifest.json     # PWA manifest
└── checksum.sha256   # Checksum dos arquivos
```

---

## 🖥️ Splash Screen (Baleia Orca)

O splash screen do Willy apresenta uma animação de baleia orca com:

### Animações
- **Entrada**: Orca vem de longe e cresce (1500ms)
- **Nadando**: Movimento suave de ondas (loop)
- **Esguicho**: Partículas de água animadas
- **Olhos piscando**: Fade out/in a cada 2 segundos
- **Boca**: Abre e fecha suavemente
- **Bolhas**: Subindo ao redor

### Som (Opcional)
- **Rugido**: Tom grave simulando orca
- **Esguicho**: Tom agudo simulando água
- **Configurável**: Pode ser desativado

### Configurações do Logo
Acesse Configurações → Logo para personalizar:

| Opção | Descrição |
|-------|-----------|
| Velocidade | Velocidade da animação (lento/normal/rápido) |
| Som | Ativar/desativar som do boot |
| Tipo de Som | Rugido, Esguicho, ou Silêncio |
| Cor Customizada | Usar cor própria (não segue tema) |
| Cor | Seletor de cor para a orca |

---

## 📱 Responsividade

A interface se adapta automaticamente:

| Dispositivo | Breakpoint | Layout |
|-------------|------------|--------|
| Mobile | < 480px | Sidebar colapsada, cards em coluna |
| Tablet | 480px - 768px | Sidebar colapsada, grid adaptativo |
| Desktop | > 768px | Sidebar expandida, grid completo |

---

## 🔒 Segurança

### Credenciais
- Armazenadas criptografadas no localStorage
- Senha com hash próprio
- Sessão expira em 24 horas

### Recomendações
- Use senha forte (mínimo 6 caracteres)
- Troque a senha regularmente
- Não compartilhe credenciais

---

## 📦 Dependências Externas

A interface utiliza CDNs para bibliotecas externas:

- **Font Awesome 6.5.1** - Ícones
- **Chart.js** - Gráficos

> Nota: Em ambientes sem internet, essas bibliotecas devem ser baixadas e servidas localmente.

---

## 🛠️ Desenvolvimento

### Teste Local

1. Inicie um servidor local na pasta `embedded_resources/web_interface/`
2. Acesse `http://localhost:8080`

### Deploy no Dispositivo

1. Compile o firmware com os arquivos da interface
2. Os arquivos serão embarcados na partição LittleFS
3. Acesse `http://[IP_DO_DISPOSITIVO]/`

---

## 📝 Changelog

### v2.1 (Atual)
- Renomeado para Willy
- Interface completamente em Português BR
- Sistema de autenticação com primeiro acesso
- Gráficos real-time com Chart.js
- Splash screen com baleia orca animada
- Configurações de logo personalizáveis
- Múltiplos temas
- PWA support

---

## 🤝 Contribuição

Para contribuir com o projeto:

1. Faça um fork do repositório
2. Crie uma branch para sua feature
3. Commit suas mudanças
4. Abra um Pull Request

---

## 📄 Licença

Este projeto é open source. Consulte o arquivo LICENSE para mais detalhes.

---

**Willy 2.1** - Hub de Segurança Ofensiva para Pentest

*Use com responsabilidade. Apenas em ambientes autorizados.*