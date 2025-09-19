
#include <stdio.h>
#include <lvgl.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "app_manager.h"

#include "lvgl_manager.h"

#include "screens/error_screen.h"
#include "managers/touch_manager.h"
#include "managers/encoder_manager.h"

static const char *APP_MANAGER_TAG = "app_manager";

static SemaphoreHandle_t app_manager_mutex = NULL;
// Weak stub; application may override to register apps
__attribute__((weak)) void app_manager_register_all(void) {}

static app_descriptor_t app_table[APP_ID_COUNT] = {0};

static app_id_t current_app = APP_ID_ENERGY;
static void app_manager_on_encoder(encoder_event_t evt);
static void app_manager_on_touch(lv_indev_drv_t *drv, lv_indev_data_t *data);
static app_encoder_cb_t s_app_encoders[APP_ID_COUNT] = {0};
// Defer app switches requested by touch to avoid LVGL lock reentrancy from indev read_cb
static volatile bool s_next_app_requested = false;
static volatile bool s_touch_down = false;

void app_manager_init(void) {
    // Create mutex for thread safety
    if (!app_manager_mutex) {
        app_manager_mutex = xSemaphoreCreateMutex();
    }

    // Allow application to register apps before init (weak stub if not provided by app)
    app_manager_register_all();
    // Initialize all registered apps (model/controller/view)
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
    if (app_table[app_id].get_root) {
        root = app_table[app_id].get_root();
    }
    if (!root && app_table[app_id].create_root) {
        root = app_table[app_id].create_root(NULL);
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

void app_manager_register(app_id_t app_id, const app_descriptor_t *desc) {
    if (!desc) return;
    if (app_id >= APP_ID_COUNT) return;
    app_table[app_id] = *desc;
}

void app_manager_tick(void) {
    // Handle any deferred app switch first (outside of LVGL lock)
    if (s_next_app_requested) {
        s_next_app_requested = false;
        app_manager_next_app();
    }
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
        // Only schedule once per touch press (edge-triggered)
        if (!s_touch_down) {
            s_touch_down = true;
            // Defer switching apps to app_manager_tick to avoid calling LVGL APIs from indev read_cb
            s_next_app_requested = true;
        }
    } else {
        // Released
        s_touch_down = false;
    }
}
