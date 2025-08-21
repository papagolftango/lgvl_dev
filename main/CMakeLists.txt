
idf_component_register(
    SRCS "main.c" "../managers/lvgl_manager.c" "../drivers/display_driver.c" "../drivers/bidi_switch_knob.c" "../managers/touch_manager.c" "../managers/wifi_manager.c" "../managers/mqtt_manager.c" "../managers/provisioning_server.c" "../ui/ui.c" "../managers/app_manager.c" "../managers/time_manager.c" "../managers/persistent_data_manager.c" "../managers/power_manager.c" "../app/energy_app.c" "../app/energy_app_mqtt.c" "../app/home_app.c" "../app/weather_app.c" "../app/clock_app.c" "../app/settings_app.c" "../app/tz_utils.c"
    INCLUDE_DIRS "." "../app" "../ui" "../components/i2c_bsp" "../managers" "../drivers"
    REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt
    PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
)


set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_OPTIONS
    -DLV_LVGL_H_INCLUDE_SIMPLE)

