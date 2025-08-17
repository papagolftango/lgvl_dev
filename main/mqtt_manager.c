#include <float.h>
#include <string.h>
#include "esp_log.h"
// Store credentials in static variables for now
static char s_mqtt_host[128] = {0};
static char s_mqtt_user[64] = {0};
static char s_mqtt_pass[64] = {0};

void mqtt_manager_set_credentials(const char *host, const char *user, const char *pass) {
    strncpy(s_mqtt_host, host, sizeof(s_mqtt_host) - 1);
    s_mqtt_host[sizeof(s_mqtt_host) - 1] = '\0';
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

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("mqtt_manager", "MQTT connected");
            // Subscribe to the topics on connect
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/vrms", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/solar", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/used", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/balance", 0);
            break;
        case MQTT_EVENT_DATA:
            if (event->topic_len && event->data_len) {
                char topic[128] = {0};
                char payload[128] = {0};
                int tlen = event->topic_len < sizeof(topic)-1 ? event->topic_len : sizeof(topic)-1;
                int dlen = event->data_len < sizeof(payload)-1 ? event->data_len : sizeof(payload)-1;
                strncpy(topic, event->topic, tlen);
                topic[tlen] = '\0';
                strncpy(payload, event->data, dlen);
                payload[dlen] = '\0';
                if (strcmp(topic, "emon/emontx3/vrms") == 0) {
                    ESP_LOGI("mqtt_manager", "Received vrms: %s", payload);
                    extern void lvgl_manager_set_vrms(const char *vrms);
                    lvgl_manager_set_vrms(payload);
                } else if (strcmp(topic, "emon/emontx3/solar") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI("mqtt_manager", "Processed solar: %.2f", value);
                    if (value > max_solar) {
                        max_solar = value;
                        ESP_LOGI("mqtt_manager", "New max solar: %.2f", max_solar);
                    }
                } else if (strcmp(topic, "emon/emontx3/used") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI("mqtt_manager", "Processed used: %.2f", value);
                    if (value > max_used) {
                        max_used = value;
                        ESP_LOGI("mqtt_manager", "New max used: %.2f", max_used);
                    }
                } else if (strcmp(topic, "emon/emontx3/balance") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI("mqtt_manager", "Processed balance: %.2f", value);
                }
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW("mqtt_manager", "MQTT disconnected");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE("mqtt_manager", "MQTT error");
            break;
        default:
            break;
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
