
# CMakeLists.txt for managers (moved out of components)

set(MANAGERS_SRCS
    encoder_manager/encoder_manager.c
    app_manager.c
    lvgl_manager.c
    mqtt_manager.c
    persistent_data_manager.c
    power_manager.c
    provisioning_server.c
    time_manager.c
    touch_manager.c
    wifi_manager.c
)

idf_component_register(
    SRCS ${MANAGERS_SRCS}
    INCLUDE_DIRS . encoder_manager
    PRIV_REQUIRES main lcd_touch_bsp
)
