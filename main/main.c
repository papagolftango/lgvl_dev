#include <time.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

#include "../drivers/user_config.h"
#include "../drivers/display_driver.h"
#include "../drivers/bidi_switch_knob.h"
#include "lcd_bl_pwm_bsp.h"

#include "../managers/power_manager.h"
#include "../managers/persistent_data_manager.h"
#include "../managers/lvgl_manager.h"
#include "../managers/touch_manager.h"
#include "../managers/app_manager.h"
#include "../managers/mqtt_manager.h"
#include "../managers/wifi_manager.h"
#include "../managers/provisioning_server.h"
#include "../managers/time_manager.h"
#include "../managers/encoder_manager.h"


// For development: Erase NVS and restart to force provisioning
void erase_nvs_and_restart() {
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE("main", "Failed to erase NVS: %s", esp_err_to_name(err));
    }
}





static const char *TAG = "example";

// --- Provisioning and WiFi/MQTT setup implementation ---


// Provisioning callback: called when credentials are received from the web form
static void provisioning_credentials_cb(const char* ssid, const char* password, const char* mqtt_host, const char* mqtt_user, const char* mqtt_pass) {
    ESP_LOGI(TAG, "Provisioning callback: received SSID='%s', PASS='%s', MQTT_HOST='%s', MQTT_USER='%s', MQTT_PASS='%s'", ssid, password, mqtt_host, mqtt_user, mqtt_pass);
    wifi_manager_set_credentials(ssid, password);
    mqtt_manager_set_credentials(mqtt_host, mqtt_user, mqtt_pass);
    provisioning_server_stop();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_manager_connect();
    // mqtt_manager_connect(); // Uncomment if your MQTT manager requires manual connect
}

// Preferred provisioning: start web server and set callback
static void handle_provisioning(void) {
    if (wifi_manager_is_provisioned()) {
        // Credentials exist, connect as STA
        wifi_manager_connect();
    } else {
        // No credentials, start provisioning
        provisioning_server_set_callback(provisioning_credentials_cb);
        provisioning_server_start();
    }
}

void app_main(void)
{
    // Initialize power manager
    power_manager_init();

    // TEMP: Erase NVS and restart to force provisioning on next boot
   // yes
   //erase_nvs_and_restart();
   //  WiFi/MQTT credentials will be set via provisioning web server

    // Initialize TCP/IP stack and event loop (required before WiFi/HTTP server)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create default WiFi AP netif (enables DHCP server for AP mode)
    esp_netif_create_default_wifi_ap();

    // --- Time manager initialization ---

    time_manager_init();

    // Initialize persistent data manager (handles NVS)
    persistent_data_manager_init();
    // ...existing code...


    // --- WiFi/MQTT provisioning and connection ---
    wifi_manager_load_credentials();
    mqtt_manager_load_credentials();
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
    encoder_manager_init();

    // LVGL mutex is now managed by lvgl_manager
    lvgl_manager_start_task();


    // Initialize and start the app system (apps, controllers, UI)
    app_manager_init();

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
