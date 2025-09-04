#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Create the Home screen and return its root object
lv_obj_t *home_screen_create(lv_obj_t *parent);

// (Optional) Destroy the Home screen
void home_screen_destroy(void);

// Example: update a value on the screen
void home_screen_set_title(const char *title);

// Get the root object for persistent screen switching
lv_obj_t *home_screen_get_root(void);

#ifdef __cplusplus
}
#endif
