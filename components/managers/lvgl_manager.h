
#ifndef LVGL_MANAGER_H
#define LVGL_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "esp_lcd_panel_io.h"


// LVGL system/manager API (no UI objects)
lv_disp_t *lvgl_manager_init(esp_lcd_panel_handle_t panel_handle);
void lvgl_manager_start_tick_timer(void);
void lvgl_manager_start_task(void);
void lvgl_manager_lock(void);
void lvgl_manager_unlock(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_MANAGER_H
