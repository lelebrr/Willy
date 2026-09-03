#include "SystemManager.h"
#include "advanced_logger.h"
#include <Arduino.h>
#include <ArduinoJson.h>

SystemManager &SystemManager::getInstance() {
  static SystemManager instance;
  return instance;
}

void SystemManager::registerModule(std::unique_ptr<IModule> module) {
  if (!module) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Tentativa de registrar módulo nulo");
    return;
  }

  String moduleName = module->getName();

  // Verificar se já existe
  if (getModule(moduleName) != nullptr) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Módulo '%s' já registrado, sobrescrevendo",
        moduleName.c_str());
    // Remove o módulo existente
    auto it = std::find_if(modules_.begin(), modules_.end(),
                           [&moduleName](const std::unique_ptr<IModule> &m) {
                             return m && m->getName() == moduleName;
                           });
    if (it != modules_.end()) {
      modules_.erase(it);
    }
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Registrando módulo: %s", moduleName.c_str());
  modules_.push_back(std::move(module));

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Total de módulos registrados: %d", modules_.size());
}

bool SystemManager::initAllModules() {
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Inicializando %d módulos...", modules_.size());

  bool allSuccess = true;
  size_t successCount = 0;

  for (auto &module : modules_) {
    if (!module) {
      AdvancedLogger::getInstance().warning(
          LogModule::SYSTEM, "Módulo nulo encontrado durante inicialização");
      continue;
    }

    String moduleName = module->getName();
    AdvancedLogger::getInstance().debug(
        LogModule::SYSTEM, "Inicializando módulo: %s", moduleName.c_str());

    if (!module->init()) {
      AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                          "Falha ao inicializar módulo: %s",
                                          moduleName.c_str());
      allSuccess = false;
    } else {
      AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                         "Módulo inicializado com sucesso: %s",
                                         moduleName.c_str());
      successCount++;
    }
  }

  // Inicializar PluginManager
  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Inicializando PluginManager");
  if (!PluginManager::getInstance().init()) {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Falha ao inicializar PluginManager");
    allSuccess = false;
  } else {
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "PluginManager inicializado com sucesso");
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM,
      "Inicialização completa: %d/%d módulos inicializados com sucesso",
      successCount, modules_.size());

  return allSuccess;
}

void SystemManager::deinitAllModules() {
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Desinicializando %d módulos...", modules_.size());

  size_t deinitCount = 0;

  // Desinicializar na ordem reversa de inicialização
  for (auto it = modules_.rbegin(); it != modules_.rend(); ++it) {
    auto &module = *it;
    if (module) {
      String moduleName = module->getName();
      AdvancedLogger::getInstance().debug(
          LogModule::SYSTEM, "Desinicializando módulo: %s", moduleName.c_str());

      try {
        module->deinit();
        AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                           "Módulo desinicializado: %s",
                                           moduleName.c_str());
        deinitCount++;
      } catch (const std::exception &e) {
        AdvancedLogger::getInstance().error(
            LogModule::SYSTEM, "Exceção ao desinicializar módulo %s: %s",
            moduleName.c_str(), e.what());
      }
    }
  }

  modules_.clear();

  // Desinicializar PluginManager
  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Desinicializando PluginManager");
  try {
    PluginManager::getInstance().deinit();
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "PluginManager desinicializado");
  } catch (const std::exception &e) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Exceção ao desinicializar PluginManager: %s",
        e.what());
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "%d módulos desinicializados", deinitCount);
}

void SystemManager::processAllModules() {
  static uint32_t lastProcessTime = 0;
  uint32_t currentTime = millis();

  // Limitar frequência de processamento se necessário
  if (currentTime - lastProcessTime < 10) { // Máximo 100Hz
    return;
  }
  lastProcessTime = currentTime;

  size_t activeCount = 0;

  for (auto &module : modules_) {
    if (module && module->isActive()) {
      try {
        module->process();
        activeCount++;
      } catch (const std::exception &e) {
        AdvancedLogger::getInstance().error(
            LogModule::SYSTEM, "Exceção no processamento do módulo %s: %s",
            module->getName().c_str(), e.what());
      }
    }
  }

  // Processar plugins
  try {
    PluginManager::getInstance().processPlugins();
  } catch (const std::exception &e) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Exceção no processamento de plugins: %s", e.what());
  }

  // Log periódico de status (a cada 60 segundos)
  static uint32_t lastStatusLog = 0;
  if (currentTime - lastStatusLog > 60000) {
    AdvancedLogger::getInstance().debug(
        LogModule::SYSTEM, "%d módulos ativos processados", activeCount);
    lastStatusLog = currentTime;
  }
}

IModule *SystemManager::getModule(const String &name) {
  for (auto &module : modules_) {
    if (module && module->getName() == name) {
      return module.get();
    }
  }
  return nullptr;
}

void SystemManager::listModules() const {
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "=== LISTA DE MÓDULOS REGISTRADOS ===");

  if (modules_.empty()) {
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Nenhum módulo registrado");
    return;
  }

  for (const auto &module : modules_) {
    if (module) {
      AdvancedLogger::getInstance().info(
          LogModule::SYSTEM, "  - %s (%s, prioridade %d)",
          module->getName().c_str(), module->isActive() ? "ativo" : "inativo",
          module->getPriority());
    }
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM, "Total: %d módulos",
                                     modules_.size());
}