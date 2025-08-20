#include "energy_app.h"
#include "mqtt_manager.h"
#include "lvgl_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

#define TAG "energy_app"

static void energy_app_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected (energy app)");
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
                    ESP_LOGI(TAG, "Received vrms: %s", payload);
                    lvgl_manager_set_vrms(payload);
                } else if (strcmp(topic, "emon/emontx3/solar") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed solar: %.2f", value);
                    // TODO: update UI with solar value
                } else if (strcmp(topic, "emon/emontx3/used") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed used: %.2f", value);
                    // TODO: update UI with used value
                } else if (strcmp(topic, "emon/emontx3/balance") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed balance: %.2f", value);
                    // TODO: update UI with balance value
                }
            }
            break;
        default:
            break;
    }
}

void energy_app_mqtt_init(void) {
    mqtt_manager_register_app_event_handler(energy_app_mqtt_event_handler, NULL);
}
