#include "PluginRegistry.h"
#include "../../core/advanced_logger.h"

// Singleton instance
PluginRegistry *PluginRegistry::instance_ = nullptr;

PluginRegistry &PluginRegistry::getInstance() {
  if (!instance_) {
    instance_ = new PluginRegistry();
  }
  return *instance_;
}

bool PluginRegistry::registerPlugin(const String &name, PluginFactory factory) {
  if (factories_.find(name) != factories_.end()) {
    logWarning("Plugin já registrado: " + name);
    return false;
  }

  factories_[name] = factory;
  logInfo("Plugin registrado: " + name);
  return true;
}

bool PluginRegistry::unregisterPlugin(const String &name) {
  auto it = factories_.find(name);
  if (it == factories_.end()) {
    logWarning("Plugin não encontrado para remover: " + name);
    return false;
  }

  factories_.erase(it);
  logInfo("Plugin removido: " + name);
  return true;
}

std::unique_ptr<IPlugin>
PluginRegistry::createPlugin(const String &name,
                             const JsonDocument &config) const {
  auto it = factories_.find(name);
  if (it == factories_.end()) {
    logWarning("Plugin não registrado: " + name);
    return nullptr;
  }

  try {
    auto plugin = it->second(name, config);
    logInfo("Plugin criado: " + name);
    return plugin;
  } catch (const std::exception &e) {
    logError("Erro ao criar plugin " + name + ": " + e.what());
    return nullptr;
  }
}

bool PluginRegistry::isRegistered(const String &name) const {
  return factories_.find(name) != factories_.end();
}

std::vector<String> PluginRegistry::listRegisteredPlugins() const {
  std::vector<String> names;
  for (const auto &pair : factories_) {
    names.push_back(pair.first);
  }
  return names;
}