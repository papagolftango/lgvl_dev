#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Create the Clock screen and return its root object
lv_obj_t *clock_screen_create(lv_obj_t *parent);

// (Optional) Destroy the Clock screen
void clock_screen_destroy(void);

// Example: update a value on the screen
void clock_screen_set_time(const char *time_str);

// Get the root object for persistent screen switching
lv_obj_t *clock_screen_get_root(void);

#ifdef __cplusplus
}
#endif
