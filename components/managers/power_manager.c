#include "power_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

static const char *TAG = "power_manager";

// State
static volatile power_state_t s_state = POWER_ACTIVE;
static uint32_t s_timeout_sec = 60; // default
static TimerHandle_t s_idle_timer = NULL;

// Callback registry (single for now; can be extended if needed)
static power_state_cb_t s_cb = NULL;
static void *s_cb_user = NULL;

static void power_manager_set_state(power_state_t new_state) {
    power_state_t old = s_state;
    s_state = new_state;
    if (s_cb && old != new_state) {
        s_cb(new_state, s_cb_user);
    }
}

static void idle_timer_cb(TimerHandle_t xTimer) {
    (void)xTimer;
    ESP_LOGI(TAG, "Inactivity timeout reached; entering IDLE");
    power_manager_set_state(POWER_IDLE);
}

void power_manager_notify_activity(void) {
    // Any user activity keeps ACTIVE and resets timer
    if (s_state != POWER_ACTIVE) {
        power_manager_set_state(POWER_ACTIVE);
    }
    if (s_idle_timer) {
        xTimerStop(s_idle_timer, 0);
        xTimerChangePeriod(s_idle_timer, pdMS_TO_TICKS(s_timeout_sec * 1000), 0);
        xTimerStart(s_idle_timer, 0);
    }
}

void power_manager_register_state_cb(power_state_cb_t cb, void *user) {
    s_cb = cb;
    s_cb_user = user;
}

bool power_manager_is_idle(void) {
    return s_state == POWER_IDLE;
}

void power_manager_set_timeout(uint32_t inactivity_seconds) {
    s_timeout_sec = inactivity_seconds > 0 ? inactivity_seconds : 1;
    if (s_idle_timer) {
        xTimerChangePeriod(s_idle_timer, pdMS_TO_TICKS(s_timeout_sec * 1000), 0);
    }
}

void power_manager_init(uint32_t inactivity_seconds) {
    ESP_LOGI(TAG, "Power manager init, timeout=%us", (unsigned)inactivity_seconds);
    s_timeout_sec = inactivity_seconds > 0 ? inactivity_seconds : s_timeout_sec;
    // Create one-shot timer that transitions to IDLE when it fires
    s_idle_timer = xTimerCreate("pm_idle", pdMS_TO_TICKS(s_timeout_sec * 1000), pdFALSE, NULL, idle_timer_cb);
    if (!s_idle_timer) {
        ESP_LOGE(TAG, "Failed to create idle timer");
        return;
    }
    // Start in ACTIVE and arm timer
    power_manager_set_state(POWER_ACTIVE);
    xTimerStart(s_idle_timer, 0);
}
