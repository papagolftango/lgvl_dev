#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void wifi_manager_start_provisioning(void);
bool wifi_manager_is_provisioned(void);
void wifi_manager_connect(void);
void wifi_manager_set_credentials(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
