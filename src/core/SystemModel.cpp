#include "SystemModel.h"
#include "advanced_logger.h"
#include <ArduinoJson.h>
#include <Preferences.h>

SystemModel &SystemModel::getInstance() {
  static SystemModel instance;
  return instance;
}

bool SystemModel::init() {
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Inicializando SystemModel");

  // Carregar configurações
  loadConfig();

  // Validar configurações carregadas
  if (!validateConfig()) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM,
        "Configurações inválidas detectadas, aplicando padrões");
    resetToDefaults();
    saveConfig();
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "SystemModel inicializado");
  return true;
}

void SystemModel::saveConfig() {
  Preferences prefs;
  if (!prefs.begin("system_config", false)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao abrir Preferences para escrita");
    return;
  }

  // Validar valores antes de salvar
  SystemConfig validatedConfig = config_;
  if (!validateConfigValues(validatedConfig)) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM,
        "Valores de configuração inválidos detectados, corrigindo");
    validatedConfig = getDefaultConfig();
  }

  prefs.putInt("bright", validatedConfig.bright);
  prefs.putBool("wifiAtStartup", validatedConfig.wifiAtStartup);
  prefs.putString("startupApp", validatedConfig.startupApp);
  prefs.putBool("instantBoot", validatedConfig.instantBoot);
  prefs.putUShort("bgColor", validatedConfig.bgColor);
  prefs.putString("themePath", validatedConfig.themePath);
  prefs.putBool("themeFS", validatedConfig.themeFS);

  prefs.end();
  AdvancedLogger::getInstance().info(LogModule::SYSTEM, "Configurações salvas");
}

void SystemModel::loadConfig() {
  Preferences prefs;
  if (!prefs.begin("system_config", true)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao abrir Preferences para leitura");
    return;
  }

  config_.bright = prefs.getInt("bright", 100);
  config_.wifiAtStartup = prefs.getBool("wifiAtStartup", false);
  config_.startupApp = prefs.getString("startupApp", "");
  config_.instantBoot = prefs.getBool("instantBoot", false);
  config_.bgColor = prefs.getUShort("bgColor", 0x0000);
  config_.themePath = prefs.getString("themePath", "");
  config_.themeFS = prefs.getBool("themeFS", false);

  prefs.end();

  // Aplicar validação após carregamento
  if (!validateConfigValues(config_)) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Configurações carregadas inválidas");
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Configurações carregadas");
}

void SystemModel::resetToDefaults() {
  config_ = getDefaultConfig();
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Configurações resetadas para padrão");
}

SystemModel::SystemConfig SystemModel::getDefaultConfig() {
  SystemConfig defaults;
  defaults.bright = 100;
  defaults.wifiAtStartup = false;
  defaults.startupApp = "";
  defaults.instantBoot = false;
  defaults.bgColor = 0x0000; // TFT_BLACK
  defaults.themePath = "";
  defaults.themeFS = false;
  LOG_INFO(SYSTEM, "Configuração padrão retornada");
  return defaults;
}

bool SystemModel::validateConfig() const {
  return validateConfigValues(config_);
}

bool SystemModel::validateConfigValues(const SystemConfig &config) const {
  // Validar brilho (0-100)
  if (config.bright < 0 || config.bright > 100) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Brilho inválido: %d (deve ser 0-100)",
        config.bright);
    return false;
  }

  // Validar cor de fundo (0-0xFFFF para TFT)
  // config.bgColor é uint16_t, sempre <= 0xFFFF
  
  // Validar caminho do tema (não vazio se especificado)
  if (!config.themePath.isEmpty() && config.themePath.length() > 256) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Caminho do tema muito longo: %d caracteres",
        config.themePath.length());
    return false;
  }

  return true;
}

void SystemModel::updateConfigFromJson(const JsonDocument &jsonConfig) {
  if (!jsonConfig.is<JsonObject>()) {
    Serial.println("[SystemModel] Erro: JSON inválido ou não é um objeto");
    return;
  }
  JsonObjectConst obj = jsonConfig.as<JsonObjectConst>();

  // Atualiza apenas campos presentes no JSON
  if (obj["bright"].is<int>()) {
    config_.bright = obj["bright"];
    Serial.printf("[SystemModel] Brilho atualizado: %d\n", config_.bright);
  }
  if (obj["wifiAtStartup"].is<bool>()) {
    config_.wifiAtStartup = obj["wifiAtStartup"];
    Serial.printf("[SystemModel] WiFi no startup atualizado: %s\n",
                  config_.wifiAtStartup ? "true" : "false");
  }
  if (obj["startupApp"].is<String>()) {
    config_.startupApp = (String)obj["startupApp"];
    Serial.printf("[SystemModel] App de startup atualizado: %s\n",
                  config_.startupApp.c_str());
  }
  if (obj["instantBoot"].is<bool>()) {
    config_.instantBoot = obj["instantBoot"];
    Serial.printf("[SystemModel] Boot instantâneo atualizado: %s\n",
                  config_.instantBoot ? "true" : "false");
  }
  if (obj["bgColor"].is<int>()) {
    config_.bgColor = obj["bgColor"];
    Serial.printf("[SystemModel] Cor de fundo atualizada: 0x%04X\n",
                  config_.bgColor);
  }
  if (obj["themePath"].is<String>()) {
    config_.themePath = (String)obj["themePath"];
    Serial.printf("[SystemModel] Caminho do tema atualizado: %s\n",
                  config_.themePath.c_str());
  }
  if (obj["themeFS"].is<bool>()) {
    config_.themeFS = obj["themeFS"];
    Serial.printf("[SystemModel] FS do tema atualizado: %s\n",
                  config_.themeFS ? "SD" : "LittleFS");
  }

  // Salva automaticamente após atualização
  saveConfig();
  Serial.println("[SystemModel] Configuração atualizada dinamicamente");
}