#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Create the Settings screen and return its root object
lv_obj_t *settings_screen_create(lv_obj_t *parent);

// (Optional) Destroy the Settings screen
void settings_screen_destroy(void);

// Example: update a value on the screen
void settings_screen_set_title(const char *title);

// Get the root object for persistent screen switching
lv_obj_t *settings_screen_get_root(void);

#ifdef __cplusplus
}
#endif
