#include <lvgl.h>
#include "esp_log.h"

static const char *ERROR_SCREEN_TAG = "error_screen";

static lv_obj_t *error_screen_root = NULL;

lv_obj_t *error_screen_create(lv_obj_t *parent) {
    if (error_screen_root) return error_screen_root;
    error_screen_root = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(error_screen_root, lv_color_hex(0xFF0000), 0);
    lv_obj_t *label = lv_label_create(error_screen_root);
    lv_label_set_text(label, "Screen Load Error");
    lv_obj_center(label);
    ESP_LOGE(ERROR_SCREEN_TAG, "Displayed fallback error screen");
    return error_screen_root;
}

lv_obj_t *error_screen_get_root(void) {
    return error_screen_root;
}
