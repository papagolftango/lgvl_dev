// All includes at the top for clarity
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "user_config.h"
#include "display_driver.h"
#include "lcd_bl_pwm_bsp.h"
#include "lvgl_manager.h"
#include "app_manager.h"
#include "energy_app.h"
#include "home_app.h"
#include "weather_app.h"
#include "clock_app.h"
#include "settings_app.h"
#include "touch_manager.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "esp_wifi.h"

// --- TEMPORARY: Hardcoded WiFi/MQTT credentials for development ---

// App declarations at file scope for clarity
static const app_t energy_app = {
    .name = "Energy",
    .init = energy_app_init,
    .tick = energy_app_tick,
    .cleanup = energy_app_cleanup,
    .process = energy_app_process
};
static const app_t home_app = {
    .name = "Home",
    .init = home_app_init,
    .tick = home_app_tick,
    .cleanup = home_app_cleanup,
    .process = home_app_process
};
static const app_t weather_app = {
    .name = "Weather",
    .init = weather_app_init,
    .tick = weather_app_tick,
    .cleanup = weather_app_cleanup,
    .process = weather_app_process
};
static const app_t clock_app = {
    .name = "Clock",
    .init = clock_app_init,
    .tick = clock_app_tick,
    .cleanup = clock_app_cleanup,
    .process = clock_app_process
};
static const app_t settings_app = {
    .name = "Settings",
    .init = settings_app_init,
    .tick = settings_app_tick,
    .cleanup = settings_app_cleanup,
    .process = settings_app_process
};
static void local_init_credentials(void) {
    // Replace these with your actual test credentials
        const char *wifi_ssid = ""; // <-- Fill in your WiFi SSID
        const char *wifi_pass = ""; // <-- Fill in your WiFi password
        const char *mqtt_host = ""; // <-- Fill in your MQTT host (e.g., mqtt://192.168.1.100)
        const char *mqtt_user = ""; // <-- Fill in your MQTT username
        const char *mqtt_pass = ""; // <-- Fill in your MQTT password
    wifi_manager_set_credentials(wifi_ssid, wifi_pass);
    mqtt_manager_set_credentials(mqtt_host, mqtt_user, mqtt_pass);
}
static const char *TAG = "example";





void app_main(void)
{
    // Initialize TCP/IP stack and event loop (required before WiFi/HTTP server)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create default WiFi AP netif (enables DHCP server for AP mode)
    esp_netif_create_default_wifi_ap();

    // Initialize NVS (required for WiFi, provisioning, etc)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ...existing code...



    // --- TEMPORARY: Use hardcoded WiFi & MQTT credentials ---
    local_init_credentials();
    ESP_LOGI(TAG, "Connecting to WiFi with hardcoded credentials...");
    wifi_manager_connect();
    // mqtt_manager_connect() is now called automatically after WiFi connects
    // --- To restore provisioning, revert to the original block above ---

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

    const app_t *apps[] = { &energy_app, &home_app, &weather_app, &clock_app, &settings_app };
    const int num_apps = sizeof(apps) / sizeof(apps[0]);
    for (int i = 0; i < num_apps; ++i) {
        app_manager_register_app(apps[i]);
    }
    app_manager_set_active(apps[0]->name); // Start with first app

    // Main loop with automatic app switching every 10 seconds
    int tick_count = 0;
    int current_app = 0;
    while (1) {
        app_manager_tick();
        vTaskDelay(pdMS_TO_TICKS(50));
        tick_count++;
        if (tick_count >= 100) { // 100 * 50ms = 5 seconds
            tick_count = 0;
            current_app = (current_app + 1) % num_apps;
            app_manager_set_active(apps[current_app]->name);
        }
    }

}
