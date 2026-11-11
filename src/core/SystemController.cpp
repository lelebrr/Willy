#include "SystemController.h"
#include "SystemManager.h"
#include "SystemModel.h"
#include "SystemView.h"
#include "advanced_logger.h"
#include "hardware_optimizer.h"
#include "main_menu.h"   // Para acessar MainMenu
#include "startup_app.h" // Para StartupApp

SystemController &SystemController::getInstance() {
  static SystemController instance;
  return instance;
}

bool SystemController::init() {
  if (initialized_) {
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "SystemController já inicializado");
    return true;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Inicializando SystemController...");

  // Otimizar para o hardware detectado
  HardwareOptimizer::getInstance().autoOptimize();

  // Inicializar componentes do MVC
  bool success = true;

  // Inicializar modelo
  if (!SystemModel::getInstance().init()) {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Falha ao inicializar SystemModel");
    success = false;
  }

  // Inicializar visão
  if (!SystemView::getInstance().init()) {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Falha ao inicializar SystemView");
    success = false;
  }

  // Inicializar gerenciador de módulos
  if (!SystemManager::getInstance().initAllModules()) {
    AdvancedLogger::getInstance().error(LogModule::SYSTEM,
                                        "Falha ao inicializar módulos");
    success = false;
  }

  if (success) {
    initialized_ = true;
    AdvancedLogger::getInstance().info(
        LogModule::SYSTEM, "SystemController inicializado com sucesso");

    // Iniciar aplicação de startup se configurada
    startStartupApp();
  } else {
    AdvancedLogger::getInstance().critical(
        LogModule::SYSTEM,
        "Falha crítica na inicialização do SystemController");
  }

  return success;
}

void SystemController::deinit() {
  if (!initialized_) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM,
        "Tentativa de desinicializar SystemController não inicializado");
    return;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Desinicializando SystemController...");

  // Desinicializar componentes na ordem reversa
  SystemManager::getInstance().deinitAllModules();
  SystemView::getInstance().deinit();

  // Salvar estado final
  SystemModel::getInstance().saveConfig();

  initialized_ = false;
  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "SystemController desinicializado");
}

void SystemController::runMainLoop() {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Tentativa de executar loop principal com "
                           "SystemController não inicializado");
    return;
  }

  // Processar entrada do usuário
  processMenuInput();

  // Atualizar estado do sistema
  updateSystemState();

  // Renderizar interface
  SystemView::getInstance().updateDisplay();

  // Executar módulos ativos
  SystemManager::getInstance().processAllModules();

  // Gerenciar ciclo de vida de aplicações
  handleAppLifecycle();
}

void SystemController::showMainMenu() {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM, "Tentativa de mostrar menu principal com "
                           "SystemController não inicializado");
    return;
  }

  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Exibindo menu principal");

  // Delega para MainMenu existente, mas com verificações
  if (SystemView::getInstance().isScreenOn()) {
    mainMenu.begin();
  } else {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Tentativa de mostrar menu com tela desligada");
  }
}

void SystemController::processMenuInput() {
  if (!initialized_)
    return;

  // Verificar entrada de botões/touch
  // Por enquanto delega para lógica existente, mas pode ser expandido
  // para usar SystemView para processamento unificado
}

void SystemController::startStartupApp() {
  if (!initialized_)
    return;

  auto &config = SystemModel::getInstance().getConfig();
  if (config.startupApp != "") {
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Iniciando aplicação de startup: %s",
                                       config.startupApp.c_str());

    if (!startupApp.startApp(config.startupApp)) {
      AdvancedLogger::getInstance().warning(
          LogModule::SYSTEM, "Falha ao iniciar aplicação de startup: %s",
          config.startupApp.c_str());
      config.startupApp = ""; // Limpar configuração inválida
      SystemModel::getInstance().saveConfig();
    } else {
      AdvancedLogger::getInstance().info(
          LogModule::SYSTEM, "Aplicação de startup iniciada com sucesso");
    }
  }
}

void SystemController::handleAppLifecycle() {
  if (!initialized_)
    return;

  // Monitorar estado das aplicações
  // Por enquanto delega para lógica existente, mas pode ser expandido
  // para gerenciar múltiplas aplicações
}

void SystemController::updateSystemState() {
  if (!initialized_)
    return;

  // Atualizar estado global baseado em sensores e módulos
  auto &globalState = SystemModel::getInstance().getGlobalState();

  // Verificar conectividade WiFi
  // (pode ser expandido para verificar outros estados)

  // Atualizar timestamp
  globalState.previousMillis = millis();
}