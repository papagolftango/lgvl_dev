#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Controller interface for weather app
void weather_controller_init(void);
void weather_controller_tick(void);
void weather_controller_cleanup(void);
void weather_controller_destroy(void);
void weather_controller_process(void);
void weather_controller_touch(void);

#ifdef __cplusplus
}
#endif
