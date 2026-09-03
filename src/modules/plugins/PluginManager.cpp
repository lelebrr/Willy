#include "PluginManager.h"
#include "../../core/SecurityUtils.h"
#include "../../core/advanced_logger.h"
#include "IPlugin.h"
#include <ArduinoJson.h>
#include <SD.h>
#include <SPIFFS.h>

// Diretório de plugins na SD card
#define PLUGINS_DIR "/plugins"

// Classe para plugins baseados em JSON
class JsonPlugin : public IPlugin {
public:
  JsonPlugin(const String &name, const JsonDocument &config)
      : name_(name), config_(config) {}

  bool init(const JsonDocument &config) override {
    // Copiar configuração
    config_ = config;

    // Executar hook de inicialização se definido
    if (config_.containsKey("on_init")) {
      // TODO: Implementar execução de scripts Lua/Python ou ações JSON
      logInfo("Executando on_init para plugin: " + name_);
    }

    active_ = true;
    return true;
  }

  void deinit() override {
    // Executar hook de desinicialização se definido
    if (config_.containsKey("on_deinit")) {
      logInfo("Executando on_deinit para plugin: " + name_);
    }

    active_ = false;
  }

  void process() override {
    // Executar processamento periódico se definido
    if (config_.containsKey("on_process")) {
      // TODO: Implementar execução periódica
    }
  }

  String getName() const override { return name_; }
  String getVersion() const override {
    return config_.containsKey("version") ? config_["version"] : "1.0.0";
  }
  String getDescription() const override {
    return config_.containsKey("description") ? config_["description"] : "";
  }
  bool isActive() const override { return active_; }

private:
  String name_;
  JsonDocument config_;
  bool active_ = false;
};

// Singleton instance
PluginManager *PluginManager::instance_ = nullptr;

PluginManager &PluginManager::getInstance() {
  if (!instance_) {
    instance_ = new PluginManager();
  }
  return *instance_;
}

bool PluginManager::init() {
  if (initialized_) {
    return true;
  }

  logInfo("Inicializando PluginManager...");

  // Carregar plugins da SD card
  int loadedCount = loadPluginsFromSD();
  logInfo("PluginManager inicializado. Plugins carregados: " +
          String(loadedCount));

  initialized_ = true;
  return true;
}

void PluginManager::deinit() {
  if (!initialized_) {
    return;
  }

  logInfo("Desinicializando PluginManager...");

  // Descarregar todos os plugins
  for (auto &pair : loadedPlugins_) {
    if (pair.second) {
      pair.second->deinit();
    }
  }
  loadedPlugins_.clear();

  initialized_ = false;
  logInfo("PluginManager desinicializado.");
}

bool PluginManager::loadPlugin(const String &name, const JsonDocument &config) {
  if (!initialized_) {
    logError("PluginManager não inicializado");
    return false;
  }

  // Verificar se já está carregado
  if (loadedPlugins_.find(name) != loadedPlugins_.end()) {
    logWarning("Plugin já carregado: " + name);
    return true;
  }

  logInfo("Carregando plugin: " + name);

  // Determinar tipo do plugin baseado na configuração
  String type = "json"; // Default para JSON
  if (config.containsKey("type")) {
    type = config["type"].as<String>();
  }

  // Criar plugin
  auto plugin = createPlugin(type, name, config);
  if (!plugin) {
    logError("Falha ao criar plugin: " + name);
    return false;
  }

  // Inicializar plugin
  if (!plugin->init(config)) {
    logError("Falha ao inicializar plugin: " + name);
    return false;
  }

  // Validar plugin
  if (!validatePlugin(plugin.get(), config)) {
    logError("Plugin não passou na validação: " + name);
    plugin->deinit();
    return false;
  }

  // Adicionar à lista de carregados
  loadedPlugins_[name] = std::move(plugin);

  logInfo("Plugin carregado com sucesso: " + name);
  return true;
}

bool PluginManager::unloadPlugin(const String &name) {
  auto it = loadedPlugins_.find(name);
  if (it == loadedPlugins_.end()) {
    logWarning("Plugin não encontrado para descarregar: " + name);
    return false;
  }

  logInfo("Descarregando plugin: " + name);

  // Desinicializar plugin
  it->second->deinit();

  // Remover da lista
  loadedPlugins_.erase(it);

  logInfo("Plugin descarregado: " + name);
  return true;
}

IPlugin *PluginManager::getPlugin(const String &name) const {
  auto it = loadedPlugins_.find(name);
  return (it != loadedPlugins_.end()) ? it->second.get() : nullptr;
}

std::vector<String> PluginManager::listLoadedPlugins() const {
  std::vector<String> names;
  for (const auto &pair : loadedPlugins_) {
    names.push_back(pair.first);
  }
  return names;
}

std::vector<String> PluginManager::listAvailablePlugins() const {
  std::vector<String> names = listLoadedPlugins();

  // Adicionar plugins da SD card não carregados
  if (SD.exists(PLUGINS_DIR)) {
    File dir = SD.open(PLUGINS_DIR);
    if (dir && dir.isDirectory()) {
      File file = dir.openNextFile();
      while (file) {
        if (!file.isDirectory()) {
          String filename = file.name();
          if (filename.endsWith(".json")) {
            String pluginName = filename.substring(0, filename.length() - 5);
            // Verificar se já está na lista
            bool alreadyListed = false;
            for (const String &name : names) {
              if (name == pluginName) {
                alreadyListed = true;
                break;
              }
            }
            if (!alreadyListed) {
              names.push_back(pluginName);
            }
          }
        }
        file = dir.openNextFile();
      }
      dir.close();
    }
  }

  return names;
}

void PluginManager::processPlugins() {
  for (auto &pair : loadedPlugins_) {
    if (pair.second && pair.second->isActive()) {
      pair.second->process();
    }
  }
}

bool PluginManager::executePreHooks(const String &operation,
                                    const JsonDocument &params) {
  bool allow = true;
  for (auto &pair : loadedPlugins_) {
    if (pair.second && pair.second->isActive()) {
      if (!pair.second->onPreHook(operation, params)) {
        allow = false;
        logInfo("Plugin " + pair.first + " bloqueou operação: " + operation);
      }
    }
  }
  return allow;
}

void PluginManager::executePostHooks(const String &operation, bool success,
                                     const JsonDocument &result) {
  for (auto &pair : loadedPlugins_) {
    if (pair.second && pair.second->isActive()) {
      pair.second->onPostHook(operation, success, result);
    }
  }
}

bool PluginManager::processCommand(const String &command,
                                   const std::vector<String> &args) {
  for (auto &pair : loadedPlugins_) {
    if (pair.second && pair.second->isActive()) {
      if (pair.second->onCommand(command, args)) {
        return true; // Comando processado
      }
    }
  }
  return false; // Nenhum plugin processou
}

int PluginManager::loadPluginsFromSD() {
  int count = 0;

  if (!SD.exists(PLUGINS_DIR)) {
    logInfo("Diretório de plugins não existe: " + String(PLUGINS_DIR));
    return 0;
  }

  File dir = SD.open(PLUGINS_DIR);
  if (!dir || !dir.isDirectory()) {
    logError("Falha ao abrir diretório de plugins");
    return 0;
  }

  logInfo("Escaneando plugins em: " + String(PLUGINS_DIR));

  File file = dir.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String filename = file.name();
      if (filename.endsWith(".json")) {
        String pluginName = filename.substring(0, filename.length() - 5);

        // Ler arquivo JSON
        JsonDocument config;
        DeserializationError error = deserializeJson(config, file);

        if (error) {
          logError("Erro ao ler plugin " + pluginName + ": " + error.c_str());
        } else {
          // Verificar se deve carregar automaticamente
          bool autoLoad = config.containsKey("auto_load")
                              ? config["auto_load"].as<bool>()
                              : true;

          if (autoLoad) {
            if (loadPlugin(pluginName, config)) {
              count++;
            }
          } else {
            logInfo("Plugin " + pluginName +
                    " configurado para carregamento manual");
          }
        }
      }
    }
    file = dir.openNextFile();
  }

  dir.close();
  return count;
}

void PluginManager::reloadAllPlugins() {
  logInfo("Recarregando todos os plugins...");

  // Salvar lista de plugins carregados
  auto loadedNames = listLoadedPlugins();

  // Descarregar todos
  for (const String &name : loadedNames) {
    unloadPlugin(name);
  }

  // Recarregar da SD
  loadPluginsFromSD();

  logInfo("Recarregamento concluído");
}

std::unique_ptr<IPlugin>
PluginManager::createPlugin(const String &type, const String &name,
                            const JsonDocument &config) {
  if (type == "json") {
    // Criar plugin baseado em JSON
    return std::unique_ptr<IPlugin>(new JsonPlugin(name, config));
  } else if (type == "native") {
    // Plugins nativos (C++) - usar registry
    return PluginRegistry::getInstance().createPlugin(name, config);
  } else {
    logError("Tipo de plugin não suportado: " + type);
    return nullptr;
  }
}

bool PluginManager::validatePlugin(IPlugin *plugin,
                                   const JsonDocument &config) {
  if (!plugin) {
    return false;
  }

  // Verificações básicas de segurança
  SecurityUtils &security = SecurityUtils::getInstance();

  // Verificar assinatura se presente
  if (config.containsKey("signature")) {
    String signature = config["signature"];
    // TODO: Implementar verificação de assinatura
    logWarning("Verificação de assinatura não implementada");
  }

  // Verificar permissões
  if (config.containsKey("permissions")) {
    JsonArray permissions = config["permissions"];
    for (JsonVariant perm : permissions) {
      String permission = perm.as<String>();
      // TODO: Verificar se plugin tem permissões adequadas
      logInfo("Verificando permissão: " + permission);
    }
  }

  // Verificações adicionais podem ser adicionadas aqui

  return true;
}