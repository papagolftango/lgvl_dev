#include <stdbool.h>
#include "tz_utils.h"
#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "app_manager.h"
#include "time_manager.h"
//#include "ui/ui.h" // UI is now initialized by app_manager
#include "ui/screens/ui_Clock.h"
#include "clock_controller.h"
// #include "ui/ui.h" // UI is now initialized by app_manager


static bool screen_active = false;

void clock_app_process(void) {
    clock_controller_process();
}

void clock_app_init(void) {
    printf("[clock_app] Creating SquareLine screen...\n");
    screen_active = true;
    printf("[clock_app] Screen initialized.\n");
    clock_controller_init();
}

void clock_app_cleanup(void) {
    clock_controller_cleanup();
    // ui_destroy();
}

void clock_app_destroy(void) {
    clock_controller_destroy();
    // Clean up model/controller/view state if needed
    // If you dynamically allocated any LVGL objects, delete them here
}

void clock_app_touch(void) {
    clock_controller_touch();
}
