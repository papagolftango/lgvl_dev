#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <stdbool.h>
#include "lvgl.h"

#ifdef __cplusplus
#define TOUCH_CB_TYPE lv_indev_drv_t *, lv_indev_data_t *
#else
#define TOUCH_CB_TYPE lv_indev_drv_t *, lv_indev_data_t *
#endif

typedef void (*touch_user_cb_t)(TOUCH_CB_TYPE);

void touch_manager_register_user_cb(touch_user_cb_t cb);
void touch_manager_unregister_user_cb(void);

#ifdef __cplusplus
extern "C" {
#endif

void touch_manager_init(lv_disp_t *disp);

#ifdef __cplusplus
}
#endif

#endif // TOUCH_MANAGER_H

