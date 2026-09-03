#ifndef __HARDWARE_OPTIMIZER_H__
#define __HARDWARE_OPTIMIZER_H__

#include "HardwareDetector.h"
#include "HardwareProfiles.h"
#include <Arduino.h>


/**
 * @brief Otimizações específicas por hardware
 *
 * Aplica configurações otimizadas baseadas no hardware detectado
 * para garantir melhor performance e compatibilidade.
 */
class HardwareOptimizer {
public:
  static HardwareOptimizer &getInstance();

  /**
   * @brief Executa otimização automática baseada no hardware
   * @return true se otimização aplicada com sucesso
   */
  bool autoOptimize();

  /**
   * @brief Otimiza configurações de display
   * @param variant Variante do ESP32
   */
  void optimizeDisplay(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de memória
   * @param variant Variante do ESP32
   */
  void optimizeMemory(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de WiFi
   * @param variant Variante do ESP32
   */
  void optimizeWiFi(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de Bluetooth
   * @param variant Variante do ESP32
   */
  void optimizeBluetooth(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de SPI
   * @param variant Variante do ESP32
   */
  void optimizeSPI(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de I2C
   * @param variant Variante do ESP32
   */
  void optimizeI2C(ESP32Variant variant);

  /**
   * @brief Otimiza configurações de display específico
   * @param display_type Tipo de display
   */
  void optimizeForDisplay(const String &display_type);

  /**
   * @brief Obtém configuração de clock SPI otimizada
   * @return Frequência de clock em Hz
   */
  uint32_t getOptimalSPIClock() const;

  /**
   * @brief Verifica se hardware tem PSRAM
   * @return true se tem PSRAM
   */
  bool hasPSRAM() const;

  /**
   * @brief Obtém quantidade de RAM disponível
   * @return Bytes de RAM livre
   */
  size_t getAvailableRAM() const;

  /**
   * @brief Obtém configuração de buffer otimizada
   * @return Tamanho de buffer em bytes
   */
  size_t getOptimalBufferSize() const;

private:
  HardwareOptimizer();
  ~HardwareOptimizer() = default;

  HardwareOptimizer(const HardwareOptimizer &) = delete;
  HardwareOptimizer &operator=(const HardwareOptimizer &) = delete;

  ESP32Variant current_variant_;
  String display_type_;
  bool psram_available_;
  size_t flash_size_;
  uint32_t spi_clock_;
  size_t buffer_size_;

  void detectHardwareCapabilities();
  void applyESP32S3Optimizations();
  void applyESP32S2Optimizations();
  void applyESP32C3Optimizations();
  void applyGenericOptimizations();
};

#endif // __HARDWARE_OPTIMIZER_H__