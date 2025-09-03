#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "screens/ui_Energy.h"

/*Update the UI arc to reflect the current balance value*/
void energy_controller_update_balance(int balance);

/*Draw the pointer for the energy balance (controller-owned implementation)*/
void draw_pointer_for_balance(float energy_balance);

/*Update all UI elements from the model (tick)*/
void energy_controller_tick(void);

void energy_controller_init(void);
void energy_controller_cleanup(void);

/*Switch to the next app*/
void switch_to_next_app(void);

#ifdef __cplusplus
}
#endif
