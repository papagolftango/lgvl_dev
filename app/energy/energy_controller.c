#include <lvgl.h>
#include "ui/screens/energy_screen.h"
#include "energy_controller.h"
#include <math.h>
#include "energy_app.h" // for balance variable, if needed
#include "managers/mqtt_manager.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h" // For esp_mqtt_event_handle_t, MQTT_EVENT_*, esp_mqtt_client_subscribe

#include <lvgl.h>
#include <math.h>
#include "esp_log.h"

#define TAG "energy_app"


// Set both main and indicator arc colors so the whole arc is the same color
extern float energy_balance, energy_solar, energy_used;
extern float energy_peak_solar, energy_peak_used;

void energy_controller_tick(void) {
    static float last_balance = NAN;
    static float last_solar = NAN;
    static float last_used = NAN;
    if (!energy_app_is_screen_active()) {
        ESP_LOGW(TAG, "energy_controller_tick called but screen_active is false. Skipping UI update.");
        return;
    }
    ESP_LOGD(TAG, "energy_controller_tick: energy_balance=%.2f energy_solar=%.2f energy_used=%.2f", energy_balance, energy_solar, energy_used);

    // Only update UI if values have changed
    // Always update pointer and peaks (could optimize, but peaks may change independently)
   
    draw_pointer_and_peaks(energy_balance, energy_peak_solar, energy_peak_used, energy_solar, energy_used);
    if (energy_used != last_used) {
        energy_screen_set_value((int)energy_used);
        last_used = energy_used;
    }
    // Add more UI updates as needed (e.g., for solar, VRMS, etc.)
}

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
                    energy_vrms = strtof(payload, NULL);
                    // TODO: update VRMS value on UI here
                } else if (strcmp(topic, "emon/emontx3/solar") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed solar: %.2f", value);
                    energy_solar = value;
                    if (value > energy_peak_solar) {
                        energy_peak_solar = value;
                        ESP_LOGI(TAG, "New peak solar: %.2f", energy_peak_solar);
                    }
                    // TODO: update UI with solar value
                } else if (strcmp(topic, "emon/emontx3/used") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed used: %.2f", value);
                    energy_used = value;
                    if (value > energy_peak_used) {
                        energy_peak_used = value;
                        ESP_LOGI(TAG, "New peak used: %.2f", energy_peak_used);
                    }
                    // TODO: update UI with used value
                } else if (strcmp(topic, "emon/emontx3/balance") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed balance: %.2f", value);
                    energy_balance = value;
                    // Update UI arc via controller
                    energy_controller_update_balance((int)value);
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

void energy_controller_init(void) {
    // Start controller logic, timers, event handlers, etc.
    // (Implement as needed)
}

void energy_controller_cleanup(void) {
    // Stop controller logic, timers, event handlers, etc.
    // (Implement as needed)
    // No need to access screen_active directly; use app destroy/cleanup logic if needed.
}

// Update the UI arc to reflect the current balance value
void energy_controller_update_balance(int balance) {

}



