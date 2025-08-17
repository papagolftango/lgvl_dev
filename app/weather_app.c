#include "weather_app.h"
#include <lvgl.h>
#include <stdio.h>

static lv_obj_t *weather_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

void weather_app_process(void) {}

void weather_app_init(void) {
    if (weather_screen) {
        printf("[weather_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[weather_app] Creating screen...\n");
    weather_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(weather_screen, lv_color_black(), 0);
    label = lv_label_create(weather_screen);
    lv_label_set_text(label, "Weather");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[weather_app] Loading screen...\n");
    lv_scr_load(weather_screen);
    screen_active = true;
}

void weather_app_tick(void) {
    if (!weather_screen || !label) return;
    lv_label_set_text(label, "Weather");
}

void weather_app_cleanup(void) {
    if (weather_screen) {
        printf("[weather_app] Cleanup: not deleting screen, just clearing pointers.\n");
        weather_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
        printf("[weather_app] Cleanup called but screen is already NULL.\n");
    }
}
