#include "haptic_manager.h"
#include "touch_manager.h"
#include "power_manager.h"

#include <stdint.h>

#include "esp_log.h"
#include "lcd_touch_bsp.h"

#include "user_config.h"
#include "haptic_manager.h"

// Touch callback for LVGL input device
typedef struct {
    lv_indev_drv_t indev_drv;
} touch_manager_t;

static touch_user_cb_t user_cb = NULL;
// When waking from IDLE via a touch press, suppress forwarding to the app until the release
static bool s_ignore_until_release = false;
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
        bool was_idle = power_manager_is_idle();
        power_manager_notify_activity();
        if (!was_idle) {
            // Only emit haptic on meaningful (non-wake) touch
            haptic_click();
        } else {
            // Consume wakeup press: do not forward this gesture to app_manager; suppress until release
            s_ignore_until_release = true;
            ESP_LOGI("TP", "Woke from IDLE via touch; consuming press and suppressing until release");
        }
        if (user_cb && !s_ignore_until_release) {
            user_cb(drv, data);
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        if (s_ignore_until_release) {
            // End of the consumed wake gesture; stop suppressing further touches
            s_ignore_until_release = false;
            ESP_LOGI("TP", "Touch release after wake consumed; resuming normal touch processing");
        } else if (user_cb) {
            user_cb(drv, data);
        }
    }
}

void touch_manager_register_user_cb(touch_user_cb_t cb) { user_cb = cb; }

void touch_manager_unregister_user_cb(void) {
    user_cb = NULL;
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
