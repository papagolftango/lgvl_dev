#include "settings_screen.h"
#include <stdio.h>

static lv_obj_t *settings_root = NULL;
static lv_obj_t *settings_label = NULL;

lv_obj_t *settings_screen_create(lv_obj_t *parent) {
    if (settings_root) return settings_root;
    settings_root = lv_obj_create(parent);
    lv_obj_set_size(settings_root, 320, 240);
    lv_obj_set_style_bg_color(settings_root, lv_color_hex(0x333333), LV_PART_MAIN);

    // Example: add a label
    settings_label = lv_label_create(settings_root);
    lv_label_set_text(settings_label, "Settings");
    lv_obj_align(settings_label, LV_ALIGN_CENTER, 0, 0);
    return settings_root;
}

void settings_screen_destroy(void) {
    if (settings_root) {
        lv_obj_del(settings_root);
        settings_root = NULL;
        settings_label = NULL;
    }
}

void settings_screen_set_title(const char *title) {
    if (settings_label && title) {
        lv_label_set_text(settings_label, title);
    }
}

lv_obj_t *settings_screen_get_root(void) {
    return settings_root;
}
