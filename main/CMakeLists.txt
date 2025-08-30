# Ensure idf_component_register is properly closed

    idf_component_register(
        SRCS
            "main.c"
            "../drivers/display_driver.c"
            "../drivers/bidi_switch_knob.c"
            # Shared UI sources
            "../app/ui/ui.c"
            "../app/ui/screens/ui_Energy.c"
            "../app/ui/screens/ui_Home.c"
            "../app/ui/screens/ui_Settings.c"
            "../app/ui/screens/ui_Weather.c"
            "../app/ui/screens/ui_Clock.c"
            "../app/ui/ui_comp_hook.c"
            "../app/ui/ui_custom_load.c"
            "../app/ui/images/ui_img_black_circle_360_png.c"
            # App and controller sources
            "../app/energy/energy_app.c"
            "../app/energy/energy_controller.c"
            "../app/settings/settings_app.c"
            "../app/settings/settings_controller.c"
            "../app/home/home_app.c"
            "../app/home/home_controller.c"
            "../app/weather/weather_app.c"
            "../app/weather/weather_controller.c"
            # Managers
            "../managers/encoder_manager.c"
            "../managers/app_manager.c"
            "../managers/lvgl_manager.c"
            "../managers/wifi_manager.c"
            "../managers/mqtt_manager.c"
            "../managers/persistent_data_manager.c"
            "../managers/touch_manager.c"
            "../managers/provisioning_server.c"
            "../managers/power_manager.c"
            "../managers/time_manager.c"
        INCLUDE_DIRS "." "../app" "../app/ui" "../app/energy" "../app/home" "../app/settings" "../app/weather" "../components/i2c_bsp" "../managers" "../drivers"
        REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt
        PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
    )

