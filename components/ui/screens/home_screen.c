#include "home_screen.h"
#include <stdio.h>

static lv_obj_t *home_root = NULL;
static lv_obj_t *home_label = NULL;

lv_obj_t *home_screen_create(lv_obj_t *parent) {
    if (home_root) return home_root;
    home_root = lv_obj_create(parent);
    lv_obj_set_size(home_root, 320, 240);
    lv_obj_set_style_bg_color(home_root, lv_color_hex(0x222222), LV_PART_MAIN);

    // Example: add a label
    home_label = lv_label_create(home_root);
    lv_label_set_text(home_label, "Home");
    lv_obj_align(home_label, LV_ALIGN_CENTER, 0, 0);
    return home_root;
}

void home_screen_destroy(void) {
    if (home_root) {
        lv_obj_del(home_root);
        home_root = NULL;
        home_label = NULL;
    }
}

void home_screen_set_title(const char *title) {
    if (home_label && title) {
        lv_label_set_text(home_label, title);
    }
}

lv_obj_t *home_screen_get_root(void) {
    return home_root;
}
