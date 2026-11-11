#include "DynamicConfigManager.h"
#include "SystemModel.h"
#include <SD.h>
#include <SPIFFS.h>
#include <globals.h>

DynamicConfigManager &DynamicConfigManager::getInstance() {
  static DynamicConfigManager instance;
  return instance;
}

bool DynamicConfigManager::init() {
  Serial.println("[DynamicConfigManager] Inicializando...");

  // Carrega configuração crítica do NVS como fallback
  CriticalConfig critical = loadCriticalConfig();
  Serial.println(
      "[DynamicConfigManager] Configuração crítica carregada do NVS");

  // Tenta carregar configuração padrão
  loadDefaultConfig();

  // Tenta carregar do arquivo se SD card estiver montado
  if (SystemModel::getInstance().getGlobalState().sdcardMounted) {
    if (loadConfigFromFile("/config/system_config.json")) {
      Serial.println(
          "[DynamicConfigManager] Configuração carregada do SD card");
    } else {
      Serial.println("[DynamicConfigManager] Falha ao carregar configuração do "
                     "SD card, usando padrão");
    }
  } else {
    Serial.println("[DynamicConfigManager] SD card não montado, usando "
                   "configuração padrão");
  }

  Serial.println("[DynamicConfigManager] Inicialização completa");
  return true;
}

void DynamicConfigManager::process() {
  // Verifica hot-reload a cada 5 segundos
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();

    if (checkFileModified()) {
      Serial.println("[DynamicConfigManager] Arquivo de configuração "
                     "modificado, recarregando...");
      reloadConfig();
    }
  }
}

void DynamicConfigManager::deinit() {
  // Salva configuração crítica atual no NVS antes de desinicializar
  auto &systemModel = SystemModel::getInstance();
  CriticalConfig critical;
  critical.bright = systemModel.getConfig().bright;
  critical.wifiAtStartup = systemModel.getConfig().wifiAtStartup;
  critical.startupApp = systemModel.getConfig().startupApp;
  critical.instantBoot = systemModel.getConfig().instantBoot;
  critical.bgColor = systemModel.getConfig().bgColor;
  critical.themePath = systemModel.getConfig().themePath;
  critical.themeFS = systemModel.getConfig().themeFS;

  saveCriticalConfig(critical);
  Serial.println("[DynamicConfigManager] Configuração crítica salva no NVS");
}

bool DynamicConfigManager::loadConfigFromFile(const String &filename) {
  if (!SystemModel::getInstance().getGlobalState().sdcardMounted) {
    Serial.println("[DynamicConfigManager] SD card não montado");
    return false;
  }

  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.printf("[DynamicConfigManager] Falha ao abrir arquivo: %s\n",
                  filename.c_str());
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("[DynamicConfigManager] Erro ao parsear JSON: %s\n",
                  error.c_str());
    return false;
  }

  // Valida configuração
  if (!validateConfig(doc)) {
    Serial.println("[DynamicConfigManager] Configuração inválida");
    return false;
  }

  // Aplica configuração
  applyConfig(doc);

  // Atualiza estado interno
  configFilename_ = filename;
  currentConfig_ = doc;
  configLoaded_ = true;
  lastModified_ = file.getLastWrite(); // Nota: pode não funcionar em todos os
                                       // sistemas de arquivos

  Serial.printf("[DynamicConfigManager] Configuração carregada de: %s\n",
                filename.c_str());
  return true;
}

bool DynamicConfigManager::reloadConfig() {
  if (configFilename_.isEmpty()) {
    Serial.println(
        "[DynamicConfigManager] Nenhum arquivo de configuração definido");
    return false;
  }

  return loadConfigFromFile(configFilename_);
}

void DynamicConfigManager::setSchemaValidator(SchemaValidator validator) {
  schemaValidator_ = validator;
}

void DynamicConfigManager::setConfigUpdateCallback(
    ConfigUpdateCallback callback) {
  updateCallback_ = callback;
}

void DynamicConfigManager::saveCriticalConfig(const CriticalConfig &config) {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, false);

  prefs.putInt("bright", config.bright);
  prefs.putBool("wifiAtStartup", config.wifiAtStartup);
  prefs.putString("startupApp", config.startupApp);
  prefs.putBool("instantBoot", config.instantBoot);
  prefs.putUShort("bgColor", config.bgColor);
  prefs.putString("themePath", config.themePath);
  prefs.putBool("themeFS", config.themeFS);

  prefs.end();
}

DynamicConfigManager::CriticalConfig
DynamicConfigManager::loadCriticalConfig() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, true);

  CriticalConfig config;
  config.bright = prefs.getInt("bright", 100);
  config.wifiAtStartup = prefs.getBool("wifiAtStartup", false);
  config.startupApp = prefs.getString("startupApp", "");
  config.instantBoot = prefs.getBool("instantBoot", false);
  config.bgColor = prefs.getUShort("bgColor", 0x0000);
  config.themePath = prefs.getString("themePath", "");
  config.themeFS = prefs.getBool("themeFS", false);

  prefs.end();
  return config;
}

void DynamicConfigManager::reloadConfigCommand() {
  if (reloadConfig()) {
    serialDevice->println("Configuração recarregada com sucesso");
  } else {
    serialDevice->println("Falha ao recarregar configuração");
  }
}

bool DynamicConfigManager::checkFileModified() {
  if (configFilename_.isEmpty() ||
      !SystemModel::getInstance().getGlobalState().sdcardMounted) {
    return false;
  }

  File file = SD.open(configFilename_, FILE_READ);
  if (!file) {
    return false;
  }

  time_t currentModified = file.getLastWrite();
  file.close();

  if (currentModified != lastModified_) {
    lastModified_ = currentModified;
    return true;
  }

  return false;
}

bool DynamicConfigManager::validateConfig(const JsonDocument &config) {
  // Validação básica: deve ser um objeto
  if (!config.is<JsonObject>()) {
    Serial.println(
        "[DynamicConfigManager] Configuração deve ser um objeto JSON");
    return false;
  }

  // Usa validador customizado se definido
  if (schemaValidator_) {
    return schemaValidator_(config);
  }

  // Validação padrão: verifica campos conhecidos
  config.as<JsonObjectConst>();

  // Campos obrigatórios podem ser adicionados aqui
  // Por exemplo:
  // if (!obj.containsKey("version")) {
  //   Serial.println("[DynamicConfigManager] Campo 'version' obrigatório");
  //   return false;
  // }

  return true;
}

void DynamicConfigManager::applyConfig(const JsonDocument &config) {
  // Usa o método do SystemModel para atualização dinâmica
  SystemModel::getInstance().updateConfigFromJson(config);

  // Chama callback de atualização se definido
  if (updateCallback_) {
    updateCallback_(config);
  }

  Serial.println("[DynamicConfigManager] Configuração aplicada ao sistema");
}

void DynamicConfigManager::loadDefaultConfig() {
  // Carrega configuração padrão do SystemModel
  auto &systemModel = SystemModel::getInstance();
  systemModel.loadConfig();

  // Converte para JsonDocument
  currentConfig_.clear();
  currentConfig_["bright"] = systemModel.getConfig().bright;
  currentConfig_["wifiAtStartup"] = systemModel.getConfig().wifiAtStartup;
  currentConfig_["startupApp"] = systemModel.getConfig().startupApp;
  currentConfig_["instantBoot"] = systemModel.getConfig().instantBoot;
  currentConfig_["bgColor"] = systemModel.getConfig().bgColor;
  currentConfig_["themePath"] = systemModel.getConfig().themePath;
  currentConfig_["themeFS"] = systemModel.getConfig().themeFS;

  configLoaded_ = true;
  Serial.println("[DynamicConfigManager] Configuração padrão carregada");
}