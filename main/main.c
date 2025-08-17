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
#include "ui.h"
#include "touch_manager.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "esp_wifi.h"

// --- TEMPORARY: Hardcoded WiFi/MQTT credentials for development ---
static void local_init_credentials(void) {
    // Replace these with your actual test credentials
    const char *wifi_ssid = "ZyXEL8E5D32";
    const char *wifi_pass = "FDE0ABA88073";
    const char *mqtt_host = "mqtt://192.168.68.66";
    const char *mqtt_user = "emonpi";
    const char *mqtt_pass = "emonpimqtt2016";
    wifi_manager_set_credentials(wifi_ssid, wifi_pass);
    mqtt_manager_set_credentials(mqtt_host, mqtt_user, mqtt_pass);
}
// Touch event handler to toggle color (file scope, not inside app_main)
void label_touch_event_cb(lv_event_t *e) {
    static bool is_red = true;
    lv_obj_t *label = lv_event_get_target(e);
    if (is_red) {
        lv_obj_set_style_text_color(label, lv_color_hex(0x00FF00), 0); // Green
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0); // Red
    }
    is_red = !is_red;
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

    // Initialize UI (SquareLine-ready)
    lvgl_manager_lock();
    ui_init();
    lvgl_manager_unlock();

}
