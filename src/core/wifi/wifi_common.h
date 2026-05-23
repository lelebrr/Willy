#include "core/display.h"
#include <NTPClient.h>
#include <WiFi.h>

#ifndef __WIFI_COMMON_H__
#define __WIFI_COMMON_H__
class WifiCommon {
public:
    /**
     * @brief disconnects and turns off wifi module
     */
    static void disconnect();

    /**
     * @brief Opens a menu to connect to a wifi
     * @param mode connection mode(AP, STA, AP_STA)
     * @note This is the primary entry point for establishing connections
     * @note returns false if wifi is already connected
     */
    static bool connectMenu(wifi_mode_t mode = WIFI_MODE_STA);

    /**
     * @brief Scans the networks and tries to connect to a known network
     * @param mode connection mode(void)
     * @note This is the primary entry point for establishing connections in the Headless environment
     * @note returns true if connected successfully
     */
    static bool connectToKnownNet(void);

    /**
     * @brief returns MAC adress
     */
    static String checkMAC();

    /**
     * @brief tries to connect to min(found_networks, maxSearch) networks
     * using stored passwords
     * @TODO fix: rn it skips open networks due to password == "" check
     */
    static void connectTask(void *pvParameters);

    /**
     * @brief Ensures esp_netif and the default event loop are initialized (idempotent)
     */
    static void ensurePlatform();

    /**
     * @brief sets up wifi in AP mode
     * @note wifi.mode should be set before calling the method
     */
    static bool _setupAP();

private:
    /**
     * @brief Connects to wifiNetwork
     */
    static bool _wifiConnect(const String &ssid, int encryption);
    static bool _connectToWifiNetwork(const String &ssid, const String &pwd);

    static void updateTimezoneTask(void *pvParameters);
};

#endif
