
#include "ui_Home.h"
#include "ui_Settings.h"
#include "ui_Weather.h"
#include <lvgl.h>

void ui_Home_screen_load(void) {
    if (!ui_Home) ui_Home_screen_init();
    lv_scr_load(ui_Home);
}



void ui_Settings_screen_load(void) {
    if (!ui_Settings) ui_Settings_screen_init();
    lv_scr_load(ui_Settings);
}

void ui_Weather_screen_load(void) {
    if (!ui_Weather) ui_Weather_screen_init();
    lv_scr_load(ui_Weather);
}

