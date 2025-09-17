
#include <stdio.h>
#include <lvgl.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "app_manager.h"

#include "lvgl_manager.h"

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

#include "screens/error_screen.h"

#include "energy/energy_controller.h"
#include "home/home_controller.h"
#include "settings/settings_controller.h"
#include "weather/weather_controller.h"
#include "clock/clock_controller.h"
#include "managers/touch_manager.h"
#include "managers/encoder_manager.h"

static const char *APP_MANAGER_TAG = "app_manager";

static SemaphoreHandle_t app_manager_mutex = NULL;

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
        .name = "Clock",
        .app_init = clock_app_init,
        .screen_load = (app_screen_load_fn)clock_screen_create,
        .controller_init = clock_controller_init,
        .controller_cleanup = clock_controller_cleanup,
        .app_destroy = clock_app_destroy,
        .tick = clock_controller_tick,
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
static void app_manager_on_encoder(encoder_event_t evt);
static void app_manager_on_touch(lv_indev_drv_t *drv, lv_indev_data_t *data);
static app_encoder_cb_t s_app_encoders[APP_ID_COUNT] = {0};

void app_manager_init(void) {
    // Create mutex for thread safety
    if (!app_manager_mutex) {
        app_manager_mutex = xSemaphoreCreateMutex();
    }

    // Initialize all apps (model/controller/view)
    for (int i = 0; i < APP_ID_COUNT; ++i) {
        if (app_table[i].app_init)
            app_table[i].app_init();
    }
    // Register global input handlers
    touch_manager_register_user_cb(app_manager_on_touch);
    encoder_manager_register_user_cb(app_manager_on_encoder);
    // Optionally, load the default app
    app_manager_set_active(current_app);
}

void app_manager_set_active(app_id_t app_id) {
    if (app_id >= APP_ID_COUNT) return;
    if (app_manager_mutex) xSemaphoreTake(app_manager_mutex, portMAX_DELAY);

    ESP_LOGI(APP_MANAGER_TAG, "Switching from app %d to app %d (%s)", current_app, app_id, app_table[app_id].name);

    // Persistent screen pattern: do not destroy previous app's screen on switch

    // Cleanup previous app
    if (app_table[current_app].controller_cleanup)
        app_table[current_app].controller_cleanup();

    lv_obj_t *root = NULL;
    switch (app_id) {
        case APP_ID_ENERGY:
            root = energy_screen_get_root();
            if (!root) root = energy_screen_create(NULL);
            break;
        case APP_ID_HOME:
            root = home_screen_get_root();
            if (!root) root = home_screen_create(NULL);
            break;
        case APP_ID_CLOCK:
            root = clock_screen_get_root();
            if (!root) root = clock_screen_create(NULL);
            break;
        case APP_ID_SETTINGS:
            root = settings_screen_get_root();
            if (!root) root = settings_screen_create(NULL);
            break;
        case APP_ID_WEATHER:
            root = weather_screen_get_root();
            if (!root) root = weather_screen_create(NULL);
            break;
        default:
            break;
    }
    if (root) {
        // Ensure LVGL operations are serialized while switching screens
        lvgl_manager_lock();
        lv_scr_load(root);
        lvgl_manager_unlock();
    } else {
        ESP_LOGE(APP_MANAGER_TAG, "Failed to create/load screen for app %d (%s), loading error screen!", app_id, app_table[app_id].name);
        root = error_screen_get_root();
        if (!root) root = error_screen_create(NULL);
        if (root) {
            lvgl_manager_lock();
            lv_scr_load(root);
            lvgl_manager_unlock();
        } else {
            ESP_LOGE(APP_MANAGER_TAG, "Failed to create/load error screen! Display may be unstable.");
        }
    }

    // Init new app controller
    if (app_table[app_id].controller_init)
        app_table[app_id].controller_init();

    current_app = app_id;
    if (app_manager_mutex) xSemaphoreGive(app_manager_mutex);
}

app_id_t app_manager_get_active(void) {
    return current_app;
}

const app_descriptor_t *app_manager_get_descriptor(app_id_t app_id) {
    if (app_id >= APP_ID_COUNT) return NULL;
    return &app_table[app_id];
}

void app_manager_tick(void) {
    // Only call tick for the active app. Many ticks perform LVGL object updates, so
    // ensure we hold the LVGL mutex to avoid concurrent access with lv_timer_handler().
    if (app_table[current_app].tick) {
        lvgl_manager_lock();
        app_table[current_app].tick();
        lvgl_manager_unlock();
    }
}

void app_manager_next_app(void) {
    app_id_t next_app = (current_app + 1) % APP_ID_COUNT;
    app_manager_set_active(next_app);
}

void app_manager_register_encoder_cb(app_id_t app, app_encoder_cb_t cb) {
    if (app >= APP_ID_COUNT) return;
    s_app_encoders[app] = cb;
}

void app_manager_unregister_encoder_cb(app_id_t app) {
    if (app >= APP_ID_COUNT) return;
    s_app_encoders[app] = NULL;
}

static void app_manager_on_encoder(encoder_event_t evt) {
    if (current_app < APP_ID_COUNT && s_app_encoders[current_app]) {
        (void)s_app_encoders[current_app](evt); // ignore return: no global fallback
    }
}

static void app_manager_on_touch(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    LV_UNUSED(drv);
    if (data->state == LV_INDEV_STATE_PRESSED) {
        app_manager_next_app();
    }
}
