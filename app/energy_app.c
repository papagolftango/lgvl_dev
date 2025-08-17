
#include "energy_app.h"
#include <lvgl.h>
#include <stdio.h>


static lv_obj_t *energy_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

// Example background data (replace with real data/event logic)
static float latest_vrms = 0.0f;

// Called always, even when not active
void energy_app_process(void) {
    // Example: update latest_vrms from MQTT or other source
    // latest_vrms = ...;
    // Do NOT touch LVGL objects here!
}


void energy_app_init(void) {
    if (energy_screen) {
        printf("[energy_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[energy_app] Creating screen...\n");
    energy_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(energy_screen, lv_color_black(), 0);
    label = lv_label_create(energy_screen);
    lv_label_set_text(label, "Energy");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[energy_app] Loading screen...\n");
    lv_scr_load(energy_screen);
    screen_active = true;
}


// Called only when this app is active (for UI updates)
void energy_app_tick(void) {
    if (!energy_screen || !label) return;
    // Update UI with latest data
    char buf[32];
    lv_label_set_text(label, "Energy");
}


void energy_app_cleanup(void) {
    if (energy_screen) {
        printf("[energy_app] Cleanup: not deleting screen, just clearing pointers.\n");
        // Do NOT call lv_obj_del on a screen loaded with lv_scr_load!
        energy_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
        printf("[energy_app] Cleanup called but screen is already NULL.\n");
    }
}
