#include "WifiMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/wifi/webInterface.h"
#include "core/wifi/wg.h"
#include "core/wifi/wifi_common.h"
#include "core/wifi/wifi_mac.h"
#include "modules/ethernet/ARPScanner.h"
#include "modules/wifi/ap_info.h"
#include "modules/wifi/clients.h"
#include "modules/wifi/evil_portal.h"
#include "modules/wifi/karma_attack.h"
#include "modules/wifi/responder.h"
#include "modules/wifi/scan_hosts.h"
#include "modules/wifi/sniffer.h"
#include "modules/wifi/wifi_atks.h"
#include "modules/wifi/wifi_heatmap.h"
#include "modules/wifi/advanced_atks.h"
#include "modules/wifi/wps_atks.h"



#ifndef LITE_VERSION
#include "modules/wifi/wifi_recover.h"
#include "modules/pwnagotchi/pwnagotchi.h"
#endif

// #include "modules/reverseShell/reverseShell.h"
//  Developed by Fourier (github.com/9dl)
//  Use BruceC2 to interact with the reverse shell server
//  BruceC2: https://github.com/9dl/Bruce-C2
//  To use BruceC2:
//  1. Start Reverse Shell Mode in Bruce
//  2. Start BruceC2 and wait.
//  3. Visit 192.168.4.1 in your browser to access the web interface for shell executing.

// 32bit: https://github.com/9dl/Bruce-C2/releases/download/v1.0/BruceC2_windows_386.exe
// 64bit: https://github.com/9dl/Bruce-C2/releases/download/v1.0/BruceC2_windows_amd64.exe
#include "modules/wifi/tcp_utils.h"

// global toggle - controls whether scanNetworks includes hidden SSIDs
bool showHiddenNetworks = false;

void WifiMenu::optionsMenu() {
    returnToMenu = false;
    options.clear();
    if (isWebUIActive) {
        drawMainBorderWithTitle("WiFi", true);
        padprintln("");
        padprintln("Iniciar uma funcao WiFi ira provavelmente parar a WebUI");
        padprintln("");
        padprintln("Sel: Continuar");
        padprintln("Qualquer: Menu");
        while (1) {
            if (check(SelPress)) { break; }
            if (check(AnyKeyPress)) { return; }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
        options = {
            Option("Conectar WiFi", lambdaHelper(wifiConnectMenu, WIFI_STA)),
            Option("Iniciar WiFi AP", [=]() {
                 wifiConnectMenu(WIFI_AP);
                 displayInfo("pwd: " + bruceConfig.wifiAp.pwd, true);
             }),
        };
    }
    if (WiFi.getMode() != WIFI_MODE_NULL)    options.push_back(Option("Desligar WiFi", wifiDisconnect));
    if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_APSTA) {
        options.push_back(Option("Info AP", []() {
            wifi_ap_record_t info;
            if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
                displayAPInfo(info);
            } else {
                displayError("Failed to get AP info");
            }
        }));
    }
    options.push_back(Option("Ataques WiFi", [=]() {
                         if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
                             if (displayMessage("Atacar em modo AP pode\ndesconectar clientes.\nContinuar?", "Não",
                                                nullptr, "Sim", TFT_YELLOW) != 2)
                                 return;
                         }
                         wifi_atk_menu();
                     }));
    options.push_back(Option("Ataques Avancados", [=]() {
                         if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
                             if (displayMessage("Atacar em modo AP pode\ndesconectar clientes.\nContinuar?", "Não",
                                                nullptr, "Sim", TFT_YELLOW) != 2)
                                 return;
                         }
                         advancedAtksMenu();
                     }));
    options.push_back(Option("WPS Attacks", [=]() {
                         if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
                             if (displayMessage("Atacar em modo AP pode\ndesconectar clientes.\nContinuar?", "Não",
                                                nullptr, "Sim", TFT_YELLOW) != 2)
                                 return;
                         }
                         wpsAttacksMenu();
                     }));
    options.push_back(Option("WiFi Sniffers", [=]() {
                         if (WiFi.getMode() != WIFI_OFF) {
                             if (displayMessage("Sniffer ira resetar a\nconexao atual.\nContinuar?", "Não", nullptr,
                                                "Sim", TFT_YELLOW) != 2)
                                 return;
                         }
                         wifiSniffersMenu();
                     }));
    options.push_back(Option("WiFi Attacks", [=]() {
                         if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
                             if (displayMessage("Atacar em modo AP pode\ndesconectar clientes.\nContinuar?", "Não",
                                                nullptr, "Sim", TFT_YELLOW) != 2)
                                 return;
                         }
                         wifiAttacksMenu();
                     }));

    options.push_back({"Evil Portal", [=]() {
                           if (isWebUIActive || server) {
                               stopWebUi();
                               wifiDisconnect();
                           }
                           EvilPortal();
                       }});
    // options.push_back({"ReverseShell", [=]()       { ReverseShell(); }});
#ifndef LITE_VERSION
    options.push_back(Option("Ouvir TCP", listenTcpPort));
    options.push_back(Option("Cliente TCP", clientTCP));
    options.push_back(Option("TelNET", telnet_setup));
    options.push_back(Option("SSH", lambdaHelper(ssh_setup, String(""))));
    options.push_back({"Farejadores", [this]() {
                           std::vector<Option> snifferOptions;
                           snifferOptions.push_back({"Sniffer Bruto", sniffer_setup});
                           snifferOptions.push_back({"Sniffer Probe Request", karma_setup});
                           snifferOptions.push_back({"Voltar", [this]() { optionsMenu(); }});

                           loopOptions(snifferOptions, MENU_TYPE_SUBMENU, "Farejadores");
                       }});
    options.push_back({"Escanear Hosts", [=]() {
                           bool doScan = true;
                           if (!wifiConnected) doScan = wifiConnectMenu();

                           if (doScan) {
                               esp_netif_t *esp_netinterface =
                                   esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
                               if (esp_netinterface == nullptr) {
                                   Serial.println("Failed to get netif handle");
                                   return;
                               }
                               ARPScanner{esp_netinterface};
                           }
                       }});
    options.push_back({"Wireguard", wg_setup});
    options.push_back({"Responder", responder});
    options.push_back({"Brucegotchi", brucegotchi_start});
    options.push_back({"Recuperar Senha", wifi_recover_menu});
    options.push_back({"WiFi Heatmap", wifiHeatmap});
    options.push_back({"Traffic Fingerprint", encryptedTrafficFingerprint});
#endif

    options.push_back({"Config", [this]() { configMenu(); }});

    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "WiFi");

    options.clear();
}

void WifiMenu::configMenu() {
    std::vector<Option> wifiOptions;

    wifiOptions.push_back({"Mudar MAC", wifiMACMenu});
    wifiOptions.push_back({"Add Evil Wifi", addEvilWifiMenu});
    wifiOptions.push_back({"Remover Evil Wifi", removeEvilWifiMenu});

    // Evil Wifi Settings submenu (unchanged)
    wifiOptions.push_back({"Config Evil Wifi", [this]() {
                               std::vector<Option> evilOptions;

                               evilOptions.push_back({"Modo Senha", setEvilPasswordMode});
                               evilOptions.push_back({"Renomear /creds", setEvilEndpointCreds});
                               evilOptions.push_back({"Permitir /creds", setEvilAllowGetCreds});
                               evilOptions.push_back({"Renomear /ssid", setEvilEndpointSsid});
                               evilOptions.push_back({"Permitir /ssid", setEvilAllowSetSsid});
                               evilOptions.push_back({"Mostrar endpoints", setEvilAllowEndpointDisplay});
                               evilOptions.push_back({"Voltar", [this]() { configMenu(); }});
                               loopOptions(evilOptions, MENU_TYPE_SUBMENU, "Config Evil Wifi");
                           }});

    {

        String hidden__wifi_option = String("Redes Ocultas: ") + (showHiddenNetworks ? "LIG" : "DES");

        // construct Option explicitly using char* label
        Option opt(hidden__wifi_option.c_str(), [this]() {
            showHiddenNetworks = !showHiddenNetworks;
            displayInfo(String("Redes Ocultas: ") + (showHiddenNetworks ? "LIG" : "DES"), true);
            configMenu();
        });

        wifiOptions.push_back(opt);
    }
    wifiOptions.push_back({"Voltar", [this]() { optionsMenu(); }});
    loopOptions(wifiOptions, MENU_TYPE_SUBMENU, "Config WiFi");
}

void WifiMenu::drawIcon(float scale) {
    clearIconArea();
    int deltaY = scale * 20;
    int radius = scale * 6;

    tft.fillCircle(iconCenterX, iconCenterY + deltaY, radius, bruceConfig.priColor);
    tft.drawArc(
        iconCenterX,
        iconCenterY + deltaY,
        deltaY + radius,
        deltaY,
        130,
        230,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY + deltaY,
        2 * deltaY + radius,
        2 * deltaY,
        130,
        230,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
