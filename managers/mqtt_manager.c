#include <float.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

// Store credentials in static variables for now
static char s_mqtt_host[128] = {0};
static char s_mqtt_user[64] = {0};
static char s_mqtt_pass[64] = {0};

void mqtt_manager_set_credentials(const char *host, const char *user, const char *pass) {
    // If host does not start with mqtt:// or mqtts://, prepend mqtt://
    if (strncmp(host, "mqtt://", 7) != 0 && strncmp(host, "mqtts://", 8) != 0) {
        snprintf(s_mqtt_host, sizeof(s_mqtt_host), "mqtt://%s", host);
    } else {
        strncpy(s_mqtt_host, host, sizeof(s_mqtt_host) - 1);
        s_mqtt_host[sizeof(s_mqtt_host) - 1] = '\0';
    }
    strncpy(s_mqtt_user, user, sizeof(s_mqtt_user) - 1);
    s_mqtt_user[sizeof(s_mqtt_user) - 1] = '\0';
    strncpy(s_mqtt_pass, pass, sizeof(s_mqtt_pass) - 1);
    s_mqtt_pass[sizeof(s_mqtt_pass) - 1] = '\0';
}
#include "mqtt_manager.h"
#include "provisioning_server.h"

void mqtt_manager_start_provisioning(void) {
    // Start provisioning web server for MQTT credentials
    provisioning_server_start();
}

bool mqtt_manager_is_provisioned(void) {
    // TODO: Check NVS for stored MQTT credentials
    return false;
}




#include "mqtt_client.h"

static esp_mqtt_client_handle_t s_mqtt_client = NULL;

// Track max values for solar and used
static float max_solar = -FLT_MAX;
static float max_used = -FLT_MAX;

void mqtt_manager_reset_max_solar(void) {
    max_solar = -FLT_MAX;
}

void mqtt_manager_reset_max_used(void) {
    max_used = -FLT_MAX;
}


// App event handler registration
static mqtt_app_event_handler_t s_app_event_handler = NULL;
static void *s_app_event_handler_args = NULL;

void mqtt_manager_register_app_event_handler(mqtt_app_event_handler_t handler, void *handler_args) {
    s_app_event_handler = handler;
    s_app_event_handler_args = handler_args;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    if (s_app_event_handler) {
        s_app_event_handler(s_app_event_handler_args, base, event_id, event_data);
    }
}

void mqtt_manager_connect(void) {
    ESP_LOGI("mqtt_manager", "Connecting to MQTT: HOST='%s', USER='%s', PASS='%s'", s_mqtt_host, s_mqtt_user, s_mqtt_pass);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = s_mqtt_host,
        .credentials.username = s_mqtt_user,
        .credentials.authentication.password = s_mqtt_pass,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        ESP_LOGE("mqtt_manager", "Failed to create MQTT client");
        return;
    }
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);
}
