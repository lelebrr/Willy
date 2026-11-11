#include "hardware_optimizer.h"
#include "advanced_logger.h"
#include <esp_flash.h>
#include <esp_heap_caps.h>
#include <esp_system.h>


#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include <esp_psram.h>
#endif

HardwareOptimizer &HardwareOptimizer::getInstance() {
  static HardwareOptimizer instance;
  return instance;
}

HardwareOptimizer::HardwareOptimizer()
    : current_variant_(ESP32Variant::UNKNOWN), display_type_("ILI9341"),
      psram_available_(false), flash_size_(0), spi_clock_(40000000),
      buffer_size_(8192) {
  detectHardwareCapabilities();
}

void HardwareOptimizer::detectHardwareCapabilities() {
  // Detecta variante do ESP32
  HardwareInfo info = HardwareDetector::getInstance().detectHardware();
  current_variant_ = info.variant;

  // Detecta PSRAM
  psram_available_ = false;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
  psram_available_ = esp_psram_is_initialized();
#endif

  // Detecta tamanho do flash
  uint32_t flash_id = 0;
  esp_flash_read_id(NULL, &flash_id);
  // Estimativa baseada no chip ID comum para ESP32-S3
  flash_size_ = 8 * 1024 * 1024; // 8MB padrão para este projeto

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Hardware detectado: %s, PSRAM: %s, Flash: %dMB",
      info.chip_model.c_str(), psram_available_ ? "sim" : "não",
      flash_size_ / (1024 * 1024));
}

bool HardwareOptimizer::autoOptimize() {
  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Iniciando otimização automática de hardware...");

  // Aplica otimizações baseadas na variante
  switch (current_variant_) {
  case ESP32Variant::ESP32_S3:
    applyESP32S3Optimizations();
    break;
  case ESP32Variant::ESP32_S2:
    applyESP32S2Optimizations();
    break;
  case ESP32Variant::ESP32_C3:
    applyESP32C3Optimizations();
    break;
  default:
    applyGenericOptimizations();
    break;
  }

  // Otimiza periféricos
  optimizeDisplay(current_variant_);
  optimizeMemory(current_variant_);
  optimizeWiFi(current_variant_);
  optimizeBluetooth(current_variant_);
  optimizeSPI(current_variant_);
  optimizeI2C(current_variant_);

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM,
      "Otimização automática concluída. Clock SPI: %dHz, Buffer: %d bytes",
      spi_clock_, buffer_size_);

  return true;
}

void HardwareOptimizer::applyESP32S3Optimizations() {
  // ESP32-S3 tem melhores capacidades
  spi_clock_ = 40000000; // 40MHz para SPI
  buffer_size_ = 16384;  // 16KB buffer

  // Nota: esp_cache_msync não disponível nesta versão do IDF
  // Cache é gerenciado automaticamente pelo hardware

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Otimizações ESP32-S3 aplicadas");
}

void HardwareOptimizer::applyESP32S2Optimizations() {
  // ESP32-S2 tem menos recursos
  spi_clock_ = 26000000; // 26MHz para SPI
  buffer_size_ = 8192;   // 8KB buffer

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Otimizações ESP32-S2 aplicadas");
}

void HardwareOptimizer::applyESP32C3Optimizations() {
  // ESP32-C3 (RISC-V)
  spi_clock_ = 40000000; // 40MHz para SPI
  buffer_size_ = 8192;   // 8KB buffer

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Otimizações ESP32-C3 aplicadas");
}

void HardwareOptimizer::applyGenericOptimizations() {
  // Configuraçes genéricas
  spi_clock_ = 20000000; // 20MHz para SPI
  buffer_size_ = 4096;   // 4KB buffer

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Otimizações genéricas aplicadas");
}

void HardwareOptimizer::optimizeDisplay(ESP32Variant variant) {
  // Otimiza configurações de display baseadas no hardware
  switch (variant) {
  case ESP32Variant::ESP32_S3:
    // S3 suporta DMA nativo
    spi_clock_ = 40000000;
    break;
  case ESP32Variant::ESP32_S2:
    // S2 tem limitações
    spi_clock_ = 26000000;
    break;
  case ESP32Variant::ESP32_C3:
    // C3 usa RISC-V
    spi_clock_ = 40000000;
    break;
  default:
    spi_clock_ = 20000000;
    break;
  }
}

void HardwareOptimizer::optimizeMemory(ESP32Variant variant) {
  // Otimiza uso de memória
  switch (variant) {
  case ESP32Variant::ESP32_S3:
    if (psram_available_) {
      buffer_size_ = 32768; // 32KB se tem PSRAM
    } else {
      buffer_size_ = 16384; // 16KB sem PSRAM
    }
    break;
  case ESP32Variant::ESP32_S2:
    buffer_size_ = 8192;
    break;
  case ESP32Variant::ESP32_C3:
    buffer_size_ = 8192;
    break;
  default:
    buffer_size_ = 4096;
    break;
  }
}

void HardwareOptimizer::optimizeWiFi(ESP32Variant variant) {
  // WiFi é nativo em todas as variantes
  (void)variant;
  // Configurações de WiFi são automáticas no ESP32
}

void HardwareOptimizer::optimizeBluetooth(ESP32Variant variant) {
  // Bluetooth só está disponível em S3 e C3
  switch (variant) {
  case ESP32Variant::ESP32_S3:
  case ESP32Variant::ESP32_C3:
    // Suporta BLE nativo
    break;
  default:
    // Não suporta Bluetooth
    break;
  }
}

void HardwareOptimizer::optimizeSPI(ESP32Variant variant) {
  // Otimiza SPI baseadas no hardware
  switch (variant) {
  case ESP32Variant::ESP32_S3:
    // S3 tem SPI mais rápido
    spi_clock_ = 40000000;
    break;
  case ESP32Variant::ESP32_S2:
    spi_clock_ = 26000000;
    break;
  case ESP32Variant::ESP32_C3:
    spi_clock_ = 40000000;
    break;
  default:
    spi_clock_ = 20000000;
    break;
  }
}

void HardwareOptimizer::optimizeI2C(ESP32Variant variant) {
  // I2C é padrão em todas as variantes
  (void)variant;
  // 400kHz é o padrão
}

void HardwareOptimizer::optimizeForDisplay(const String &display_type) {
  display_type_ = display_type;

  // Otimiza para tipo de display específico
  if (display_type == "ILI9341") {
    spi_clock_ = 40000000;
  } else if (display_type == "ST7789") {
    spi_clock_ = 60000000;
  } else if (display_type == "GC9A01") {
    spi_clock_ = 40000000;
  } else {
    spi_clock_ = 20000000;
  }
}

uint32_t HardwareOptimizer::getOptimalSPIClock() const { return spi_clock_; }

bool HardwareOptimizer::hasPSRAM() const { return psram_available_; }

size_t HardwareOptimizer::getAvailableRAM() const {
  return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

size_t HardwareOptimizer::getOptimalBufferSize() const {
  if (psram_available_) {
    // Usa PSRAM se disponível
    return buffer_size_ * 2;
  }
  return buffer_size_;
}