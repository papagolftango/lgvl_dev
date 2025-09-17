#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void encoder_manager_init(void);

typedef enum {
	ENCODER_EVT_LEFT = 0,
	ENCODER_EVT_RIGHT = 1,
} encoder_event_t;

typedef void (*encoder_user_cb_t)(encoder_event_t evt);

void encoder_manager_register_user_cb(encoder_user_cb_t cb);
void encoder_manager_unregister_user_cb(void);

