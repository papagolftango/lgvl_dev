#pragma once
#ifdef __cplusplus
extern "C" {
#endif



void energy_controller_init(void);
void energy_controller_cleanup(void);
void energy_controller_tick(void);
void energy_controller_next_mode(void);
void energy_controller_prev_mode(void);

#ifdef __cplusplus
}
#endif
