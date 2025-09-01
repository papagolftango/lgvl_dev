
# Register ui as an ESP-IDF component
idf_component_register(
    SRCS
        "screens/ui_Energy.c"
        "screens/ui_Weather.c"
        "screens/ui_Clock.c"
        "screens/ui_Settings.c"
        "screens/ui_Home.c"
        "ui.c"
        "components/ui_comp_hook.c"
        "ui_helpers.c"
        "images/ui_img_gauge_face_kw_final_png.c"
    INCLUDE_DIRS "." "screens" "images" "components"
    REQUIRES lvgl__lvgl
)
