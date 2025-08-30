#include <stdio.h>
#include <lvgl.h>
#include "settings_app.h"
#include "app_manager.h"
#include "ui/screens/ui_Settings.h"
#include "settings_controller.h"

// Static/global variables
static bool screen_active = false;

void settings_app_process(void) {}

void settings_app_init(void) {
    printf("[settings_app] Creating SquareLine screen...\n");
    screen_active = true;
    printf("[settings_app] Screen initialized.\n");
    settings_controller_init();
}


void settings_app_cleanup(void) {
    // Add cleanup logic if needed
}

void settings_app_destroy(void) {
    // Clean up model/controller/view state if needed
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}
