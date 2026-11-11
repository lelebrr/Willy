#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include "core/display.h"
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFi.h>


class WifiManager {
public:
  WifiManager();
  ~WifiManager();

  // Initialize WiFi platform
  void ensureWifiPlatform();

  // Connection methods
  bool connectToKnownNet(void);
  bool connectMenu(wifi_mode_t mode = WIFI_MODE_STA);
  bool disconnect();

  // Getter methods
  String getMacAddress();
  bool isConnected();
  String getIpAddress();

  // Task methods
  void startConnectTask(void *pvParameters);
  void startTimezoneTask(void *pvParameters);

  // Private helper methods
private:
  bool _connectToWifiNetwork(const String &ssid, const String &pwd);
  bool _setupAP();
  bool _wifiConnect(const String &ssid, int encryption);
};

extern WifiManager wifiManager;

#endif
