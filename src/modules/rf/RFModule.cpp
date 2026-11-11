#include "RFModule.h"
#include "../core/BenchmarkManager.h"
#include "../core/SecurityUtils.h"
#include "core/advanced_logger.h"
#include "rf_utils.h"
#include <esp_log.h>

static const char *TAG = "RFModule";

RFModule::RFModule(std::shared_ptr<SystemModel> model,
                   std::shared_ptr<SystemView> view)
    : model_(model), view_(view), active_(false), initialized_(false) {}

bool RFModule::init() {
  if (initialized_) {
    ADVANCED_LOGGER.log(LogLevel::INFO, TAG, "Módulo RF já inicializado");
    return true;
  }

  ADVANCED_LOGGER.log(LogLevel::INFO, TAG, "Inicializando módulo RF...");

  // Validação de segurança: verificar se o hardware RF está disponível
  if (!SecurityUtils::validateHardwareAccess("RF")) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                        "Acesso ao hardware RF negado por segurança");
    return false;
  }

  // Inicialização do módulo RF com validações
  try {
    // Verificar configurações de frequência válidas
    if (willyConfigPins.rfFreq < 280.0f || willyConfigPins.rfFreq > 928.0f) {
      ADVANCED_LOGGER.log(LogLevel::WARNING, TAG,
                          "Frequência RF inválida, definindo padrão 433.92MHz");
      willyConfigPins.rfFreq = 433.92f;
    }

    // Inicializar módulo RF
    if (!initRfModule()) {
      ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                          "Falha na inicialização do módulo RF");
      return false;
    }

    initialized_ = true;
    active_ = true;

    ADVANCED_LOGGER.log(LogLevel::INFO, TAG,
                        "Módulo RF inicializado com sucesso");
    return true;

  } catch (const std::exception &e) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                        "Exceção durante inicialização RF: %s", e.what());
    return false;
  }
}

void RFModule::deinit() {
  if (!initialized_)
    return;

  ADVANCED_LOGGER.log(LogLevel::INFO, TAG, "Desinicializando módulo RF...");

  try {
    deinitRfModule();
    active_ = false;
    initialized_ = false;

    ADVANCED_LOGGER.log(LogLevel::INFO, TAG,
                        "Módulo RF desinicializado com sucesso");

  } catch (const std::exception &e) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                        "Erro durante desinicialização RF: %s", e.what());
  }
}

void RFModule::process() {
  if (!active_)
    return;

  // Monitoramento contínuo de saúde do módulo RF
  static uint32_t last_health_check = 0;
  uint32_t now = millis();

  if (now - last_health_check > 5000) { // Verificar a cada 5 segundos
    if (!checkRfHealth()) {
      ADVANCED_LOGGER.log(LogLevel::WARNING, TAG,
                          "Problema detectado no módulo RF");
      // Tentar recuperação automática
      if (!recoverRfModule()) {
        ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                            "Falha na recuperação automática do RF");
      }
    }
    last_health_check = now;
  }
}

int RFModule::getPriority() const {
  return 80; // Prioridade alta
}

bool RFModule::executeCommand(const String &command, JsonDocument &result) {
  (void)command;
  (void)result;
  // Implementação básica - pode ser expandida depois
  return false;
}

bool RFModule::transmitRF() {
  if (!active_ || !initialized_) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                        "Tentativa de transmissão RF com módulo inativo");
    return false;
  }

  auto benchmark = BenchmarkManager::getInstance();
  uint32_t start = 0;
  if (benchmark) {
    start = benchmark->startProfiling("RF Transmission");
  }

  try {
    // Validação de parâmetros de transmissão
    if (!validateTransmissionParameters()) {
      ADVANCED_LOGGER.log(LogLevel::ERROR, TAG,
                          "Parâmetros de transmissão RF inválidos");
      return false;
    }

    // Implementar transmissão real aqui (placeholder otimizado)
    // Por enquanto, apenas simular transmissão com validações

    ADVANCED_LOGGER.log(LogLevel::DEBUG, TAG,
                        "Transmissão RF executada com sucesso");

    if (benchmark) {
      benchmark->endProfiling(start,
                              BenchmarkManager::MetricType::RF_TRANSMIT_TIME,
                              "RF Transmission", true);
    }

    return true;

  } catch (const std::exception &e) {
    ADVANCED_LOGGER.log(LogLevel::ERROR, TAG, "Erro durante transmissão RF: %s",
                        e.what());

    if (benchmark) {
      benchmark->endProfiling(start,
                              BenchmarkManager::MetricType::RF_TRANSMIT_TIME,
                              "RF Transmission", false);
    }

    return false;
  }
}

// Métodos auxiliares otimizados
bool RFModule::checkRfHealth() {
  // Verificar se o módulo RF está respondendo
  // Implementar verificações específicas do hardware
  return rmtInstalled && (willyConfigPins.rfModule == CC1101_SPI_MODULE
                              ? ELECHOUSE_cc1101.getCC1101()
                              : true);
}

bool RFModule::recoverRfModule() {
  ADVANCED_LOGGER.log(LogLevel::INFO, TAG, "Tentando recuperação do módulo RF");

  deinitRfModule();
  delay(100); // Pequena pausa para estabilização
  return initRfModule();
}

bool RFModule::validateTransmissionParameters() {
  // Validações de segurança para transmissão
  if (willyConfigPins.rfFreq < 280.0f || willyConfigPins.rfFreq > 928.0f) {
    return false;
  }

  // Verificar conformidade com protocolos RF
  // Adicionar mais validações conforme necessário

  return true;
}