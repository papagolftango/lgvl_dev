

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "../drivers/user_config.h"
#include "../drivers/display_driver.h"
#include "lcd_bl_pwm_bsp.h"
#include "../managers/lvgl_manager.h"
#include "../managers/touch_manager.h"
#include "../managers/app_manager.h"
#include "energy_app.h"
#include "home_app.h"
#include "weather_app.h"
#include "clock_app.h"
#include "settings_app.h"
#include "wifi_manager.h"
#include "../managers/mqtt_manager.h"
#include "../managers/wifi_manager.h"
#include "../managers/mqtt_manager.h"
#include "../managers/provisioning_server.h"
#include "../drivers/bidi_switch_knob.h"



// Forward declaration for app switching
static void switch_to_next_app(void);

static void knob_left_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned LEFT");
    // Optionally implement previous app logic here
}

static void knob_right_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned RIGHT");
    switch_to_next_app();
}


// Encoder callbacks

// For development: Erase NVS and restart to force provisioning
void erase_nvs_and_restart() {
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE("main", "Failed to erase NVS: %s", esp_err_to_name(err));
    }
}


// --- App definitions (move to a separate file if desired) ---
static const app_t energy_app = { "Energy", energy_app_init, energy_app_tick, energy_app_cleanup, energy_app_process };
static const app_t home_app = { "Home", home_app_init, home_app_tick, home_app_cleanup, home_app_process };
static const app_t weather_app = { "Weather", weather_app_init, weather_app_tick, weather_app_cleanup, weather_app_process };
static const app_t clock_app = { "Clock", clock_app_init, clock_app_tick, clock_app_cleanup, clock_app_process };
static const app_t settings_app = { "Settings", settings_app_init, settings_app_tick, settings_app_cleanup, settings_app_process };

static const app_t *apps[] = { &energy_app, &home_app, &weather_app, &clock_app, &settings_app };
static const int num_apps = sizeof(apps) / sizeof(apps[0]);
static int current_app = 0; // File-scope for shared access
static void switch_to_next_app(void) {
    current_app = (current_app + 1) % num_apps;
    app_manager_set_active(apps[current_app]->name);
}



static const char *TAG = "example";

// --- Provisioning and WiFi/MQTT setup implementation ---

// For development: Always use hardcoded credentials, skip provisioning
static void handle_provisioning(void) {
    ESP_LOGI(TAG, "[DEV] Skipping provisioning, using hardcoded WiFi credentials");
    wifi_manager_connect();
}

void app_main(void)

{

    // --- Encoder initialization ---
    knob_config_t cfg;
    cfg.gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN;
    cfg.gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN;
    knob_handle_t knob = iot_knob_create(&cfg);
    iot_knob_register_cb(knob, KNOB_LEFT, knob_left_cb, NULL);
    iot_knob_register_cb(knob, KNOB_RIGHT, knob_right_cb, NULL);



    // Set hardcoded WiFi credentials (for development only)
    
     wifi_manager_set_credentials("","");

    // Set hardcoded MQTT credentials (for development only)
    mqtt_manager_set_credentials("192.168.68.66", "emonpi", "emonpimqtt2016");


    // Initialize TCP/IP stack and event loop (required before WiFi/HTTP server)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create default WiFi AP netif (enables DHCP server for AP mode)
    esp_netif_create_default_wifi_ap();

    // --- Time manager initialization ---
    #include "../managers/time_manager.h"
    time_manager_init();

    // Initialize NVS (required for WiFi, provisioning, etc)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ...existing code...


    // --- WiFi/MQTT provisioning and connection ---
    handle_provisioning();

    // For development only: Uncomment to force hardcoded credentials
    // local_init_credentials();
    // wifi_manager_connect();

    // --- Usual app initialization ---
    ESP_LOGI(TAG, "Calling display_init...");
    esp_lcd_panel_handle_t panel_handle = display_init();
    if (!panel_handle) {
        ESP_LOGE(TAG, "display_init failed! Panel handle is NULL. Aborting app_main.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    ESP_LOGI(TAG, "display_init complete, panel_handle=%p", panel_handle);

    ESP_LOGI(TAG, "Set backlight to 50%%");
    setUpduty(LCD_PWM_MODE_50);
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_disp_t *disp = lvgl_manager_init(panel_handle);
    ESP_LOGI(TAG, "Install LVGL tick timer");
    lvgl_manager_start_tick_timer();

    // Register touch input device
    touch_manager_init(disp);

    // LVGL mutex is now managed by lvgl_manager
    lvgl_manager_start_task();


    // Register and start apps
    for (int i = 0; i < num_apps; ++i) {
        app_manager_register_app(apps[i]);
    }
    app_manager_set_active(apps[0]->name); // Start with first app


    bool last_synced = false;
    while (1) {
        app_manager_tick();
        vTaskDelay(pdMS_TO_TICKS(50));
        // Touch events are now handled by the touch manager and app logic

        // Only log SNTP sync status when it changes
        extern bool time_manager_is_synced(void);
        bool now_synced = time_manager_is_synced();
        if (now_synced != last_synced) {
            ESP_LOGI("main", "SNTP synced: %s", now_synced ? "YES" : "NO");
            last_synced = now_synced;
        }
    }

}
