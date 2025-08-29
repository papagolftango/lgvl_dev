#include <stdio.h>
#include <lvgl.h>
#include "settings_app.h"
#include "app_manager.h"
#include "ui/ui_Settings.h"

// Static/global variables
static bool screen_active = false;

void settings_app_process(void) {}

void settings_app_init(void) {
    printf("[settings_app] Creating SquareLine screen...\n");
    ui_Settings_screen_init();
    printf("[settings_app] Loading SquareLine screen...\n");
    lv_scr_load(ui_Settings);
    screen_active = true;
}

void settings_app_tick(void) {
    if (!ui_Settings || !ui_settingName) return;
    // Example: update the label if needed
    lv_label_set_text(ui_settingName, "Settings");
}

void settings_app_cleanup(void) {
    // Add cleanup logic if needed
}

void settings_app_destroy(void) {
    // Clean up model/controller/view state if needed
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}
