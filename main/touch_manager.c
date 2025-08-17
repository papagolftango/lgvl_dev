#include "touch_manager.h"
#include "user_config.h"
#include "lcd_touch_bsp.h"
#include "esp_log.h"
#include <stdint.h>

// Touch callback for LVGL input device
typedef struct {
    lv_indev_drv_t indev_drv;
} touch_manager_t;

static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t tp_x;
    uint16_t tp_y;
    uint8_t win = tpGetCoordinates(&tp_x, &tp_y);
    if (win) {
#ifdef EXAMPLE_Rotate_90
        data->point.x = tp_y;
        data->point.y = (LCD_V_RES - tp_x);
#else
        data->point.x = tp_x;
        data->point.y = tp_y;
#endif
        if (data->point.x > LCD_H_RES) data->point.x = LCD_H_RES;
        if (data->point.y > LCD_V_RES) data->point.y = LCD_V_RES;
        data->state = LV_INDEV_STATE_PRESSED;
        ESP_LOGI("TP", "Touch detected: (%d, %d)", data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void touch_manager_init(lv_disp_t *disp)
{
    static lv_indev_drv_t indev_drv; // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);
}
