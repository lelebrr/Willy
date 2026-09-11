# BadUSB Advanced Suite - Willy Firmware

## Visão Geral
Suite completa de 55+ funções HID/USB/BLE para o Willy Firmware. Acesse via menu **Outros → BadUSB & HID → BadUSB Suite Avancada**.

## Arquitetura
- **Header**: `src/modules/badusb_ble/badusb_advanced.h`
- **Implementação**: `src/modules/badusb_ble/badusb_advanced.cpp` (~1500 linhas)
- **Integração**: `src/core/menu_items/OthersMenu.cpp`
- **Correção**: `bleClickerSetup()` em `src/modules/others/clicker.cpp` implementado

## Categorias (8 menus)

### 1. Keyboard Attacks (10 funções)
| Função | Descrição |
|--------|-----------|
| `keyLoggerExfil` | Keylogger com exfiltração via PowerShell |
| `reverseShellBuilder` | Reverse shell configurável (IP/Porta) |
| `uacBypass` | Bypass UAC via fodhelper.exe |
| `powershellObfuscator` | PowerShell ofuscado |
| `cmdPersist` | Persistência via startup folder |
| `ransomwareSim` | Simulação educacional de ransomware |
| `credHarvesting` | Captura de credenciais via prompt |
| `browserDataExfil` | Exfiltração de dados do navegador |
| `clipboardHijacker` | Hijack de clipboard |
| `shutdownLoop` | Loop de desligamento |

### 2. Mouse Attacks (8 funções)
| Função | Descrição |
|--------|-----------|
| `mouseJiggler` | Anti-sleep jiggler adaptativo |
| `mouseGridPattern` | Padrão de movimento grid |
| `mouseRandomWalk` | Caminhada aleatória |
| `clickFlood` | Flood de cliques configurável |
| `dragDropAttack` | Drag and drop payload |
| `mouseDrawing` | Desenhar padrão circular |
| `scrollJammmer` | Jammer de scroll contínuo |
| `mouseEvasion` | Evasão de mouse |

### 3. Combo Attacks (8 funções)
| Função | Descrição |
|--------|-----------|
| `comboKeyMouse` | Keyboard + Mouse combo |
| `wormPropagation` | Worm de propagação USB |
| `multiStagePayload` | Payload multi-stage (3 stages) |
| `timedPayload` | Payload com timer configurável |
| `conditionalTrigger` | Gatilho condicional |
| `chainedCommands` | Comandos encadeados |
| `autoExploit` | Auto exploit chain |
| `massInfection` | Infecção em massa |

### 4. Social Engineering (8 funções)
| Função | Descrição |
|--------|-----------|
| `fakeUpdateScreen` | Tela de atualização falsa com progresso |
| `fakeLoginPrompt` | Prompt de login falso |
| `fakeErrorDialog` | Diálogo de erro falso |
| `fakeBlueScreen` | Tela azul da morte falsa (BSOD) |
| `popupSpam` | Spam de pop-ups configurável |
| `fakeShutdown` | Desligamento falso visual |
| `decoyDocument` | Documento isca |
| `attentionGrabber` | Chamador de atenção |

### 5. Exfiltration (7 funções)
| Função | Descrição |
|--------|-----------|
| `notepadExfil` | Exfiltração via notepad |
| `powershellExfil` | Exfiltração via PowerShell |
| `browserExfil` | Exfiltração de navegador |
| `wifiProfileExfil` | Exfiltração de perfis WiFi |
| `clipboardExfil` | Exfiltração de clipboard |
| `systemInfoExfil` | Exfiltração de info do sistema |
| `fileExfil` | Exfiltração de arquivos |

### 6. Persistence & Stealth (7 funções)
| Função | Descrição |
|--------|-----------|
| `startupPersist` | Persistência via startup folder |
| `scheduledTaskPersist` | Persistência via tarefa agendada |
| `registryPersist` | Persistência via registro Windows |
| `stealthMode` | Modo stealth ativo |
| `antiDetection` | Anti-detecção (desativa antivirus) |
| `cleanupTraces` | Limpeza de rastros |
| `selfDestruct` | Auto-destruição com confirmação |

### 7. Utility & Testing (7 funções)
| Função | Descrição |
|--------|-----------|
| `hidBenchTest` | Benchmark de performance HID |
| `keyDelayTester` | Teste de delay de teclas |
| `layoutConverter` | Conversor de layout (Win/Mac/Linux) |
| `scriptValidator` | Validador de script SD |
| `connectionTest` | Teste de conexão HID |
| `payloadPreview` | Preview de configuração |
| `exportToSD` | Export para SD |

### 8. Configurações
| Config | Descrição |
|--------|-----------|
| OS Target | Windows/Mac/Linux/Auto |
| Key Delay | Delay entre teclas (0-500ms) |
| Shell Padrao | PowerShell/CMD |
| Stealth Mode | Modo stealth |
| Export SD | Exportar logs para SD |

## Estruturas de Dados

### BadUSBAdvConfig
```cpp
struct BadUSBAdvConfig {
    int defaultDelay;      // Delay padrão entre ações (ms)
    int keyDelay;          // Delay entre teclas (ms)
    int payloadDelay;      // Delay entre payloads (ms)
    bool showFeedback;     // Mostrar feedback visual
    bool autoClose;        // Auto-fechar janelas
    int osTarget;          // 0=Windows, 1=Mac, 2=Linux, 3=Auto
    String defaultShell;   // Shell padrão
    bool stealthMode;      // Modo stealth ativo
    bool exportToSD;       // Exportar logs para SD
};
```

## Visuais
- Cards coloridos por categoria (Cyan, Green, Orange, Magenta, Red, Yellow, White)
- Header animado com título
- Footer com instruções
- Barras de progresso
- Confirmação visual com SIM/NAO
- Tela de boas-vindas com categorias

## Interface HID
- Usa `HIDInterface` abstrato (USB ou BLE)
- `badInitHid()` inicializa automaticamente
- `badOpenShell()` detecta OS e abre shell correto
- `badSendString()` envia texto com delay configurável

## Correções Implementadas
1. **bleClickerSetup()** - Implementado com BLE HID Mouse real (antes era placeholder)
2. **Menu integration** - BadUSB Suite Avancada adicionada ao menu

## Para Acessar
```
Menu → Outros → BadUSB & HID → BadUSB Suite Avancada
```

## Notas de Segurança
- Todas as funções são para fins **educacionais e de pesquisa em segurança**
- Use apenas em ambientes controlados e autorizados
- As simulações não causam danos reais ao sistema
