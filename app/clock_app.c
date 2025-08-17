#include "clock_app.h"
#include <lvgl.h>
#include <stdio.h>

static lv_obj_t *clock_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

void clock_app_process(void) {}

void clock_app_init(void) {
    if (clock_screen) {
        printf("[clock_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[clock_app] Creating screen...\n");
    clock_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(clock_screen, lv_color_black(), 0);
    label = lv_label_create(clock_screen);
    lv_label_set_text(label, "Clock");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[clock_app] Loading screen...\n");
    lv_scr_load(clock_screen);
    screen_active = true;
}

void clock_app_tick(void) {
    if (!clock_screen || !label) return;
    lv_label_set_text(label, "Clock");
}

void clock_app_cleanup(void) {
    if (clock_screen) {
        printf("[clock_app] Cleanup: not deleting screen, just clearing pointers.\n");
        clock_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
        printf("[clock_app] Cleanup called but screen is already NULL.\n");
    }
}
