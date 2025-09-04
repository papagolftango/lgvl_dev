#ifndef PROVISIONING_SERVER_H
#define PROVISIONING_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

void provisioning_server_start(void);
void provisioning_server_stop(void);
void provisioning_server_reset(void); // <-- Added function declaration here

// Optionally, add a callback type for when credentials are received
typedef void (*provisioning_server_credentials_cb_t)(const char* ssid, const char* password, const char* mqtt_host, const char* mqtt_user, const char* mqtt_pass);
void provisioning_server_set_callback(provisioning_server_credentials_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // PROVISIONING_SERVER_H
