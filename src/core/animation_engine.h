#ifndef ANIMATION_ENGINE_H
#define ANIMATION_ENGINE_H

#include "futuristic_icons.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

// Configurações de otimização para ESP32-S3
#define ANIMATION_FRAME_RATE 30
#define MAX_MATRIX_DROPS 50
#define SCANLINE_HEIGHT 2
#define GLOW_INTENSITY_STEPS 16
#define GLITCH_MAX_DURATION 200

// Cores RGB565 do plano
#define CYAN_NEON 0x07FF
#define MAGENTA_NEON 0xF81F
#define PURPLE_NEON 0x7819
#define ELECTRIC_BLUE 0x03FF
#define MATRIX_GREEN 0x07E0
#define SHOCK_PINK 0xF8FC
#define DEEP_BLACK 0x0000
#define GLOW_WHITE 0xFFFF
#define GLOW_YELLOW 0xFFE0

/**
 * @brief Motor de animações futuristas para display TFT
 * Otimizado para ESP32-S3 com timers e sprites
 */
class AnimationEngine {
public:
  static AnimationEngine &getInstance();

  // Inicialização
  bool init(TFT_eSPI *tft);
  void deinit();

  // Controle de animações
  void startAnimation();
  void stopAnimation();
  void pauseAnimation();
  void resumeAnimation();

  // Configurações
  void setFrameRate(uint8_t fps);
  void enableEffect(bool scanlines, bool glows, bool matrixRain,
                    bool borderGlow, bool glitch);

  // Efeitos específicos
  void triggerGlitchEffect(uint16_t duration);
  void addGlowEffect(int16_t x, int16_t y, uint16_t width, uint16_t height,
                     uint16_t color);

  // Renderização
  void update();
  void render();

  // Classes de efeito específicas
  class ScanlinesEffect;
  class GlowEffect;
  class MatrixRainEffect;
  class BorderGlowEffect;
  class GlitchEffect;

  // Novos efeitos "Quantum Flow"
  class QuantumFlowEffect;
  class NeonPulseEffect;
  class CircuitFlowEffect;

private:
  AnimationEngine() = default;
  ~AnimationEngine() = default;

  TFT_eSPI *tft_ = nullptr;
  TFT_eSprite *sprite_ = nullptr;
  hw_timer_t *animationTimer_ = nullptr;

  bool initialized_ = false;
  bool running_ = false;
  bool paused_ = false;

  uint8_t frameRate_ = ANIMATION_FRAME_RATE;
  uint32_t lastFrameTime_ = 0;

  // Flags de efeitos habilitados
  bool scanlinesEnabled_ = true;
  bool glowsEnabled_ = true;
  bool matrixRainEnabled_ = true;
  bool borderGlowEnabled_ = true;
  bool glitchEnabled_ = false;

  // Instâncias dos efeitos
  ScanlinesEffect *scanlines_;
  GlowEffect *glows_;
  MatrixRainEffect *matrixRain_;
  BorderGlowEffect *borderGlow_;
  GlitchEffect *glitch_;

  // Timer ISR
  static void IRAM_ATTR onTimer();
};

/**
 * @brief Efeito de scanlines que varrem a tela
 */
class AnimationEngine::ScanlinesEffect {
public:
  ScanlinesEffect(TFT_eSPI *tft);
  ~ScanlinesEffect();

  void init();
  void update();
  void render();

  void setSpeed(uint8_t speed);
  void setColor(uint16_t color);
  void setOpacity(uint8_t opacity);

private:
  TFT_eSPI *tft_;
  int16_t currentY_ = 0;
  uint8_t speed_ = 3;
  uint16_t color_ = GLOW_WHITE;
  uint8_t opacity_ = 128;
};

/**
 * @brief Efeitos de glow ao redor dos elementos
 */
class AnimationEngine::GlowEffect {
public:
  GlowEffect(TFT_eSPI *tft);
  ~GlowEffect();

  void init();
  void update();
  void render();

  void addGlow(int16_t x, int16_t y, uint16_t width, uint16_t height,
               uint16_t color);
  void clearGlows();
  void setPulseSpeed(uint8_t speed);

private:
  struct GlowElement {
    int16_t x, y;
    uint16_t width, height;
    uint16_t color;
    uint8_t intensity;
  };

  TFT_eSPI *tft_;
  std::vector<GlowElement> glows_;
  uint8_t pulsePhase_ = 0;
  uint8_t pulseSpeed_ = 2;
};

/**
 * @brief Chuva de caracteres Matrix-style
 */
class AnimationEngine::MatrixRainEffect {
public:
  MatrixRainEffect(TFT_eSPI *tft);
  ~MatrixRainEffect();

  void init();
  void update();
  void render();

  void setDropCount(uint8_t count);
  void setSpeed(uint8_t speed);
  void setColor(uint16_t color);

private:
  struct MatrixDrop {
    int16_t x;
    int16_t y;
    uint8_t length;
    uint8_t speed;
    char chars[10];
  };

  TFT_eSPI *tft_;
  MatrixDrop drops_[MAX_MATRIX_DROPS];
  uint8_t dropCount_ = 20;
  uint8_t baseSpeed_ = 2;
  uint16_t color_ = MATRIX_GREEN;

  void generateDrop(uint8_t index);
  char getRandomChar();
};

/**
 * @brief Glow nas bordas da tela
 */
class AnimationEngine::BorderGlowEffect {
public:
  BorderGlowEffect(TFT_eSPI *tft);
  ~BorderGlowEffect();

  void init();
  void update();
  void render();

  void setColor(uint16_t color);
  void setThickness(uint8_t thickness);
  void setPulseSpeed(uint8_t speed);

private:
  TFT_eSPI *tft_;
  uint16_t color_ = CYAN_NEON;
  uint8_t thickness_ = 3;
  uint8_t pulsePhase_ = 0;
  uint8_t pulseSpeed_ = 1;
};

/**
 * @brief Efeitos de glitch/distortion digital
 */
class AnimationEngine::GlitchEffect {
public:
  GlitchEffect(TFT_eSPI *tft);
  ~GlitchEffect();

  void init();
  void update();
  void render();

  void triggerGlitch(uint16_t duration = 100);
  void setIntensity(uint8_t intensity);

private:
  enum GlitchType { RGB_SHIFT, PIXEL_CORRUPTION, LINE_DISTORTION };

  TFT_eSPI *tft_;
  bool active_ = false;
  uint32_t startTime_ = 0;
  uint16_t duration_ = 0;
  uint8_t intensity_ = 50;

  void applyRGBShift();
  void applyPixelCorruption();
  void applyLineDistortion();
};

/**
 * @brief Efeito "Quantum Flow" - Partículas quânticas com movimento caótico
 */
class AnimationEngine::QuantumFlowEffect {
public:
  QuantumFlowEffect(TFT_eSPI *tft) : tft_(tft) {}
  ~QuantumFlowEffect() = default;

  void init();
  void update();
  void render();

  void setIntensity(uint8_t intensity);

private:
  static constexpr int MAX_QUANTUM_PARTICLES = 50;

  struct QuantumParticle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    uint16_t color;
    uint8_t size;
  };

  TFT_eSPI *tft_;
  std::vector<QuantumParticle> particles_;
  float phase_ = 0;
  uint8_t intensity_ = 255;
};

/**
 * @brief Efeito "Neon Pulse" - Pulso neon pulsante concêntrico
 */
class AnimationEngine::NeonPulseEffect {
public:
  NeonPulseEffect(TFT_eSPI *tft) : tft_(tft) {}
  ~NeonPulseEffect() = default;

  void init();
  void update();
  void render();

  void setPulseSpeed(uint8_t speed);

private:
  TFT_eSPI *tft_;
  float pulsePhase_ = 0;
  float pulseSpeed_ = 1.0f;
  int16_t centerX_ = 0;
  int16_t centerY_ = 0;
};

/**
 * @brief Efeito "Circuit Flow" - Fluxo de circuito digital
 */
class AnimationEngine::CircuitFlowEffect {
public:
  CircuitFlowEffect(TFT_eSPI *tft) : tft_(tft) {}
  ~CircuitFlowEffect() = default;

  void init();
  void update();
  void render();

  void setFlowSpeed(uint8_t speed);

private:
  static constexpr int MAX_CIRCUIT_NODES = 12;

  struct CircuitNode {
    int16_t x, y;
    bool active;
    uint16_t phase;
  };

  struct CircuitConnection {
    int from;
    int to;
  };

  TFT_eSPI *tft_;
  std::vector<CircuitNode> nodes_;
  std::vector<CircuitConnection> connections_;
  uint16_t flowPhase_ = 0;
  uint8_t flowSpeed_ = 1;
};

#endif // ANIMATION_ENGINE_H