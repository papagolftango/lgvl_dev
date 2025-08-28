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

// Called only when this app is active (for UI updates)



void energy_app_tick(void) {
    if (!screen_active) {
        ESP_LOGW(TAG, "energy_app_tick called but screen_active is false. Skipping UI update.");
        return;
    }
    ESP_LOGD(TAG, "energy_app_tick: ui_balance=%p ui_Bar1=%p ui_Bar2=%p", ui_balance, ui_Bar1, ui_Bar2);
    ESP_LOGD(TAG, "energy_app_tick: energy_balance=%.2f energy_solar=%.2f energy_used=%.2f", energy_balance, energy_solar, energy_used);
    if (ui_balance) {
        ESP_LOGD(TAG, "Updating ui_balance arc");
        lv_arc_set_value(ui_balance, (int)energy_balance);
    }
    if (ui_Bar1) {
        ESP_LOGD(TAG, "Updating ui_Bar1 bar");
        lv_bar_set_value(ui_Bar1, (int)energy_solar, LV_ANIM_OFF);
    }
    if (ui_Bar2) {
        ESP_LOGD(TAG, "Updating ui_Bar2 bar");
        lv_bar_set_value(ui_Bar2, (int)energy_used, LV_ANIM_OFF);
    }
}

void energy_app_process(void) {
    // Example: update latest_vrms from MQTT or other source
    // latest_vrms = ...;
    // Do NOT touch LVGL objects here!
}


void energy_app_init(void) {
    ESP_LOGI(TAG, "energy_app_init: begin");
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
