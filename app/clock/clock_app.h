#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void clock_app_init(void);
void clock_app_tick(void);
void clock_app_cleanup(void);
void clock_app_process(void);
void clock_app_touch(void);
#ifdef __cplusplus
}
#endif
