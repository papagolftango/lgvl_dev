idf_component_register(
    SRCS images/ui_img_gauge_face_kw_final_png.c
        images/ui_img_black_circle_360_png.c
        screens/clock_screen.c
        screens/energy_screen.c
        screens/home_screen.c
        screens/settings_screen.c
        screens/weather_screen.c
        screens/error_screen.c
    INCLUDE_DIRS . .. images screens ../../drivers
    REQUIRES lvgl__lvgl
)
