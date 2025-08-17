idf_component_register(
    SRCS "lvgl_manager.c" "display_driver.c" "main.c" "touch_manager.c" "wifi_manager.c" "mqtt_manager.c" "provisioning_server.c" "../ui/ui.c" "app_manager.c" "../app/energy_app.c" "../app/home_app.c" "../app/weather_app.c" "../app/clock_app.c" "../app/settings_app.c"
    INCLUDE_DIRS "." "../app"
    INCLUDE_DIRS "." "../ui" "../components/i2c_bsp"
    REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt
    PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
)

set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_OPTIONS
    -DLV_LVGL_H_INCLUDE_SIMPLE)
