
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Energy app data variables (shared with MQTT handler)
extern float energy_vrms;
extern float energy_solar;
extern float energy_used;
extern float energy_balance;
extern float energy_peak_solar;
extern float energy_peak_used;


void energy_app_init(void);
void energy_app_tick(void);
void energy_app_process(void);
void energy_app_touch(void);
void energy_app_set_vrms(const char *vrms);
void energy_app_set_solar(float value);
void energy_app_set_used(float value);
void energy_app_set_balance(float value);
bool energy_app_is_screen_active(void);

// App manager lifecycle
void ui_Energy_screen_load(void);
void energy_app_destroy(void);

#ifdef __cplusplus
}
#endif
