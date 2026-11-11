#ifndef __PERIPHERAL_ABSTRACTION_H__
#define __PERIPHERAL_ABSTRACTION_H__

#include "PinAbstraction.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <memory>
#include <vector>

/**
 * @brief Tipos de periféricos suportados
 */
enum class PeripheralType {
  PERIPH_DISPLAY,
  PERIPH_SDCARD,
  PERIPH_ETHERNET,
  PERIPH_USB,
  PERIPH_CAMERA,
  PERIPH_RFID,
  PERIPH_IR,
  PERIPH_NRF24,
  PERIPH_LORA,
  PERIPH_GPS,
  PERIPH_BLUETOOTH,
  PERIPH_WIFI,
  PERIPH_SERIAL,
  PERIPH_I2C,
  PERIPH_SPI,
  PERIPH_ADC,
  PERIPH_DAC,
  PERIPH_PWM,
  PERIPH_TOUCH
};

/**
 * @brief Status de inicialização de periférico
 */
enum class PeripheralStatus {
  NOT_INITIALIZED,
  INITIALIZING,
  READY,
  ERROR,
  PERIPH_DISABLED
};

/**
 * @brief Configuração base de periférico
 */
struct PeripheralConfig {
  PeripheralType type;
  String name;
  bool enabled;
  JsonDocument config;

  PeripheralConfig(PeripheralType t = PeripheralType::PERIPH_DISPLAY,
                   const String &n = "", bool e = true)
      : type(t), name(n), enabled(e) {}
};

/**
 * @brief Interface base para periféricos
 */
class IPeripheral {
public:
  virtual ~IPeripheral() = default;

  virtual PeripheralType getType() const = 0;
  virtual String getName() const = 0;
  virtual PeripheralStatus getStatus() const = 0;

  virtual bool initialize(const JsonDocument &config) = 0;
  virtual bool deinitialize() = 0;
  virtual bool isAvailable() const = 0;

  virtual String getInfo() const = 0;
  virtual JsonDocument getStatusJson() const = 0;
};

/**
 * @brief Abstração de periféricos
 *
 * Gerencia diferentes tipos de periféricos de forma unificada,
 * com configuração dinâmica e detecção automática de disponibilidade.
 */
class PeripheralAbstraction {
public:
  static PeripheralAbstraction &getInstance();

  /**
   * @brief Registra um periférico
   * @param peripheral Ponteiro único para o periférico
   * @return true se registrado com sucesso
   */
  static bool registerPeripheral(std::unique_ptr<IPeripheral> peripheral);

  /**
   * @brief Remove um periférico
   * @param name Nome do periférico
   */
  static void unregisterPeripheral(const String &name);

  /**
   * @brief Obtém periférico por nome
   * @param name Nome do periférico
   * @return Ponteiro para o periférico ou nullptr
   */
  static IPeripheral *getPeripheral(const String &name);

  /**
   * @brief Lista todos os periféricos registrados
   * @return Mapa de periféricos
   */
  static std::map<String, IPeripheral *> getAllPeripherals();

  /**
   * @brief Lista periféricos por tipo
   * @param type Tipo de periférico
   * @return Vetor de periféricos
   */
  static std::vector<IPeripheral *> getPeripheralsByType(PeripheralType type);

  /**
   * @brief Inicializa periférico por nome
   * @param name Nome do periférico
   * @param config Configuração JSON
   * @return true se inicializado com sucesso
   */
  static bool initializePeripheral(const String &name,
                                   const JsonDocument &config);

  /**
   * @brief Desinicializa periférico por nome
   * @param name Nome do periférico
   * @return true se desinicializado com sucesso
   */
  static bool deinitializePeripheral(const String &name);

  /**
   * @brief Verifica se periférico está disponível
   * @param name Nome do periférico
   * @return true se disponível
   */
  static bool isPeripheralAvailable(const String &name);

  /**
   * @brief Obtém status de todos os periféricos
   * @return Documento JSON com status
   */
  static JsonDocument getSystemStatus();

  /**
   * @brief Configura pinos para um periférico
   * @param peripheral_name Nome do periférico
   * @param pin_configs Configurações dos pinos
   * @return true se configurado com sucesso
   */
  static bool
  configurePeripheralPins(const String &peripheral_name,
                          const std::vector<PinConfig> &pin_configs);

  /**
   * @brief Carrega configuração de periféricos do arquivo
   * @param config_path Caminho do arquivo de configuração
   * @return true se carregado com sucesso
   */
  static bool loadPeripheralConfig(const String &config_path);

  /**
   * @brief Salva configuração de periféricos no arquivo
   * @param config_path Caminho do arquivo de configuração
   * @return true se salvo com sucesso
   */
  static bool savePeripheralConfig(const String &config_path);

private:
  static std::map<String, std::unique_ptr<IPeripheral>> peripherals_;
  static std::map<String, std::vector<PinConfig>> peripheral_pins_;

  // Singleton pattern
  PeripheralAbstraction(const PeripheralAbstraction &) = delete;
  PeripheralAbstraction &operator=(const PeripheralAbstraction &) = delete;

private:
  PeripheralAbstraction() = default;
  ~PeripheralAbstraction() = default;
};

#endif // __PERIPHERAL_ABSTRACTION_H__