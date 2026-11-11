#ifndef __HARDWARE_DETECTOR_H__
#define __HARDWARE_DETECTOR_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <esp_chip_info.h>
#include <esp_system.h>
#include <soc/efuse_reg.h>
#include <esp_mac.h>
#include <soc/soc.h>

/**
 * @brief Tipos de ESP32 suportados
 */
enum class ESP32Variant {
  ESP32_CLASSIC,
  ESP32_S2,
  ESP32_S3,
  ESP32_C3,
  ESP32_C6,
  ESP32_H2,
  ESP32, // Adicionado para compatibilidade
  UNKNOWN
};

/**
 * @brief Informações de hardware detectadas
 */
struct HardwareInfo {
  ESP32Variant variant;
  uint32_t chip_revision;
  uint32_t flash_size;
  uint32_t psram_size;
  uint8_t cores;
  String chip_model;
  String mac_address;
  bool has_psram;
  bool has_bluetooth;
  bool has_wifi;
  uint32_t cpu_freq_mhz;
  uint32_t flash_freq_mhz;

  // Informações específicas de pinos
  std::map<String, int> available_pins;
  std::map<String, int> default_pinout;

  // Capacidades de hardware
  bool supports_display;
  bool supports_sdcard;
  bool supports_ethernet;
  bool supports_usb;
  bool supports_camera;
};

/**
 * @brief Detector automático de hardware ESP32
 *
 * Responsável por detectar automaticamente a variante do ESP32
 * e suas capacidades de hardware.
 */
class HardwareDetector {
public:
  static HardwareDetector &getInstance();

  /**
   * @brief Detecta informações de hardware
   * @return Estrutura com informações detectadas
   */
  HardwareInfo detectHardware();

  /**
   * @brief Obtém variante do ESP32
   * @return Variante detectada
   */
  ESP32Variant getESP32Variant();

  /**
   * @brief Verifica se o hardware suporta um periférico
   * @param peripheral Nome do periférico
   * @return true se suportado
   */
  bool supportsPeripheral(const String &peripheral);

  /**
   * @brief Obtém pino padrão para uma função
   * @param function Nome da função (ex: "SD_CS", "TFT_CS")
   * @return Número do pino ou -1 se não disponível
   */
  int getDefaultPin(const String &function);

  /**
   * @brief Lista todos os pinos disponíveis
   * @return Mapa de pinos disponíveis
   */
  std::map<String, int> getAvailablePins();

  /**
   * @brief Gera relatório de diagnóstico de hardware
   * @return String com relatório
   */
  String generateHardwareReport();

private:
  HardwareDetector();
  ~HardwareDetector() = default;

  HardwareDetector(const HardwareDetector &) = delete;
  HardwareDetector &operator=(const HardwareDetector &) = delete;

  /**
   * @brief Detecta variante específica do ESP32
   */
  ESP32Variant detectESP32Variant();

  /**
   * @brief Detecta tamanho da flash
   */
  uint32_t detectFlashSize();

  /**
   * @brief Detecta tamanho da PSRAM
   */
  uint32_t detectPsramSize();

  /**
   * @brief Detecta pinos disponíveis baseado na variante
   */
  void detectAvailablePins(HardwareInfo &info);

  /**
   * @brief Define pinout padrão baseado na variante
   */
  void setDefaultPinout(HardwareInfo &info);

  /**
   * @brief Detecta capacidades de hardware
   */
  void detectCapabilities(HardwareInfo &info);

  /**
   * @brief Valida informações de hardware detectadas
   * @param info Informações a validar
   * @return true se válidas
   */
  bool validateHardwareInfo(const HardwareInfo &info);

  HardwareInfo cached_info_;
  bool detected_;
};

#endif // __HARDWARE_DETECTOR_H__