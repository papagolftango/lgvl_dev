#include "tz_utils.h"

#include <stdio.h>
#include <lvgl.h>
#include "clock_app.h"
#include "app_manager.h"
#include "time_manager.h"

// Static/global variables
static lv_obj_t *clock_screen = NULL;
static lv_obj_t *label = NULL;
static lv_obj_t *time_box = NULL;
static lv_obj_t *time_label = NULL;
static bool screen_active = false;

void clock_app_process(void) {}

void clock_app_init(void) {
    if (clock_screen) {
        printf("[clock_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[clock_app] Creating screen...\n");
    clock_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(clock_screen, lv_color_black(), 0);
    label = lv_label_create(clock_screen);
    lv_label_set_text(label, "Clock");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -50);

    // Create a container (box) for the time label
    time_box = lv_obj_create(clock_screen);
    lv_obj_set_size(time_box, lv_obj_get_width(clock_screen), 100); // 100px height, full width
    lv_obj_align_to(time_box, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_opa(time_box, LV_OPA_TRANSP, 0); // transparent background
    lv_obj_set_style_border_width(time_box, 0, 0);
    lv_obj_set_style_border_opa(time_box, LV_OPA_TRANSP, 0);

    // Create time label inside the box
    time_label = lv_label_create(time_box);
    lv_label_set_text(time_label, "--:--:--\n--/--/----");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(time_label, lv_obj_get_width(time_box));
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    // Optionally set the same font as label if custom font is used
    // lv_obj_set_style_text_font(time_label, lv_obj_get_style_text_font(label, 0), 0);

    printf("[clock_app] Loading screen...\n");
    lv_scr_load(clock_screen);
    screen_active = true;
}

void clock_app_tick(void) {
    if (!clock_screen || !label || !time_label) return;
    lv_label_set_text(label, "Clock");
    // Update time label: show only HH:MM:SS and DD-MM-YYYY (British format) on new line, both centered
    char timebuf[32];
    time_manager_get_timestr(timebuf, sizeof(timebuf));
    // timebuf is "YYYY-MM-DD HH:MM:SS"
    char tzbuf[32];
    get_current_tz(tzbuf, sizeof(tzbuf));
    char displaybuf[80];
    if (strlen(timebuf) >= 19 && timebuf[10] == ' ' && timebuf[13] == ':' && timebuf[16] == ':') {
        // Extract date parts
        char dd[3] = {timebuf[8], timebuf[9], '\0'};
        char mm[3] = {timebuf[5], timebuf[6], '\0'};
        char yyyy[5] = {timebuf[0], timebuf[1], timebuf[2], timebuf[3], '\0'};
        // timebuf[11..18] is HH:MM:SS
        snprintf(displaybuf, sizeof(displaybuf), "%.*s\n%s-%s-%s\nTZ: %s", 8, timebuf+11, dd, mm, yyyy, tzbuf);
    } else {
        snprintf(displaybuf, sizeof(displaybuf), "%s\nTZ: %s", timebuf, tzbuf);
    }
    lv_label_set_text(time_label, displaybuf);
}

void clock_app_cleanup(void) {
    if (clock_screen) {
        printf("[clock_app] Cleanup: not deleting screen, just clearing pointers.\n");
        clock_screen = NULL;
        label = NULL;
        time_box = NULL;
        time_label = NULL;
        screen_active = false;
    } else {
        printf("[clock_app] Cleanup called but screen is already NULL.\n");
    }
}

void clock_app_touch(void) {
    app_manager_next_app();
}
