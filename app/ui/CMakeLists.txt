SET(SOURCES screens/ui_Energy.c
    screens/ui_Home.c
    screens/ui_Settings.c
    screens/ui_Weather.c
    screens/ui_clock.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
