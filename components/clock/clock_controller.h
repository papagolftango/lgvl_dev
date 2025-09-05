#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Controller interface for clock app
void clock_controller_init(void);
void clock_controller_tick(void);
void clock_controller_cleanup(void);
void clock_controller_destroy(void);
void clock_controller_process(void);
void clock_controller_touch(void);

#ifdef __cplusplus
}
#endif
