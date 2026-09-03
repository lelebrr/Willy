#include "RFIDModule.h"
#include "../core/BenchmarkManager.h"
#include "../core/advanced_logger.h"

RFIDModule::RFIDModule(std::shared_ptr<SystemModel> model,
                       std::shared_ptr<SystemView> view)
    : model_(model), view_(view), active_(false), initialized_(false) {}

bool RFIDModule::init() {
  if (initialized_)
    return true;

  AdvancedLogger::log(LogModule::RFID, LogLevel::INFO,
                      "Initializing RFID module...");

  // Inicialização específica do RFID
  // Por enquanto, apenas marca como inicializado

  initialized_ = true;
  active_ = true;

  AdvancedLogger::log(LogModule::RFID, LogLevel::INFO,
                      "RFID module initialized");
  return true;
}

void RFIDModule::deinit() {
  if (!initialized_)
    return;

  AdvancedLogger::log(LogModule::RFID, LogLevel::INFO,
                      "Deinitializing RFID module...");

  active_ = false;
  initialized_ = false;

  AdvancedLogger::log(LogModule::RFID, LogLevel::INFO,
                      "RFID module deinitialized");
}

void RFIDModule::process() {
  if (!active_)
    return;

  // Processamento contínuo do módulo RFID
  // Por enquanto, sem processamento específico no loop
}

int RFIDModule::getPriority() const {
  return 90; // Prioridade mais alta
}

bool RFIDModule::executeCommand(const String &command, JsonDocument &result) {
  (void)command;
  (void)result;
  // Implementação básica - pode ser expandida depois
  return false;
}

int RFIDModule::getPriority() const {
  return 90; // Prioridade mais alta
}

bool RFIDModule::executeCommand(const String &command, JsonDocument &result) {
  (void)command;
  (void)result;
  // Implementação básica - pode ser expandida depois
  return false;
}

bool RFIDModule::readRFID() {
  auto benchmark = BenchmarkManager::getInstance();
  uint32_t start = 0;
  if (benchmark) {
    start = benchmark->startProfiling("RFID Reading");
  }

  // Implementação real de leitura RFID aqui
  // Por enquanto, simular leitura
  AdvancedLogger::log(LogModule::RFID, LogLevel::DEBUG,
                      "Performing RFID read operation");

  // Simular tempo de leitura
  vTaskDelay(pdMS_TO_TICKS(10));

  if (benchmark) {
    benchmark->endProfiling(start, BenchmarkManager::MetricType::RFID_READ_TIME,
                            "RFID Reading", true);
  }

  AdvancedLogger::log(LogModule::RFID, LogLevel::INFO, "RFID read completed");
  return true;
}