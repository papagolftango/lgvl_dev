#include "esp_log.h"
static const char *TAG = "energy_app";


#include <stdio.h>
#include <lvgl.h>
#include "energy_app.h"
#include "app_manager.h"
#include "time_manager.h"

// Static/global variables
static lv_obj_t *energy_screen = NULL;
static lv_obj_t *label = NULL;
static bool screen_active = false;

// Energy data variables (shared with MQTT handler)
float energy_vrms = 0.0f;
float energy_solar = 0.0f;
float energy_used = 0.0f;
float energy_balance = 0.0f;
float energy_peak_solar = 0.0f;
float energy_peak_used = 0.0f;

// Forward declarations
static void energy_daily_actions_cb(void);

// Daily callback to clear peaks
static void energy_daily_actions_cb(void) {
    energy_peak_solar = 0.0f;
    energy_peak_used = 0.0f;
    ESP_LOGI(TAG, "Daily reset: peak_solar and peak_used cleared.");
}

#include <stdio.h>
#include <lvgl.h>
#include "energy_app.h"
#include "app_manager.h"


// Called always, even when not active
void energy_app_process(void) {
    // Example: update latest_vrms from MQTT or other source
    // latest_vrms = ...;
    // Do NOT touch LVGL objects here!
}

void energy_app_init(void) {
    if (energy_screen) {
    ESP_LOGI(TAG, "Screen already exists, skipping init.");
        return;
    }
    // Register MQTT event handler and subscribe to topics
    extern void energy_app_mqtt_init(void);
    energy_app_mqtt_init();
    // Register daily callback to clear peaks
    time_manager_register_day_callback(energy_daily_actions_cb);
    ESP_LOGI(TAG, "Creating screen...");
    energy_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(energy_screen, lv_color_black(), 0);
    label = lv_label_create(energy_screen);
    lv_label_set_text(label, "Energy");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    ESP_LOGI(TAG, "Loading screen...");
    lv_scr_load(energy_screen);
    screen_active = true;
}


// Called only when this app is active (for UI updates)

void energy_app_tick(void) {
    if (!energy_screen || !label) return;
    // Update UI with latest data
    char buf[32];
    lv_label_set_text(label, "Energy");
}



void energy_app_cleanup(void) {
    if (energy_screen) {
    ESP_LOGI(TAG, "Cleanup: not deleting screen, just clearing pointers.");
        // Do NOT call lv_obj_del on a screen loaded with lv_scr_load!
        energy_screen = NULL;
        label = NULL;
        screen_active = false;
    } else {
    ESP_LOGI(TAG, "Cleanup called but screen is already NULL.");
    }
}

void energy_app_touch(void) {
    app_manager_next_app();
}
