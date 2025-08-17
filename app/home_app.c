
#include "home_app.h"
#include <lvgl.h>
#include <stdio.h>


static lv_obj_t *home_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

// Example background data (replace with real data/event logic)
static int home_counter = 0;

// Called always, even when not active
void home_app_process(void) {
    // Example: update home_counter from events, etc.
    // home_counter++;
    // Do NOT touch LVGL objects here!
}


void home_app_init(void) {
    if (home_screen) {
        printf("[home_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[home_app] Creating screen...\n");
    home_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(home_screen, lv_color_black(), 0);
    label = lv_label_create(home_screen);
    char buf[32];
    snprintf(buf, sizeof(buf), "Home %d", home_counter);
    lv_label_set_text(label, buf);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[home_app] Loading screen...\n");
    lv_scr_load(home_screen);
    screen_active = true;
}


void home_app_tick(void) {
    if (!home_screen || !label) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "Home %d", home_counter);
    lv_label_set_text(label, buf);
}


void home_app_cleanup(void) {
    if (home_screen) {
        printf("[home_app] Cleanup: not deleting screen, just clearing pointers.\n");
        // Do NOT call lv_obj_del on a screen loaded with lv_scr_load!
        home_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
        printf("[home_app] Cleanup called but screen is already NULL.\n");
    }
}
