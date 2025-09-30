
#pragma once
#include <math.h>
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Draw the energy balance pointer and peak markers on the energy screen
void draw_pointer_and_peaks(float energy_balance, float peak_solar, float peak_used, float curr_solar, float curr_used);

// Create the Energy screen and return its root object
lv_obj_t *energy_screen_create(lv_obj_t *parent);

// (Optional) Destroy the Energy screen
void energy_screen_destroy(void);

// Example: update a value on the screen
void energy_screen_set_value(int value);

// Get the root object for persistent screen switching
lv_obj_t *energy_screen_get_root(void);

// Cycle the center display mode (next/previous). Safe to call from controller.
void energy_screen_next_mode(void);
void energy_screen_prev_mode(void);

#ifdef CONFIG_TEST_MODE
// Test-mode helpers: expose current mode name and allow forcing a mode by name.
// Returns a const pointer to an internal static string (do not free).
const char *energy_screen_get_mode_name(void);
// Force mode by name (case-sensitive match against internal list). Returns true if applied.
bool energy_screen_set_mode_name(const char *name);
#endif

#ifdef __cplusplus
}
#endif
