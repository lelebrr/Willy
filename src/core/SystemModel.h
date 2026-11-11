#ifndef __SYSTEM_MODEL_H__
#define __SYSTEM_MODEL_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

/**
 * @brief Modelo do sistema - gerencia configurações e estado global
 * Parte do padrão MVC (Model)
 */
class SystemModel {
public:
  // Estados globais
  struct GlobalState {
    bool wifiConnected = false;
    bool bleConnected = false;
    bool sdcardMounted = false;
    bool gpsConnected = false;
    bool isSleeping = false;
    bool isScreenOff = false;
    bool dimmer = false;
    int8_t interpreter_state = -1;
    unsigned long previousMillis = 0;
    int prog_handler = 0; // 0 - Flash, 1 - LittleFS, 3 - Download
    String cachedPassword = "";
    String currentLoaderApp = "";
    bool appRequiresClose = false;
    String startupAppLuaScript = "";
  };

  // Configurações
  struct SystemConfig {
    int bright = 100;
    bool wifiAtStartup = false;
    String startupApp = "";
    bool instantBoot = false;
    uint16_t bgColor = 0x0000; // TFT_BLACK
    String themePath = "";
    bool themeFS = false; // false = LittleFS, true = SD
  };

  static SystemModel &getInstance();

  // Inicialização
  bool init();

  // Estado global
  GlobalState &getGlobalState() { return globalState_; }
  const GlobalState &getGlobalState() const { return globalState_; }

  // Configurações
  SystemConfig &getConfig() { return config_; }
  const SystemConfig &getConfig() const { return config_; }

  // Utilitários
  void saveConfig();
  void loadConfig();
  void resetToDefaults();
  bool validateConfig() const;
  SystemConfig getDefaultConfig();
  bool validateConfigValues(const SystemConfig &config) const;

  // Atualização dinâmica (para DynamicConfigManager)
  void updateConfigFromJson(const JsonDocument &jsonConfig);

private:
  SystemModel() = default;

public:
  ~SystemModel() = default;

  GlobalState globalState_;
  SystemConfig config_;
};

#endif // __SYSTEM_MODEL_H__