#include <stdio.h>
#include <lvgl.h>
#include "weather_app.h"
#include "app_manager.h"
#include "ui/screens/ui_Weather.h"
#include "weather_controller.h"

// Static/global variables
static bool screen_active = false;

void weather_app_process(void) {}

void weather_app_init(void) {
    printf("[weather_app] Creating SquareLine screen...\n");
    screen_active = true;
    printf("[weather_app] Screen initialized.\n");
    weather_controller_init();
}


void weather_app_cleanup(void) {
    // Add cleanup logic if needed
}

void weather_app_destroy(void) {
    // Clean up model/controller/view state if needed
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}
