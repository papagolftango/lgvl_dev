#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ui/ui_Energy.h"

// Call this to update the UI arc to reflect the current balance value
void energy_controller_update_balance(int balance);

// Call this to update all UI elements from the model (tick)
void energy_controller_tick(void);

void energy_controller_init(void);
void energy_controller_cleanup(void);

#ifdef __cplusplus
}
#endif
