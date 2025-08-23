
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdint.h>
#include <esp_event.h>

typedef void (*mqtt_app_event_handler_t)(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_manager_set_credentials(const char *host, const char *user, const char *pass);
void mqtt_manager_register_app_event_handler(mqtt_app_event_handler_t handler, void *handler_args);

#endif // MQTT_MANAGER_H
