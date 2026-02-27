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
#include "../core/display.h"
#include "../core/config.h"
#include "../core/mykeyboard.h"
#include "modules/others/audio.h"

#define ACCENT_COLOR 0x00FFFF

// ====================== CONFIGURAÇÕES ======================
struct WillyConfig {
    int velocidade = 1;           // 0 = lento, 1 = normal, 2 = rápido
    bool somAtivado = true;
    int tipoSom = 0;              // 0 = rugido + esguicho, 1 = só esguicho
    uint32_t corPrimaria = 0x9B00FF; // Roxo neon
};

static WillyConfig willyCfg;

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
    if (!LittleFS.begin()) return;
    File f = LittleFS.open("/willy_splash.conf", "r");
    if (f) {
        willyCfg.velocidade = f.readStringUntil('\n').toInt();
        willyCfg.somAtivado = f.readStringUntil('\n') == "1";
        willyCfg.tipoSom = f.readStringUntil('\n').toInt();
        willyCfg.corPrimaria = strtol(f.readStringUntil('\n').c_str(), NULL, 16);
        f.close();
    }
}

void save_willy_config() {
    File f = LittleFS.open("/willy_splash.conf", "w");
    if (f) {
        f.println(willyCfg.velocidade);
        f.println(willyCfg.somAtivado ? "1" : "0");
        f.println(willyCfg.tipoSom);
        f.printf("%04lX\n", (unsigned long)willyCfg.corPrimaria);
        f.close();
    }
}

// ====================== SOM DA ORCA ======================
static void play_orca_boot_sound() {
    if (!willyCfg.somAtivado) return;

    if (willyCfg.tipoSom == 0 || willyCfg.tipoSom == 1) {
        // Rugido grave da orca
        _tone(180, 280);
        delay(320);
        _tone(140, 420);
        delay(450);
    }
    if (willyCfg.tipoSom == 0 || willyCfg.tipoSom == 1) {
        // Esguicho digital
        _tone(920, 90);
        delay(100);
        _tone(1250, 70);
        delay(80);
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
    lv_obj_set_style_bg_color(orca_body, lv_color_hex(0x1A0033), 0);
    lv_obj_set_style_border_color(orca_body, lv_color_hex(willyCfg.corPrimaria), 0);
    lv_obj_set_style_border_width(orca_body, 6, 0);
    lv_obj_set_style_radius(orca_body, 45, 0);

    // Barbatana dorsal
    orca_fin = lv_obj_create(orca_container);
    lv_obj_set_size(orca_fin, 28, 45);
    lv_obj_set_pos(orca_fin, 65, -18);
    lv_obj_set_style_bg_color(orca_fin, lv_color_hex(willyCfg.corPrimaria), 0);
    lv_obj_set_style_radius(orca_fin, 10, 0);

    // Cauda
    orca_tail = lv_obj_create(orca_container);
    lv_obj_set_size(orca_tail, 42, 38);
    lv_obj_set_pos(orca_tail, 125, 32);
    lv_obj_set_style_bg_color(orca_tail, lv_color_hex(willyCfg.corPrimaria), 0);
    lv_obj_set_style_radius(orca_tail, 25, 0);

    // Olho com glow forte
    orca_eye = lv_obj_create(orca_container);
    lv_obj_set_size(orca_eye, 18, 18);
    lv_obj_set_pos(orca_eye, 38, 28);
    lv_obj_set_style_bg_color(orca_eye, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(orca_eye, 50, 0);

    orca_glow = lv_obj_create(orca_eye);
    lv_obj_set_size(orca_glow, 11, 11);
    lv_obj_set_pos(orca_glow, 3, 3);
    lv_obj_set_style_bg_color(orca_glow, lv_color_hex(ACCENT_COLOR), 0);
    lv_obj_set_style_radius(orca_glow, 50, 0);
    lv_obj_set_style_shadow_color(orca_glow, lv_color_hex(ACCENT_COLOR), 0);
    lv_obj_set_style_shadow_width(orca_glow, 25, 0);
    lv_obj_set_style_shadow_spread(orca_glow, 8, 0);

    // Textos
    willy_text = lv_label_create(parent);
    lv_label_set_text(willy_text, "WILLY");
    lv_obj_set_style_text_color(willy_text, lv_color_hex(willyCfg.corPrimaria), 0);
    lv_obj_set_style_text_font(willy_text, &lv_font_montserrat_28, 0);
    lv_obj_set_style_opa(willy_text, 0, 0);
    lv_obj_align(willy_text, LV_ALIGN_CENTER, 0, 78);

    version_text = lv_label_create(parent);
    lv_label_set_text(version_text, "v2.1");
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
        lv_obj_set_style_bg_color(circuits[i], lv_color_hex(willyCfg.corPrimaria), 0);
        lv_obj_set_style_radius(circuits[i], 0, 0);
        lv_obj_set_style_border_width(circuits[i], 0, 0);
        lv_obj_set_style_bg_opa(circuits[i], LV_OPA_80, 0);
    }
}

// ====================== ANIMAÇÕES + PARTÍCULAS ======================
static void start_animations() {
    int base = (willyCfg.velocidade == 0) ? 2600 : (willyCfg.velocidade == 2 ? 1400 : 1900);

    // Orca entra nadando
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, orca_container);
    lv_anim_set_values(&a, -220, 0);
    lv_anim_set_time(&a, base);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    // Nadando suave (wiggle)
    lv_anim_init(&a);
    lv_anim_set_var(&a, orca_container);
    lv_anim_set_values(&a, -8, 8);
    lv_anim_set_time(&a, base - 400);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_playback_time(&a, base - 400);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Cauda balançando
    lv_anim_init(&a);
    lv_anim_set_var(&a, orca_tail);
    lv_anim_set_values(&a, -420, 420);
    lv_anim_set_time(&a, 750);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_angle);
    lv_anim_set_playback_time(&a, 750);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Glow forte no olho
    lv_anim_init(&a);
    lv_anim_set_var(&a, orca_glow);
    lv_anim_set_values(&a, 120, 255);
    lv_anim_set_time(&a, 680);
    lv_anim_set_playback_time(&a, 680);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // Bolhas + partículas de circuito
    for (int i = 0; i < 15; i++) {
        lv_anim_init(&a);
        lv_anim_set_var(&a, bubbles[i]);
        lv_anim_set_values(&a, 140, -30);
        lv_anim_set_time(&a, 1800 + (i * 140));
        lv_anim_set_delay(&a, i * 120);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
    }

    for (int i = 0; i < 8; i++) {
        lv_anim_init(&a);
        lv_anim_set_var(&a, circuits[i]);
        lv_anim_set_values(&a, 140, -30);
        lv_anim_set_time(&a, 2000 + (i * 200));
        lv_anim_set_delay(&a, i * 150 + 500);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
    }

    // Textos aparecendo
    lv_anim_init(&a);
    lv_anim_set_var(&a, willy_text);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 1100);
    lv_anim_set_delay(&a, base - 300);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, version_text);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 900);
    lv_anim_set_delay(&a, base + 200);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&a);
}

// ====================== TRANSITION + FINAL ======================
static void finish_splash(lv_timer_t *t) {
    lv_obj_t *parent = (lv_obj_t*)t->user_data;
    lv_obj_fade_out(parent, 800, 0);
    lv_timer_del(t);
}

// ====================== FUNÇÃO PRINCIPAL ======================
void show_willy_splash(lv_obj_t *parent) {
    load_willy_config();

    lv_obj_clean(parent);
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0A001F), 0);

    create_orca(parent);
    create_particles(parent);
    start_animations();

    play_orca_boot_sound();

    // Transição suave para o menu após 4.8 segundos
    lv_timer_create(finish_splash, 4800, parent);
}
