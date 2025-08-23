#include "encoder_manager.h"
#include "../drivers/user_config.h" // Updated include path
#include "../drivers/bidi_switch_knob.h" // Updated include path
#include "app_manager.h"
#include "esp_log.h"

static void knob_left_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned LEFT");
    // Optionally, implement previous app logic here
}

static void knob_right_cb(void *arg, void *data) {
    ESP_LOGI("encoder", "Knob turned RIGHT");
    app_manager_next_app();
}

void encoder_manager_init(void) {
    knob_config_t cfg;
    cfg.gpio_encoder_a = EXAMPLE_ENCODER_ECA_PIN;
    cfg.gpio_encoder_b = EXAMPLE_ENCODER_ECB_PIN;
    knob_handle_t knob = iot_knob_create(&cfg);
    iot_knob_register_cb(knob, KNOB_LEFT, knob_left_cb, NULL);
    iot_knob_register_cb(knob, KNOB_RIGHT, knob_right_cb, NULL);
}
