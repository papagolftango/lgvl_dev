// home_app.c - Home app logic
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "touch_manager.h"
#include "home_app.h"
#include "managers/app_manager.h"
#include "managers/time_manager.h"
#include "ui/screens/home_screen.h"
#include "managers/mqtt_manager.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "home_controller.h"

// Static/global variables
static lv_obj_t *home_screen = NULL;
static lv_obj_t *label = NULL;
static lv_obj_t *dbg_week_label = NULL;     // shows week number
static lv_obj_t *dbg_sched_label = NULL;    // shows schedule hex
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
static void update_bin_debug_labels(void);
static void home_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

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
    // Use SquareLine Studio generated screen
    printf("[home_app] Creating SquareLine screen...\n");
    // Assign label pointer to the LVGL label object
    // label = ui_homeName; // Removed: ui_homeName is not defined
    screen_active = true;
    // Register home app's touch callback
    touch_manager_register_user_cb(home_app_touch_cb);
    // Register daily callback
    time_manager_register_day_callback(daily_actions_cb);
    home_controller_init();

    // Subscribe to MOTD topic via MQTT manager
    mqtt_manager_register_app_event_handler(home_mqtt_event_handler, NULL);

    // Create debug labels near bottom of display
    lv_obj_t *root = home_screen_get_root();
    if (!root) root = lv_scr_act();
    dbg_week_label = lv_label_create(root);
    lv_obj_align(dbg_week_label, LV_ALIGN_BOTTOM_MID, 0, -34);
    dbg_sched_label = lv_label_create(root);
    lv_obj_align(dbg_sched_label, LV_ALIGN_BOTTOM_MID, 0, -16);
    update_bin_debug_labels();
}


void home_app_cleanup(void) {
    printf("[home_app] Cleanup: destroying SquareLine screen.\n");
  //  ui_Screen1_screen_destroy();
    screen_active = false;
    // Unregister home app's touch callback
    touch_manager_unregister_user_cb();
}

void home_app_destroy(void) {
    // Clean up model/controller/view state if needed
    home_screen = NULL;
    label = NULL;
    screen_active = false;
    // If you dynamically allocated any LVGL objects, delete them here
}

// Stub implementations to resolve linker errors
static void process_bin_touch(void) {}
static void daily_actions_cb(void) {
    // Recompute current ISO week and refresh debug labels once per day
    update_bin_debug_labels();
}
static int iso_week_number(const struct tm *tm) {
    // ISO week: weeks start on Monday, week 1 has Jan 4th
    // Compute using C library where available via strftime %V, else fallback
    char buf[4];
    if (strftime(buf, sizeof(buf), "%V", tm) > 0) {
        return atoi(buf);
    }
    // Fallback (simple): approximate using day-of-year/7 + 1
    return tm->tm_yday / 7 + 1;
}

static void update_bin_debug_labels(void) {
    struct tm now;
    time_manager_get_localtime(&now);
    int week = iso_week_number(&now);
    if (week < 1) week = 1; if (week > 52) week = 52;
    // Retrieve schedule entry from local table
    uint8_t entry = bin_schedule[(week - 1) % 52];

    if (dbg_week_label) {
        static const char *W[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        const char *dow = (now.tm_wday >= 0 && now.tm_wday <= 6) ? W[now.tm_wday] : "";
        char txt[40];
        snprintf(txt, sizeof(txt), "Week: %d %s", week, dow);
        lv_label_set_text(dbg_week_label, txt);
    }
    if (dbg_sched_label) {
        char txt[32];
        snprintf(txt, sizeof(txt), "Sched: 0x%02X", entry);
        lv_label_set_text(dbg_sched_label, txt);
    }
}

static void home_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    (void)handler_args; (void)base;
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            // Subscribe to your MOTD topic
            esp_mqtt_client_subscribe(event->client, "home/motd", 0);
            break;
        case MQTT_EVENT_DATA:
            if (event->topic_len && event->data_len) {
                char topic[128] = {0};
                char payload[256] = {0};
                int tlen = event->topic_len < sizeof(topic)-1 ? event->topic_len : sizeof(topic)-1;
                int dlen = event->data_len < sizeof(payload)-1 ? event->data_len : sizeof(payload)-1;
                strncpy(topic, event->topic, tlen); topic[tlen] = '\0';
                strncpy(payload, event->data, dlen); payload[dlen] = '\0';
                if (strcmp(topic, "home/motd") == 0) {
                    home_controller_set_motd(payload);
                }
            }
            break;
        default:
            break;
    }
}

// Internal function to process bin touch (for testing)
