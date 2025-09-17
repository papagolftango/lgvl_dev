#include "esp_log.h"
#include "encoder_manager.h"
#include "user_config.h"
#include "bidi_switch_knob.h"
#include "power_manager.h"
#include "encoder_manager.h"

static encoder_user_cb_t s_user_cb = NULL;
static volatile bool s_consume_wake_event = false;

static void knob_left_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned LEFT");
    bool was_idle = power_manager_is_idle();
    power_manager_notify_activity();
    if (was_idle || s_consume_wake_event) {
        // Consume this first event used to wake the system
        s_consume_wake_event = false;
        ESP_LOGI("encoder", "Woke from IDLE via encoder; consuming LEFT event");
        return;
    }
    if (s_user_cb) s_user_cb(ENCODER_EVT_LEFT);
}

static void knob_right_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned RIGHT");
    bool was_idle = power_manager_is_idle();
    power_manager_notify_activity();
    if (was_idle || s_consume_wake_event) {
        s_consume_wake_event = false;
        ESP_LOGI("encoder", "Woke from IDLE via encoder; consuming RIGHT event");
        return;
    }
    if (s_user_cb) s_user_cb(ENCODER_EVT_RIGHT);
}

void encoder_manager_init(void) {
    knob_config_t cfg;
    cfg.gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN;
    cfg.gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN;
    knob_handle_t knob = iot_knob_create(&cfg);
    iot_knob_register_cb(knob, KNOB_LEFT, knob_left_cb, NULL);
    iot_knob_register_cb(knob, KNOB_RIGHT, knob_right_cb, NULL);
}

void encoder_manager_register_user_cb(encoder_user_cb_t cb) { s_user_cb = cb; }
void encoder_manager_unregister_user_cb(void) { s_user_cb = NULL; }
