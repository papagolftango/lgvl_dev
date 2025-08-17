#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void home_app_init(void);
void home_app_tick(void);
void home_app_cleanup(void);
// Called always, even when not active
void home_app_process(void);

#ifdef __cplusplus
}
#endif
