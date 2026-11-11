#include "hardware_commands.h"
#include "HardwareDetector.h"
#include "PeripheralAbstraction.h"
#include "PinAbstraction.h"
#include "advanced_logger.h"
#include <ArduinoJson.h>
#include <Wire.h>

HardwareCommands &HardwareCommands::getInstance() {
  static HardwareCommands instance;
  return instance;
}

HardwareCommands::HardwareCommands() {}

void HardwareCommands::registerCommands() {
  // Comandos são registrados no CLI principal
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Comandos de hardware registrados");
}

bool HardwareCommands::processCommand(const String &command,
                                      const std::vector<String> &args) {
  if (command == "hwinfo" || command == "hardware") {
    cmdHardwareInfo(args);
    return true;
  } else if (command == "pininfo") {
    cmdPinInfo(args);
    return true;
  } else if (command == "peripheral") {
    cmdPeripheralInfo(args);
    return true;
  } else if (command == "configpin") {
    cmdConfigurePin(args);
    return true;
  } else if (command == "testpin") {
    cmdTestPin(args);
    return true;
  } else if (command == "i2cscan") {
    cmdScanI2C(args);
    return true;
  } else if (command == "hwreport") {
    cmdHardwareReport(args);
    return true;
  } else if (command == "resetpins") {
    cmdResetPins(args);
    return true;
  } else if (command == "periphstatus") {
    cmdPeripheralStatus(args);
    return true;
  } else if (command == "loadhwconfig") {
    cmdLoadHardwareConfig(args);
    return true;
  } else if (command == "savehwconfig") {
    cmdSaveHardwareConfig(args);
    return true;
  }

  return false;
}

void HardwareCommands::cmdHardwareInfo(const std::vector<String> &args) {
  AdvancedLogger &logger = AdvancedLogger::getInstance();
  logger.info(LogModule::SYSTEM, "Retrieving hardware information");

  HardwareInfo info = HardwareDetector::getInstance().detectHardware();

  Serial.println("=== INFORMAÇÕES DE HARDWARE ===");
  Serial.printf("Modelo: %s\n", info.chip_model.c_str());
  Serial.printf("Revisão: %lu\n", info.chip_revision);
  Serial.printf("Cores: %u\n", info.cores);
  Serial.printf("CPU: %lu MHz\n", info.cpu_freq_mhz);
  Serial.printf("Flash: %lu MB\n", info.flash_size / (1024 * 1024));
  Serial.printf("PSRAM: %lu MB\n", info.psram_size / (1024 * 1024));
  Serial.printf("WiFi: %s\n", info.has_wifi ? "Sim" : "Não");
  Serial.printf("Bluetooth: %s\n", info.has_bluetooth ? "Sim" : "Não");
  Serial.printf("MAC: %s\n", info.mac_address.c_str());
  Serial.println();
}

void HardwareCommands::cmdPinInfo(const std::vector<String> &args) {
  auto pins = PinAbstraction::getConfiguredPins();

  Serial.println("=== PINOS CONFIGURADOS ===");
  if (pins.empty()) {
    Serial.println("Nenhum pino configurado");
  } else {
    for (auto &pin : pins) {
      Serial.printf("GPIO%d: %s (%s)\n", pin.first,
                    pin.second.description.c_str(),
                    pin.second.inverted ? "invertido" : "normal");
    }
  }
  Serial.println();
}

void HardwareCommands::cmdPeripheralInfo(const std::vector<String> &args) {
  auto peripherals = PeripheralAbstraction::getAllPeripherals();

  Serial.println("=== PERIFÉRICOS REGISTRADOS ===");
  if (peripherals.empty()) {
    Serial.println("Nenhum periférico registrado");
  } else {
    for (auto &p : peripherals) {
      Serial.printf("%s: %s (%s)\n", p.first.c_str(),
                    p.second->getInfo().c_str(),
                    p.second->isAvailable() ? "disponível" : "indisponível");
    }
  }
  Serial.println();
}

void HardwareCommands::cmdConfigurePin(const std::vector<String> &args) {
  if (args.size() < 2) {
    printUsage("configpin <pin> <mode> [inverted] [description]");
    return;
  }

  int pin = args[0].toInt();
  String mode_str = args[1];
  mode_str.toLowerCase();
  bool inverted = false;
  if (args.size() > 2) {
    String arg2 = args[2];
    arg2.toLowerCase();
    inverted = (args[2] == "1" || arg2 == "true");
  }
  String description =
      (args.size() > 3) ? args[3] : "Pino configurado via serial";

  PinMode mode;
  if (mode_str == "input")
    mode = PinMode::PIN_INPUT;
  else if (mode_str == "output")
    mode = PinMode::PIN_OUTPUT;
  else if (mode_str == "input_pullup")
    mode = PinMode::PIN_INPUT_PULLUP;
  else if (mode_str == "input_pulldown")
    mode = PinMode::PIN_INPUT_PULLDOWN;
  else if (mode_str == "analog")
    mode = PinMode::PIN_ANALOG_INPUT;
  else {
    Serial.println("Modo inválido. Use: input, output, input_pullup, "
                   "input_pulldown, analog");
    return;
  }

  PinConfig config(pin, mode, inverted, description);
  if (PinAbstraction::configurePin(config)) {
    Serial.printf("Pino GPIO%d configurado com sucesso\n", pin);
  } else {
    Serial.printf("Falha ao configurar pino GPIO%d\n", pin);
  }
}

void HardwareCommands::cmdTestPin(const std::vector<String> &args) {
  if (args.size() < 2) {
    printUsage("testpin <pin> <read|write> [value]");
    return;
  }

  int pin = args[0].toInt();
  String operation = args[1];
  operation.toLowerCase();

  if (operation == "read") {
    PinState state = PinAbstraction::digitalRead(pin);
    Serial.printf("GPIO%d = %s\n", pin,
                  state == PinState::PIN_HIGH ? "HIGH" : "LOW");
  } else if (operation == "write") {
    if (args.size() < 3) {
      printUsage("testpin <pin> write <0|1>");
      return;
    }
    PinState state = (args[2] == "1") ? PinState::PIN_HIGH : PinState::PIN_LOW;
    PinAbstraction::digitalWrite(pin, state);
    Serial.printf("GPIO%d definido como %s\n", pin,
                  state == PinState::PIN_HIGH ? "HIGH" : "LOW");
  } else {
    Serial.println("Operação inválida. Use: read ou write");
  }
}

void HardwareCommands::cmdScanI2C(const std::vector<String> &args) {
  int sda = 21; // Pinos padrão ESP32
  int scl = 22;

  if (args.size() >= 2) {
    sda = args[0].toInt();
    scl = args[1].toInt();
  }

  Wire.begin(sda, scl);

  Serial.printf("Escaneando barramento I2C (SDA=%d, SCL=%d)...\n", sda, scl);
  Serial.println("Endereços encontrados:");

  int devices = 0;
  for (int address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("0x%02X ", address);
      devices++;
    }
  }

  if (devices == 0) {
    Serial.println("Nenhum dispositivo encontrado");
  } else {
    Serial.printf("\n%d dispositivo(s) encontrado(s)\n", devices);
  }
  Serial.println();
}

void HardwareCommands::cmdHardwareReport(const std::vector<String> &args) {
  String report = HardwareDetector::getInstance().generateHardwareReport();
  Serial.println(report);
}

void HardwareCommands::cmdResetPins(const std::vector<String> &args) {
  PinAbstraction::resetAllPins();
  Serial.println("Todos os pinos resetados para INPUT");
}

void HardwareCommands::cmdPeripheralStatus(const std::vector<String> &args) {
  JsonDocument status = PeripheralAbstraction::getSystemStatus();
  printJsonResponse(status);
}

void HardwareCommands::cmdLoadHardwareConfig(const std::vector<String> &args) {
  String path = (args.size() > 0) ? args[0] : "/hardware_config.json";
  if (PeripheralAbstraction::loadPeripheralConfig(path)) {
    Serial.printf("Configuração carregada de %s\n", path.c_str());
  } else {
    Serial.printf("Falha ao carregar configuração de %s\n", path.c_str());
  }
}

void HardwareCommands::cmdSaveHardwareConfig(const std::vector<String> &args) {
  String path = (args.size() > 0) ? args[0] : "/hardware_config.json";
  if (PeripheralAbstraction::savePeripheralConfig(path)) {
    Serial.printf("Configuração salva em %s\n", path.c_str());
  } else {
    Serial.printf("Falha ao salvar configuração em %s\n", path.c_str());
  }
}

void HardwareCommands::printUsage(const String &command) {
  Serial.printf("Uso: %s\n", command.c_str());
}

void HardwareCommands::printJsonResponse(const JsonDocument &doc) {
  serializeJsonPretty(doc, Serial);
  Serial.println();
}