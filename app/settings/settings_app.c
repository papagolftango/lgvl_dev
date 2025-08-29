#include <stdio.h>
#include <lvgl.h>
#include "settings_app.h"
#include "app_manager.h"

// Static/global variables
static lv_obj_t *settings_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

void settings_app_process(void) {}

void settings_app_init(void) {
    if (settings_screen) {
        printf("[settings_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[settings_app] Creating screen...\n");
    settings_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(settings_screen, lv_color_black(), 0);
    label = lv_label_create(settings_screen);
    lv_label_set_text(label, "Settings");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[settings_app] Loading screen...\n");
    lv_scr_load(settings_screen);
    screen_active = true;
}

void settings_app_tick(void) {
    if (!settings_screen || !label) return;
    lv_label_set_text(label, "Settings");
}

void settings_app_cleanup(void) {
 
}

void settings_app_destroy(void) {
    // Clean up model/controller/view state if needed
    settings_screen = NULL;
    label = NULL;
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}
