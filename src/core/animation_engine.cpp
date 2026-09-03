#include "animation_engine.h"
#include "SystemView.h"
#include <driver/timer.h>
#include <esp_timer.h>

// Implementação do Singleton
AnimationEngine &AnimationEngine::getInstance() {
  static AnimationEngine instance;
  return instance;
}

// Timer ISR para animações
void IRAM_ATTR AnimationEngine::onTimer() {
  AnimationEngine &engine = AnimationEngine::getInstance();
  if (engine.running_ && !engine.paused_) {
    engine.update();
  }
}

bool AnimationEngine::init(TFT_eSPI *tft) {
  if (initialized_)
    return true;

  tft_ = tft;
  if (!tft_)
    return false;

  // Criar sprite para renderização otimizada
  sprite_ = new TFT_eSprite(tft_);
  sprite_->createSprite(tft_->width(), tft_->height());

  // Configurar timer de hardware para animações
  animationTimer_ = timerBegin(1000000); // 1MHz
  timerAttachInterrupt(animationTimer_, &AnimationEngine::onTimer);
  timerAlarm(animationTimer_, 1000000 / frameRate_, true,
             0); // Alarme a cada frame

  // Inicializar efeitos
  scanlines_ = new ScanlinesEffect(tft_);
  glows_ = new GlowEffect(tft_);
  matrixRain_ = new MatrixRainEffect(tft_);
  borderGlow_ = new BorderGlowEffect(tft_);
  glitch_ = new GlitchEffect(tft_);

  scanlines_->init();
  glows_->init();
  matrixRain_->init();
  borderGlow_->init();
  glitch_->init();

  initialized_ = true;
  return true;
}

void AnimationEngine::deinit() {
  if (!initialized_)
    return;

  stopAnimation();

  if (animationTimer_) {
    timerEnd(animationTimer_);
    animationTimer_ = nullptr;
  }

  delete scanlines_;
  delete glows_;
  delete matrixRain_;
  delete borderGlow_;
  delete glitch_;

  if (sprite_) {
    sprite_->deleteSprite();
    delete sprite_;
    sprite_ = nullptr;
  }

  initialized_ = false;
}

void AnimationEngine::startAnimation() {
  if (!initialized_)
    return;
  running_ = true;
  paused_ = false;
}

void AnimationEngine::stopAnimation() {
  running_ = false;
  paused_ = false;
}

void AnimationEngine::pauseAnimation() { paused_ = true; }

void AnimationEngine::resumeAnimation() { paused_ = false; }

void AnimationEngine::setFrameRate(uint8_t fps) {
  frameRate_ = fps;
  if (animationTimer_) {
    timerAlarm(animationTimer_, 1000000 / fps, true, 0);
  }
}

void AnimationEngine::enableEffect(bool scanlines, bool glows, bool matrixRain,
                                   bool borderGlow, bool glitch) {
  scanlinesEnabled_ = scanlines;
  glowsEnabled_ = glows;
  matrixRainEnabled_ = matrixRain;
  borderGlowEnabled_ = borderGlow;
  glitchEnabled_ = glitch;
}

void AnimationEngine::triggerGlitchEffect(uint16_t duration) {
  if (glitch_) {
    glitchEnabled_ = true; // Habilitar efeito se não estiver
    glitch_->triggerGlitch(duration);
  }
}

void AnimationEngine::addGlowEffect(int16_t x, int16_t y, uint16_t width,
                                    uint16_t height, uint16_t color) {
  if (glows_) {
    glowsEnabled_ = true; // Habilitar efeito se não estiver
    glows_->addGlow(x, y, width, height, color);
  }
}

// ==================== QUANTUM FLOW EFFECT ====================
// Novo efeito avançado para "Quantum Flow" com partículas quânticas
void AnimationEngine::QuantumFlowEffect::init() {
  particles_.clear();
  // Inicializar partículas quânticas
  for (int i = 0; i < MAX_QUANTUM_PARTICLES; i++) {
    QuantumParticle p;
    p.x = random(tft_->width());
    p.y = random(tft_->height());
    p.vx = (random(100) - 50) / 50.0f; // Velocidade entre -1 e 1
    p.vy = (random(100) - 50) / 50.0f;
    p.life = random(100, 255);
    p.maxLife = p.life;
    p.color =
        (random(2) == 0) ? 0x07FF : 0x03EF; // Neon Aqua primário ou secundário
    p.size = random(1, 3);
    particles_.push_back(p);
  }
  phase_ = 0;
}

void AnimationEngine::QuantumFlowEffect::update() {
  phase_ += 0.05f;

  for (auto &p : particles_) {
    // Movimento quântico com oscilação
    p.x += p.vx + sin(phase_ + p.y * 0.01f) * 0.5f;
    p.y += p.vy + cos(phase_ + p.x * 0.01f) * 0.5f;

    // Vida da partícula
    p.life -= 0.5f;

    // Reincarnação quântica
    if (p.life <= 0 || p.x < -10 || p.x > tft_->width() + 10 || p.y < -10 ||
        p.y > tft_->height() + 10) {
      p.x = random(tft_->width());
      p.y = random(tft_->height());
      p.vx = (random(100) - 50) / 50.0f;
      p.vy = (random(100) - 50) / 50.0f;
      p.life = p.maxLife;
      p.color = (random(2) == 0) ? 0x07FF : 0x03EF;
    }
  }
}

void AnimationEngine::QuantumFlowEffect::render() {
  if (!tft_)
    return;

  for (const auto &p : particles_) {
    uint8_t alpha = (p.life / p.maxLife) * 255;
    uint16_t blendedColor = tft_->alphaBlend(alpha, p.color, DEEP_BLACK);

    if (p.size == 1) {
      tft_->drawPixel(p.x, p.y, blendedColor);
    } else {
      tft_->fillCircle(p.x, p.y, p.size, blendedColor);
    }

    // Efeito de cauda quântica
    if (alpha > 128) {
      tft_->drawLine(p.x, p.y, p.x - p.vx * 3, p.y - p.vy * 3,
                     tft_->alphaBlend(alpha / 2, p.color, DEEP_BLACK));
    }
  }
}

void AnimationEngine::QuantumFlowEffect::setIntensity(uint8_t intensity) {
  intensity_ = intensity;
  // Ajustar número de partículas baseado na intensidade
  if (particles_.size() > 0) {
    particles_.resize((intensity_ * MAX_QUANTUM_PARTICLES) / 255);
  }
}

// ==================== NEON PULSE EFFECT ====================
// Efeito de pulso neon pulsante
void AnimationEngine::NeonPulseEffect::init() {
  pulsePhase_ = 0;
  centerX_ = tft_->width() / 2;
  centerY_ = tft_->height() / 2;
}

void AnimationEngine::NeonPulseEffect::update() {
  pulsePhase_ += pulseSpeed_;
  if (pulsePhase_ >= 360)
    pulsePhase_ = 0;
}

void AnimationEngine::NeonPulseEffect::render() {
  if (!tft_)
    return;

  float radius = 20 + 15 * sin(pulsePhase_ * 3.14159 / 180.0);
  uint8_t alpha = 128 + 127 * sin(pulsePhase_ * 3.14159 / 180.0);

  // Desenhar círculos concêntricos
  for (int i = 0; i < 3; i++) {
    uint16_t color = (i == 0) ? 0x07FF : 0x03EF;
    uint8_t ringAlpha = alpha - (i * 40);
    if (ringAlpha > 0) {
      uint16_t blendedColor = tft_->alphaBlend(ringAlpha, color, DEEP_BLACK);
      tft_->drawCircle(centerX_, centerY_, radius + (i * 8), blendedColor);
    }
  }
}

void AnimationEngine::NeonPulseEffect::setPulseSpeed(uint8_t speed) {
  pulseSpeed_ = speed / 10.0f;
}

// ==================== CIRCUIT FLOW EFFECT ====================
// Efeito de fluxo de circuito digital
void AnimationEngine::CircuitFlowEffect::init() {
  nodes_.clear();
  connections_.clear();

  // Criar nós de circuito
  for (int i = 0; i < MAX_CIRCUIT_NODES; i++) {
    CircuitNode n;
    n.x = random(20, tft_->width() - 20);
    n.y = random(20, tft_->height() - 20);
    n.active = false;
    n.phase = random(360);
    nodes_.push_back(n);
  }

  // Criar conexões
  for (size_t i = 0; i < nodes_.size() - 1; i++) {
    connections_.push_back({(int)i, (int)(i + 1)});
  }
  // Conexão circular
  connections_.push_back({(int)nodes_.size() - 1, 0});

  flowPhase_ = 0;
}

void AnimationEngine::CircuitFlowEffect::update() {
  flowPhase_ += 2;

  // Ativar nós sequencialmente
  for (size_t i = 0; i < nodes_.size(); i++) {
    nodes_[i].phase += 5;
    if (nodes_[i].phase >= 360)
      nodes_[i].phase = 0;

    // Ativar nó baseado na fase do fluxo
    int activationPoint = (flowPhase_ + i * (360 / nodes_.size())) % 360;
    nodes_[i].active = (activationPoint < 60);
  }
}

void AnimationEngine::CircuitFlowEffect::render() {
  if (!tft_)
    return;

  // Desenhar conexões
  for (const auto &conn : connections_) {
    const auto &n1 = nodes_[conn.from];
    const auto &n2 = nodes_[conn.to];

    if (n1.active || n2.active) {
      uint8_t alpha = max(n1.active ? 200 : 0, n2.active ? 200 : 0);
      uint16_t color = tft_->alphaBlend(alpha, 0x07FF, DEEP_BLACK);
      tft_->drawLine(n1.x, n1.y, n2.x, n2.y, color);
    }
  }

  // Desenhar nós
  for (const auto &n : nodes_) {
    if (n.active) {
      tft_->fillCircle(n.x, n.y, 3, 0x07FF);
      // Efeito de brilho
      tft_->drawCircle(n.x, n.y, 6, tft_->alphaBlend(128, 0x07FF, DEEP_BLACK));
    } else {
      tft_->drawPixel(n.x, n.y, tft_->alphaBlend(64, 0x03EF, DEEP_BLACK));
    }
  }
}

void AnimationEngine::CircuitFlowEffect::setFlowSpeed(uint8_t speed) {
  flowSpeed_ = speed;
}

void AnimationEngine::update() {
  uint32_t currentTime = millis();
  if (currentTime - lastFrameTime_ < (1000 / frameRate_))
    return;
  lastFrameTime_ = currentTime;

  // Atualizar efeitos ativos
  if (scanlinesEnabled_)
    scanlines_->update();
  if (glowsEnabled_)
    glows_->update();
  if (matrixRainEnabled_)
    matrixRain_->update();
  if (borderGlowEnabled_)
    borderGlow_->update();
  if (glitchEnabled_)
    glitch_->update();
}

void AnimationEngine::render() {
  if (!initialized_ || !sprite_)
    return;

  // Limpar sprite
  sprite_->fillSprite(DEEP_BLACK);

  // Renderizar efeitos na ordem correta
  if (matrixRainEnabled_)
    matrixRain_->render();
  if (glowsEnabled_)
    glows_->render();
  if (borderGlowEnabled_)
    borderGlow_->render();
  if (scanlinesEnabled_)
    scanlines_->render();
  if (glitchEnabled_)
    glitch_->render();

  // Push sprite para display
  sprite_->pushSprite(0, 0);
}

// ==================== SCANLINES EFFECT ====================

AnimationEngine::ScanlinesEffect::ScanlinesEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::ScanlinesEffect::~ScanlinesEffect() {}

void AnimationEngine::ScanlinesEffect::init() { currentY_ = 0; }

void AnimationEngine::ScanlinesEffect::update() {
  currentY_ += speed_;
  if (currentY_ >= tft_->height()) {
    currentY_ = -SCANLINE_HEIGHT;
  }
}

void AnimationEngine::ScanlinesEffect::render() {
  if (!tft_)
    return;

  // Desenhar scanline com opacidade
  tft_->drawFastHLine(0, currentY_, tft_->width(), color_);
  if (currentY_ + 1 < tft_->height()) {
    tft_->drawFastHLine(0, currentY_ + 1, tft_->width(),
                        tft_->alphaBlend(opacity_, color_, DEEP_BLACK));
  }
}

void AnimationEngine::ScanlinesEffect::setSpeed(uint8_t speed) {
  speed_ = speed;
}
void AnimationEngine::ScanlinesEffect::setColor(uint16_t color) {
  color_ = color;
}
void AnimationEngine::ScanlinesEffect::setOpacity(uint8_t opacity) {
  opacity_ = opacity;
}

// ==================== GLOW EFFECT ====================

AnimationEngine::GlowEffect::GlowEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::GlowEffect::~GlowEffect() {}

void AnimationEngine::GlowEffect::init() {
  glows_.clear();
  pulsePhase_ = 0;
}

void AnimationEngine::GlowEffect::update() {
  pulsePhase_ += pulseSpeed_;
  if (pulsePhase_ >= 255)
    pulsePhase_ = 0;
}

void AnimationEngine::GlowEffect::render() {
  if (!tft_ || glows_.empty())
    return;

  // Calcular intensidade pulsante
  float intensity = (sin(pulsePhase_ * 3.14159 / 128.0) + 1.0) * 0.5;

  for (const auto &glow : glows_) {
    // Desenhar glow com intensidade variável
    uint8_t alpha = glow.intensity * intensity;
    tft_->drawRect(glow.x - 1, glow.y - 1, glow.width + 2, glow.height + 2,
                   tft_->alphaBlend(alpha, glow.color, DEEP_BLACK));
  }
}

void AnimationEngine::GlowEffect::addGlow(int16_t x, int16_t y, uint16_t width,
                                          uint16_t height, uint16_t color) {
  GlowElement glow = {x, y, width, height, color, 128};
  glows_.push_back(glow);
}

void AnimationEngine::GlowEffect::clearGlows() { glows_.clear(); }

void AnimationEngine::GlowEffect::setPulseSpeed(uint8_t speed) {
  pulseSpeed_ = speed;
}

// ==================== MATRIX RAIN EFFECT ====================

AnimationEngine::MatrixRainEffect::MatrixRainEffect(TFT_eSPI *tft)
    : tft_(tft) {}

AnimationEngine::MatrixRainEffect::~MatrixRainEffect() {}

void AnimationEngine::MatrixRainEffect::init() {
  for (uint8_t i = 0; i < dropCount_; i++) {
    generateDrop(i);
  }
}

void AnimationEngine::MatrixRainEffect::update() {
  for (uint8_t i = 0; i < dropCount_; i++) {
    drops_[i].y += drops_[i].speed;
    if (drops_[i].y > tft_->height() + 20) {
      generateDrop(i);
    }
  }
}

void AnimationEngine::MatrixRainEffect::render() {
  if (!tft_)
    return;

  for (uint8_t i = 0; i < dropCount_; i++) {
    const auto &drop = drops_[i];

    // Renderizar caracteres da gota
    for (uint8_t j = 0; j < drop.length && j < 10; j++) {
      int16_t y = drop.y - j * 8;
      if (y >= 0 && y < tft_->height()) {
        uint8_t alpha = 255 - (j * 25); // Fade out
        tft_->drawChar(drop.x, y, drop.chars[j],
                       tft_->alphaBlend(alpha, color_, DEEP_BLACK),
                       tft_->alphaBlend(alpha, DEEP_BLACK, color_), 1);
      }
    }
  }
}

void AnimationEngine::MatrixRainEffect::setDropCount(uint8_t count) {
  dropCount_ = min(count, (uint8_t)MAX_MATRIX_DROPS);
}

void AnimationEngine::MatrixRainEffect::setSpeed(uint8_t speed) {
  baseSpeed_ = speed;
}
void AnimationEngine::MatrixRainEffect::setColor(uint16_t color) {
  color_ = color;
}

void AnimationEngine::MatrixRainEffect::generateDrop(uint8_t index) {
  drops_[index].x = random(tft_->width());
  drops_[index].y = random(-50, 0);
  drops_[index].length = random(5, 15);
  drops_[index].speed = baseSpeed_ + random(-1, 2);

  // Gerar caracteres aleatórios
  for (uint8_t i = 0; i < drops_[index].length && i < 10; i++) {
    drops_[index].chars[i] = getRandomChar();
  }
}

char AnimationEngine::MatrixRainEffect::getRandomChar() {
  const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()";
  return chars[random(sizeof(chars) - 1)];
}

// ==================== BORDER GLOW EFFECT ====================

AnimationEngine::BorderGlowEffect::BorderGlowEffect(TFT_eSPI *tft)
    : tft_(tft) {}

AnimationEngine::BorderGlowEffect::~BorderGlowEffect() {}

void AnimationEngine::BorderGlowEffect::init() { pulsePhase_ = 0; }

void AnimationEngine::BorderGlowEffect::update() {
  pulsePhase_ += pulseSpeed_;
  if (pulsePhase_ >= 255)
    pulsePhase_ = 0;
}

void AnimationEngine::BorderGlowEffect::render() {
  if (!tft_)
    return;

  // Calcular intensidade pulsante
  float intensity = (sin(pulsePhase_ * 3.14159 / 128.0) + 1.0) * 0.5;
  uint8_t alpha = 255 * intensity;

  uint16_t blendedColor = tft_->alphaBlend(alpha, color_, DEEP_BLACK);

  // Desenhar borda com glow
  for (uint8_t t = 0; t < thickness_; t++) {
    // Topo
    tft_->drawFastHLine(t, t, tft_->width() - 2 * t, blendedColor);
    // Baixo
    tft_->drawFastHLine(t, tft_->height() - 1 - t, tft_->width() - 2 * t,
                        blendedColor);
    // Esquerda
    tft_->drawFastVLine(t, t, tft_->height() - 2 * t, blendedColor);
    // Direita
    tft_->drawFastVLine(tft_->width() - 1 - t, t, tft_->height() - 2 * t,
                        blendedColor);
  }
}

void AnimationEngine::BorderGlowEffect::setColor(uint16_t color) {
  color_ = color;
}
void AnimationEngine::BorderGlowEffect::setThickness(uint8_t thickness) {
  thickness_ = thickness;
}
void AnimationEngine::BorderGlowEffect::setPulseSpeed(uint8_t speed) {
  pulseSpeed_ = speed;
}

// ==================== GLITCH EFFECT ====================

AnimationEngine::GlitchEffect::GlitchEffect(TFT_eSPI *tft) : tft_(tft) {}

AnimationEngine::GlitchEffect::~GlitchEffect() {}

void AnimationEngine::GlitchEffect::init() { active_ = false; }

void AnimationEngine::GlitchEffect::update() {
  if (active_ && millis() - startTime_ > duration_) {
    active_ = false;
  }
}

void AnimationEngine::GlitchEffect::render() {
  if (!active_ || !tft_)
    return;

  // Aplicar efeito baseado no tipo aleatório
  uint8_t effectType = random(3);
  switch (effectType) {
  case 0:
    applyRGBShift();
    break;
  case 1:
    applyPixelCorruption();
    break;
  case 2:
    applyLineDistortion();
    break;
  }
}

void AnimationEngine::GlitchEffect::triggerGlitch(uint16_t duration) {
  active_ = true;
  startTime_ = millis();
  duration_ = duration;
}

void AnimationEngine::GlitchEffect::setIntensity(uint8_t intensity) {
  intensity_ = intensity;
}

void AnimationEngine::GlitchEffect::applyRGBShift() {
  // Deslocamento RGB simples (simulado com linhas coloridas)
  for (uint8_t i = 0; i < intensity_ / 10; i++) {
    int16_t y = random(tft_->height());
    uint16_t color = random(0xFFFF);
    tft_->drawFastHLine(0, y, tft_->width(), color);
  }
}

void AnimationEngine::GlitchEffect::applyPixelCorruption() {
  // Corromper pixels aleatórios
  for (uint16_t i = 0; i < intensity_; i++) {
    int16_t x = random(tft_->width());
    int16_t y = random(tft_->height());
    uint16_t color = random(0xFFFF);
    tft_->drawPixel(x, y, color);
  }
}

void AnimationEngine::GlitchEffect::applyLineDistortion() {
  // Distortion de linhas horizontais
  for (uint8_t i = 0; i < intensity_ / 20; i++) {
    int16_t y = random(tft_->height());
    int16_t offset = random(-5, 6);
    if (y + offset >= 0 && y + offset < tft_->height()) {
      // Copiar linha com offset (simplificado)
      tft_->drawFastHLine(0, y + offset, tft_->width(), tft_->readPixel(0, y));
    }
  }
}