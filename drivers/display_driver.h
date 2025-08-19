
// display_driver.h -- Display driver interface for LVGL/ESP-IDF
#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "lvgl.h"
#include "esp_lcd_panel_ops.h"

/**
 * @brief Initialize the display hardware (SPI bus, panel, etc.)
 * @return esp_lcd_panel_handle_t for use as LVGL user_data
 */
esp_lcd_panel_handle_t display_init(void);

// LVGL display driver callbacks
void display_driver_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void display_driver_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area);

#endif // DISPLAY_DRIVER_H
