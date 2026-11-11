/**
 * Willy 2.1 - Splash Screen Cyberpunk Orca v2.0
 * 5 melhorias implementadas:
 * 1. Som de boot (rugido + esguicho)
 * 2. Partículas neon (bolhas + circuitos flutuantes)
 * 3. Glow forte na orca
 * 4. Transição suave para o menu principal
 * 5. Configurações salvas no LittleFS/SD
 */

#include "willy_splash.h"
#include "../core/config.h"
#include "../core/display.h"
#include "../core/mykeyboard.h"
#include "../modules/others/audio.h"
#include <Arduino.h>
#include <LittleFS.h>

#define ACCENT_COLOR 0x00E6FF

// LVGL animation wrapper functions — lv_obj_set_style_* takes 3 args
// (obj, value, selector) but lv_anim_exec_xcb_t passes only 2 (obj, value).
// These wrappers bridge the gap safely.
static void anim_set_x(void *obj, int32_t v) {
  lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)v);
}
static void anim_set_y(void *obj, int32_t v) {
  lv_obj_set_y((lv_obj_t *)obj, (lv_coord_t)v);
}
static void anim_set_opa(void *obj, int32_t v) {
  lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}
static void anim_set_transform_angle(void *obj, int32_t v) {
  lv_obj_set_style_transform_angle((lv_obj_t *)obj, (int16_t)v, 0);
}

// ====================== CONFIGURAÇÕES ======================
struct WillySplashConfig {
  int velocidade = 1; // 0 = lento, 1 = normal, 2 = rápido
  bool somAtivado = true;
  int tipoSom = 0;                 // 0 = rugido + esguicho, 1 = só esguicho
  uint32_t corPrimaria = 0x00E6FF; // Ciano neon futurista
  int animationMode =
      0; // 0 = Clássico, 1 = Quantum Flow, 2 = Circuit Flow, 3 = Neon Pulse
  int effectIntensity = 5; // 1-10 intensidade dos efeitos
};

static WillySplashConfig willySplashCfg;

// ====================== OBJETOS ======================
static lv_obj_t *orca_container = nullptr;
static lv_obj_t *orca_body = nullptr;
static lv_obj_t *orca_fin = nullptr;
static lv_obj_t *orca_tail = nullptr;
static lv_obj_t *orca_eye = nullptr;
static lv_obj_t *orca_glow = nullptr;
static lv_obj_t *willy_text = nullptr;
static lv_obj_t *version_text = nullptr;

static lv_obj_t *bubbles[15] = {nullptr};
static lv_obj_t *circuits[8] = {nullptr};

// ====================== FUNÇÕES DE CONFIG ======================
void load_willy_config() {
  // LittleFS is already mounted by begin_storage() in main.cpp
  // Do NOT call LittleFS.begin() again here.
  File f = LittleFS.open("/willy_splash.conf", "r");
  if (f) {
    willySplashCfg.velocidade = f.readStringUntil('\n').toInt();
    willySplashCfg.somAtivado = f.readStringUntil('\n') == "1";
    willySplashCfg.tipoSom = f.readStringUntil('\n').toInt();
    willySplashCfg.corPrimaria =
        strtol(f.readStringUntil('\n').c_str(), NULL, 16);
    // Novos campos - com valores padrão se não existirem
    if (f.available()) {
      willySplashCfg.animationMode = f.readStringUntil('\n').toInt();
    } else {
      willySplashCfg.animationMode = 0; // Clássico padrão
    }
    if (f.available()) {
      willySplashCfg.effectIntensity = f.readStringUntil('\n').toInt();
    } else {
      willySplashCfg.effectIntensity = 5; // Intensidade média
    }
    f.close();
  }
}

void save_willy_config() {
  File f = LittleFS.open("/willy_splash.conf", "w");
  if (f) {
    f.println(willySplashCfg.velocidade);
    f.println(willySplashCfg.somAtivado ? "1" : "0");
    f.println(willySplashCfg.tipoSom);
    f.printf("%04lX\n", (unsigned long)willySplashCfg.corPrimaria);
    f.println(willySplashCfg.animationMode);
    f.println(willySplashCfg.effectIntensity);
    f.close();
  }
}

// ====================== SOM DA ORCA ======================
static void play_orca_boot_sound() {
  if (!willySplashCfg.somAtivado)
    return;

  if (willySplashCfg.tipoSom == 0 || willySplashCfg.tipoSom == 1) {
    // Rugido grave da orca
    _tone(180, 280);
    vTaskDelay(pdMS_TO_TICKS(320));
    _tone(140, 420);
    vTaskDelay(pdMS_TO_TICKS(450));
  }
  if (willySplashCfg.tipoSom == 0 || willySplashCfg.tipoSom == 1) {
    // Esguicho digital
    _tone(920, 90);
    vTaskDelay(pdMS_TO_TICKS(100));
    _tone(1250, 70);
    vTaskDelay(pdMS_TO_TICKS(80));
    _tone(680, 110);
  }
}

// ====================== DESENHO DA ORCA ======================
static void create_orca(lv_obj_t *parent) {
  orca_container = lv_obj_create(parent);
  lv_obj_set_size(orca_container, 160, 100);
  lv_obj_set_pos(orca_container, -200, 50);
  lv_obj_set_style_bg_opa(orca_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(orca_container, 0, 0);

  // Corpo
  orca_body = lv_obj_create(orca_container);
  lv_obj_set_size(orca_body, 125, 65);
  lv_obj_set_pos(orca_body, 15, 18);
  lv_obj_set_style_bg_color(orca_body, lv_color_hex(0x061018), 0);
  lv_obj_set_style_border_color(orca_body,
                                lv_color_hex(willySplashCfg.corPrimaria), 0);
  lv_obj_set_style_border_width(orca_body, 6, 0);
  lv_obj_set_style_radius(orca_body, 45, 0);

  // Barbatana dorsal
  orca_fin = lv_obj_create(orca_container);
  lv_obj_set_size(orca_fin, 28, 45);
  lv_obj_set_pos(orca_fin, 65, -18);
  lv_obj_set_style_bg_color(orca_fin, lv_color_hex(willySplashCfg.corPrimaria),
                            0);
  lv_obj_set_style_radius(orca_fin, 10, 0);

  // Cauda
  orca_tail = lv_obj_create(orca_container);
  lv_obj_set_size(orca_tail, 42, 38);
  lv_obj_set_pos(orca_tail, 125, 32);
  lv_obj_set_style_bg_color(orca_tail, lv_color_hex(willySplashCfg.corPrimaria),
                            0);
  lv_obj_set_style_radius(orca_tail, 25, 0);

  // Olho com glow forte
  orca_eye = lv_obj_create(orca_container);
  lv_obj_set_size(orca_eye, 18, 18);
  lv_obj_set_pos(orca_eye, 38, 28);
  lv_obj_set_style_bg_color(orca_eye, lv_color_hex(0xE6F7FF), 0);
  lv_obj_set_style_radius(orca_eye, 50, 0);

  orca_glow = lv_obj_create(orca_eye);
  lv_obj_set_size(orca_glow, 11, 11);
  lv_obj_set_pos(orca_glow, 3, 3);
  lv_obj_set_style_bg_color(orca_glow, lv_color_hex(ACCENT_COLOR), 0);
  lv_obj_set_style_radius(orca_glow, 50, 0);
  lv_obj_set_style_shadow_color(orca_glow, lv_color_hex(ACCENT_COLOR), 0);
  lv_obj_set_style_shadow_width(orca_glow, 30, 0);
  lv_obj_set_style_shadow_spread(orca_glow, 8, 0);

  // Textos
  willy_text = lv_label_create(parent);
  lv_label_set_text(willy_text, "WILLY");
  lv_obj_set_style_text_color(willy_text,
                              lv_color_hex(willySplashCfg.corPrimaria), 0);
  lv_obj_set_style_text_font(willy_text, &lv_font_montserrat_28, 0);
  lv_obj_set_style_opa(willy_text, 0, 0);
  lv_obj_align(willy_text, LV_ALIGN_CENTER, 0, 78);

  version_text = lv_label_create(parent);
  lv_label_set_text(version_text, WILLY_VERSION);
  lv_obj_set_style_text_color(version_text, lv_color_hex(ACCENT_COLOR), 0);
  lv_obj_set_style_text_font(version_text, &lv_font_montserrat_14, 0);
  lv_obj_set_style_opa(version_text, 0, 0);
  lv_obj_align(version_text, LV_ALIGN_CENTER, 0, 105);
}

static void create_particles(lv_obj_t *parent) {
  for (int i = 0; i < 15; i++) {
    bubbles[i] = lv_obj_create(parent);
    lv_obj_set_size(bubbles[i], 4 + (i % 3), 4 + (i % 3));
    lv_obj_set_pos(bubbles[i], 20 + (i * 15) % 200, 140);
    lv_obj_set_style_bg_color(bubbles[i], lv_color_hex(ACCENT_COLOR), 0);
    lv_obj_set_style_radius(bubbles[i], 50, 0);
    lv_obj_set_style_border_width(bubbles[i], 0, 0);
    lv_obj_set_style_bg_opa(bubbles[i], LV_OPA_60, 0);
  }

  for (int i = 0; i < 8; i++) {
    circuits[i] = lv_obj_create(parent);
    lv_obj_set_size(circuits[i], 3 + (i % 2), 3 + (i % 2));
    lv_obj_set_pos(circuits[i], 10 + (i * 25) % 200, 140);
    lv_obj_set_style_bg_color(circuits[i],
                              lv_color_hex(willySplashCfg.corPrimaria), 0);
    lv_obj_set_style_radius(circuits[i], 0, 0);
    lv_obj_set_style_border_width(circuits[i], 0, 0);
    lv_obj_set_style_bg_opa(circuits[i], LV_OPA_80, 0);
  }
}

// ====================== ANIMAÇÕES + PARTÍCULAS ======================
static void start_animations() {
  int base = (willySplashCfg.velocidade == 0)
                 ? 2600
                 : (willySplashCfg.velocidade == 2 ? 1400 : 1900);

  // Orca entra nadando
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, orca_container);
  lv_anim_set_values(&a, -220, 0);
  lv_anim_set_time(&a, base);
  lv_anim_set_exec_cb(&a, anim_set_x);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
  lv_anim_start(&a);

  // Nadando suave (wiggle)
  lv_anim_init(&a);
  lv_anim_set_var(&a, orca_container);
  lv_anim_set_values(&a, -8, 8);
  lv_anim_set_time(&a, base - 400);
  lv_anim_set_exec_cb(&a, anim_set_y);
  lv_anim_set_playback_time(&a, base - 400);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);

  // Cauda balançando
  lv_anim_init(&a);
  lv_anim_set_var(&a, orca_tail);
  lv_anim_set_values(&a, -420, 420);
  lv_anim_set_time(&a, 750);
  lv_anim_set_exec_cb(&a, anim_set_transform_angle);
  lv_anim_set_playback_time(&a, 750);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);

  // Glow forte no olho
  lv_anim_init(&a);
  lv_anim_set_var(&a, orca_glow);
  lv_anim_set_values(&a, 120, 255);
  lv_anim_set_time(&a, 680);
  lv_anim_set_playback_time(&a, 680);
  lv_anim_set_exec_cb(&a, anim_set_opa);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);

  // Bolhas + partículas de circuito
  for (int i = 0; i < 15; i++) {
    lv_anim_init(&a);
    lv_anim_set_var(&a, bubbles[i]);
    lv_anim_set_values(&a, 140, -30);
    lv_anim_set_time(&a, 1800 + (i * 140));
    lv_anim_set_delay(&a, i * 120);
    lv_anim_set_exec_cb(&a, anim_set_y);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }

  for (int i = 0; i < 8; i++) {
    lv_anim_init(&a);
    lv_anim_set_var(&a, circuits[i]);
    lv_anim_set_values(&a, 140, -30);
    lv_anim_set_time(&a, 2000 + (i * 200));
    lv_anim_set_delay(&a, i * 150 + 500);
    lv_anim_set_exec_cb(&a, anim_set_y);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }

  // Textos aparecendo
  lv_anim_init(&a);
  lv_anim_set_var(&a, willy_text);
  lv_anim_set_values(&a, 0, 255);
  lv_anim_set_time(&a, 1100);
  lv_anim_set_delay(&a, base - 300);
  lv_anim_set_exec_cb(&a, anim_set_opa);
  lv_anim_start(&a);

  lv_anim_init(&a);
  lv_anim_set_var(&a, version_text);
  lv_anim_set_values(&a, 0, 255);
  lv_anim_set_time(&a, 900);
  lv_anim_set_delay(&a, base + 200);
  lv_anim_set_exec_cb(&a, anim_set_opa);
  lv_anim_start(&a);
}

// ====================== TRANSITION + FINAL ======================
static void finish_splash(lv_timer_t *t) {
  lv_obj_t *parent = (lv_obj_t *)t->user_data;
  lv_obj_fade_out(parent, 800, 0);
  lv_timer_del(t);
}

// ====================== NOVOS EFEITOS QUANTUM ======================
static void create_quantum_particles(lv_obj_t *parent) {
  // Criar partículas quânticas para o efeito Quantum Flow
  int particleCount = willySplashCfg.effectIntensity * 3; // 15-30 partículas

  for (int i = 0; i < particleCount; i++) {
    lv_obj_t *particle = lv_obj_create(parent);
    int size = random(2, 5);
    lv_obj_set_size(particle, size, size);

    // Posição inicial aleatória
    int x = random(20, tftWidth - 20);
    int y = random(20, tftHeight - 20);
    lv_obj_set_pos(particle, x, y);

    // Cor baseada no tema Neon Aqua
    uint16_t color = (i % 2 == 0) ? 0x06FF : 0x03B5; // Neon Aqua futurista
    lv_obj_set_style_bg_color(particle, lv_color_hex(color), 0);
    lv_obj_set_style_radius(particle, 50, 0); // Circular
    lv_obj_set_style_bg_opa(particle, LV_OPA_80, 0);

    // Animação de movimento quântico
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, particle);

    // Movimento caótico com oscilação
    int targetX = x + random(-40, 40);
    int targetY = y + random(-40, 40);

    lv_anim_set_values(&a, x, targetX);
    lv_anim_set_time(&a, 2000 + random(0, 1000));
    lv_anim_set_exec_cb(&a, anim_set_x);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, particle);
    lv_anim_set_values(&a, y, targetY);
    lv_anim_set_time(&a, 2000 + random(0, 1000));
    lv_anim_set_exec_cb(&a, anim_set_y);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Pulsar opacidade
    lv_anim_init(&a);
    lv_anim_set_var(&a, particle);
    lv_anim_set_values(&a, LV_OPA_40, LV_OPA_100);
    lv_anim_set_time(&a, 1500);
    lv_anim_set_exec_cb(&a, anim_set_opa);
    lv_anim_set_playback_time(&a, 1500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }
}

static void create_circuit_flow(lv_obj_t *parent) {
  // Criar nós de circuito digital
  int nodeCount = willySplashCfg.effectIntensity * 2; // 10-20 nós

  for (int i = 0; i < nodeCount; i++) {
    lv_obj_t *node = lv_obj_create(parent);
    lv_obj_set_size(node, 4, 4);

    // Posição em grid
    int x = 30 + (i % 5) * ((tftWidth - 60) / 5);
    int y = 40 + (i / 5) * ((tftHeight - 80) / 4);
    lv_obj_set_pos(node, x, y);

    lv_obj_set_style_bg_color(node, lv_color_hex(0x06FF), 0);
    lv_obj_set_style_radius(node, 0, 0); // Quadrado
    lv_obj_set_style_bg_opa(node, LV_OPA_60, 0);

    // Animação de ativação sequencial
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, node);
    lv_anim_set_values(&a, LV_OPA_20, LV_OPA_100);
    lv_anim_set_time(&a, 800);
    lv_anim_set_delay(&a, i * 100);
    lv_anim_set_exec_cb(&a, anim_set_opa);
    lv_anim_set_playback_time(&a, 800);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }
}

static void create_neon_pulse(lv_obj_t *parent) {
  // Criar anéis concêntricos de pulso neon
  int ringCount = 3;

  for (int i = 0; i < ringCount; i++) {
    lv_obj_t *ring = lv_obj_create(parent);
    int size = 40 + (i * 25);
    lv_obj_set_size(ring, size, size);
    lv_obj_set_pos(ring, (tftWidth - size) / 2, (tftHeight - size) / 2);

    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 2, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x06FF), 0);
    lv_obj_set_style_radius(ring, size / 2, 0);

    // Animação de pulso
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ring);
    lv_anim_set_values(&a, LV_OPA_20, LV_OPA_80);
    lv_anim_set_time(&a, 1500);
    lv_anim_set_delay(&a, i * 300);
    lv_anim_set_exec_cb(&a, anim_set_opa);
    lv_anim_set_playback_time(&a, 1500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Animação de escala
    lv_anim_init(&a);
    lv_anim_set_var(&a, ring);
    lv_anim_set_values(&a, size, size + 10);
    lv_anim_set_time(&a, 1500);
    lv_anim_set_delay(&a, i * 300);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
      lv_obj_set_size((lv_obj_t *)obj, v, v);
      lv_obj_set_pos((lv_obj_t *)obj, (tftWidth - v) / 2, (tftHeight - v) / 2);
    });
    lv_anim_set_playback_time(&a, 1500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }
}

// ====================== FUNÇÃO PRINCIPAL ======================
void show_willy_splash(lv_obj_t *parent) {
  load_willy_config();

  lv_obj_clean(parent);
  lv_obj_set_style_bg_color(parent, lv_color_hex(0x05070D), 0);

  // Escolher efeito baseado no modo
  switch (willySplashCfg.animationMode) {
  case 1: // Quantum Flow
    create_quantum_particles(parent);
    break;

  case 2: // Circuit Flow
    create_circuit_flow(parent);
    break;

  case 3: // Neon Pulse
    create_neon_pulse(parent);
    break;

  case 0: // Clássico (Orca) - padrão
  default:
    create_orca(parent);
    create_particles(parent);
    start_animations();
    break;
  }

  play_orca_boot_sound();

  // Transição suave para o menu após 4.8 segundos
  lv_timer_create(finish_splash, 4200, parent);
}
