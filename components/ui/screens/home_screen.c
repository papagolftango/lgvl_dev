#include "home_screen.h"
#include <stdio.h>

static lv_obj_t *home_root = NULL;
static lv_obj_t *motd_label = NULL;
static lv_style_t style_motd;
static bool style_inited = false;

lv_obj_t *home_screen_create(lv_obj_t *parent) {
    if (home_root) return home_root;
    home_root = lv_obj_create(parent);
    lv_obj_set_size(home_root, 360, 360);
    lv_obj_set_style_bg_color(home_root, lv_color_hex(0x09032E), LV_PART_MAIN);

    if (!style_inited) {
        lv_style_init(&style_motd);
#if LV_FONT_MONTSERRAT_20
        lv_style_set_text_font(&style_motd, &lv_font_montserrat_20);
#elif LV_FONT_MONTSERRAT_18
        lv_style_set_text_font(&style_motd, &lv_font_montserrat_18);
#endif
        lv_style_set_text_color(&style_motd, lv_color_hex(0xFFFFFF));
        lv_style_set_text_opa(&style_motd, LV_OPA_100);
        // Slower scroll speed for readability
        lv_style_set_anim_speed(&style_motd, 20);
        style_inited = true;
    }

    motd_label = lv_label_create(home_root);
    lv_obj_add_style(motd_label, &style_motd, LV_PART_MAIN);
    lv_obj_set_width(motd_label, 320);
    lv_label_set_long_mode(motd_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(motd_label, "Welcome home – Message of the day");
    lv_obj_align(motd_label, LV_ALIGN_CENTER, 0, 0);
    return home_root;
}

void home_screen_destroy(void) {
    if (home_root) {
        lv_obj_del(home_root);
        home_root = NULL;
        motd_label = NULL;
    }
}

void home_screen_set_motd(const char *text) {
    if (!motd_label || !text) return;
    lv_label_set_text(motd_label, text);
}

lv_obj_t *home_screen_get_root(void) {
    return home_root;
}
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
