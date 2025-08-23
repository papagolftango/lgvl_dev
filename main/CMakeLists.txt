
idf_component_register(
    SRCS "main.c" "../managers/lvgl_manager.c" "../drivers/display_driver.c" "../drivers/bidi_switch_knob.c" "../managers/touch_manager.c" "../managers/wifi_manager.c" "../managers/mqtt_manager.c" "../managers/provisioning_server.c" "../ui/ui.c" "../managers/app_manager.c" "../managers/time_manager.c" "../managers/persistent_data_manager.c" "../managers/power_manager.c" "../app/energy/energy_app.c" "../app/energy/energy_app_mqtt.c" "../app/energy/ui/ui.c" "../app/energy/ui/ui_energy.c" "../app/home/home_app.c" "../app/home/ui/ui.c" "../app/weather/weather_app.c" "../app/weather/ui/ui.c" "../app/clock/clock_app.c" "../app/clock/ui/ui.c" "../app/clock/ui/screens/ui_Screen1.c" "../app/settings/settings_app.c" "../app/settings/ui/ui.c"
    INCLUDE_DIRS "." "../app" "../ui" "../components/i2c_bsp" "../managers" "../drivers"
    REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt
    PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
)


set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_OPTIONS
    -DLV_LVGL_H_INCLUDE_SIMPLE)

