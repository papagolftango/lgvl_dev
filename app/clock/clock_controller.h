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
void clock_controller_toggle_12_24(void);
void clock_controller_show_date_briefly(void);

#ifdef __cplusplus
}
#endif
