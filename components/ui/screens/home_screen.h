#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Create the Home screen and return its root object
lv_obj_t *home_screen_create(lv_obj_t *parent);

// Destroy the Home screen (if created)
void home_screen_destroy(void);

// Update the message of the day on the Home screen
void home_screen_set_motd(const char *text);

// Get the root object for persistent screen switching
lv_obj_t *home_screen_get_root(void);

#ifdef __cplusplus
}
#endif
