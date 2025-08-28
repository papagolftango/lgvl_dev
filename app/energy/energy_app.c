#include "esp_log.h"
static const char *TAG = "energy_app";

#include <stdio.h>
#include <lvgl.h>
#include "energy_app.h"
#include "app_manager.h"
#include "time_manager.h"
#include "ui/ui_Energy.h"

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




// Forward declaration for controller tick
#include "energy_controller.h"
void energy_app_tick(void) {
    energy_controller_tick();
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
    ESP_LOGI(TAG, "Creating SquareLine screen...");

    ui_Energy_screen_init();
    ESP_LOGI(TAG, "ui_Energy_screen_init done, ui_Energy=%p", ui_Energy);
    lv_scr_load(ui_Energy);
    ESP_LOGI(TAG, "lv_scr_load done");
    screen_active = true;
    ESP_LOGI(TAG, "energy_app_init: end");
}




void energy_app_cleanup(void) {
    ESP_LOGI(TAG, "energy_app_cleanup: begin");
    ui_Energy_screen_destroy();
    ESP_LOGI(TAG, "ui_Energy_screen_destroy called");
    screen_active = false;
    ESP_LOGI(TAG, "energy_app_cleanup: end");
}

void energy_app_touch(void) {
    app_manager_next_app();
}
