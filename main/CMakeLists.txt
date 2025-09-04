# Ensure idf_component_register is properly closed

idf_component_register(
    SRCS
        "main.c"
        "../drivers/display_driver.c"
        "../drivers/bidi_switch_knob.c"
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
    # Managers are now a component, do not include here
                       INCLUDE_DIRS "." "../app" "../app/energy" "../app/home" "../app/settings" "../app/weather" "../components/i2c_bsp" "../components/managers" "../drivers" "../components/ui" "../components/ui/screens"
    REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt ui
    PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
)



