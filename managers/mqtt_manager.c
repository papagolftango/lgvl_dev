
// Standard C
#include <float.h>
#include <stdio.h>
#include <string.h>

// ESP-IDF
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "mqtt_client.h"

// Project headers
#include "mqtt_manager.h"
#include "provisioning_server.h"

#define MQTT_NVS_NAMESPACE "mqtt_cfg"
#define MQTT_NVS_KEY_HOST "host"
#define MQTT_NVS_KEY_USER "user"
#define MQTT_NVS_KEY_PASS "pass"

static char s_mqtt_host[128] = {0};
static char s_mqtt_user[64] = {0};
static char s_mqtt_pass[64] = {0};

static esp_err_t mqtt_manager_save_credentials_to_nvs(const char *host, const char *user, const char *pass) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(MQTT_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;
    err = nvs_set_str(nvs_handle, MQTT_NVS_KEY_HOST, host);
    if (err == ESP_OK) err = nvs_set_str(nvs_handle, MQTT_NVS_KEY_USER, user);
    if (err == ESP_OK) err = nvs_set_str(nvs_handle, MQTT_NVS_KEY_PASS, pass);
    if (err == ESP_OK) err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}
static esp_err_t mqtt_manager_load_credentials_from_nvs(char *host, size_t host_len, char *user, size_t user_len, char *pass, size_t pass_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(MQTT_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;
    err = nvs_get_str(nvs_handle, MQTT_NVS_KEY_HOST, host, &host_len);
    if (err == ESP_OK) err = nvs_get_str(nvs_handle, MQTT_NVS_KEY_USER, user, &user_len);
    if (err == ESP_OK) err = nvs_get_str(nvs_handle, MQTT_NVS_KEY_PASS, pass, &pass_len);
    nvs_close(nvs_handle);
    return err;
}

// Extra MQTT event debug logging
static void mqtt_debug_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW("mqtt_debug", "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE("mqtt_debug", "MQTT_EVENT_ERROR: error_type=%d", event->error_handle->error_type);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT_DATA: topic=%.*s, data=%.*s", event->topic_len, event->topic, event->data_len, event->data);
            break;
        default:
            ESP_LOGI("mqtt_debug", "MQTT_EVENT id=%d", event_id);
            break;
    }
}
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
    mqtt_manager_save_credentials_to_nvs(s_mqtt_host, s_mqtt_user, s_mqtt_pass);
}


void mqtt_manager_start_provisioning(void) {
    // Start provisioning web server for MQTT credentials
    provisioning_server_start();
}

bool mqtt_manager_is_provisioned(void) {
    char nvs_host[128] = {0};
    char nvs_user[64] = {0};
    char nvs_pass[64] = {0};
    esp_err_t nvs_err = mqtt_manager_load_credentials_from_nvs(nvs_host, sizeof(nvs_host), nvs_user, sizeof(nvs_user), nvs_pass, sizeof(nvs_pass));
    return (nvs_err == ESP_OK && strlen(nvs_host) > 0);
}

void mqtt_manager_load_credentials(void) {
    char nvs_host[128] = {0};
    char nvs_user[64] = {0};
    char nvs_pass[64] = {0};
    esp_err_t nvs_err = mqtt_manager_load_credentials_from_nvs(nvs_host, sizeof(nvs_host), nvs_user, sizeof(nvs_user), nvs_pass, sizeof(nvs_pass));
    if (nvs_err == ESP_OK) {
        strncpy(s_mqtt_host, nvs_host, sizeof(s_mqtt_host) - 1);
        s_mqtt_host[sizeof(s_mqtt_host) - 1] = '\0';
        strncpy(s_mqtt_user, nvs_user, sizeof(s_mqtt_user) - 1);
        s_mqtt_user[sizeof(s_mqtt_user) - 1] = '\0';
        strncpy(s_mqtt_pass, nvs_pass, sizeof(s_mqtt_pass) - 1);
        s_mqtt_pass[sizeof(s_mqtt_pass) - 1] = '\0';
    }
}






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
    // Register both the app handler and debug handler
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_debug_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);
}
