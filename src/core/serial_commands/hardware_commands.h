#ifndef __HARDWARE_COMMANDS_H__
#define __HARDWARE_COMMANDS_H__

#include <Arduino.h>
#include <functional>
#include <vector>
#include <ArduinoJson.h>
#include "PinAbstraction.h"


/**
 * @brief Comandos seriais para diagnóstico e controle de hardware
 */
class HardwareCommands {
public:
  static HardwareCommands &getInstance();

  /**
   * @brief Registra todos os comandos de hardware
   */
  void registerCommands();

  /**
   * @brief Processa comando de hardware
   * @param command Comando a processar
   * @param args Argumentos do comando
   * @return true se comando processado com sucesso
   */
  bool processCommand(const String &command, const std::vector<String> &args);

private:
  HardwareCommands();
  ~HardwareCommands() = default;

  HardwareCommands(const HardwareCommands &) = delete;
  HardwareCommands &operator=(const HardwareCommands &) = delete;

  // Comandos individuais
  void cmdHardwareInfo(const std::vector<String> &args);
  void cmdPinInfo(const std::vector<String> &args);
  void cmdPeripheralInfo(const std::vector<String> &args);
  void cmdConfigurePin(const std::vector<String> &args);
  void cmdTestPin(const std::vector<String> &args);
  void cmdScanI2C(const std::vector<String> &args);
  void cmdHardwareReport(const std::vector<String> &args);
  void cmdResetPins(const std::vector<String> &args);
  void cmdPeripheralStatus(const std::vector<String> &args);
  void cmdLoadHardwareConfig(const std::vector<String> &args);
  void cmdSaveHardwareConfig(const std::vector<String> &args);

  // Utilitários
  void printUsage(const String &command);
  void printJsonResponse(const JsonDocument &doc);
  bool parsePinArgs(const std::vector<String> &args, int &pin, PinMode &mode,
                    bool &inverted);
};

#endif // __HARDWARE_COMMANDS_H__