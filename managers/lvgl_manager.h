void lvgl_manager_start_tick_timer(void);
#ifndef LVGL_MANAGER_H
#define LVGL_MANAGER_H

#include "lvgl.h"
#include "esp_lcd_panel_io.h"

#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

// Global labels for VRMS display
extern lv_obj_t *vrms_label;
extern lv_obj_t *vrms_value_label;
void lvgl_manager_set_vrms(const char *vrms);

#ifdef __cplusplus
extern "C" {
#endif

lv_disp_t *lvgl_manager_init(esp_lcd_panel_handle_t panel_handle);
void lvgl_manager_lock(void);
void lvgl_manager_unlock(void);
void lvgl_manager_start_task(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_MANAGER_H
