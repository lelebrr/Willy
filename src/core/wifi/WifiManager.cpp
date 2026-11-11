#include "core/wifi/WifiManager.h"
#include "core/wifi/wifi_common.h"
#include <globals.h>

WifiManager::WifiManager() {}

WifiManager::~WifiManager() {}

void WifiManager::ensureWifiPlatform() { ::ensureWifiPlatform(); }

bool WifiManager::connectToKnownNet(void) { return ::wifiConnecttoKnownNet(); }

bool WifiManager::connectMenu(wifi_mode_t mode) {
  return ::wifiConnectMenu(mode);
}

bool WifiManager::disconnect() {
  ::wifiDisconnect();
  return true;
}

String WifiManager::getMacAddress() { return ::checkMAC(); }

bool WifiManager::isConnected() { return ::wifiConnected; }

String WifiManager::getIpAddress() { return ::wifiIP; }

void WifiManager::startConnectTask(void *pvParameters) {
  ::wifiConnectTask(pvParameters);
}

void WifiManager::startTimezoneTask(void *pvParameters) {
  ::updateTimezoneTask(pvParameters);
}

bool WifiManager::_connectToWifiNetwork(const String &ssid, const String &pwd) {
  return ::_connectToWifiNetwork(ssid, pwd);
}

bool WifiManager::_setupAP() { return ::_setupAP(); }

bool WifiManager::_wifiConnect(const String &ssid, int encryption) {
  return ::_wifiConnect(ssid, encryption);
}

// Global instance
WifiManager wifiManager;
