// home_app.c - Home app logic
#include <stdio.h>
#include "lvgl.h"
#include "touch_manager.h"
#include "home_app.h"
#include "app_manager.h"
#include "time_manager.h"

// Static/global variables
static lv_obj_t *home_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;
static int home_counter = 0; // Example background data (replace with real data/event logic)

// Bin schedule for 52 weeks: top nibble = day of week (2=Tuesday), bottom nibble = bin bitmap
// Odd weeks: general bin (0x2), Even weeks: recycle+garden (0x5)
static uint8_t bin_schedule[52] = {
    /* Weeks 1-52 */
    0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25,
    0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25,
    0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25,
    0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25, 0x22, 0x25
};

// Icon display flags (set from message bits)
static bool show_recycle_bin_icon = false;
static bool show_general_waste_bin_icon = false;
static bool show_garden_bin_icon = false;
// Tip lorry icon: show when bin_state == BIN_STATE_EMPTYING

// Forward declarations
static void process_bin_touch(void);
static void home_app_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void daily_actions_cb(void);

// Touch callback for home app
static void home_app_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if (data->state == LV_INDEV_STATE_PRESSED) {
        process_bin_touch();
    }
}

// Called always, even when not active
void home_app_process(void) {
    // Example: update home_counter from events, etc.
    // home_counter++;
    // Do NOT touch LVGL objects here!
}

void home_app_init(void) {
    if (home_screen) {
        printf("[home_app] Screen already exists, skipping init.\n");
        return;
    }
    printf("[home_app] Creating screen...\n");
    home_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(home_screen, lv_color_black(), 0);
    label = lv_label_create(home_screen);
    char buf[32];
    snprintf(buf, sizeof(buf), "Home %d", home_counter);
    lv_label_set_text(label, buf);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    printf("[home_app] Loading screen...\n");
    lv_scr_load(home_screen);
    screen_active = true;
    // Register home app's touch callback
    touch_manager_register_user_cb(home_app_touch_cb);
    // Register daily callback
    time_manager_register_day_callback(daily_actions_cb);
}

void home_app_tick(void) {
    if (!home_screen || !label) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "Home %d", home_counter);
    lv_label_set_text(label, buf);
}

void home_app_cleanup(void) {
    if (home_screen) {
        printf("[home_app] Cleanup: not deleting screen, just clearing pointers.\n");
        // Do NOT call lv_obj_del on a screen loaded with lv_scr_load!
        home_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
        printf("[home_app] Cleanup called but screen is already NULL.\n");
    }
    // Unregister home app's touch callback
    touch_manager_unregister_user_cb();
}

// Stub implementations to resolve linker errors
static void process_bin_touch(void) {}
static void daily_actions_cb(void) {}

// Internal function to process bin touch (for testing)
