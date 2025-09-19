idf_component_register(
    SRCS
        "main.c"
        "../drivers/display_driver.c"
        "../drivers/bidi_switch_knob.c"
    INCLUDE_DIRS "." "../components/i2c_bsp" "../components/managers" "../drivers" "../components/ui" "../components/ui/screens"
    REQUIRES esp_netif esp_event nvs_flash esp_wifi esp_http_server mqtt ui utils managers energy clock home settings weather
    PRIV_REQUIRES lcd_touch_bsp lcd_bl_pwm_bsp
)



