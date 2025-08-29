#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// App ID enum
typedef enum {
    APP_ID_ENERGY = 0,
    APP_ID_HOME,
    APP_ID_CLOCK,
    APP_ID_SETTINGS,
    APP_ID_WEATHER,
    APP_ID_COUNT
} app_id_t;

// App function pointer types
typedef void (*app_init_fn)(void); // Calls model/controller/view init for the app
typedef void (*app_screen_load_fn)(void);
typedef void (*app_controller_init_fn)(void);
typedef void (*app_controller_cleanup_fn)(void);
typedef void (*app_destroy_fn)(void);
typedef void (*app_tick_fn)(void);

// App descriptor struct
typedef struct {
    const char *name;
    app_init_fn app_init; // single entry point for all app init
    app_screen_load_fn screen_load;
    app_controller_init_fn controller_init;
    app_controller_cleanup_fn controller_cleanup;
    app_destroy_fn app_destroy; // single entry point for all app destroy
    app_tick_fn tick;
} app_descriptor_t;

void app_manager_destroy(void);
void app_manager_init(void);
void app_manager_set_active(app_id_t app_id);
app_id_t app_manager_get_active(void);
const app_descriptor_t *app_manager_get_descriptor(app_id_t app_id);
void app_manager_tick(void);
void app_manager_next_app(void);
void app_manager_call_active_touch(void);
void app_manager_cleanup(void);

#ifdef __cplusplus
}
#endif
