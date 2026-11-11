#include "PinAbstraction.h"
#include "HardwareDetector.h"
#include "advanced_logger.h"

// Inicialização de membros estáticos
std::map<int, PinConfig> PinAbstraction::configured_pins_;
std::map<int, std::function<void()>> PinAbstraction::change_callbacks_;

bool PinAbstraction::configurePin(const PinConfig &config) {
  if (config.pin_number < 0) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Número de pino inválido: %d", config.pin_number);
    return false;
  }

  // Verificar se o pino está disponível no hardware
  auto available = HardwareDetector::getInstance().getAvailablePins();
  if (available.find("GPIO" + String(config.pin_number)) == available.end()) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Pino GPIO%d não disponível no hardware",
        config.pin_number);
    return false;
  }

  // Configurar modo do pino
  uint8_t arduino_mode;
  switch (config.mode) {
  case PinMode::PIN_INPUT:
    arduino_mode = INPUT;
    break;
  case PinMode::PIN_OUTPUT:
    arduino_mode = OUTPUT;
    break;
  case PinMode::PIN_INPUT_PULLUP:
    arduino_mode = INPUT_PULLUP;
    break;
  case PinMode::PIN_INPUT_PULLDOWN:
    arduino_mode = INPUT_PULLDOWN;
    break;
  case PinMode::PIN_OUTPUT_OPEN_DRAIN:
    arduino_mode = OUTPUT_OPEN_DRAIN;
    break;
  case PinMode::PIN_ANALOG_INPUT:
    arduino_mode = INPUT; // Analog input usa INPUT
    break;
  default:
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Modo de pino inválido para GPIO%d",
                                        config.pin_number);
    return false;
  }

  pinMode(config.pin_number, arduino_mode);

  // Armazenar configuração
  configured_pins_[config.pin_number] = config;

  AdvancedLogger::getInstance().info(
      LogModule::SYSTEM, "Pino GPIO%d configurado: %s", config.pin_number,
      config.description.c_str());

  return true;
}

bool PinAbstraction::configurePins(const std::vector<PinConfig> &configs) {
  bool success = true;
  for (const auto &config : configs) {
    if (!configurePin(config)) {
      success = false;
    }
  }
  return success;
}

void PinAbstraction::digitalWrite(int pin, PinState state, bool inverted) {
  PinState actual_state =
      inverted ? (state == PinState::PIN_HIGH ? PinState::PIN_LOW : PinState::PIN_HIGH)
               : state;
  ::digitalWrite(pin, static_cast<uint8_t>(actual_state));

  // Chamar callback se registrado
  auto it = change_callbacks_.find(pin);
  if (it != change_callbacks_.end() && it->second) {
    it->second();
  }
}

PinState PinAbstraction::digitalRead(int pin, bool inverted) {
  int value = ::digitalRead(pin);
  PinState state = static_cast<PinState>(value);
  return inverted ? (state == PinState::PIN_HIGH ? PinState::PIN_LOW : PinState::PIN_HIGH)
                  : state;
}

void PinAbstraction::analogWrite(int pin, int value, bool inverted) {
  int actual_value = inverted ? (255 - value) : value;
  ::analogWrite(pin, actual_value);

  // Chamar callback se registrado
  auto it = change_callbacks_.find(pin);
  if (it != change_callbacks_.end() && it->second) {
    it->second();
  }
}

int PinAbstraction::analogRead(int pin, bool inverted) {
  int value = ::analogRead(pin);
  return inverted ? (4095 - value) : value; // ESP32 usa 12-bit ADC
}

bool PinAbstraction::isPinAvailable(int pin) {
  auto available_pins = HardwareDetector::getInstance().getAvailablePins();
  return available_pins.find("GPIO" + String(pin)) != available_pins.end();
}

String PinAbstraction::getPinDescription(int pin) {
  auto it = configured_pins_.find(pin);
  return (it != configured_pins_.end()) ? it->second.description : "";
}

std::map<int, PinConfig> PinAbstraction::getConfiguredPins() {
  return configured_pins_;
}

void PinAbstraction::removePinConfig(int pin) {
  auto it = configured_pins_.find(pin);
  if (it != configured_pins_.end()) {
    configured_pins_.erase(it);
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "Configuração removida do pino GPIO%d", pin);
  }
}

void PinAbstraction::resetAllPins() {
  for (auto &config : configured_pins_) {
    pinMode(config.first, INPUT); // Reset para INPUT
  }
  configured_pins_.clear();
  change_callbacks_.clear();
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Todos os pinos resetados");
}

void PinAbstraction::registerChangeCallback(int pin,
                                            std::function<void()> callback) {
  if (callback) {
    change_callbacks_[pin] = callback;
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "Callback registrado para pino GPIO%d", pin);
  }
}

void PinAbstraction::unregisterChangeCallback(int pin) {
  auto it = change_callbacks_.find(pin);
  if (it != change_callbacks_.end()) {
    change_callbacks_.erase(it);
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Callback removido do pino GPIO%d", pin);
  }
}
