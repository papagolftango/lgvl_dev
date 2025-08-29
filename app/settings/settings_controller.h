#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Controller interface for settings app
void settings_controller_init(void);
void settings_controller_tick(void);
void settings_controller_cleanup(void);
void settings_controller_destroy(void);
void settings_controller_process(void);
void settings_controller_touch(void);

#ifdef __cplusplus
}
#endif
