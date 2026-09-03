#ifndef __SYSTEM_VIEW_H__
#define __SYSTEM_VIEW_H__

#include <Arduino.h>
#include <TFT_eSPI.h>

/**
 * @brief Visão do sistema - gerencia UI e display
 * Parte do padrão MVC (View)
 */
class SystemView {
public:
  static SystemView &getInstance();

  // Inicialização
  bool init();
  void deinit();

  // Controle de display
  void wakeUpScreen();
  void dimScreen();
  void turnOffScreen();
  void setBrightness(int brightness);

  // Renderização
  void clearScreen(uint16_t color = 0x0000);
  void fillScreen(uint16_t color = 0x0000);
  void updateDisplay();

  // Estado
  bool isInitialized() const { return initialized_; }
  bool isScreenOn() const { return screenOn_; }
  bool isDisplayAvailable() const;

  // Acesso aos objetos de display (para compatibilidade)
  void *getTft() const;    // Retorna tft_logger*
  void *getSprite() const; // Retorna tft_sprite*

  // Integração com Animation Engine
  void enableAnimations(bool enable);
  void setAnimationFrameRate(uint8_t fps);
  void triggerGlitchEffect(uint16_t duration = 100);
  void addGlowEffect(int16_t x, int16_t y, uint16_t width, uint16_t height,
                     uint16_t color);

  // Otimizações de performance
  void renderCachedIcon(uint8_t index, int16_t x, int16_t y);
  void updateDisplayOptimized();
  void swapBuffers();

private:
  SystemView() = default;

public:
  ~SystemView() = default;

  bool initialized_ = false;
  bool screenOn_ = false;

  // Otimizações de performance para ESP32-S3
  TFT_eSprite *backBuffer_ = nullptr;  // Double buffering
  TFT_eSprite *spriteCache_ = nullptr; // Cache de sprites
  bool doubleBufferingEnabled_ = true;

  // Cache de ícones futuristas
  struct CachedIcon {
    const uint16_t *bitmap;
    uint16_t width;
    uint16_t height;
    TFT_eSprite *sprite;
    bool loaded;
  };
  CachedIcon iconCache_[16]; // Cache para até 16 ícones

  // Métodos de otimização
  void initDoubleBuffering();
  void initSpriteCache();
  void loadIconToCache(uint8_t index, const uint16_t *bitmap, uint16_t width,
                       uint16_t height);
  void applyInitialSettings();
};

#endif // __SYSTEM_VIEW_H__