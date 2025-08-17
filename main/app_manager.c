#include "app_manager.h"
#include <string.h>

#define MAX_APPS 8

static const app_t *registered_apps[MAX_APPS];
static int app_count = 0;
static const app_t *active_app = NULL;

void app_manager_register_app(const app_t *app) {
    if (app_count < MAX_APPS) {
        registered_apps[app_count++] = app;
    }
}

void app_manager_set_active(const char *name) {
    for (int i = 0; i < app_count; ++i) {
        if (strcmp(registered_apps[i]->name, name) == 0) {
            if (active_app && active_app->cleanup) active_app->cleanup();
            active_app = registered_apps[i];
            if (active_app->init) active_app->init();
            break;
        }
    }
}


void app_manager_tick(void) {
    // Call process for all registered apps (background logic)
    for (int i = 0; i < app_count; ++i) {
        if (registered_apps[i]->process) registered_apps[i]->process();
    }
    // Only call tick for the active app (UI updates)
    if (active_app && active_app->tick) active_app->tick();
}

void app_manager_cleanup(void) {
    if (active_app && active_app->cleanup) active_app->cleanup();
    active_app = NULL;
}

const app_t *app_manager_get_active(void) {
    return active_app;
}
