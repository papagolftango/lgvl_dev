#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Controller interface for home app
void home_controller_init(void);
void home_controller_tick(void);
void home_controller_cleanup(void);
void home_controller_destroy(void);
void home_controller_process(void);
void home_controller_touch(void);

#ifdef __cplusplus
}
#endif
