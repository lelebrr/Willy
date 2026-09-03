#include "PeripheralAbstraction.h"
#include "HardwareDetector.h"
#include "advanced_logger.h"
#include <SD.h>
#include <SPIFFS.h>

// Inicialização de membros estáticos
std::map<String, std::unique_ptr<IPeripheral>>
    PeripheralAbstraction::peripherals_;
std::map<String, std::vector<PinConfig>>
    PeripheralAbstraction::peripheral_pins_;

PeripheralAbstraction &PeripheralAbstraction::getInstance() {
  static PeripheralAbstraction instance{}; // Usando construtor padrão
  return instance;
}

bool PeripheralAbstraction::registerPeripheral(
    std::unique_ptr<IPeripheral> peripheral) {
  if (!peripheral) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Tentativa de registrar periférico nulo");
    return false;
  }

  String name = peripheral->getName();
  if (peripherals_.find(name) != peripherals_.end()) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Periférico '%s' já registrado", name.c_str());
    return false;
  }

  peripherals_[name] = std::move(peripheral);
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Periférico '%s' registrado", name.c_str());
  return true;
}

void PeripheralAbstraction::unregisterPeripheral(const String &name) {
  auto it = peripherals_.find(name);
  if (it != peripherals_.end()) {
    // Desinicializar se necessário
    if (it->second->getStatus() == PeripheralStatus::READY) {
      it->second->deinitialize();
    }
    peripherals_.erase(it);
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "Periférico '%s' removido", name.c_str());
  }
}

IPeripheral *PeripheralAbstraction::getPeripheral(const String &name) {
  auto it = peripherals_.find(name);
  return (it != peripherals_.end()) ? it->second.get() : nullptr;
}

std::map<String, IPeripheral *> PeripheralAbstraction::getAllPeripherals() {
  std::map<String, IPeripheral *> result;
  for (auto &pair : peripherals_) {
    result[pair.first] = pair.second.get();
  }
  return result;
}

std::vector<IPeripheral *>
PeripheralAbstraction::getPeripheralsByType(PeripheralType type) {
  std::vector<IPeripheral *> result;
  for (auto &pair : peripherals_) {
    if (pair.second->getType() == type) {
      result.push_back(pair.second.get());
    }
  }
  return result;
}

bool PeripheralAbstraction::initializePeripheral(const String &name,
                                                 const JsonDocument &config) {
  IPeripheral *peripheral = getPeripheral(name);
  if (!peripheral) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Periférico '%s' não encontrado", name.c_str());
    return false;
  }

  if (!peripheral->isAvailable()) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Periférico '%s' não disponível no hardware",
        name.c_str());
    return false;
  }

  bool success = peripheral->initialize(config);
  if (success) {
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "Periférico '%s' inicializado", name.c_str());
  } else {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Falha ao inicializar periférico '%s'",
                                        name.c_str());
  }
  return success;
}

bool PeripheralAbstraction::deinitializePeripheral(const String &name) {
  IPeripheral *peripheral = getPeripheral(name);
  if (!peripheral) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Periférico '%s' não encontrado", name.c_str());
    return false;
  }

  bool success = peripheral->deinitialize();
  if (success) {
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "Periférico '%s' desinicializado", name.c_str());
  } else {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao desinicializar periférico '%s'",
        name.c_str());
  }
  return success;
}

bool PeripheralAbstraction::isPeripheralAvailable(const String &name) {
  IPeripheral *peripheral = getPeripheral(name);
  return peripheral && peripheral->isAvailable();
}

JsonDocument PeripheralAbstraction::getSystemStatus() {
  JsonDocument status;

  JsonArray peripherals = status["peripherals"].to<JsonArray>();
  for (auto &pair : peripherals_) {
    JsonObject p = peripherals.add<JsonObject>();
    p["name"] = pair.first;
    p["type"] = static_cast<int>(pair.second->getType());
    p["status"] = static_cast<int>(pair.second->getStatus());
    p["available"] = pair.second->isAvailable();
    p["info"] = pair.second->getInfo();
  }

  status["total_peripherals"] = peripherals_.size();
  status["timestamp"] = millis();

  return status;
}

bool PeripheralAbstraction::configurePeripheralPins(
    const String &peripheral_name, const std::vector<PinConfig> &pin_configs) {
  if (!PinAbstraction::configurePins(pin_configs)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao configurar pinos para periférico '%s'",
        peripheral_name.c_str());
    return false;
  }

  peripheral_pins_[peripheral_name] = pin_configs;
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Pinos configurados para periférico '%s'",
                                     peripheral_name.c_str());
  return true;
}

bool PeripheralAbstraction::loadPeripheralConfig(const String &config_path) {
  if (!SPIFFS.begin(true)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao montar SPIFFS para carregar configuração");
    return false;
  }

  if (!SPIFFS.exists(config_path)) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Arquivo de configuração '%s' não encontrado",
        config_path.c_str());
    return false;
  }

  File file = SPIFFS.open(config_path, "r");
  if (!file) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao abrir arquivo de configuração '%s'",
        config_path.c_str());
    return false;
  }

  JsonDocument config;
  DeserializationError error = deserializeJson(config, file);
  file.close();

  if (error) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Erro ao parsear configuração: %s", error.c_str());
    return false;
  }

  // Aplicar configuração
  JsonArray peripherals_config = config["peripherals"];
  for (JsonObject p : peripherals_config) {
    String name = p["name"];
    bool enabled = p["enabled"] | true;

    if (enabled) {
      JsonDocument p_config = p["config"];
      initializePeripheral(name, p_config);
    }
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Configuração de periféricos carregada de '%s'",
      config_path.c_str());
  return true;
}

bool PeripheralAbstraction::savePeripheralConfig(const String &config_path) {
  if (!SPIFFS.begin(true)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao montar SPIFFS para salvar configuração");
    return false;
  }

  JsonDocument config;

  JsonArray peripherals_config = config["peripherals"].to<JsonArray>();
  for (auto &pair : peripherals_) {
    JsonObject p = peripherals_config.add<JsonObject>();
    p["name"] = pair.first;
    p["type"] = static_cast<int>(pair.second->getType());
    p["enabled"] =
        (pair.second->getStatus() != PeripheralStatus::PERIPH_DISABLED);
    p["status"] = static_cast<int>(pair.second->getStatus());
    p["available"] = pair.second->isAvailable();
  }

  config["timestamp"] = millis();

  File file = SPIFFS.open(config_path, "w");
  if (!file) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao criar arquivo de configuração '%s'",
        config_path.c_str());
    return false;
  }

  serializeJsonPretty(config, file);
  file.close();

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Configuração de periféricos salva em '%s'",
      config_path.c_str());
  return true;
}