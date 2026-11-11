#ifndef WILLY_WEB_SERVER_H
#define WILLY_WEB_SERVER_H

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <map>
#include <vector>

class WillyWebServer {
private:
  ::WebServer server;
  String ssid;
  String password;
  bool isAuthenticated;
  String username;
  static const int BATTERY_PIN = 4;

  // Device state
  struct DeviceState {
    bool relay1 = false;
    bool relay2 = false;
    bool relay3 = false;
    bool relay4 = false;
    float temperature = 25.0;
    float humidity = 60.0;
    int wifiSignal = -60;
    int batteryLevel = 85;
    unsigned long uptime = 0;
    std::vector<String> logs;
  } deviceState;

  // Authentication methods
  bool checkAuthentication();
  void generateToken();

  // File serving methods
  void serveFile(const String &path, const String &contentType);
  void handleNotFound();
  String formatUptime(unsigned long uptime);
  int readBatteryLevel();

  // API methods
  void handleApiState();
  void handleApiRelay();
  void handleApiSettings();
  void handleApiLogs();
  void handleApiSystem();

  // Web page methods
  void handleRoot();
  void handleLogin();
  void handleLogout();
  void handleDashboard();
  void handleSettings();
  void handleLogs();
  void handleSystem();

  // Static file methods
  void handleCss();
  void handleJs();
  void handleImages();

public:
  WillyWebServer(const String &ssid, const String &password);
  void begin();
  void handleClient();
  void updateDeviceState();
  void addLog(const String &message);

  // Device control methods
  void setRelay(int relay, bool state);
  void setTemperature(float temp);
  void setHumidity(float hum);
  void setWifiSignal(int signal);
  void setBatteryLevel(int level);

  // Getters
  bool getRelayState(int relay);
  float getTemperature();
  float getHumidity();
  int getWifiSignal();
  int getBatteryLevel();
  unsigned long getUptime();
  std::vector<String> getLogs();
};

#endif // WILLY_WEB_SERVER_H
