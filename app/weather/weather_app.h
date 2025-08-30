#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void weather_app_init(void);
void weather_app_tick(void);
void weather_app_cleanup(void);
void weather_app_process(void);
void weather_app_touch(void);
void weather_app_destroy(void);
#ifdef __cplusplus
}
#endif
