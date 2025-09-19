idf_component_register(
    SRCS "weather_app.c" "weather_controller.c"
    INCLUDE_DIRS "."
    REQUIRES managers ui
)
