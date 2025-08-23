void ui_energy_destroy(void);
#pragma once
#include <lvgl.h>
#include "../../managed_components/lvgl__lvgl/src/extra/widgets/meter/lv_meter.h"

lv_obj_t *ui_energy_meter(void);
lv_meter_indicator_t *ui_energy_needle_balance(void);
lv_meter_indicator_t *ui_energy_needle_peak(void);
void ui_energy_create(lv_obj_t *parent, float initial_balance, float initial_peak);
void ui_energy_update(float balance, float peak);
