#include <lvgl.h>
#include "app_manager.h"
#include "ui.h"
#include "energy/energy_app.h"
#include "home/home_app.h"
#include "clock/clock_app.h"
#include "settings/settings_app.h"
#include "weather/weather_app.h"
#include "screens/energy_screen.h"
#include "screens/home_screen.h"
#include "screens/settings_screen.h"
#include "screens/weather_screen.h"
#include "screens/clock_screen.h"
#include "ui_custom_load.h"
#include "energy/energy_controller.h"
#include "home/home_controller.h"
#include "settings/settings_controller.h"
#include "weather/weather_controller.h"
#include <stdio.h>




static const app_descriptor_t app_table[APP_ID_COUNT] = {
    {
        .name = "Energy",
        .app_init = energy_app_init,
        .screen_load = (app_screen_load_fn)energy_screen_create,
        .controller_init = energy_controller_init,
        .controller_cleanup = energy_controller_cleanup,
        .app_destroy = energy_app_destroy,
        .tick = energy_controller_tick,
    },
    {
        .name = "Home",
        .app_init = home_app_init,
        .screen_load = (app_screen_load_fn)home_screen_create,
        .controller_init = home_controller_init,
        .controller_cleanup = home_controller_cleanup,
        .app_destroy = home_app_destroy,
        .tick = home_controller_tick,
    },
    {
        .name = "Clock",
        .app_init = clock_app_init,
        .screen_load = (app_screen_load_fn)clock_screen_create,
        .controller_init = clock_controller_init,
        .controller_cleanup = clock_controller_cleanup,
        .app_destroy = clock_app_destroy,
        .tick = clock_controller_tick,
    },
    {
        .name = "Settings",
        .app_init = settings_app_init,
        .screen_load = (app_screen_load_fn)settings_screen_create,
        .controller_init = settings_controller_init,
        .controller_cleanup = settings_controller_cleanup,
        .app_destroy = settings_app_destroy,
        .tick = settings_controller_tick,
    },
    {
        .name = "Weather",
        .app_init = weather_app_init,
        .screen_load = (app_screen_load_fn)weather_screen_create,
        .controller_init = weather_controller_init,
        .controller_cleanup = weather_controller_cleanup,
        .app_destroy = weather_app_destroy,
        .tick = weather_controller_tick,
    }
};

static app_id_t current_app = APP_ID_ENERGY;

void app_manager_init(void) {
    // Initialize UI (theme and screens)
    ui_init();

    // Initialize all apps (model/controller/view)
    for (int i = 0; i < APP_ID_COUNT; ++i) {
        if (app_table[i].app_init)
            app_table[i].app_init();
    }
    // Optionally, load the default app
    app_manager_set_active(current_app);
}

void app_manager_set_active(app_id_t app_id) {
    if (app_id >= APP_ID_COUNT) return;

    printf("[app_manager] Switching from app %d to app %d (%s)\n", current_app, app_id, app_table[app_id].name);

    // Persistent screen pattern: do not destroy previous app's screen on switch

    // Cleanup previous app
    if (app_table[current_app].controller_cleanup)
        app_table[current_app].controller_cleanup();

    // Show new app's persistent screen (assumes screen object is global, e.g., ui_Energy)
    switch (app_id) {
        case APP_ID_ENERGY:
            lv_scr_load(energy_screen_get_root());
            break;
    case APP_ID_HOME:
        lv_scr_load(home_screen_get_root());
        break;
        case APP_ID_CLOCK:
            lv_scr_load(clock_screen_get_root());
            break;
        case APP_ID_SETTINGS:
            lv_scr_load(settings_screen_get_root());
            break;
        case APP_ID_WEATHER:
            lv_scr_load(weather_screen_get_root());
            break;
        default:
            break;
    }

    // Init new app controller
    if (app_table[app_id].controller_init)
        app_table[app_id].controller_init();

    current_app = app_id;
}

app_id_t app_manager_get_active(void) {
    return current_app;
}

const app_descriptor_t *app_manager_get_descriptor(app_id_t app_id) {
    if (app_id >= APP_ID_COUNT) return NULL;
    return &app_table[app_id];
}

void app_manager_tick(void) {
    // Only call tick for the active app
    if (app_table[current_app].tick)
        app_table[current_app].tick();
}

void app_manager_next_app(void) {
    app_id_t next_app = (current_app + 1) % APP_ID_COUNT;
    app_manager_set_active(next_app);
}
