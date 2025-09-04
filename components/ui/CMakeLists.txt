
    SRCS images/ui_img_gauge_face_kw_final_png.c
         images/ui_img_black_circle_360_png.c
    INCLUDE_DIRS . ../../drivers
    REQUIRES lvgl__lvgl
)
    idf_component_register(
        SRCS images/ui_img_gauge_face_kw_final_png.c
             images/ui_img_black_circle_360_png.c
        INCLUDE_DIRS . ../../drivers
        REQUIRES lvgl__lvgl
    )
