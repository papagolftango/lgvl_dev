#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Create the Weather screen and return its root object
lv_obj_t *weather_screen_create(lv_obj_t *parent);

// (Optional) Destroy the Weather screen
void weather_screen_destroy(void);

// Example: update a value on the screen
void weather_screen_set_status(const char *status);

// Get the root object for persistent screen switching
lv_obj_t *weather_screen_get_root(void);

#ifdef __cplusplus
}
#endif
