#ifndef __PIN_ABSTRACTION_H__
#define __PIN_ABSTRACTION_H__

#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>

/**
 * @brief Modos de operação dos pinos
 */
enum class PinMode {
  PIN_INPUT,
  PIN_OUTPUT,
  PIN_INPUT_PULLUP,
  PIN_INPUT_PULLDOWN,
  PIN_OUTPUT_OPEN_DRAIN,
  PIN_ANALOG_INPUT
};

/**
 * @brief Estados digitais dos pinos
 */
enum class PinState { PIN_LOW = 0, PIN_HIGH = 1 };

/**
 * @brief Configuração de pino
 */
struct PinConfig {
  int pin_number;
  PinMode mode;
  bool inverted;
  String description;
  std::function<void()> on_change_callback;

  PinConfig(int pin = -1, PinMode m = PinMode::PIN_INPUT, bool inv = false,
            const String &desc = "", std::function<void()> cb = nullptr)
      : pin_number(pin), mode(m), inverted(inv), description(desc),
        on_change_callback(cb) {}
};

/**
 * @brief Abstração de pinos GPIO
 *
 * Fornece uma interface unificada para trabalhar com pinos GPIO
 * em diferentes variantes do ESP32, com suporte a configuração
 * dinâmica e callbacks.
 */
class PinAbstraction {
public:
  /**
   * @brief Configura um pino
   * @param config Configuração do pino
   * @return true se configurado com sucesso
   */
  static bool configurePin(const PinConfig &config);

  /**
   * @brief Configura múltiplos pinos
   * @param configs Vetor de configurações
   * @return true se todos configurados com sucesso
   */
  static bool configurePins(const std::vector<PinConfig> &configs);

  /**
   * @brief Define estado digital de um pino
   * @param pin Número do pino
   * @param state Estado (HIGH/LOW)
   * @param inverted Se deve inverter o sinal
   */
  static void digitalWrite(int pin, PinState state, bool inverted = false);

  /**
   * @brief Lê estado digital de um pino
   * @param pin Número do pino
   * @param inverted Se deve inverter o sinal
   * @return Estado lido
   */
  static PinState digitalRead(int pin, bool inverted = false);

  /**
   * @brief Escreve valor analógico (PWM)
   * @param pin Número do pino
   * @param value Valor (0-255 para 8-bit, 0-1023 para 10-bit)
   * @param inverted Se deve inverter o sinal
   */
  static void analogWrite(int pin, int value, bool inverted = false);

  /**
   * @brief Lê valor analógico
   * @param pin Número do pino
   * @param inverted Se deve inverter o sinal
   * @return Valor lido
   */
  static int analogRead(int pin, bool inverted = false);

  /**
   * @brief Verifica se um pino está disponível
   * @param pin Número do pino
   * @return true se disponível
   */
  static bool isPinAvailable(int pin);

  /**
   * @brief Obtém descrição de um pino
   * @param pin Número do pino
   * @return Descrição ou string vazia
   */
  static String getPinDescription(int pin);

  /**
   * @brief Lista todos os pinos configurados
   * @return Mapa de pinos configurados
   */
  static std::map<int, PinConfig> getConfiguredPins();

  /**
   * @brief Remove configuração de um pino
   * @param pin Número do pino
   */
  static void removePinConfig(int pin);

  /**
   * @brief Reseta todos os pinos para estado padrão
   */
  static void resetAllPins();

  /**
   * @brief Registra callback para mudança de estado
   * @param pin Número do pino
   * @param callback Função a ser chamada
   */
  static void registerChangeCallback(int pin, std::function<void()> callback);

  /**
   * @brief Remove callback de mudança de estado
   * @param pin Número do pino
   */
  static void unregisterChangeCallback(int pin);

private:
  static std::map<int, PinConfig> configured_pins_;
  static std::map<int, std::function<void()>> change_callbacks_;

  // Singleton pattern
  PinAbstraction() = delete;
  ~PinAbstraction() = delete;
  PinAbstraction(const PinAbstraction &) = delete;
  PinAbstraction &operator=(const PinAbstraction &) = delete;
};

#endif // __PIN_ABSTRACTION_H__