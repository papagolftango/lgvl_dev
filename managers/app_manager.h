#pragma once

#ifdef __cplusplus
extern "C" {
#endif



typedef void (*app_init_fn)(void);
typedef void (*app_tick_fn)(void);
typedef void (*app_cleanup_fn)(void);
typedef void (*app_process_fn)(void); // Called always, even when not active
typedef struct {
    const char *name;
    app_init_fn init;
    app_tick_fn tick;
    app_cleanup_fn cleanup;
    app_process_fn process;
} app_t;

void app_manager_next_app(void);

void app_manager_register_app(const app_t *app);
void app_manager_set_active(const char *name);
void app_manager_tick(void);

void app_manager_call_active_touch(void);
void app_manager_cleanup(void);

const app_t *app_manager_get_active(void);

#ifdef __cplusplus
}
#endif
