#include <stdbool.h>
#include "tz_utils.h"
#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "managers/app_manager.h"
#include "managers/time_manager.h"
//#include "ui/ui.h" // UI is now initialized by app_manager
#include "clock_controller.h"
#include "managers/encoder_manager.h"

static bool screen_active = false;

void clock_app_process(void) {
    clock_controller_process();
}

void clock_app_init(void) {
    printf("[clock_app] Creating SquareLine screen...\n");
    screen_active = true;
    printf("[clock_app] Screen initialized.\n");
    clock_controller_init();
    // Register encoder actions for this app: LEFT = toggle 12/24, RIGHT = show date briefly
    extern void app_manager_register_encoder_cb(app_id_t app, bool (*cb)(encoder_event_t evt));
    bool clock_on_encoder(encoder_event_t evt) {
        if (evt == ENCODER_EVT_LEFT) { clock_controller_toggle_12_24(); return true; }
        if (evt == ENCODER_EVT_RIGHT) { clock_controller_show_date_briefly(); return true; }
        return false;
    }
    app_manager_register_encoder_cb(APP_ID_CLOCK, clock_on_encoder);
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
