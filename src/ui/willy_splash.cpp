/**
 * Willy 2.1 - Splash Screen com Baleia Orca Animada
 * Hub de Segurança Ofensiva para Pentest
 *
 * Animação vetorial LVGL - Leve e otimizado para ESP32
 */

#include <lvgl.h>
#include "../core/config.h"
#include "../core/display.h"

// Configurações do Logo Willy
struct WillyLogoConfig {
    int velocidade = 1;        // 0=lento, 1=normal, 2=rápido
    bool somAtivado = true;    // Som do boot
    int tipoSom = 0;           // 0=rugido+esguicho, 1=só esguicho, 2=silêncio
    bool corCustomizada = false;
    uint16_t corPrimaria = 0x07FF; // Ciano neon padrão
};

WillyLogoConfig willyLogoConfig;

// Variáveis dos objetos animados
static lv_obj_t *orca_container = nullptr;
static lv_obj_t *orca_body = nullptr;
static lv_obj_t *orca_dorsal_fin = nullptr;
static lv_obj_t *orca_tail = nullptr;
static lv_obj_t *orca_eye = nullptr;
static lv_obj_t *orca_eye_white = nullptr;
static lv_obj_t *orca_belly = nullptr;
static lv_obj_t *willy_label = nullptr;
static lv_obj_t *version_label = nullptr;
static lv_obj_t *bubbles[10] = {nullptr};
static lv_obj_t *water_drops[10] = {nullptr};

// Tempos de animação baseados na velocidade
static int getAnimTime(int baseTime) {
    switch(willyLogoConfig.velocidade) {
        case 0: return baseTime * 1.5;      // Lento
        case 2: return baseTime / 1.5;      // Rápido
        default: return baseTime;            // Normal
    }
}

// Callback para animação de wiggle (nadando)
static void wiggle_anim_cb(void *var, int32_t v) {
    lv_obj_set_y((lv_obj_t*)var, v);
}

// Callback para bolhas subindo
static void bubble_anim_cb(void *var, int32_t v) {
    lv_obj_set_y((lv_obj_t*)var, v);
    lv_obj_set_style_opa((lv_obj_t*)var, v > 30 ? (v * 2) : 255, 0);
}

// Desenha a baleia orca vetorial
static void draw_orca(lv_obj_t *parent) {
    uint16_t primary_color = willyLogoConfig.corCustomizada ?
                             willyLogoConfig.corPrimaria :
                             bruceConfig.priColor;
    uint16_t white_color = 0xFFFF;
    uint16_t black_color = 0x0000;

    // Container principal da orca
    orca_container = lv_obj_create(parent);
    lv_obj_set_size(orca_container, 120, 60);
    lv_obj_set_pos(orca_container, -150, 0); // Começa fora da tela
    lv_obj_set_style_bg_opa(orca_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(orca_container, 0, 0);
    lv_obj_set_style_pad_all(orca_container, 0, 0);
    lv_obj_align(orca_container, LV_ALIGN_CENTER, 0, 0);

    // Corpo principal da orca
    orca_body = lv_obj_create(orca_container);
    lv_obj_set_size(orca_body, 100, 45);
    lv_obj_set_pos(orca_body, 0, 5);
    lv_obj_set_style_bg_color(orca_body, lv_color_hex(primary_color), 0);
    lv_obj_set_style_radius(orca_body, 50, 0);
    lv_obj_set_style_border_width(orca_body, 0, 0);
    lv_obj_set_style_pad_all(orca_body, 0, 0);

    // Barriga branca
    orca_belly = lv_obj_create(orca_container);
    lv_obj_set_size(orca_belly, 70, 20);
    lv_obj_set_pos(orca_belly, 15, 25);
    lv_obj_set_style_bg_color(orca_belly, lv_color_hex(white_color), 0);
    lv_obj_set_style_radius(orca_belly, 50, 0);
    lv_obj_set_style_border_width(orca_belly, 0, 0);
    lv_obj_set_style_pad_all(orca_belly, 0, 0);

    // Barbatana dorsal
    orca_dorsal_fin = lv_obj_create(orca_container);
    lv_obj_set_size(orca_dorsal_fin, 15, 30);
    lv_obj_set_pos(orca_dorsal_fin, 45, -20);
    lv_obj_set_style_bg_color(orca_dorsal_fin, lv_color_hex(primary_color), 0);
    lv_obj_set_style_radius(orca_dorsal_fin, 5, 0);
    lv_obj_set_style_transform_angle(orca_dorsal_fin, 350, 0);
    lv_obj_set_style_border_width(orca_dorsal_fin, 0, 0);
    lv_obj_set_style_pad_all(orca_dorsal_fin, 0, 0);

    // Cauda
    orca_tail = lv_obj_create(orca_container);
    lv_obj_set_size(orca_tail, 30, 25);
    lv_obj_set_pos(orca_tail, 85, 15);
    lv_obj_set_style_bg_color(orca_tail, lv_color_hex(primary_color), 0);
    lv_obj_set_style_radius(orca_tail, 50, 0);
    lv_obj_set_style_border_width(orca_tail, 0, 0);
    lv_obj_set_style_pad_all(orca_tail, 0, 0);

    // Olho
    orca_eye_white = lv_obj_create(orca_container);
    lv_obj_set_size(orca_eye_white, 12, 10);
    lv_obj_set_pos(orca_eye_white, 15, 15);
    lv_obj_set_style_bg_color(orca_eye_white, lv_color_hex(white_color), 0);
    lv_obj_set_style_radius(orca_eye_white, 50, 0);
    lv_obj_set_style_border_width(orca_eye_white, 0, 0);
    lv_obj_set_style_pad_all(orca_eye_white, 0, 0);

    orca_eye = lv_obj_create(orca_eye_white);
    lv_obj_set_size(orca_eye, 6, 6);
    lv_obj_set_pos(orca_eye, 3, 2);
    lv_obj_set_style_bg_color(orca_eye, lv_color_hex(black_color), 0);
    lv_obj_set_style_radius(orca_eye, 50, 0);
    lv_obj_set_style_border_width(orca_eye, 0, 0);
    lv_obj_set_style_pad_all(orca_eye, 0, 0);

    // Labels
    willy_label = lv_label_create(parent);
    lv_label_set_text(willy_label, "WILLY");
    lv_obj_set_style_text_color(willy_label, lv_color_hex(primary_color), 0);
    lv_obj_set_style_text_font(willy_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_opa(willy_label, 0, 0);
    lv_obj_align(willy_label, LV_ALIGN_CENTER, 0, 60);

    version_label = lv_label_create(parent);
    lv_label_set_text(version_label, "v2.1");
    lv_obj_set_style_text_color(version_label, lv_color_hex(primary_color), 0);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_opa(version_label, 0, 0);
    lv_obj_align(version_label, LV_ALIGN_CENTER, 0, 90);

    // Bolhas (Mais bolhas e menores)
    for(int i = 0; i < 10; i++) {
        bubbles[i] = lv_obj_create(parent);
        int size = 3 + (i % 4);
        lv_obj_set_size(bubbles[i], size, size);
        lv_obj_set_pos(bubbles[i], 40 + (i * 18), 100);
        lv_obj_set_style_bg_color(bubbles[i], lv_color_hex(white_color), 0);
        lv_obj_set_style_bg_opa(bubbles[i], LV_OPA_40, 0);
        lv_obj_set_style_radius(bubbles[i], 50, 0);
        lv_obj_set_style_border_width(bubbles[i], 0, 0);
    }

    // Gotas de água (Fountain effect)
    for(int i = 0; i < 10; i++) {
        water_drops[i] = lv_obj_create(parent);
        lv_obj_set_size(water_drops[i], 3, 3);
        lv_obj_set_pos(water_drops[i], 50, 40); // Base do esguicho
        lv_obj_set_style_bg_color(water_drops[i], lv_color_hex(0x00A0FF), 0);
        lv_obj_set_style_radius(water_drops[i], 50, 0);
        lv_obj_set_style_opa(water_drops[i], 0, 0);
    }
}

// Inicia todas as animações
static void start_animations(lv_obj_t *parent) {
    int baseTime = getAnimTime(1800);

    // Entrada da Orca
    lv_anim_t move_x;
    lv_anim_init(&move_x);
    lv_anim_set_var(&move_x, orca_container);
    lv_anim_set_values(&move_x, -160, 0);
    lv_anim_set_time(&move_x, baseTime);
    lv_anim_set_exec_cb(&move_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&move_x, lv_anim_path_ease_out);
    lv_anim_start(&move_x);

    // Wiggle (nadando)
    lv_anim_t wiggle;
    lv_anim_init(&wiggle);
    lv_anim_set_var(&wiggle, orca_container);
    lv_anim_set_values(&wiggle, -8, 8);
    lv_anim_set_time(&wiggle, getAnimTime(1200));
    lv_anim_set_exec_cb(&wiggle, wiggle_anim_cb);
    lv_anim_set_path_cb(&wiggle, lv_anim_path_linear);
    lv_anim_set_playback_time(&wiggle, getAnimTime(1200));
    lv_anim_set_repeat_count(&wiggle, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&wiggle);

    // Olhos
    lv_anim_t blink;
    lv_anim_init(&blink);
    lv_anim_set_var(&blink, orca_eye_white);
    lv_anim_set_values(&blink, 255, 0);
    lv_anim_set_time(&blink, 150);
    lv_anim_set_exec_cb(&blink, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_delay(&blink, 3000);
    lv_anim_set_playback_time(&blink, 150);
    lv_anim_set_repeat_count(&blink, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&blink);

    // Fade labels
    lv_anim_t text_fade;
    lv_anim_init(&text_fade);
    lv_anim_set_var(&text_fade, willy_label);
    lv_anim_set_values(&text_fade, 0, 255);
    lv_anim_set_time(&text_fade, 1000);
    lv_anim_set_delay(&text_fade, baseTime - 500);
    lv_anim_set_exec_cb(&text_fade, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&text_fade);

    lv_anim_t version_fade;
    lv_anim_init(&version_fade);
    lv_anim_set_var(&version_fade, version_label);
    lv_anim_set_values(&version_fade, 0, 255);
    lv_anim_set_time(&version_fade, 800);
    lv_anim_set_delay(&version_fade, baseTime);
    lv_anim_set_exec_cb(&version_fade, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&version_fade);

    // Bolhas subindo
    for(int i = 0; i < 10; i++) {
        lv_anim_t bubble_up;
        lv_anim_init(&bubble_up);
        lv_anim_set_var(&bubble_up, bubbles[i]);
        lv_anim_set_values(&bubble_up, 120, -40);
        lv_anim_set_time(&bubble_up, 2000 + (i * 200));
        lv_anim_set_delay(&bubble_up, i * 400);
        lv_anim_set_exec_cb(&bubble_up, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_repeat_count(&bubble_up, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&bubble_up);
    }

    // Esguicho (Fountain effect)
    for(int i = 0; i < 10; i++) {
        lv_anim_t drop_x, drop_y, drop_opa;

        // Horizontal arc
        lv_anim_init(&drop_x);
        lv_anim_set_var(&drop_x, water_drops[i]);
        lv_anim_set_values(&drop_x, 50, 30 + (i * 4));
        lv_anim_set_time(&drop_x, 1000);
        lv_anim_set_delay(&drop_x, baseTime + (i * 100));
        lv_anim_set_exec_cb(&drop_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_repeat_count(&drop_x, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&drop_x);

        // Vertical spout arc
        lv_anim_init(&drop_y);
        lv_anim_set_var(&drop_y, water_drops[i]);
        lv_anim_set_values(&drop_y, 40, -10);
        lv_anim_set_time(&drop_y, 500);
        lv_anim_set_playback_time(&drop_y, 500);
        lv_anim_set_delay(&drop_y, baseTime + (i * 100));
        lv_anim_set_exec_cb(&drop_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_repeat_count(&drop_y, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&drop_y);

        // Opacity pulse
        lv_anim_init(&drop_opa);
        lv_anim_set_var(&drop_opa, water_drops[i]);
        lv_anim_set_values(&drop_opa, 0, 200);
        lv_anim_set_time(&drop_opa, 200);
        lv_anim_set_playback_time(&drop_opa, 800);
        lv_anim_set_delay(&drop_opa, baseTime + (i * 100));
        lv_anim_set_exec_cb(&drop_opa, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
        lv_anim_set_repeat_count(&drop_opa, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&drop_opa);
    }
}

// Reproduz som da orca
static void play_orca_sound() {
    if(!willyLogoConfig.somAtivado || willyLogoConfig.tipoSom == 2) return;

#if defined(HAS_SPEAKER) || defined(BUZZER_PIN)
    int pin = -1;
#ifdef BUZZER_PIN
    pin = BUZZER_PIN;
#elif defined(SPEAKER_PIN)
    pin = SPEAKER_PIN;
#endif

    if(pin >= 0) {
        if(willyLogoConfig.tipoSom == 0 || willyLogoConfig.tipoSom == 1) {
            // Rugido grave (orca vindo)
            if(willyLogoConfig.tipoSom == 0) {
                tone(pin, 150, 300);
                delay(350);
                tone(pin, 120, 400);
                delay(450);
            }

            // Esguicho (água)
            tone(pin, 800, 100);
            delay(100);
            tone(pin, 1000, 80);
            delay(80);
            tone(pin, 700, 120);
            delay(100);
        }
    }
#endif
}

// Função principal para mostrar splash screen
void show_willy_splash(lv_obj_t *parent) {
    // Limpa tela
    lv_obj_clean(parent);

    // Fundo
    lv_obj_set_style_bg_color(parent, lv_color_hex(bruceConfig.bgColor), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    // Desenha a orca
    draw_orca(parent);

    // Inicia animações
    start_animations(parent);

    // Toca som (se ativado)
    play_orca_sound();
}

// Carrega configurações do logo do SD
void load_willy_logo_config() {
    // Tenta carregar do SD
    if(SD.begin()) {
        File f = SD.open("/willy_logo.conf", FILE_READ);
        if(f) {
            // Lê configurações
            String line = f.readStringUntil('\n');
            while(line.length() > 0) {
                if(line.startsWith("velocidade=")) {
                    willyLogoConfig.velocidade = line.substring(10).toInt();
                } else if(line.startsWith("som=")) {
                    willyLogoConfig.somAtivado = line.substring(4) == "on";
                } else if(line.startsWith("tipo_som=")) {
                    willyLogoConfig.tipoSom = line.substring(9).toInt();
                } else if(line.startsWith("cor_custom=")) {
                    willyLogoConfig.corCustomizada = line.substring(11) == "on";
                } else if(line.startsWith("cor=")) {
                    willyLogoConfig.corPrimaria = strtol(line.substring(4).c_str(), NULL, 16);
                }
                line = f.readStringUntil('\n');
            }
            f.close();
        }
    }

    // Valida valores
    if(willyLogoConfig.velocidade < 0 || willyLogoConfig.velocidade > 2) {
        willyLogoConfig.velocidade = 1;
    }
    if(willyLogoConfig.tipoSom < 0 || willyLogoConfig.tipoSom > 2) {
        willyLogoConfig.tipoSom = 0;
    }
}

// Salva configurações do logo no SD
void save_willy_logo_config() {
    if(SD.begin()) {
        File f = SD.open("/willy_logo.conf", FILE_WRITE);
        if(f) {
            f.printf("velocidade=%d\n", willyLogoConfig.velocidade);
            f.printf("som=%s\n", willyLogoConfig.somAtivado ? "on" : "off");
            f.printf("tipo_som=%d\n", willyLogoConfig.tipoSom);
            f.printf("cor_custom=%s\n", willyLogoConfig.corCustomizada ? "on" : "off");
            f.printf("cor=%04X\n", willyLogoConfig.corPrimaria);
            f.close();
        }
    }
}

// Getters e Setters para configurações do logo
int get_willy_logo_velocidade() { return willyLogoConfig.velocidade; }
void set_willy_logo_velocidade(int v) {
    willyLogoConfig.velocidade = v;
    save_willy_logo_config();
}

bool get_willy_logo_som() { return willyLogoConfig.somAtivado; }
void set_willy_logo_som(bool v) {
    willyLogoConfig.somAtivado = v;
    save_willy_logo_config();
}

int get_willy_logo_tipo_som() { return willyLogoConfig.tipoSom; }
void set_willy_logo_tipo_som(int v) {
    willyLogoConfig.tipoSom = v;
    save_willy_logo_config();
}

bool get_willy_logo_cor_customizada() { return willyLogoConfig.corCustomizada; }
void set_willy_logo_cor_customizada(bool v) {
    willyLogoConfig.corCustomizada = v;
    save_willy_logo_config();
}

uint16_t get_willy_logo_cor() { return willyLogoConfig.corPrimaria; }
void set_willy_logo_cor(uint16_t v) {
    willyLogoConfig.corPrimaria = v;
    save_willy_logo_config();
}
