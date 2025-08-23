#include "encoder_manager.h"
#include "../../drivers/user_config.h"
#include "../../drivers/bidi_switch_knob.h"
#include "../app_manager.h"
#include "esp_log.h"

#include "encoder_manager.h"
#include "user_config.h"
#include "bidi_switch_knob.h"
#include "app_manager.h"

static void knob_left_cb(void *arg)
{
    app_manager_send_event(APP_EVENT_ENCODER_LEFT, NULL);
}

static void knob_right_cb(void *arg)
{
    app_manager_send_event(APP_EVENT_ENCODER_RIGHT, NULL);
}

void encoder_manager_init(void)
{
    bidi_switch_knob_set_left_callback(knob_left_cb, NULL);
    bidi_switch_knob_set_right_callback(knob_right_cb, NULL);
}
