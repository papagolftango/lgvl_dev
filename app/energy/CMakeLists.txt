# Energy app CMakeLists.txt
idf_component_register(SRCS
    "energy_app.c"
    "energy_controller.c"
    
    INCLUDE_DIRS "." "../../components/ui/screens"
)
