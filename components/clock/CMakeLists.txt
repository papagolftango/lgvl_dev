idf_component_register(SRCS "clock_app.c" "clock_controller.c"
                       INCLUDE_DIRS "." "../.." "../../app"
                       PRIV_REQUIRES lvgl__lvgl managers)
