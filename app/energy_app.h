#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void energy_app_init(void);
void energy_app_tick(void);
void energy_app_cleanup(void);
// Called always, even when not active
void energy_app_process(void);
void energy_app_set_vrms(const char *vrms);
void energy_app_set_solar(float value);
void energy_app_set_used(float value);
void energy_app_set_balance(float value);

#ifdef __cplusplus
}
#endif
