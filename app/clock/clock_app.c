#include "tz_utils.h"

#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "app_manager.h"
#include "time_manager.h"

#include "ui/ui.h"



void clock_app_process(void) {}

void clock_app_init(void) {
    printf("[clock_app] Initializing SquareLine UI...\n");
    ui_init();
}

void clock_app_tick(void) {
    // Update the SquareLine-generated time label (uic_time) if available
    extern lv_obj_t *uic_time;
    if (uic_time) {
        char buf[32];
        time_manager_get_timestr(buf, sizeof(buf));
        // Expecting buf = "YYYY-MM-DD HH:MM:SS"; extract HH:MM:SS
        char time_only[16] = "--:--:--";
        if (strlen(buf) >= 19 && buf[10] == ' ') {
            strncpy(time_only, buf + 11, 8);
            time_only[8] = '\0';
        }
        lv_label_set_text(uic_time, time_only);
    }
}

void clock_app_cleanup(void) {
    ui_destroy();
}

void clock_app_touch(void) {
    app_manager_next_app();
}
