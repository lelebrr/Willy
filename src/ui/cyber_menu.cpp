#include "cyber_menu.h"
#include "../core/config.h"
#include "../core/display.h"
#include "../core/utils.h"
#include "../core/main_menu.h"
#include <Arduino.h>
#include <time.h>

// Forward declarations of Bruce callbacks - implemented below
static void wifi_cb(lv_event_t *e);
static void ble_cb(lv_event_t *e);
static void ir_cb(lv_event_t *e);
static void nfc_cb(lv_event_t *e);
static void sub_cb(lv_event_t *e);
static void nrf_cb(lv_event_t *e);
static void gps_cb(lv_event_t *e);
static void attacks_cb(lv_event_t *e);
static void core_cb(lv_event_t *e);
static void logs_cb(lv_event_t *e);
static void rfid_cb(lv_event_t *e);
static void sd_cb(lv_event_t *e);

// Implementation of icon callbacks
static void wifi_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] Wi-Fi menu opened");
}

static void ble_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] BLE menu opened");
}

static void ir_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] IR menu opened");
}

static void nfc_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] NFC menu opened");
}

static void sub_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] Sub-GHz menu opened");
}

static void nrf_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] NRF24 menu opened");
}

static void gps_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] GPS menu opened");
}

static void attacks_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] Attacks menu opened");
}

static void core_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] Core menu opened");
}

static void logs_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] Logs menu opened");
}

static void rfid_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] RFID menu opened");
}

static void sd_cb(lv_event_t *e) {
    Serial.println("[CyberMenu] SD menu opened");
}

// Timer callback for top bar updates
static void update_bar_timer_cb(lv_timer_t *timer);

// Generic function to create modern cyber icons with animations
lv_obj_t *create_cyber_icon(lv_obj_t *parent, const char *icon_text, const lv_point_t *shape_points, uint16_t point_count, lv_color_t primary, lv_color_t secondary, lv_color_t accent, int x, int y, void (*click_cb)(lv_event_t *e), int index) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 90, 90);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_set_style_border_color(btn, secondary, 0);
    lv_obj_set_style_border_width(btn, 4, 0);
    lv_obj_set_style_shadow_color(btn, accent, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    // Modern gradient background
    lv_style_t *style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
    lv_style_init(style);
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_color(style, primary);
    lv_style_set_bg_grad_color(style, secondary);
    lv_style_set_bg_opa(style, LV_OPA_90);
    lv_obj_add_style(btn, style, 0);

    // Text label
    lv_obj_t *text = lv_label_create(btn);
    lv_label_set_text(text, icon_text);
    lv_obj_align(text, LV_ALIGN_BOTTOM_MID, 0, 5);
    lv_obj_set_style_text_color(text, lv_color_white(), 0);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_14, 0);

    // Animation 1: Cascade Fade-in
    lv_anim_t a_fade;
    lv_anim_init(&a_fade);
    lv_anim_set_var(&a_fade, btn);
    lv_anim_set_values(&a_fade, 0, LV_OPA_100);
    lv_anim_set_time(&a_fade, 400);
    lv_anim_set_delay(&a_fade, 100 * index);
    lv_anim_set_exec_cb(&a_fade, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_path_cb(&a_fade, lv_anim_path_ease_in_out);
    lv_anim_start(&a_fade);

    // Animation 2: Slide-up
    lv_anim_t a_slide;
    lv_anim_init(&a_slide);
    lv_anim_set_var(&a_slide, btn);
    lv_anim_set_values(&a_slide, y + 40, y);
    lv_anim_set_time(&a_slide, 400);
    lv_anim_set_delay(&a_slide, 100 * index);
    lv_anim_set_exec_cb(&a_slide, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a_slide, lv_anim_path_ease_in_out);
    lv_anim_start(&a_slide);

    // Animation 3: Neon Pulse (Border)
    lv_anim_t a_pulse;
    lv_anim_init(&a_pulse);
    lv_anim_set_var(&a_pulse, btn);
    lv_anim_set_values(&a_pulse, 0, 8); // Shadow thickness pulse
    lv_anim_set_time(&a_pulse, 1500);
    lv_anim_set_playback_time(&a_pulse, 1500);
    lv_anim_set_repeat_count(&a_pulse, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a_pulse, (lv_anim_exec_xcb_t)lv_obj_set_style_shadow_width);
    lv_anim_start(&a_pulse);

    // Hover effect (Modern & Smooth)
    auto hover_cb = [](lv_event_t *e) {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_obj_set_style_transform_zoom(obj, 310, 0); // ~1.2x
        lv_obj_set_style_shadow_width(obj, 25, 0);
        lv_obj_set_style_shadow_spread(obj, 10, 0);
        lv_obj_set_style_shadow_opa(obj, LV_OPA_100, 0);
        lv_obj_set_style_border_color(obj, lv_color_white(), 0);
        lv_obj_set_style_border_width(obj, 6, 0);
    };

    auto nohover_cb = [](lv_event_t *e) {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_obj_set_style_transform_zoom(obj, 256, 0); // 1x
        lv_obj_set_style_shadow_width(obj, 0, 0);
        lv_obj_set_style_border_width(obj, 4, 0);
        // Reset to theme color
        lv_color_t sec = lv_color_hex(bruceConfig.secColor);
        lv_obj_set_style_border_color(obj, sec, 0);
    };

    lv_obj_add_event_cb(btn, hover_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn, nohover_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(btn, click_cb, LV_EVENT_CLICKED, NULL);

    return btn;
}

// Top notification bar
lv_obj_t *create_notification_bar(lv_obj_t *parent) {
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, LV_PCT(100), 40);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x001122), 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_border_width(bar, 0, 0);

    // Time label
    lv_obj_t *time_label = lv_label_create(bar);
    lv_label_set_text(time_label, "--:--");
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);

    // Battery label
    lv_obj_t *battery = lv_label_create(bar);
    lv_label_set_text(battery, "--%");
    lv_obj_align(battery, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_color(battery, lv_color_white(), 0);

    // Connectivity icons
    lv_obj_t *wifi_icon = lv_label_create(bar);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_icon, LV_ALIGN_CENTER, -30, 0);
    lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x00FF00), 0);

    lv_obj_t *ble_icon = lv_label_create(bar);
    lv_label_set_text(ble_icon, LV_SYMBOL_BLUETOOTH);
    lv_obj_align(ble_icon, LV_ALIGN_CENTER, 30, 0);
    lv_obj_set_style_text_color(ble_icon, lv_color_hex(0x0000FF), 0);

    // Notification popup
    lv_obj_t *notify = lv_label_create(bar);
    lv_label_set_text(notify, "System Ready");
    lv_obj_set_style_text_color(notify, lv_color_hex(0xFF0000), 0);
    lv_obj_align(notify, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_opa(notify, LV_OPA_0, 0);

    // Popup animation
    lv_anim_t a_notify;
    lv_anim_init(&a_notify);
    lv_anim_set_var(&a_notify, notify);
    lv_anim_set_values(&a_notify, 0, LV_OPA_100);
    lv_anim_set_time(&a_notify, 500);
    lv_anim_set_delay(&a_notify, 1000);
    lv_anim_set_exec_cb(&a_notify, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_start(&a_notify);

    lv_timer_create(update_bar_timer_cb, 1000, bar);

    return bar;
}

// Callback for top bar updates (Real-time Battery & Time)
static void update_bar_timer_cb(lv_timer_t *timer) {
    lv_obj_t *bar = (lv_obj_t *)timer->user_data;
    lv_obj_t *time_label = lv_obj_get_child(bar, 0);
    lv_obj_t *battery_label = lv_obj_get_child(bar, 1);

    // Update Time
    time_t now;
    time(&now);
    struct tm *timeInfo = localtime(&now);
    if (timeInfo->tm_year > 100) { // Check if NTP synced
        lv_label_set_text_fmt(time_label, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
    } else {
        lv_label_set_text(time_label, "--:--");
    }

    // Update Battery
    int bat = getBattery();
    lv_label_set_text_fmt(battery_label, "%d%%", bat);

    // Change color based on battery level
    if (bat < 20) lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFF0000), 0);
    else if (bat < 50) lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFFFF00), 0);
    else lv_obj_set_style_text_color(battery_label, lv_color_hex(0x00FF00), 0);
}

void setup_cyber_menu(lv_obj_t *menu) {
    lv_obj_set_style_bg_color(menu, lv_color_hex(0x001122), 0);

    // Use Bruce's primary/secondary colors for theme integration
    lv_color_t primary = lv_color_hex(bruceConfig.priColor);
    lv_color_t secondary = lv_color_hex(bruceConfig.secColor);
    lv_color_t accent = lv_color_hex(0x00FFFF); // Cyber cyan

    create_notification_bar(menu);

    // Menu icons placement (3x4 grid)
    int icon_w = 90, icon_h = 90;
    int spacing_x = 15, spacing_y = 25;
    int start_x = 10, start_y = 50;

    const char *labels[] = {"Wi-Fi", "BLE", "IR", "NFC", "Sub-GHz", "NRF24", "GPS", "Attacks", "Core", "Logs", "RFID", "SD"};
    void (*callbacks[])(lv_event_t *) = {wifi_cb, ble_cb, ir_cb, nfc_cb, sub_cb, nrf_cb, gps_cb, attacks_cb, core_cb, logs_cb, rfid_cb, sd_cb};

    // Vector shapes (Simplified SVG-like paths)
    // Updated shapes for ALL 12 icons
    static const lv_point_t wifi_shape[] = {{20,60}, {40,40}, {60,60}, {80,40}, {100,60}};
    static const lv_point_t ble_shape[] = {{50,10}, {30,40}, {50,70}, {70,40}, {50,10}};
    static const lv_point_t ir_shape[] = {{20,20}, {80,20}, {80,80}, {20,80}, {20,20}};
    static const lv_point_t nfc_shape[] = {{30,30}, {70,30}, {70,70}, {30,70}, {30,30}};
    static const lv_point_t sub_shape[] = {{20,50}, {50,20}, {80,50}, {50,80}, {20,50}};
    static const lv_point_t nrf_shape[] = {{10,10}, {90,10}, {50,90}, {10,10}};
    static const lv_point_t gps_shape[] = {{50,10}, {80,50}, {50,90}, {20,50}, {50,10}};
    static const lv_point_t attacks_shape[] = {{20,80}, {50,20}, {80,80}, {20,80}};
    static const lv_point_t core_shape[] = {{50,50}, {80,20}, {80,80}, {20,80}, {20,20}, {50,50}};
    static const lv_point_t logs_shape[] = {{20,20}, {80,20}, {80,40}, {20,40}, {20,60}, {80,60}};
    static const lv_point_t rfid_shape[] = {{50,10}, {90,50}, {50,90}, {10,50}, {50,10}};
    static const lv_point_t sd_shape[] = {{20,10}, {70,10}, {80,20}, {80,90}, {20,90}, {20,10}};

    const lv_point_t* shapes[] = {wifi_shape, ble_shape, ir_shape, nfc_shape, sub_shape, nrf_shape, gps_shape, attacks_shape, core_shape, logs_shape, rfid_shape, sd_shape};
    uint16_t counts[] = {5, 5, 5, 5, 5, 4, 5, 4, 6, 6, 5, 6};

    for(int i = 0; i < 12; i++) {
        int x = start_x + (i % 3) * (icon_w + spacing_x);
        int y = start_y + (i / 3) * (icon_h + spacing_y);
        create_cyber_icon(menu, labels[i], shapes[i], counts[i], primary, secondary, accent, x, y, callbacks[i], i);
    }
}
