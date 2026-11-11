#include "plugin_commands.h"
#include "../../modules/plugins/PluginManager.h"
#include <ArduinoJson.h>
#include <globals.h>

uint32_t pluginListCallback(cmd *c) {
  Command cmd(c);

  auto &pm = PluginManager::getInstance();

  // Listar plugins carregados
  auto loaded = pm.listLoadedPlugins();
  serialDevice->println("Plugins carregados:");
  for (const String &name : loaded) {
    auto plugin = pm.getPlugin(name);
    if (plugin) {
      serialDevice->printf("  %s v%s - %s\n", plugin->getName().c_str(),
                           plugin->getVersion().c_str(),
                           plugin->getDescription().c_str());
    }
  }

  // Listar plugins disponíveis
  auto available = pm.listAvailablePlugins();
  serialDevice->println("\nPlugins disponíveis:");
  for (const String &name : available) {
    bool isLoaded = false;
    for (const String &loadedName : loaded) {
      if (loadedName == name) {
        isLoaded = true;
        break;
      }
    }
    if (!isLoaded) {
      serialDevice->printf("  %s (não carregado)\n", name.c_str());
    }
  }

  return true;
}

uint32_t pluginLoadCallback(cmd *c) {
  Command cmd(c);
  AdvancedLogger &logger = AdvancedLogger::getInstance();

  Argument nameArg = cmd.getArgument(0);
  if (nameArg.getValue().length() == 0) {
    logger.warning(LogModule::SYSTEM, "Plugin load command missing arguments");
    serialDevice->println("Uso: plugin load <nome> [config_json]");
    return false;
  }

  String name = nameArg.getValue();

  // Validações de segurança
  if (name.length() == 0 || name.length() > 50) {
    logger.warning(LogModule::SYSTEM, "Invalid plugin name length: %d",
                   name.length());
    serialDevice->println("ERROR: Nome do plugin inválido");
    return false;
  }

  // Sanitizar nome (apenas alfanumérico, underscore, hífen)
  for (char ch : name) {
    if (!isalnum(ch) && ch != '_' && ch != '-') {
      logger.warning(LogModule::SYSTEM, "Invalid character in plugin name: %c",
                     ch);
      serialDevice->println(
          "ERROR: Nome do plugin contém caracteres inválidos");
      return false;
    }
  }

  JsonDocument config;

  Argument configArg = cmd.getArgument(1);
  if (configArg.getValue().length() > 0) {
    String configStr = configArg.getValue();
    // Limitar tamanho do JSON
    if (configStr.length() > 2048) {
      logger.warning(LogModule::SYSTEM, "Config JSON too large: %d bytes",
                     configStr.length());
      serialDevice->println("ERROR: Configuração JSON muito grande");
      return false;
    }

    DeserializationError error = deserializeJson(config, configStr);
    if (error) {
      logger.error(LogModule::SYSTEM, "JSON deserialization error: %s",
                   error.c_str());
      serialDevice->printf("Erro no JSON de configuração: %s\n", error.c_str());
      return false;
    }
  }

  logger.info(LogModule::SYSTEM, "Loading plugin: %s", name.c_str());
  auto &pm = PluginManager::getInstance();
  if (pm.loadPlugin(name, config)) {
    serialDevice->printf("Plugin '%s' carregado com sucesso\n", name.c_str());
    logger.info(LogModule::SYSTEM, "Plugin '%s' loaded successfully",
                name.c_str());
    return true;
  } else {
    serialDevice->printf("Falha ao carregar plugin '%s'\n", name.c_str());
    logger.error(LogModule::SYSTEM, "Failed to load plugin '%s'", name.c_str());
    return false;
  }
}

uint32_t pluginUnloadCallback(cmd *c) {
  Command cmd(c);

  Argument unloadNameArg = cmd.getArgument(0);
  if (unloadNameArg.getValue().length() == 0) {
    serialDevice->println("Uso: plugin unload <nome>");
    return false;
  }

  String name = unloadNameArg.getValue();

  auto &pm = PluginManager::getInstance();
  if (pm.unloadPlugin(name)) {
    serialDevice->printf("Plugin '%s' descarregado com sucesso\n",
                         name.c_str());
    return true;
  } else {
    serialDevice->printf("Falha ao descarregar plugin '%s'\n", name.c_str());
    return false;
  }
}

uint32_t pluginReloadCallback(cmd *c) {
  Command cmd(c);

  auto &pm = PluginManager::getInstance();
  pm.reloadAllPlugins();
  serialDevice->println("Todos os plugins recarregados");
  return true;
}

uint32_t pluginInfoCallback(cmd *c) {
  Command cmd(c);

  Argument infoNameArg = cmd.getArgument(0);
  if (infoNameArg.getValue().length() == 0) {
    serialDevice->println("Uso: plugin info <nome>");
    return false;
  }

  String name = infoNameArg.getValue();

  auto &pm = PluginManager::getInstance();
  auto plugin = pm.getPlugin(name);

  if (!plugin) {
    serialDevice->printf("Plugin '%s' não encontrado\n", name.c_str());
    return false;
  }

  serialDevice->printf("Plugin: %s\n", plugin->getName().c_str());
  serialDevice->printf("Versão: %s\n", plugin->getVersion().c_str());
  serialDevice->printf("Descrição: %s\n", plugin->getDescription().c_str());
  serialDevice->printf("Ativo: %s\n", plugin->isActive() ? "Sim" : "Não");

  return true;
}

uint32_t pluginScanCallback(cmd *c) {
  Command cmd(c);

  auto &pm = PluginManager::getInstance();
  int count = pm.loadPluginsFromSD();
  serialDevice->printf("Escaneamento concluído. %d plugins carregados da SD\n",
                       count);
  return true;
}

void createPluginCommands(SimpleCLI *cli) {
  Command plugin = cli->addCommand("plugin", pluginListCallback);
  plugin.addPosArg("action", "list");

  // Subcomandos
  Command pluginLoad = cli->addCommand("plugin/load", pluginLoadCallback);
  pluginLoad.addPosArg("name");
  pluginLoad.addPosArg("config", "{}");

  Command pluginUnload = cli->addCommand("plugin/unload", pluginUnloadCallback);
  pluginUnload.addPosArg("name");

  Command pluginReload = cli->addCommand("plugin/reload", pluginReloadCallback);

  Command pluginInfo = cli->addCommand("plugin/info", pluginInfoCallback);
  pluginInfo.addPosArg("name");

  Command pluginScan = cli->addCommand("plugin/scan", pluginScanCallback);
}