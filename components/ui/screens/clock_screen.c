#include "clock_screen.h"
#include <stdio.h>

static lv_obj_t *clock_root = NULL;
static lv_obj_t *clock_label = NULL;

lv_obj_t *clock_screen_create(lv_obj_t *parent) {
    if (clock_root) return clock_root;
    clock_root = lv_obj_create(parent);
    lv_obj_set_size(clock_root, 320, 240);
    lv_obj_set_style_bg_color(clock_root, lv_color_hex(0x000000), LV_PART_MAIN);

    // Example: add a label
    clock_label = lv_label_create(clock_root);
    lv_label_set_text(clock_label, "00:00");
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, 0);
    return clock_root;
}

void clock_screen_destroy(void) {
    if (clock_root) {
        lv_obj_del(clock_root);
        clock_root = NULL;
        clock_label = NULL;
    }
}

void clock_screen_set_time(const char *time_str) {
    if (clock_label && time_str) {
        lv_label_set_text(clock_label, time_str);
    }
}

lv_obj_t *clock_screen_get_root(void) {
    return clock_root;
}
