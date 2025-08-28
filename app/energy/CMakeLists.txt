# Energy app CMakeLists.txt
idf_component_register(SRCS
    "energy_app.c"
    "energy_app_mqtt.c"
    "ui/ui_energy.c"
    "ui/ui.c"
    "ui/ui_energy.h"
    "ui/ui.h"
    
        "energy_controller.c"
        INCLUDE_DIRS "." "ui"
)
