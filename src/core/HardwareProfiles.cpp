#include "HardwareProfiles.h"
#include "HardwareDetector.h"
#include "advanced_logger.h"
#include <SPIFFS.h>

HardwareProfiles &HardwareProfiles::getInstance() {
  static HardwareProfiles instance;
  return instance;
}

HardwareProfiles::HardwareProfiles() : current_profile_("none") {
  loadDefaultProfiles();
}

void HardwareProfiles::loadDefaultProfiles() {
  createESP32S3Profiles();
  createESP32S2Profiles();
  createESP32C3Profiles();
  createGenericProfiles();

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Profiles de hardware padrão carregados: %d profiles",
      profiles_.size());
}

bool HardwareProfiles::autoDetectAndApplyProfile() {
  HardwareInfo info = HardwareDetector::getInstance().detectHardware();

  String best_profile = "";

  // Tenta encontrar profile específico baseado na variante
  switch (info.variant) {
  case ESP32Variant::ESP32_S3:
    best_profile = "esp32-s3-willy"; // Profile padrão
    break;
  case ESP32Variant::ESP32_S2:
    best_profile = "esp32-s2-generic";
    break;
  case ESP32Variant::ESP32_C3:
    best_profile = "esp32-c3-generic";
    break;
  default:
    best_profile = "esp32-generic";
    break;
  }

  if (applyProfile(best_profile)) {
    current_profile_ = best_profile;
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Profile automático aplicado: %s",
                                       best_profile.c_str());
    return true;
  }

  AdvancedLogger::getInstance().warning(
      LogModule::SYSTEM, "Falha ao aplicar profile automático: %s",
      best_profile.c_str());
  return false;
}

bool HardwareProfiles::applyProfile(const String &profile_name) {
  auto it = profiles_.find(profile_name);
  if (it == profiles_.end()) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Profile não encontrado: %s", profile_name.c_str());
    return false;
  }

  const HardwareProfile &profile = it->second;

  // Verifica se a variante é compatível
  HardwareInfo info = HardwareDetector::getInstance().detectHardware();
  if (profile.variant != ESP32Variant::UNKNOWN &&
      profile.variant != info.variant) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Profile %s não compatível com variante %s",
        profile_name.c_str(), info.chip_model.c_str());
  }

  // Aplica configuração de pinos
  if (!applyPinConfiguration(profile)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM,
        "Falha ao aplicar configuração de pinos do profile %s",
        profile_name.c_str());
    return false;
  }

  // Aplica configuração de periféricos
  if (!applyPeripheralConfiguration(profile)) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM,
        "Alguns periféricos do profile %s falharam ao inicializar",
        profile_name.c_str());
  }

  current_profile_ = profile_name;
  AdvancedLogger::getInstance().info(LogModule::SYSTEM, "Profile aplicado: %s",
                                     profile_name.c_str());
  return true;
}

std::vector<String> HardwareProfiles::listProfiles() {
  std::vector<String> names;
  for (auto &pair : profiles_) {
    names.push_back(pair.first);
  }
  return names;
}

const HardwareProfile *HardwareProfiles::getProfile(const String &name) const {
  auto it = profiles_.find(name);
  return (it != profiles_.end()) ? &it->second : nullptr;
}

bool HardwareProfiles::createCustomProfile(const HardwareProfile &profile) {
  if (profiles_.find(profile.name) != profiles_.end()) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Profile já existe: %s", profile.name.c_str());
    return false;
  }

  profiles_[profile.name] = profile;
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Profile personalizado criado: %s",
                                     profile.name.c_str());
  return true;
}

bool HardwareProfiles::saveProfilesToFile(const String &path) {
  if (!SPIFFS.begin(true)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao montar SPIFFS para salvar profiles");
    return false;
  }

  JsonDocument doc;

  JsonArray profiles_array = doc["profiles"].to<JsonArray>();
  for (auto &pair : profiles_) {
    JsonObject p = profiles_array.add<JsonObject>();
    p["name"] = pair.second.name;
    p["description"] = pair.second.description;
    p["variant"] = static_cast<int>(pair.second.variant);

    // Salva pinos
    JsonArray pins = p["pins"].to<JsonArray>();
    for (auto &pin : pair.second.pin_configs) {
      JsonObject pin_obj = pins.add<JsonObject>();
      pin_obj["pin"] = pin.pin_number;
      pin_obj["mode"] = static_cast<int>(pin.mode);
      pin_obj["inverted"] = pin.inverted;
      pin_obj["description"] = pin.description;
    }

    // Salva periféricos
    JsonObject peripherals = p["peripherals"].to<JsonObject>();
    for (auto &peri : pair.second.peripheral_configs) {
      peripherals[peri.first] = peri.second;
    }
  }

  File file = SPIFFS.open(path, "w");
  if (!file) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao criar arquivo de profiles: %s",
        path.c_str());
    return false;
  }

  serializeJsonPretty(doc, file);
  file.close();

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Profiles salvos em: %s", path.c_str());
  return true;
}

bool HardwareProfiles::loadProfilesFromFile(const String &path) {
  if (!SPIFFS.begin(true)) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao montar SPIFFS para carregar profiles");
    return false;
  }

  if (!SPIFFS.exists(path)) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Arquivo de profiles não encontrado: %s",
        path.c_str());
    return false;
  }

  File file = SPIFFS.open(path, "r");
  if (!file) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Falha ao abrir arquivo de profiles: %s",
        path.c_str());
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Erro ao parsear profiles: %s", error.c_str());
    return false;
  }

  JsonArray profiles_array = doc["profiles"];
  for (JsonObject p : profiles_array) {
    HardwareProfile profile(p["name"], p["description"]);
    profile.variant = static_cast<ESP32Variant>(p["variant"].as<int>());

    // Carrega pinos
    JsonArray pins = p["pins"];
    for (JsonObject pin_obj : pins) {
      PinConfig pin(pin_obj["pin"],
                    static_cast<PinMode>(pin_obj["mode"].as<int>()),
                    pin_obj["inverted"], pin_obj["description"]);
      profile.pin_configs.push_back(pin);
    }

    // Carrega periféricos
    JsonObject peripherals = p["peripherals"];
    for (JsonPair kv : peripherals) {
      profile.peripheral_configs[kv.key().c_str()] = kv.value();
    }

    profiles_[profile.name] = profile;
  }

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Profiles carregados de: %s", path.c_str());
  return true;
}

String HardwareProfiles::generateProfileReport() {
  String report = "=== RELATÓRIO DE PROFILE ===\n";
  report += "Profile atual: " + current_profile_ + "\n\n";

  if (current_profile_ != "none") {
    const HardwareProfile *profile = getProfile(current_profile_);
    if (profile) {
      report += "Nome: " + profile->name + "\n";
      report += "Descrição: " + profile->description + "\n";
      report +=
          "Variante: " + String(static_cast<int>(profile->variant)) + "\n\n";

      report += "PINOS CONFIGURADOS:\n";
      for (auto &pin : profile->pin_configs) {
        report +=
            "GPIO" + String(pin.pin_number) + ": " + pin.description + "\n";
      }

      report += "\nPERIFÉRICOS:\n";
      for (auto &peri : profile->peripheral_configs) {
        report += peri.first + "\n";
      }
    }
  }

  report += "\nPROFILES DISPONÍVEIS:\n";
  for (auto &name : listProfiles()) {
    report += "- " + name + "\n";
  }

  return report;
}

void HardwareProfiles::createESP32S3Profiles() {
  // Profile para ESP32-S3 com display ILI9341 (Willy padrão)
  HardwareProfile willy_s3;
  willy_s3.name = "esp32-s3-willy";
  willy_s3.description =
      "ESP32-S3 com display ILI9341 (configuração Willy padrão)";
  willy_s3.variant = ESP32Variant::ESP32_S3;

  // Configuração de pinos para display ILI9341
  willy_s3.pin_configs.push_back(
      PinConfig(10, PinMode::PIN_OUTPUT, false, "TFT_CS"));
  willy_s3.pin_configs.push_back(
      PinConfig(11, PinMode::PIN_OUTPUT, false, "TFT_DC"));
  willy_s3.pin_configs.push_back(
      PinConfig(12, PinMode::PIN_OUTPUT, false, "TFT_RST"));
  willy_s3.pin_configs.push_back(
      PinConfig(13, PinMode::PIN_OUTPUT, false, "TFT_MOSI"));
  willy_s3.pin_configs.push_back(
      PinConfig(14, PinMode::PIN_OUTPUT, false, "TFT_SCK"));

  // Configuração de pinos para SD card
  willy_s3.pin_configs.push_back(
      PinConfig(15, PinMode::PIN_OUTPUT, false, "SD_CS"));
  willy_s3.pin_configs.push_back(
      PinConfig(16, PinMode::PIN_INPUT, false, "SD_MOSI"));
  willy_s3.pin_configs.push_back(
      PinConfig(17, PinMode::PIN_OUTPUT, false, "SD_MISO"));
  willy_s3.pin_configs.push_back(
      PinConfig(18, PinMode::PIN_OUTPUT, false, "SD_SCK"));

  // Configuração do periférico display
  JsonDocument display_config;
  display_config["type"] = "ILI9341";
  display_config["width"] = 320;
  display_config["height"] = 240;
  display_config["rotation"] = 1;
  willy_s3.peripheral_configs["display"] = display_config;

  profiles_[willy_s3.name] = willy_s3;

  // Profile para ESP32-S3 com display ST7789
  HardwareProfile st7789_s3;
  st7789_s3.name = "esp32-s3-st7789";
  st7789_s3.description = "ESP32-S3 com display ST7789";
  st7789_s3.variant = ESP32Variant::ESP32_S3;

  st7789_s3.pin_configs.push_back(
      PinConfig(10, PinMode::PIN_OUTPUT, false, "TFT_CS"));
  st7789_s3.pin_configs.push_back(
      PinConfig(11, PinMode::PIN_OUTPUT, false, "TFT_DC"));
  st7789_s3.pin_configs.push_back(
      PinConfig(12, PinMode::PIN_OUTPUT, false, "TFT_RST"));
  st7789_s3.pin_configs.push_back(
      PinConfig(13, PinMode::PIN_OUTPUT, false, "TFT_MOSI"));
  st7789_s3.pin_configs.push_back(
      PinConfig(14, PinMode::PIN_OUTPUT, false, "TFT_SCK"));

  JsonDocument st7789_config;
  st7789_config["type"] = "ST7789";
  st7789_config["width"] = 240;
  st7789_config["height"] = 240;
  st7789_config["rotation"] = 0;
  st7789_s3.peripheral_configs["display"] = st7789_config;

  profiles_[st7789_s3.name] = st7789_s3;
}

void HardwareProfiles::createESP32S2Profiles() {
  HardwareProfile s2_generic;
  s2_generic.name = "esp32-s2-generic";
  s2_generic.description = "ESP32-S2 genérico";
  s2_generic.variant = ESP32Variant::ESP32_S2;

  // Configuração básica
  s2_generic.pin_configs.push_back(
      PinConfig(34, PinMode::PIN_OUTPUT, false, "TFT_CS"));
  s2_generic.pin_configs.push_back(
      PinConfig(33, PinMode::PIN_OUTPUT, false, "TFT_DC"));
  s2_generic.pin_configs.push_back(
      PinConfig(35, PinMode::PIN_OUTPUT, false, "TFT_RST"));

  profiles_[s2_generic.name] = s2_generic;
}

void HardwareProfiles::createESP32C3Profiles() {
  HardwareProfile c3_generic;
  c3_generic.name = "esp32-c3-generic";
  c3_generic.description = "ESP32-C3 genérico (limitado por pinos)";
  c3_generic.variant = ESP32Variant::ESP32_C3;

  // Configuração limitada para ESP32-C3
  c3_generic.pin_configs.push_back(
      PinConfig(7, PinMode::PIN_OUTPUT, false, "TFT_CS"));
  c3_generic.pin_configs.push_back(
      PinConfig(6, PinMode::PIN_OUTPUT, false, "TFT_DC"));
  c3_generic.pin_configs.push_back(
      PinConfig(5, PinMode::PIN_OUTPUT, false, "TFT_RST"));

  profiles_[c3_generic.name] = c3_generic;
}

void HardwareProfiles::createGenericProfiles() {
  HardwareProfile generic;
  generic.name = "esp32-generic";
  generic.description = "ESP32 genérico";
  generic.variant = ESP32Variant::ESP32_CLASSIC;

  // Configuração padrão ESP32
  generic.pin_configs.push_back(
      PinConfig(15, PinMode::PIN_OUTPUT, false, "TFT_CS"));
  generic.pin_configs.push_back(PinConfig(2, PinMode::PIN_OUTPUT, false, "TFT_DC"));
  generic.pin_configs.push_back(
      PinConfig(4, PinMode::PIN_OUTPUT, false, "TFT_RST"));
  generic.pin_configs.push_back(
      PinConfig(23, PinMode::PIN_OUTPUT, false, "TFT_MOSI"));
  generic.pin_configs.push_back(
      PinConfig(18, PinMode::PIN_OUTPUT, false, "TFT_SCK"));
  generic.pin_configs.push_back(PinConfig(5, PinMode::PIN_OUTPUT, false, "SD_CS"));

  profiles_[generic.name] = generic;
}

bool HardwareProfiles::applyPinConfiguration(const HardwareProfile &profile) {
  return PinAbstraction::configurePins(profile.pin_configs);
}

bool HardwareProfiles::applyPeripheralConfiguration(
    const HardwareProfile &profile) {
  bool success = true;
  for (auto &peri : profile.peripheral_configs) {
    if (!PeripheralAbstraction::initializePeripheral(peri.first, peri.second)) {
      AdvancedLogger::getInstance().warning(
          LogModule::SYSTEM, "Falha ao inicializar periférico %s do profile",
          peri.first.c_str());
      success = false;
    }
  }
  return success;
}
