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
#if CONFIG_PM_ENABLE
#include "esp_pm.h"
#endif

#include "user_config.h"
#include "../drivers/display_driver.h"
#include "../drivers/bidi_switch_knob.h"
#include "lcd_bl_pwm_bsp.h"

#include "power_manager.h"
#include "persistent_data_manager.h"
#include "lvgl_manager.h"
#include "touch_manager.h"
#include "app_manager.h"
#include "mqtt_manager.h"
#include "wifi_manager.h"
#include "provisioning_server.h"
#include "time_manager.h"
#include "encoder_manager.h"
#include "haptic_manager.h"

// Include app & controller and screen headers for registration
#include "energy_app.h"
#include "energy_controller.h"
#include "ui/screens/energy_screen.h"
#include "clock_app.h"
#include "clock_controller.h"
#include "ui/screens/clock_screen.h"
#include "home_app.h"
#include "home_controller.h"
#include "ui/screens/home_screen.h"
#include "settings_app.h"
#include "settings_controller.h"
#include "ui/screens/settings_screen.h"
#include "weather_app.h"
#include "weather_controller.h"
#include "ui/screens/weather_screen.h"

static const char *TAG = "Home Help";

#if CONFIG_PM_ENABLE
// Power management lock to prevent light sleep while ACTIVE
static esp_pm_lock_handle_t s_no_ls_lock = NULL;

static void pm_init(void) {
    // Configure Dynamic Frequency Scaling (DFS) and allow light sleep when unlocked
    esp_pm_config_t pm = {
        .max_freq_mhz = 240,
#if CONFIG_IDF_TARGET_ESP32
        .min_freq_mhz = 40,
#else
        // ESP32-S3 typical min is 80 MHz
        .min_freq_mhz = 80,
#endif
        .light_sleep_enable = true,
    };
    esp_err_t err = esp_pm_configure(&pm);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_pm_configure failed: %s", esp_err_to_name(err));
    }
    // Create a NO_LIGHT_SLEEP lock so we can keep system responsive while ACTIVE
    err = esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "ui_active", &s_no_ls_lock);
    if (err == ESP_OK && s_no_ls_lock) {
        // Default to ACTIVE at boot: prevent light sleep until we go IDLE
        esp_pm_lock_acquire(s_no_ls_lock);
    } else {
        ESP_LOGW(TAG, "esp_pm_lock_create failed: %s", esp_err_to_name(err));
    }
}
#endif

// Dim/restore backlight on power state changes
static void power_state_changed(power_state_t state, void *user) {
    (void)user;
    if (state == POWER_IDLE) {
    // Dim backlight when idle (fade out over ~10 seconds)
#if CONFIG_PM_ENABLE
    // Keep light sleep disabled until fade-out completes; LEDC fade uses ISR/timers
    if (s_no_ls_lock) {
        esp_pm_lock_acquire(s_no_ls_lock);
    }
#endif
    lcd_bl_pwm_bsp_fade_to_wait(LCD_PWM_MODE_0, 2000, true);
#if CONFIG_PM_ENABLE
    // Allow light sleep while idle after fade fully completed
        if (s_no_ls_lock) {
            esp_pm_lock_release(s_no_ls_lock);
        }
#endif
    } else {
        // Restore backlight when active (quick fade-in)
    // Prevent light sleep during fade-in for snappy wake and to avoid LEDC ISR issues
#if CONFIG_PM_ENABLE
        if (s_no_ls_lock) { esp_pm_lock_acquire(s_no_ls_lock); }
#endif
        lcd_bl_pwm_bsp_fade_to_wait(LCD_PWM_MODE_50, 250, true);
    }
}

// For development: Erase NVS and restart to force provisioning
void erase_nvs_and_restart() {
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE("main", "Failed to erase NVS: %s", esp_err_to_name(err));
    }
}

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

// Helper to check display init every time app_main runs
static esp_lcd_panel_handle_t safe_display_init(void) {
    ESP_LOGI(TAG, "Calling display_init...");
    esp_lcd_panel_handle_t panel_handle = display_init();
    if (!panel_handle) {
        ESP_LOGE(TAG, "display_init failed! Panel handle is NULL. Aborting app_main.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    ESP_LOGI(TAG, "display_init complete, panel_handle=%p", panel_handle);
    return panel_handle;
}


// Provide app registrations to the app manager (override weak stub)
extern void app_manager_register(app_id_t app_id, const app_descriptor_t *desc);
void app_manager_register_all(void) {
    app_manager_register(APP_ID_ENERGY, &(app_descriptor_t){
        .name = "Energy",
        .app_init = energy_app_init,
        .controller_init = energy_controller_init,
        .controller_cleanup = energy_controller_cleanup,
        .tick = energy_controller_tick,
        .get_root = energy_screen_get_root,
        .create_root = energy_screen_create,
    });
    app_manager_register(APP_ID_CLOCK, &(app_descriptor_t){
        .name = "Clock",
        .app_init = clock_app_init,
        .controller_init = clock_controller_init,
        .controller_cleanup = clock_controller_cleanup,
        .tick = clock_controller_tick,
        .get_root = clock_screen_get_root,
        .create_root = clock_screen_create,
    });
    app_manager_register(APP_ID_HOME, &(app_descriptor_t){
        .name = "Home",
        .app_init = home_app_init,
        .controller_init = home_controller_init,
        .controller_cleanup = home_controller_cleanup,
        .tick = home_controller_tick,
        .get_root = home_screen_get_root,
        .create_root = home_screen_create,
    });
    app_manager_register(APP_ID_SETTINGS, &(app_descriptor_t){
        .name = "Settings",
        .app_init = settings_app_init,
        .controller_init = settings_controller_init,
        .controller_cleanup = settings_controller_cleanup,
        .tick = settings_controller_tick,
        .get_root = settings_screen_get_root,
        .create_root = settings_screen_create,
    });
    app_manager_register(APP_ID_WEATHER, &(app_descriptor_t){
        .name = "Weather",
        .app_init = weather_app_init,
        .controller_init = weather_controller_init,
        .controller_cleanup = weather_controller_cleanup,
        .tick = weather_controller_tick,
        .get_root = weather_screen_get_root,
        .create_root = weather_screen_create,
    });
}

void app_main(void)
{
    // TEMP: Erase NVS and restart to force provisioning on next boot
    //erase_nvs_and_restart();

    // Initialize TCP/IP stack and event loop (required before WiFi/HTTP server)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create default WiFi AP netif (enables DHCP server for AP mode)
    esp_netif_create_default_wifi_ap();

#if CONFIG_PM_ENABLE
    // Configure power management (DFS + light sleep) and keep light sleep disabled until idle
    pm_init();
#else
    ESP_LOGI(TAG, "CONFIG_PM_ENABLE is disabled; DFS/light sleep not configured");
#endif

    // --- Time manager initialization ---
    time_manager_init();

    // Initialize persistent data manager (handles NVS)
    persistent_data_manager_init();

    // --- WiFi/MQTT provisioning and connection ---
    wifi_manager_load_credentials();
    mqtt_manager_load_credentials();
    handle_provisioning();

    esp_lcd_panel_handle_t panel_handle = safe_display_init();

    ESP_LOGI(TAG, "Set backlight to 50%%");
    setUpduty(LCD_PWM_MODE_50);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_disp_t *disp = lvgl_manager_init(panel_handle);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    lvgl_manager_start_tick_timer();
  
    // Initialize haptic manager (after I2C is up, before app_manager)
    ESP_LOGI(TAG, "Initialize haptic manager");
    haptic_manager_init();

    // Register touch input device
    touch_manager_init(disp);

    // Startup the rotary encoder manager
    encoder_manager_init();

    // LVGL mutex is now managed by lvgl_manager
    lvgl_manager_start_task();

    // Initialize and start the app system (apps, controllers, UI)
    app_manager_init();

    // Initialize power manager with inactivity timeout (seconds) and callback
    power_manager_init(10);
    power_manager_register_state_cb(power_state_changed, NULL);

    bool last_synced = false;
    while (1) {
    app_manager_tick();
        vTaskDelay(pdMS_TO_TICKS(50));

        // Only log SNTP sync status when it changes
        bool now_synced = time_manager_is_synced();
        if (now_synced != last_synced) {
            ESP_LOGI("main", "SNTP synced: %s", now_synced ? "YES" : "NO");
            last_synced = now_synced;
        }
    }
}
