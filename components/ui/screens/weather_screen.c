#include "weather_screen.h"
#include <stdio.h>

static lv_obj_t *weather_root = NULL;
static lv_obj_t *weather_label = NULL;

lv_obj_t *weather_screen_create(lv_obj_t *parent) {
    if (weather_root) return weather_root;
    weather_root = lv_obj_create(parent);
    lv_obj_set_size(weather_root, 320, 240);
    lv_obj_set_style_bg_color(weather_root, lv_color_hex(0x003366), LV_PART_MAIN);

    // Example: add a label
    weather_label = lv_label_create(weather_root);
    lv_label_set_text(weather_label, "Weather");
    lv_obj_align(weather_label, LV_ALIGN_CENTER, 0, 0);
    return weather_root;
}

void weather_screen_destroy(void) {
    if (weather_root) {
        lv_obj_del(weather_root);
        weather_root = NULL;
        weather_label = NULL;
    }
}

void weather_screen_set_status(const char *status) {
    if (weather_label && status) {
        lv_label_set_text(weather_label, status);
    }
}

lv_obj_t *weather_screen_get_root(void) {
    return weather_root;
}
