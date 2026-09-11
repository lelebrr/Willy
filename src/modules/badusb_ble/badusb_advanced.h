/**
 * @file badusb_advanced.h
 * @brief Willy BadUSB Advanced Suite - 55+ HID Attack Functions
 * @author Willy Team
 * @date 2026
 */

#ifndef __BADUSB_ADVANCED_H__
#define __BADUSB_ADVANCED_H__

#include <Arduino.h>

// ============================================================================
// Main entry point
// ============================================================================
void badusbAdvancedSuiteMenu();

// ============================================================================
// 1. KEYBOARD ATTACKS (10 functions)
// ============================================================================
void keyLoggerExfil();           // Keylogger com exfiltração via notepad
void reverseShellBuilder();      // Reverse shell PowerShell
void uacBypass();                // Bypass UAC via fodhelper
void powershellObfuscator();    // PowerShell ofuscado
void cmdPersist();               // Persistência via startup
void ransomwareSim();            // Simulação educacional de ransomware
void credHarvesting();           // Captura de credenciais
void browserDataExfil();         // Exfiltração de dados do navegador
void clipboardHijacker();        // Hijack de clipboard
void shutdownLoop();             // Loop de desligamento

// ============================================================================
// 2. MOUSE ATTACKS (8 functions)
// ============================================================================
void mouseJiggler();             // Anti-sleep jiggler
void mouseGridPattern();         // Padrão de movimento grid
void mouseRandomWalk();          // Caminhada aleatória
void clickFlood();               // Flood de cliques
void dragDropAttack();           // Drag and drop payload
void mouseDrawing();             // Desenhar no paint
void scrollJammmer();            // Jammer de scroll
void mouseEvasion();             // Evasão de mouse

// ============================================================================
// 3. COMBO ATTACKS (8 functions)
// ============================================================================
void comboKeyMouse();            // Keyboard + Mouse combo
void wormPropagation();          // Worm de propagação
void multiStagePayload();        // Payload multi-stage
void timedPayload();             // Payload com timer
void conditionalTrigger();       // Gatilho condicional
void chainedCommands();          // Comandos encadeados
void autoExploit();              // Auto exploit chain
void massInfection();            // Infecção em massa

// ============================================================================
// 4. SOCIAL ENGINEERING (8 functions)
// ============================================================================
void fakeUpdateScreen();         // Tela de atualização falsa
void fakeLoginPrompt();          // Prompt de login falso
void fakeErrorDialog();          // Diálogo de erro falso
void fakeBlueScreen();           // Tela azul da morte falsa
void popupSpam();                // Spam de pop-ups
void fakeShutdown();             // Desligamento falso
void decoyDocument();            // Documento isca
void attentionGrabber();         // Chamador de atenção

// ============================================================================
// 5. EXFILTRATION (7 functions)
// ============================================================================
void notepadExfil();             // Exfiltração via notepad
void powershellExfil();          // Exfiltração via PowerShell
void browserExfil();             // Exfiltração de navegador
void wifiProfileExfil();         // Exfiltração de perfis WiFi
void clipboardExfil();           // Exfiltração de clipboard
void systemInfoExfil();          // Exfiltração de info do sistema
void fileExfil();                // Exfiltração de arquivos

// ============================================================================
// 6. PERSISTENCE & STEALTH (7 functions)
// ============================================================================
void startupPersist();           // Persistência via startup
void scheduledTaskPersist();     // Persistência via tarefa agendada
void registryPersist();          // Persistência via registro
void stealthMode();              // Modo steath
void antiDetection();            // Anti-detecção
void cleanupTraces();            // Limpeza de rastros
void selfDestruct();             // Auto-destruição

// ============================================================================
// 7. UTILITY & TESTING (7 functions)
// ============================================================================
void hidBenchTest();             // Benchmark de performance
void keyDelayTester();           // Teste de delay de teclas
void layoutConverter();          // Conversor de layout
void scriptValidator();          // Validador de script
void connectionTest();           // Teste de conexão
void payloadPreview();           // Preview de payload
void exportToSD();               // Export para SD

// ============================================================================
// Menu functions
// ============================================================================
void badusbKeyboardAtksMenu();   // Submenu de ataques de teclado
void badusbMouseAtksMenu();      // Submenu de ataques de mouse
void badusbComboAtksMenu();      // Submenu de ataques combo
void badusbSocialEngMenu();      // Submenu de engenharia social
void badusbExfilMenu();          // Submenu de exfiltração
void badusbPersistMenu();        // Submenu de persistência
void badusbUtilityMenu();        // Submenu de utilitários
void badusbConfigMenu();         // Submenu de configurações

// ============================================================================
// Configuration
// ============================================================================
struct BadUSBAdvConfig {
    int defaultDelay;            // Delay padrão entre ações (ms)
    int keyDelay;                // Delay entre teclas (ms)
    int payloadDelay;            // Delay entre payloads (ms)
    bool showFeedback;           // Mostrar feedback visual
    bool autoClose;              // Auto-fechar janelas
    int osTarget;                // 0=Windows, 1=Mac, 2=Linux, 3=Auto
    String defaultShell;         // Shell padrão
    bool stealthMode;            // Modo stealth ativo
    bool exportToSD;             // Exportar logs para SD

    BadUSBAdvConfig() :
        defaultDelay(500),
        keyDelay(10),
        payloadDelay(1000),
        showFeedback(true),
        autoClose(false),
        osTarget(3),
        defaultShell("powershell"),
        stealthMode(false),
        exportToSD(false) {}
};

extern BadUSBAdvConfig badUSBAdvConfig;

#endif // __BADUSB_ADVANCED_H__
