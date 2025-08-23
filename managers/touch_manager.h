
#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <lvgl.h>

typedef void (*touch_user_cb_t)(lv_indev_drv_t *drv, lv_indev_data_t *data);

void touch_manager_init(lv_disp_t *disp);
void touch_manager_register_user_cb(touch_user_cb_t cb);
void touch_manager_unregister_user_cb(void);

#endif // TOUCH_MANAGER_H

