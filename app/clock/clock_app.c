#include "tz_utils.h"

#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "app_manager.h"
#include "time_manager.h"
#include "clock_controller.h"
#include "ui/ui.h"



void clock_app_process(void) {
    clock_controller_process();
}

void clock_app_init(void) {
    printf("[clock_app] Initializing SquareLine UI...\n");
    ui_init();
    clock_controller_init();
}

void clock_app_tick(void) {
    clock_controller_tick();
    // Update the SquareLine-generated time label (ui_Label4) if available
    extern lv_obj_t *ui_Label4;
    if (ui_Label4) {
        char buf[32];
        time_manager_get_timestr(buf, sizeof(buf));
        // Expecting buf = "YYYY-MM-DD HH:MM:SS"; extract HH:MM:SS
        char time_only[16] = "--:--:--";
        if (strlen(buf) >= 19 && buf[10] == ' ') {
            strncpy(time_only, buf + 11, 8);
            time_only[8] = '\0';
        }
        lv_label_set_text(ui_Label4, time_only);
    }
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
