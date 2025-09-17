#include "esp_log.h"
#include <stdio.h>
#include <lvgl.h>
#include "energy_app.h"
#include "managers/app_manager.h"
#include "managers/time_manager.h"


#include "energy_controller.h"
#include "managers/touch_manager.h"
#include "managers/encoder_manager.h"

static const char *TAG = "energy_app";

static bool energy_on_encoder(encoder_event_t evt) {
    if (evt == ENCODER_EVT_RIGHT) { energy_controller_next_mode(); return true; }
    if (evt == ENCODER_EVT_LEFT)  { energy_controller_prev_mode(); return true; }
    return false;
}
// Static/global variables
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
extern void energy_app_mqtt_init(void);

// Daily callback to clear peaks
static void energy_daily_actions_cb(void) {
    energy_peak_solar = 0.0f;
    energy_peak_used = 0.0f;
    ESP_LOGI(TAG, "Daily reset: peak_solar and peak_used cleared.");
}

// Getter for screen_active (for controller)
bool energy_app_is_screen_active(void) {
    return screen_active;
}

void energy_app_process(void) {
    // Example: update latest_vrms from MQTT or other source
    // latest_vrms = ...;
    // Do NOT touch LVGL objects here!
}

void energy_app_init(void) {
    ESP_LOGI(TAG, "energy_app_init: begin");
    // Ensure DEBUG logs are visible for this tag
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    // Register MQTT event handler and subscribe to topics
    energy_app_mqtt_init();

    // Register daily callback to clear peaks
    time_manager_register_day_callback(energy_daily_actions_cb);
    // Register touch callback for this app (if needed for gestures)
    touch_manager_register_user_cb(energy_app_touch);
    // Register encoder handler for this app
    app_manager_register_encoder_cb(APP_ID_ENERGY, energy_on_encoder);
    screen_active = true;
    ESP_LOGI(TAG, "energy_app_init: end");
    energy_controller_init();
}


void energy_app_cleanup(void) {
    ESP_LOGI(TAG, "energy_app_cleanup: begin");
    screen_active = false;
    ESP_LOGI(TAG, "energy_app_cleanup: end");
}

void energy_app_destroy(void) {
    // Clean up model/controller/view state if needed
    // Example: set pointers to NULL, free memory, etc.
    energy_vrms = 0.0f;
    energy_solar = 0.0f;
    energy_used = 0.0f;
    energy_balance = 0.0f;
    energy_peak_solar = 0.0f;
    energy_peak_used = 0.0f;
    screen_active = false;
}

// Only trigger haptic feedback on touch (now handled in touch_manager)
void energy_app_touch(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    // No-op: all touch haptics handled globally
}
