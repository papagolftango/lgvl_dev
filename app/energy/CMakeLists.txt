# Energy app CMakeLists.txt
idf_component_register(SRCS
    "energy_app.c"
    "energy_controller.c"
    "ui/ui_Energy.c"
    "ui/ui.c"
    "ui/ui_Energy.h"
    "ui/ui.h"
    
        "energy_controller.c"
        INCLUDE_DIRS "." "ui"
)
