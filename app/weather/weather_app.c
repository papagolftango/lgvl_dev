#include <stdio.h>
#include <lvgl.h>
#include "weather_app.h"
#include "app_manager.h"
#include "screens/ui_Weather.h"
#include "weather_controller.h"

/*Static/global variables*/
static bool screen_active = false;

/**
 * Process weather app data
 * Update weather-related data (currently empty)
 */
void weather_app_process(void)
{
    /*Weather processing logic here*/
}

/**
 * Initialize the weather application
 * Creates the SquareLine screen and initializes the controller
 */
void weather_app_init(void)
{
    printf("[weather_app] Creating SquareLine screen...\n");
    screen_active = true;
    printf("[weather_app] Screen initialized.\n");
    weather_controller_init();
}

/**
 * Clean up the weather application resources
 * Add cleanup logic if needed
 */
void weather_app_cleanup(void)
{
    /*Add cleanup logic if needed*/
}

/**
 * Destroy the weather application
 * Clean up model/controller/view state
 */
void weather_app_destroy(void)
{
    /*Clean up model/controller/view state if needed*/
    screen_active = false;
    /*If you dynamically allocated any LVGL objects, delete them here*/
}
