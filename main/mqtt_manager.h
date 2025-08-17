#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_manager_start_provisioning(void);
bool mqtt_manager_is_provisioned(void);
void mqtt_manager_connect(void);
void mqtt_manager_set_credentials(const char *host, const char *user, const char *pass);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
