#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// App ID enum
typedef enum {
  APP_ID_ENERGY = 0,
  APP_ID_CLOCK,
  APP_ID_HOME,
  APP_ID_SETTINGS,
  APP_ID_WEATHER,
  APP_ID_COUNT
} app_id_t;

// App function pointer types
typedef void (*app_init_fn)(void); // Calls model/controller/view init for the app
typedef void (*app_controller_init_fn)(void);
typedef void (*app_controller_cleanup_fn)(void);
typedef void (*app_tick_fn)(void);
typedef lv_obj_t *(*app_get_root_fn)(void);
typedef lv_obj_t *(*app_create_root_fn)(lv_obj_t *parent);

// App descriptor struct
typedef struct {
    const char *name;
    app_init_fn app_init; // single entry point for all app init
    app_controller_init_fn controller_init;
    app_controller_cleanup_fn controller_cleanup;
    app_tick_fn tick;
  app_get_root_fn get_root;      // returns existing root or NULL
  app_create_root_fn create_root; // creates root when NULL
} app_descriptor_t;

void app_manager_destroy(void);
void app_manager_init(void);
void app_manager_set_active(app_id_t app_id);
app_id_t app_manager_get_active(void);
const app_descriptor_t *app_manager_get_descriptor(app_id_t app_id);
// Registration API: call before app_manager_init to populate available apps
void app_manager_register(app_id_t app_id, const app_descriptor_t *desc);
// Optional helper implemented by the application to register all apps
void app_manager_register_all(void);
void app_manager_tick(void);
void app_manager_next_app(void);
void app_manager_call_active_touch(void);
void app_manager_cleanup(void);

#include "managers/encoder_manager.h" // for encoder_event_t

// Per-app encoder handler registration: one callback per app, returning true if handled
typedef bool (*app_encoder_cb_t)(encoder_event_t evt);
void app_manager_register_encoder_cb(app_id_t app, app_encoder_cb_t cb);
void app_manager_unregister_encoder_cb(app_id_t app);

#ifdef __cplusplus
}
#endif
