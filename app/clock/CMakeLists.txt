idf_component_register(
    SRCS "clock_app.c" "clock_controller.c"
    INCLUDE_DIRS "."
    REQUIRES managers ui
    PRIV_REQUIRES utils
)

