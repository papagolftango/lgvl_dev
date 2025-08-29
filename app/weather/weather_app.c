#include <stdio.h>
#include <lvgl.h>
#include "weather_app.h"
#include "app_manager.h"
#include "ui/ui_Weather.h"

// Static/global variables
static bool screen_active = false;

void weather_app_process(void) {}

void weather_app_init(void) {
    printf("[weather_app] Creating SquareLine screen...\n");
    ui_Weather_screen_init();
    printf("[weather_app] Loading SquareLine screen...\n");
    lv_scr_load(ui_Weather);
    screen_active = true;
}

void weather_app_tick(void) {
    if (!ui_Weather || !ui_weatherName) return;
    // Example: update the label if needed
    lv_label_set_text(ui_weatherName, "Weather");
}

void weather_app_cleanup(void) {
    // Add cleanup logic if needed
}

void weather_app_destroy(void) {
    // Clean up model/controller/view state if needed
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}
