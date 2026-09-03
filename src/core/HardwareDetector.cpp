#include "HardwareDetector.h"
#include "advanced_logger.h"

HardwareDetector &HardwareDetector::getInstance() {
  static HardwareDetector instance;
  return instance;
}

HardwareDetector::HardwareDetector() : detected_(false) {}

HardwareInfo HardwareDetector::detectHardware() {
  if (detected_) {
    return cached_info_;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Iniciando detecção de hardware");

  HardwareInfo info;

  // Detectar variante do ESP32
  info.variant = detectESP32Variant();
  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Variante detectada: %d",
                                      static_cast<int>(info.variant));

  // Obter informações básicas do chip
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  info.chip_revision = chip_info.revision;
  info.cores = chip_info.cores;
  info.has_bluetooth = chip_info.features & CHIP_FEATURE_BT;
  info.has_wifi = chip_info.features & CHIP_FEATURE_WIFI_BGN;
  info.cpu_freq_mhz = ESP.getCpuFreqMHz();

  // Detectar flash e PSRAM
  info.flash_size = detectFlashSize();
  info.psram_size = detectPsramSize();
  info.has_psram = (info.psram_size > 0);

  // Obter modelo do chip
  switch (info.variant) {
  case ESP32Variant::ESP32:
    info.chip_model = "ESP32";
    break;
  case ESP32Variant::ESP32_S2:
    info.chip_model = "ESP32-S2";
    break;
  case ESP32Variant::ESP32_S3:
    info.chip_model = "ESP32-S3";
    break;
  case ESP32Variant::ESP32_C3:
    info.chip_model = "ESP32-C3";
    break;
  case ESP32Variant::ESP32_C6:
    info.chip_model = "ESP32-C6";
    break;
  case ESP32Variant::ESP32_H2:
    info.chip_model = "ESP32-H2";
    break;
  default:
    info.chip_model = "Unknown";
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Variante ESP32 desconhecida detectada");
    break;
  }

  // Obter endereço MAC
  uint8_t mac[6];
  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
    char mac_str[18];
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);
    info.mac_address = String(mac_str);
  } else {
    info.mac_address = "00:00:00:00:00:00";
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Falha ao ler endereço MAC");
  }

  // Detectar pinos disponíveis
  detectAvailablePins(info);

  // Definir pinout padrão
  setDefaultPinout(info);

  // Detectar capacidades
  detectCapabilities(info);

  // Validar informações detectadas
  if (!validateHardwareInfo(info)) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Informações de hardware inválidas detectadas");
  }

  // Cache das informações
  cached_info_ = info;
  detected_ = true;

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM,
      "Hardware detectado: %s, Rev %d, %d cores, %dMB Flash, %dMB PSRAM, MAC: "
      "%s",
      info.chip_model.c_str(), info.chip_revision, info.cores,
      info.flash_size / (1024 * 1024), info.psram_size / (1024 * 1024),
      info.mac_address.c_str());

  return info;
}

ESP32Variant HardwareDetector::getESP32Variant() {
  return detectHardware().variant;
}

bool HardwareDetector::supportsPeripheral(const String &peripheral) {
  HardwareInfo info = detectHardware();

  if (peripheral == "DISPLAY")
    return info.supports_display;
  if (peripheral == "SDCARD")
    return info.supports_sdcard;
  if (peripheral == "ETHERNET")
    return info.supports_ethernet;
  if (peripheral == "USB")
    return info.supports_usb;
  if (peripheral == "CAMERA")
    return info.supports_camera;

  return false;
}

int HardwareDetector::getDefaultPin(const String &function) {
  HardwareInfo info = detectHardware();
  auto it = info.default_pinout.find(function);
  return (it != info.default_pinout.end()) ? it->second : -1;
}

std::map<String, int> HardwareDetector::getAvailablePins() {
  return detectHardware().available_pins;
}

String HardwareDetector::generateHardwareReport() {
  HardwareInfo info = detectHardware();

  String report = "=== RELATÓRIO DE HARDWARE ===\n";
  report += "Modelo: " + info.chip_model + "\n";
  report += "Revisão: " + String(info.chip_revision) + "\n";
  report += "Cores: " + String(info.cores) + "\n";
  report += "CPU: " + String(info.cpu_freq_mhz) + " MHz\n";
  report += "Flash: " + String(info.flash_size / (1024 * 1024)) + " MB\n";
  report += "PSRAM: " + String(info.psram_size / (1024 * 1024)) + " MB\n";
  report += "WiFi: " + String(info.has_wifi ? "Sim" : "Não") + "\n";
  report += "Bluetooth: " + String(info.has_bluetooth ? "Sim" : "Não") + "\n";
  report += "MAC: " + info.mac_address + "\n\n";

  report += "PINOS DISPONÍVEIS:\n";
  for (auto &pin : info.available_pins) {
    report += pin.first + ": GPIO" + String(pin.second) + "\n";
  }

  report += "\nPINOUT PADRÃO:\n";
  for (auto &pin : info.default_pinout) {
    report += pin.first + ": GPIO" + String(pin.second) + "\n";
  }

  report += "\nCAPACIDADES:\n";
  report += "Display: " + String(info.supports_display ? "Sim" : "Não") + "\n";
  report += "SD Card: " + String(info.supports_sdcard ? "Sim" : "Não") + "\n";
  report +=
      "Ethernet: " + String(info.supports_ethernet ? "Sim" : "Não") + "\n";
  report += "USB: " + String(info.supports_usb ? "Sim" : "Não") + "\n";
  report += "Camera: " + String(info.supports_camera ? "Sim" : "Não") + "\n";

  return report;
}

ESP32Variant HardwareDetector::detectESP32Variant() {
#if CONFIG_IDF_TARGET_ESP32
  return ESP32Variant::ESP32;
#elif CONFIG_IDF_TARGET_ESP32S2
  return ESP32Variant::ESP32_S2;
#elif CONFIG_IDF_TARGET_ESP32S3
  return ESP32Variant::ESP32_S3;
#elif CONFIG_IDF_TARGET_ESP32C3
  return ESP32Variant::ESP32_C3;
#elif CONFIG_IDF_TARGET_ESP32C6
  return ESP32Variant::ESP32_C6;
#elif CONFIG_IDF_TARGET_ESP32H2
  return ESP32Variant::ESP32_H2;
#else
  // Fallback: tentar detectar via efuse
  uint32_t chip_ver =
      REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
  if (chip_ver == 0)
    return ESP32Variant::ESP32;
  if (chip_ver >= 1 && chip_ver <= 3)
    return ESP32Variant::ESP32_S2;
  if (chip_ver >= 4 && chip_ver <= 6)
    return ESP32Variant::ESP32_S3;
  if (chip_ver >= 7 && chip_ver <= 9)
    return ESP32Variant::ESP32_C3;
  return ESP32Variant::UNKNOWN;
#endif
}

uint32_t HardwareDetector::detectFlashSize() { return ESP.getFlashChipSize(); }

uint32_t HardwareDetector::detectPsramSize() { return ESP.getPsramSize(); }

void HardwareDetector::detectAvailablePins(HardwareInfo &info) {
  // Pinos GPIO disponíveis variam por variante
  switch (info.variant) {
  case ESP32Variant::ESP32:
    // ESP32 tem GPIO 0-39, mas alguns são reservados
    for (int i = 0; i <= 39; i++) {
      if (i != 6 && i != 7 && i != 8 && i != 9 && i != 10 &&
          i != 11) { // Flash pins
        info.available_pins["GPIO" + String(i)] = i;
      }
    }
    break;

  case ESP32Variant::ESP32_S2:
    // ESP32-S2 tem GPIO 0-46
    for (int i = 0; i <= 46; i++) {
      info.available_pins["GPIO" + String(i)] = i;
    }
    break;

  case ESP32Variant::ESP32_S3:
    // ESP32-S3 tem GPIO 0-48
    for (int i = 0; i <= 48; i++) {
      info.available_pins["GPIO" + String(i)] = i;
    }
    break;

  case ESP32Variant::ESP32_C3:
    // ESP32-C3 tem GPIO 0-21
    for (int i = 0; i <= 21; i++) {
      info.available_pins["GPIO" + String(i)] = i;
    }
    break;

  default:
    // Pinos padrão
    for (int i = 0; i <= 39; i++) {
      info.available_pins["GPIO" + String(i)] = i;
    }
    break;
  }
}

void HardwareDetector::setDefaultPinout(HardwareInfo &info) {
  // Pinout padrão baseado na variante
  switch (info.variant) {
  case ESP32Variant::ESP32_S3:
    // Pinout típico para ESP32-S3 com display ILI9341
    info.default_pinout["TFT_CS"] = 10;
    info.default_pinout["TFT_DC"] = 11;
    info.default_pinout["TFT_RST"] = 12;
    info.default_pinout["TFT_MOSI"] = 13;
    info.default_pinout["TFT_SCK"] = 14;
    info.default_pinout["SD_CS"] = 15;
    info.default_pinout["SD_MOSI"] = 16;
    info.default_pinout["SD_MISO"] = 17;
    info.default_pinout["SD_SCK"] = 18;
    break;

  case ESP32Variant::ESP32_S2:
    // Pinout para ESP32-S2
    info.default_pinout["TFT_CS"] = 34;
    info.default_pinout["TFT_DC"] = 33;
    info.default_pinout["TFT_RST"] = 35;
    info.default_pinout["TFT_MOSI"] = 36;
    info.default_pinout["TFT_SCK"] = 37;
    break;

  case ESP32Variant::ESP32_C3:
    // Pinout para ESP32-C3 (limitado)
    info.default_pinout["TFT_CS"] = 7;
    info.default_pinout["TFT_DC"] = 6;
    info.default_pinout["TFT_RST"] = 5;
    info.default_pinout["TFT_MOSI"] = 4;
    info.default_pinout["TFT_SCK"] = 3;
    break;

  default:
    // Pinout padrão ESP32
    info.default_pinout["TFT_CS"] = 15;
    info.default_pinout["TFT_DC"] = 2;
    info.default_pinout["TFT_RST"] = 4;
    info.default_pinout["TFT_MOSI"] = 23;
    info.default_pinout["TFT_SCK"] = 18;
    info.default_pinout["SD_CS"] = 5;
    info.default_pinout["SD_MOSI"] = 23;
    info.default_pinout["SD_MISO"] = 19;
    info.default_pinout["SD_SCK"] = 18;
    break;
  }
}

void HardwareDetector::detectCapabilities(HardwareInfo &info) {
  // Capacidades baseadas na variante
  switch (info.variant) {
  case ESP32Variant::ESP32:
    info.supports_display = true;
    info.supports_sdcard = true;
    info.supports_ethernet = true;
    info.supports_usb = false;
    info.supports_camera = true;
    break;

  case ESP32Variant::ESP32_S2:
    info.supports_display = true;
    info.supports_sdcard = true;
    info.supports_ethernet = false;
    info.supports_usb = true;
    info.supports_camera = true;
    break;

  case ESP32Variant::ESP32_S3:
    info.supports_display = true;
    info.supports_sdcard = true;
    info.supports_ethernet = false;
    info.supports_usb = true;
    info.supports_camera = true;
    break;

  case ESP32Variant::ESP32_C3:
    info.supports_display = false; // Limitado por pinos
    info.supports_sdcard = false;
    info.supports_ethernet = false;
    info.supports_usb = true;
    info.supports_camera = false;
    break;

  default:
    info.supports_display = true;
    info.supports_sdcard = true;
    info.supports_ethernet = false;
    info.supports_usb = false;
    info.supports_camera = false;
    break;
  }
}

bool HardwareDetector::validateHardwareInfo(const HardwareInfo &info) {
  // Validar variante
  if (info.variant == ESP32Variant::UNKNOWN) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Variante de hardware desconhecida");
    return false;
  }

  // Validar núcleos (1-2 para ESP32)
  if (info.cores < 1 || info.cores > 2) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Número inválido de núcleos: %d", info.cores);
    return false;
  }

  // Validar frequência CPU (80-240 MHz típica)
  if (info.cpu_freq_mhz < 80 || info.cpu_freq_mhz > 320) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Frequência CPU inválida: %d MHz",
                                          info.cpu_freq_mhz);
    return false;
  }

  // Validar tamanho da flash (mínimo 1MB)
  if (info.flash_size < 1024 * 1024) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Tamanho de flash muito pequeno: %d bytes",
        info.flash_size);
    return false;
  }

  // Validar MAC address (formato básico)
  if (info.mac_address.length() != 17) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Formato de MAC address inválido: %s",
                                          info.mac_address.c_str());
    return false;
  }

  return true;
}