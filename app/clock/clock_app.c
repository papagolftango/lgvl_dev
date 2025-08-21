#include "tz_utils.h"

#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "app_manager.h"
#include "time_manager.h"

#include "ui/ui.h"



void clock_app_process(void) {}

void clock_app_init(void) {
    static bool initialized = false;
    if (initialized) {
        printf("[clock_app] UI already initialized, skipping.\n");
        return;
    }
    printf("[clock_app] Initializing SquareLine UI...\n");
    ui_init();
    initialized = true;
}

void clock_app_tick(void) {
    // Update the SquareLine-generated time label (uic_time) if available
    extern lv_obj_t *uic_time;
    if (uic_time) {
        char buf[32];
        time_manager_get_timestr(buf, sizeof(buf));
        lv_label_set_text(uic_time, buf);
    }
}

void clock_app_cleanup(void) {
    // No cleanup needed for SquareLine UI (handled by LVGL framework)
}

void clock_app_touch(void) {
    app_manager_next_app();
}
