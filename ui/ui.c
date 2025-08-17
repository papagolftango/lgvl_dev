#include "ui.h"
#include "lvgl.h"

// Example: create a simple label. Replace this with SquareLine-generated code later.
void ui_init(void) {
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "PGT");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0); // Start red
    lv_obj_set_width(label, 120);
    lv_obj_set_height(label, 80);
    lv_obj_set_style_bg_color(label, lv_color_hex(0x222222), 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_COVER, 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(label);
    // Touch event handler (optional, can be removed for SquareLine)
    extern void label_touch_event_cb(lv_event_t *e);
    lv_obj_add_event_cb(label, label_touch_event_cb, LV_EVENT_PRESSED, NULL);
}
