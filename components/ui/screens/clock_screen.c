#include "clock_screen.h"
#include <stdio.h>

// Optional: forward declare controller toggle for click
void clock_controller_touch(void);

static lv_obj_t *clock_root = NULL;
static lv_obj_t *clock_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_style_t style_time;
static lv_style_t style_date;
static bool styles_inited = false;
static lv_timer_t *date_hide_timer = NULL;

static void date_hide_timer_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    if (date_label) lv_obj_add_flag(date_label, LV_OBJ_FLAG_HIDDEN);
    if (date_hide_timer) { lv_timer_del(date_hide_timer); date_hide_timer = NULL; }
}

static void clock_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED) {
        clock_controller_touch();
    } else if (code == LV_EVENT_LONG_PRESSED) {
        // Show date for 3 seconds, then auto-hide
        if (date_label) lv_obj_clear_flag(date_label, LV_OBJ_FLAG_HIDDEN);
        if (date_hide_timer) {
            lv_timer_reset(date_hide_timer);
        } else {
            date_hide_timer = lv_timer_create(date_hide_timer_cb, 3000, NULL);
        }
    }
}

lv_obj_t *clock_screen_create(lv_obj_t *parent) {
    if (clock_root) return clock_root;
    clock_root = lv_obj_create(parent);
    lv_obj_set_size(clock_root, 360, 360);
    lv_obj_set_style_bg_color(clock_root, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_add_flag(clock_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(clock_root, clock_event_cb, LV_EVENT_ALL, NULL);

    if (!styles_inited) {
    lv_style_init(&style_time);
#if LV_FONT_MONTSERRAT_48
    lv_style_set_text_font(&style_time, &lv_font_montserrat_48);
#elif LV_FONT_MONTSERRAT_40
    lv_style_set_text_font(&style_time, &lv_font_montserrat_40);
#elif LV_FONT_MONTSERRAT_36
    lv_style_set_text_font(&style_time, &lv_font_montserrat_36);
#endif
    lv_style_set_text_color(&style_time, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_time, LV_OPA_100);

    lv_style_init(&style_date);
#if LV_FONT_MONTSERRAT_20
    lv_style_set_text_font(&style_date, &lv_font_montserrat_20);
#elif LV_FONT_MONTSERRAT_18
    lv_style_set_text_font(&style_date, &lv_font_montserrat_18);
#endif
    lv_style_set_text_color(&style_date, lv_color_hex(0xA0A0A0));
    lv_style_set_text_opa(&style_date, LV_OPA_90);
    styles_inited = true;
    }

    // Example: add a label
    clock_label = lv_label_create(clock_root);
    lv_label_set_text(clock_label, "00:00:00");
    lv_obj_add_style(clock_label, &style_time, LV_PART_MAIN);
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, -12);

    date_label = lv_label_create(clock_root);
    lv_label_set_text(date_label, "Mon 1 Jan");
    lv_obj_add_style(date_label, &style_date, LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 32);
    // Start with date hidden; long-press toggles visibility
    lv_obj_add_flag(date_label, LV_OBJ_FLAG_HIDDEN);
    return clock_root;
}

void clock_screen_destroy(void) {
    if (clock_root) {
        lv_obj_del(clock_root);
        clock_root = NULL;
        clock_label = NULL;
    date_label = NULL;
    if (date_hide_timer) { lv_timer_del(date_hide_timer); date_hide_timer = NULL; }
    }
}

void clock_screen_set_time(const char *time_str) {
    if (clock_label && time_str) {
        lv_label_set_text(clock_label, time_str);
    }
}

void clock_screen_set_date(const char *date_str) {
    if (date_label && date_str) {
        lv_label_set_text(date_label, date_str);
    }
}

lv_obj_t *clock_screen_get_root(void) {
    return clock_root;
}
