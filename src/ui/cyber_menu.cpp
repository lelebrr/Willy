#include "cyber_menu.h"
#include "../core/config.h"

// Forward declaration for battery function
extern int getBattery();
#include "../core/display.h"
#include "../core/main_menu.h"
#include "../core/utils.h"
#include <Arduino.h>
#include <time.h>

// LVGL animation wrapper functions — lv_obj_set_style_* takes 3 args
// (obj, value, selector) but lv_anim_exec_xcb_t passes only 2 (obj, value).
static void anim_set_opa(void *obj, int32_t v) {
  lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}
static void anim_set_translate_y(void *obj, int32_t v) {
  lv_obj_set_style_translate_y((lv_obj_t *)obj, (lv_coord_t)v, 0);
}
static void anim_set_shadow_width(void *obj, int32_t v) {
  lv_obj_set_style_shadow_width((lv_obj_t *)obj, (lv_coord_t)v, 0);
}

// Forward declarations of Willy callbacks - implemented below
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

volatile const char *pending_cyber_menu_name = NULL;

// Struct to track top bar labels for real-time updates safely
typedef struct {
  lv_obj_t *time_label;
  lv_obj_t *battery_label;
} CyberMenuData;

static CyberMenuData *menu_data = NULL;
static lv_obj_t *tileview_obj = NULL;

// Implementation of icon callbacks routing to backend IDs
// Indexes match main_menu.cpp options.push_back order for standard menus
// 0: WiFi, 1: BLE, 2: Ethernet (if not lite), 3: RF, 4: RFID, 5: IR
// 6: FM (if defined), 7: Files, 8: GPS, 9: NRF24, 10: Scripts, 11: LoRa
// Note: Depending on hardware, index mapping shifts. Let's use simple IDs and
// we will refine the mapping later or rely on the static index. Based on
// current MainMenu in main_menu.cpp: 0=WiFi, 1=BLE, 2=Eth, 3=RF, 4=RFID, 5=IR,
// 6=FM, 7=Files, 8=GPS, 9=NRF24, 10=Scripts, 11=LoRa. We will assign them
// properly.
static void wifi_cb(lv_event_t *e) { pending_cyber_menu_name = "WiFi"; }
static void ble_cb(lv_event_t *e) { pending_cyber_menu_name = "Bluetooth"; }
static void sub_cb(lv_event_t *e) { pending_cyber_menu_name = "Sub-GHz"; } // RF
static void rfid_cb(lv_event_t *e) { pending_cyber_menu_name = "RFID"; } // RFID
static void ir_cb(lv_event_t *e) { pending_cyber_menu_name = "Infrared"; }
static void sd_cb(lv_event_t *e) {
  pending_cyber_menu_name = "Arquivos";
} // Files
static void gps_cb(lv_event_t *e) { pending_cyber_menu_name = "GPS"; }
static void nrf_cb(lv_event_t *e) { pending_cyber_menu_name = "NRF24"; }

// Using remaining indexes logically.
static void nfc_cb(lv_event_t *e) {
  pending_cyber_menu_name = "RFID";
} // Redirect NFC to RFID
static void attacks_cb(lv_event_t *e) {
  pending_cyber_menu_name = "Outros";
} // Redirect to Others
static void core_cb(lv_event_t *e) {
  pending_cyber_menu_name = "Configuracoes";
} // Config
static void logs_cb(lv_event_t *e) {
  pending_cyber_menu_name = "Arquivos";
} // Temp

// Arrow navigation callbacks
static void arrow_event_cb(lv_event_t *e) {
  if (!tileview_obj)
    return;
  const char *symbol = (const char *)lv_event_get_user_data(e);
  lv_obj_t *act_tile = lv_tileview_get_tile_act(tileview_obj);
  if (!act_tile)
    return;

  int curr_tile = lv_obj_get_index(act_tile);

  if (strcmp(symbol, LV_SYMBOL_LEFT) == 0 && curr_tile > 0) {
    lv_obj_set_tile_id(tileview_obj, curr_tile - 1, 0, LV_ANIM_ON);
  } else if (strcmp(symbol, LV_SYMBOL_RIGHT) == 0 && curr_tile < 11) {
    lv_obj_set_tile_id(tileview_obj, curr_tile + 1, 0, LV_ANIM_ON);
  }
}

// Cleanup callback to prevent dangling timers/memory
static void cyber_menu_delete_cb(lv_event_t *e) {
  if (menu_data) {
    // LVGL timers are usually cleaned up if 'bar' is deleted,
    // but let's ensure we release the allocated struct.
    lv_mem_free(menu_data);
    menu_data = NULL;
  }
  tileview_obj = NULL;
}

// Timer callback for top bar updates
static void update_bar_timer_cb(lv_timer_t *timer);

// Generic function to create modern cyber icons with animations
lv_obj_t *create_cyber_icon(lv_obj_t *parent, const char *icon_text,
                            const lv_point_t *shape_points,
                            uint16_t point_count, lv_color_t primary,
                            lv_color_t secondary, lv_color_t accent, int x,
                            int y, void (*click_cb)(lv_event_t *e), int index) {
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, 80, 80); // Reduced from 90 to 80 to prevent clipping
  lv_obj_set_pos(btn, x, y);
  lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
  lv_obj_set_style_radius(btn, 25, 0);
  lv_obj_set_style_border_color(btn, accent, 0);
  lv_obj_set_style_border_width(btn, 3, 0);
  lv_obj_set_style_shadow_color(btn, accent, 0);
  lv_obj_set_style_shadow_width(btn, 18, 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_70, 0);

  // Use a single static style shared by all icons to avoid memory leak.
  // Previously each icon allocated a dynamic lv_style_t that was never freed.
  static lv_style_t icon_gradient_style;
  static bool style_initialized = false;
  if (!style_initialized) {
    lv_style_init(&icon_gradient_style);
    style_initialized = true;
  }
  lv_style_set_bg_grad_dir(&icon_gradient_style, LV_GRAD_DIR_VER);
  lv_style_set_bg_color(&icon_gradient_style, primary);
  lv_style_set_bg_grad_color(&icon_gradient_style, secondary);
  lv_style_set_bg_opa(&icon_gradient_style, LV_OPA_90);
  lv_obj_add_style(btn, &icon_gradient_style, 0);

  // Text label
  lv_obj_t *text = lv_label_create(btn);
  lv_label_set_text(text, icon_text);
  lv_obj_align(text, LV_ALIGN_BOTTOM_MID, 0, 8); // Adjusted alignment
  lv_obj_set_style_text_color(text, lv_color_hex(0xE6F7FF), 0);
  lv_obj_set_style_text_font(text, &lv_font_montserrat_14, 0);

  // Animation 1: Cascade Fade-in
  lv_anim_t a_fade;
  lv_anim_init(&a_fade);
  lv_anim_set_var(&a_fade, btn);
  lv_anim_set_values(&a_fade, 0, LV_OPA_100);
  lv_anim_set_time(&a_fade, 400);
  lv_anim_set_delay(&a_fade, 100 * index);
  lv_anim_set_exec_cb(&a_fade, anim_set_opa);
  lv_anim_set_path_cb(&a_fade, lv_anim_path_ease_in_out);
  lv_anim_start(&a_fade);

  // Animation 2: Slide-up (Relative Translate)
  lv_anim_t a_slide;
  lv_anim_init(&a_slide);
  lv_anim_set_var(&a_slide, btn);
  lv_anim_set_values(&a_slide, 40, 0); // Slide up 40px to its aligned center
  lv_anim_set_time(&a_slide, 400);
  lv_anim_set_delay(&a_slide, 100 * index);
  lv_anim_set_exec_cb(&a_slide, anim_set_translate_y);
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
  lv_anim_set_exec_cb(&a_pulse, anim_set_shadow_width);
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
    lv_color_t sec = lv_color_hex(willyConfig.secColor);
    lv_obj_set_style_border_color(obj, sec, 0);
  };

  lv_obj_add_event_cb(btn, hover_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(btn, nohover_cb, LV_EVENT_RELEASED, NULL);
  lv_obj_add_event_cb(btn, click_cb, LV_EVENT_CLICKED, NULL);

  // Initial centering within its future parent (tile)
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

  return btn;
}

// Top notification bar
lv_obj_t *create_notification_bar(lv_obj_t *parent) {
  lv_obj_t *bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LV_PCT(100), 40);
  lv_obj_set_pos(bar, 0, 0);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x05070D), 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_set_style_border_width(bar, 0, 0);

  // Time label
  lv_obj_t *time_label = lv_label_create(bar);
  lv_label_set_text(time_label, "--:--");
  lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);
  lv_obj_set_style_text_color(time_label, lv_color_hex(0xE6F7FF), 0);

  // Battery label
  lv_obj_t *battery = lv_label_create(bar);
  lv_label_set_text(battery, "--%");
  lv_obj_align(battery, LV_ALIGN_RIGHT_MID, -10, 0);
  lv_obj_set_style_text_color(battery, lv_color_hex(0xE6F7FF), 0);

  // Connectivity icons
  lv_obj_t *wifi_icon = lv_label_create(bar);
  lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
  lv_obj_align(wifi_icon, LV_ALIGN_CENTER, -30, 0);
  lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x00E6FF), 0);

  lv_obj_t *ble_icon = lv_label_create(bar);
  lv_label_set_text(ble_icon, LV_SYMBOL_BLUETOOTH);
  lv_obj_align(ble_icon, LV_ALIGN_CENTER, 30, 0);
  lv_obj_set_style_text_color(ble_icon, lv_color_hex(0x7A5CFF), 0);

  // Notification popup
  lv_obj_t *notify = lv_label_create(bar);
  lv_label_set_text(notify, "System Ready");
  lv_obj_set_style_text_color(notify, lv_color_hex(0x00E6FF), 0);
  lv_obj_align(notify, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_opa(notify, LV_OPA_0, 0);

  // Popup animation
  lv_anim_t a_notify;
  lv_anim_init(&a_notify);
  lv_anim_set_var(&a_notify, notify);
  lv_anim_set_values(&a_notify, 0, LV_OPA_100);
  lv_anim_set_time(&a_notify, 500);
  lv_anim_set_delay(&a_notify, 1000);
  lv_anim_set_exec_cb(&a_notify, anim_set_opa);
  lv_anim_start(&a_notify);

  // Allocate safe memory for cross-thread timer usage if not already created
  if (!menu_data) {
    menu_data = (CyberMenuData *)lv_mem_alloc(sizeof(CyberMenuData));
  }
  menu_data->time_label = time_label;
  menu_data->battery_label = battery;

  lv_timer_t *timer = lv_timer_create(update_bar_timer_cb, 1000, menu_data);
  lv_obj_add_event_cb(bar, cyber_menu_delete_cb, LV_EVENT_DELETE, timer);

  return bar;
}

// Callback for top bar updates (Real-time Battery & Time)
static void update_bar_timer_cb(lv_timer_t *timer) {
  if (!timer->user_data)
    return;
  CyberMenuData *data = (CyberMenuData *)timer->user_data;

  // Safety check: ensure labels haven't been deleted
  if (!lv_obj_is_valid(data->time_label))
    return;

  // Update Time
  time_t now;
  time(&now);
  struct tm *timeInfo = localtime(&now);
  if (timeInfo->tm_year > 100) { // Check if NTP synced
    lv_label_set_text_fmt(data->time_label, "%02d:%02d", timeInfo->tm_hour,
                          timeInfo->tm_min);
  } else {
    lv_label_set_text(data->time_label, "--:--");
  }

  // Update Battery
  if (!lv_obj_is_valid(data->battery_label))
    return;

  int battery_level = getBattery();
  lv_label_set_text_fmt(data->battery_label, "%d%%", battery_level);
}

void setup_cyber_menu(lv_obj_t *menu) {
  lv_obj_set_style_bg_color(menu, lv_color_hex(0x05070D), 0);
  lv_obj_set_size(menu, LV_PCT(100), LV_PCT(100));

  // Top bar fixed at 40px
  create_notification_bar(menu);

  // Body container for the rest of the screen (Exactly below the bar)
  lv_obj_t *body = lv_obj_create(menu);
  lv_obj_set_size(body, LV_PCT(100), LV_VER_RES - 40);
  lv_obj_set_pos(body, 0, 40);
  lv_obj_set_style_bg_opa(body, 0, 0);
  lv_obj_set_style_border_width(body, 0, 0);
  lv_obj_set_style_pad_all(body, 0, 0);
  lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_OFF);

  tileview_obj = lv_tileview_create(body);
  lv_obj_t *tv = tileview_obj;
  lv_obj_set_size(tv, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_opa(tv, 0, 0);
  lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);

  const char *labels[] = {"Wi-Fi",   "BLE",   "IR",   "NFC",
                          "Sub-GHz", "NRF24", "GPS",  "Attacks",
                          "Core",    "Logs",  "RFID", "SD"};
  void (*callbacks[])(lv_event_t *) = {wifi_cb, ble_cb,  ir_cb,   nfc_cb,
                                       sub_cb,  nrf_cb,  gps_cb,  attacks_cb,
                                       core_cb, logs_cb, rfid_cb, sd_cb};

  // Shapes (scaled for display)
  static const lv_point_t wifi_shape[] = {
      {20, 120}, {60, 80}, {100, 120}, {140, 80}, {180, 120}};
  static const lv_point_t ble_shape[] = {
      {100, 20}, {60, 80}, {100, 140}, {140, 80}, {100, 20}};
  static const lv_point_t ir_shape[] = {
      {40, 40}, {160, 40}, {160, 160}, {40, 160}, {40, 40}};
  static const lv_point_t nfc_shape[] = {
      {60, 60}, {140, 60}, {140, 140}, {60, 140}, {60, 60}};
  static const lv_point_t sub_shape[] = {
      {40, 100}, {100, 40}, {160, 100}, {100, 160}, {40, 100}};
  static const lv_point_t nrf_shape[] = {
      {20, 20}, {180, 20}, {100, 180}, {20, 20}};
  static const lv_point_t gps_shape[] = {
      {100, 20}, {160, 100}, {100, 180}, {40, 100}, {100, 20}};
  static const lv_point_t attacks_shape[] = {
      {40, 160}, {100, 40}, {160, 160}, {40, 160}};
  static const lv_point_t core_shape[] = {{100, 100}, {160, 40}, {160, 160},
                                          {40, 160},  {40, 40},  {100, 100}};
  static const lv_point_t logs_shape[] = {{40, 40}, {160, 40}, {160, 80},
                                          {40, 80}, {40, 120}, {160, 120}};
  static const lv_point_t rfid_shape[] = {
      {100, 20}, {180, 100}, {100, 180}, {20, 100}, {100, 20}};
  static const lv_point_t sd_shape[] = {{40, 20},   {140, 20}, {160, 40},
                                        {160, 180}, {40, 180}, {40, 20}};

  const lv_point_t *shapes[] = {
      wifi_shape, ble_shape,     ir_shape,   nfc_shape,  sub_shape,  nrf_shape,
      gps_shape,  attacks_shape, core_shape, logs_shape, rfid_shape, sd_shape};
  uint16_t counts[] = {5, 5, 5, 5, 5, 4, 5, 4, 6, 6, 5, 6};

  int icon_size = 110;

  // Create arrows attached to body so they don't move with tiles
  lv_obj_t *left_arrow = lv_label_create(body);
  lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_color(left_arrow, lv_color_hex(0xE6F7FF), 0);
  lv_obj_set_style_text_font(left_arrow, &lv_font_montserrat_28, 0);
  lv_obj_align(left_arrow, LV_ALIGN_LEFT_MID, 5, -20);
  lv_obj_set_size(left_arrow, 50, 80);
  lv_obj_set_style_text_align(left_arrow, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_add_flag(left_arrow, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(left_arrow, arrow_event_cb, LV_EVENT_CLICKED,
                      (void *)LV_SYMBOL_LEFT);

  lv_obj_t *right_arrow = lv_label_create(body);
  lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_color(right_arrow, lv_color_hex(0xE6F7FF), 0);
  lv_obj_set_style_text_font(right_arrow, &lv_font_montserrat_28, 0);
  lv_obj_align(right_arrow, LV_ALIGN_RIGHT_MID, -5, -20);
  lv_obj_set_size(right_arrow, 50, 80);
  lv_obj_set_style_text_align(right_arrow, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_add_flag(right_arrow, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(right_arrow, arrow_event_cb, LV_EVENT_CLICKED,
                      (void *)LV_SYMBOL_RIGHT);

  for (int i = 0; i < 12; i++) {
    lv_obj_t *tile = lv_tileview_add_tile(tv, i, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_set_style_bg_opa(tile, 0, 0);

    // Primary/secondary theme colors
    lv_color_t primary = lv_color_hex(willyConfig.priColor);
    lv_color_t secondary = lv_color_hex(willyConfig.secColor);
    lv_color_t accent = lv_color_hex(0x00E6FF);

    lv_obj_t *btn =
        create_cyber_icon(tile, labels[i], shapes[i], counts[i], primary,
                          secondary, accent, 0, 0, callbacks[i], 0);
    lv_obj_set_size(btn, icon_size, icon_size);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0,
                 -10); // Reduced offset from -20 to -10

    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
  }

  lv_obj_move_foreground(left_arrow);
  lv_obj_move_foreground(right_arrow);
}
