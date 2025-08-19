#include "mqtt_client.h"

typedef void (*mqtt_app_event_handler_t)(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_manager_register_app_event_handler(mqtt_app_event_handler_t handler, void *handler_args);

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H


#ifdef __cplusplus
extern "C" {
#endif

void mqtt_manager_set_credentials(const char *host, const char *user, const char *pass);

bool mqtt_manager_is_provisioned(void);
void mqtt_manager_reset_max_solar(void);
void mqtt_manager_reset_max_used(void);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
