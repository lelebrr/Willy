#include "SystemView.h"
#include "SystemModel.h"
#include "advanced_logger.h"
#include "animation_engine.h"
#include "display.h" // Para acessar tft e funções de display
#include "futuristic_icons.h"

SystemView &SystemView::getInstance() {
  static SystemView instance;
  return instance;
}

bool SystemView::init() {
  if (initialized_) {
    AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                        "SystemView já inicializado");
    return true;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Inicializando SystemView");

  // Verificar se display está disponível
  if (!isDisplayAvailable()) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Display não disponível, SystemView limitado");
  }

  initialized_ = true;
  screenOn_ = true;

  // Aplicar configurações iniciais
  applyInitialSettings();

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "SystemView inicializado");
  return true;
}

void SystemView::deinit() {
  if (!initialized_) {
    AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                        "SystemView não inicializado");
    return;
  }

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Desinicializando SystemView");

  // Desligar tela se necessário
  if (screenOn_) {
    turnOffScreen();
  }

  initialized_ = false;
  screenOn_ = false;

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "SystemView desinicializado");
}

void SystemView::wakeUpScreen() {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM,
        "Tentativa de acordar tela com SystemView não inicializado");
    return;
  }

  if (!isDisplayAvailable()) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Display não disponível para wakeUpScreen");
    return;
  }

  // Delega para a função global existente
  ::wakeUpScreen();
  screenOn_ = true;

  // Atualiza estado no modelo
  auto &globalState = SystemModel::getInstance().getGlobalState();
  globalState.previousMillis = millis();
  globalState.isScreenOff = false;
  globalState.dimmer = false;

  AdvancedLogger::getInstance().debug(LogModule::SYSTEM, "Tela acordada");
}

void SystemView::dimScreen() {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM,
        "Tentativa de dim tela com SystemView não inicializado");
    return;
  }

  if (!screenOn_) {
    AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                          "Tentativa de dim tela desligada");
    return;
  }

  // Implementação básica de dim - pode ser expandida para controle PWM
  screenOn_ = true; // Ainda ligado, apenas dim
  auto &globalState = SystemModel::getInstance().getGlobalState();
  globalState.dimmer = true;

  AdvancedLogger::getInstance().debug(LogModule::SYSTEM, "Tela dimmed");
}

void SystemView::turnOffScreen() {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM,
        "Tentativa de desligar tela com SystemView não inicializado");
    return;
  }

  screenOn_ = false;
  auto &globalState = SystemModel::getInstance().getGlobalState();
  globalState.isScreenOff = true;
  globalState.dimmer = false;

  // Aqui poderia implementar desligamento físico se disponível
  AdvancedLogger::getInstance().debug(LogModule::SYSTEM, "Tela desligada");
}

void SystemView::setBrightness(int brightness) {
  if (!initialized_) {
    AdvancedLogger::getInstance().error(
        LogModule::SYSTEM,
        "Tentativa de definir brilho com SystemView não inicializado");
    return;
  }

  // Validar brilho
  if (brightness < 0 || brightness > 100) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "Brilho inválido: %d (deve ser 0-100)", brightness);
    brightness = constrain(brightness, 0, 100);
  }

  // Atualiza configuração
  SystemModel::getInstance().getConfig().bright = brightness;

  // Aqui poderia implementar controle de backlight PWM se disponível
  // Por enquanto, apenas armazena o valor
  AdvancedLogger::getInstance().debug(LogModule::SYSTEM,
                                      "Brilho definido para %d%%", brightness);
}

void SystemView::clearScreen(uint16_t color) {
  if (!initialized_ || !screenOn_ || !isDisplayAvailable()) {
    return;
  }

  tft.fillScreen(color);
}

void SystemView::fillScreen(uint16_t color) { clearScreen(color); }

void SystemView::updateDisplay() {
  if (!initialized_ || !screenOn_) {
    return;
  }

  // Atualizar elementos dinâmicos da interface com animações
  AnimationEngine::getInstance().update();

  // Renderizar animações se habilitadas
  AnimationEngine::getInstance().render();

  // Delega para funções existentes
}

void *SystemView::getTft() const {
  if (!initialized_) {
    return nullptr;
  }
  return (void *)&tft;
}

void *SystemView::getSprite() const {
  if (!initialized_) {
    return nullptr;
  }
  return (void *)&sprite;
}

bool SystemView::isDisplayAvailable() const {
  // Verificar se display está fisicamente disponível
  // Por enquanto, assume disponível se inicializado
  return initialized_;
}

void SystemView::applyInitialSettings() {
  if (!initialized_) {
    return;
  }

  const auto &config = SystemModel::getInstance().getConfig();

  // Aplicar brilho inicial
  setBrightness(config.bright);

  // Aplicar cor de fundo
  if (isDisplayAvailable()) {
    clearScreen(config.bgColor);
  }

  AdvancedLogger::getInstance().debug(
      LogModule::SYSTEM,
      "Configurações iniciais aplicadas - Brilho: %d%%, BG: 0x%04X",
      config.bright, config.bgColor);

  // Inicializar otimizações de performance
  initDoubleBuffering();
  initSpriteCache();
}

// ==================== INTEGRAÇÃO COM ANIMATION ENGINE ====================

void SystemView::enableAnimations(bool enable) {
  if (!initialized_) {
    AdvancedLogger::getInstance().warning(
        LogModule::SYSTEM, "SystemView não inicializado para animações");
    return;
  }

  auto &animationEngine = AnimationEngine::getInstance();
  if (enable) {
    TFT_eSPI *tft = (TFT_eSPI *)getTft();
    if (tft && animationEngine.init(tft)) {
      animationEngine.startAnimation();
      AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                         "Animações habilitadas");
    }
  } else {
    animationEngine.stopAnimation();
    AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                       "Animações desabilitadas");
  }
}

void SystemView::setAnimationFrameRate(uint8_t fps) {
  if (!initialized_)
    return;

  AnimationEngine::getInstance().setFrameRate(fps);
  AdvancedLogger::getInstance().debug(
      LogModule::SYSTEM, "Frame rate das animações definido para %d FPS", fps);
}

void SystemView::triggerGlitchEffect(uint16_t duration) {
  if (!initialized_)
    return;

  AnimationEngine::getInstance().triggerGlitchEffect(duration);
  AdvancedLogger::getInstance().debug(
      LogModule::SYSTEM, "Efeito glitch acionado por %d ms", duration);
}

void SystemView::addGlowEffect(int16_t x, int16_t y, uint16_t width,
                               uint16_t height, uint16_t color) {
  if (!initialized_)
    return;

  AnimationEngine::getInstance().addGlowEffect(x, y, width, height, color);
  AdvancedLogger::getInstance().debug(
      LogModule::SYSTEM, "Glow adicionado em (%d,%d) %dx%d cor 0x%04X", x, y,
      width, height, color);
}

// ==================== OTIMIZAÇÕES DE PERFORMANCE ====================

void SystemView::initDoubleBuffering() {
  if (!initialized_ || !isDisplayAvailable()) {
    return;
  }

  // Criar back buffer para double buffering
  if (backBuffer_ == nullptr) {
    backBuffer_ = new TFT_eSprite(&tft);
    if (backBuffer_->createSprite(tftWidth, tftHeight)) {
      backBuffer_->setColorDepth(16);
      backBuffer_->fillSprite(willyConfig.bgColor);
      AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                         "Double buffering inicializado");
    } else {
      delete backBuffer_;
      backBuffer_ = nullptr;
      AdvancedLogger::getInstance().warning(LogModule::SYSTEM,
                                            "Falha ao criar back buffer");
    }
  }
}

void SystemView::initSpriteCache() {
  if (!initialized_ || !isDisplayAvailable()) {
    return;
  }

  // Inicializar cache de ícones
  memset(iconCache_, 0, sizeof(iconCache_));

  // Carregar ícones comuns no cache
  loadIconToCache(0, wifi_icon_32x32, 32, 32);      // WiFi
  loadIconToCache(1, rfid_icon_32x32, 32, 32);      // RFID
  loadIconToCache(2, rf_icon_32x32, 32, 32);        // RF
  loadIconToCache(3, bluetooth_icon_32x32, 32, 32); // Bluetooth
  loadIconToCache(4, gps_icon_32x32, 32, 32);       // GPS
  loadIconToCache(5, ir_icon_32x32, 32, 32);        // IR
  loadIconToCache(6, ethernet_icon_32x32, 32, 32);  // Ethernet
  loadIconToCache(7, lora_icon_32x32, 32, 32);      // LoRa

  AdvancedLogger::getInstance().info(LogModule::SYSTEM,
                                     "Cache de sprites inicializado");
}

void SystemView::loadIconToCache(uint8_t index, const uint16_t *bitmap,
                                 uint16_t width, uint16_t height) {
  if (index >= 16 || !bitmap)
    return;

  iconCache_[index].bitmap = bitmap;
  iconCache_[index].width = width;
  iconCache_[index].height = height;

  // Criar sprite para o ícone
  if (iconCache_[index].sprite == nullptr) {
    iconCache_[index].sprite = new TFT_eSprite(&tft);
    if (iconCache_[index].sprite->createSprite(width, height)) {
      iconCache_[index].sprite->setColorDepth(16);
      iconCache_[index].sprite->pushImage(0, 0, width, height, bitmap);
      iconCache_[index].loaded = true;
    }
  }
}

// Método otimizado para renderizar ícone do cache
void SystemView::renderCachedIcon(uint8_t index, int16_t x, int16_t y) {
  if (index >= 16 || !iconCache_[index].loaded || !iconCache_[index].sprite) {
    return;
  }

  // Usar DMA se disponível para transferência rápida
  if (doubleBufferingEnabled_ && backBuffer_) {
    iconCache_[index].sprite->pushToSprite(backBuffer_, x, y);
  } else {
    iconCache_[index].sprite->pushSprite(x, y);
  }
}

// Método para swap buffers (double buffering)
void SystemView::swapBuffers() {
  if (!doubleBufferingEnabled_ || !backBuffer_) {
    return;
  }

  // Transferir back buffer para tela com DMA
  backBuffer_->pushSprite(0, 0);
}

// Método otimizado de updateDisplay com double buffering
void SystemView::updateDisplayOptimized() {
  if (!initialized_ || !screenOn_) {
    return;
  }

  // Renderizar no back buffer se double buffering ativo
  if (doubleBufferingEnabled_ && backBuffer_) {
    // Limpar back buffer
    backBuffer_->fillSprite(willyConfig.bgColor);

    // Renderizar elementos no back buffer
    // (Aqui seria chamado o código de renderização dos menus/itens)

    // Swap buffers
    swapBuffers();
  } else {
    // Fallback para renderização direta
    updateDisplay();
  }
}
