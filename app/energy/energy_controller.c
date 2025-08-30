#include <lvgl.h>
#include "ui/screens/ui_Energy.h"
#include "energy_controller.h"
#include <math.h>
#include "energy_app.h" // for balance variable, if needed
#include "mqtt_manager.h"
#include "lvgl_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h" // For esp_mqtt_event_handle_t, MQTT_EVENT_*, esp_mqtt_client_subscribe

#define TAG "energy_app"



// Local helpers to update peak markers for the bars
static void ui_update_bar2_peak_marker(float peak_value) {
    // TODO: Implement actual peak marker update for Bar2 (used)
    // Example: set a custom indicator or annotation on ui_Bar2
    ESP_LOGD(TAG, "ui_update_bar2_peak_marker: peak=%.2f", peak_value);
}

static void ui_update_bar1_peak_marker(float peak_value) {
    // TODO: Implement actual peak marker update for Bar1 (solar)
    // Example: set a custom indicator or annotation on ui_Bar1
    ESP_LOGD(TAG, "ui_update_bar1_peak_marker: peak=%.2f", peak_value);
}


// Update all UI elements from the model (tick)
void energy_controller_tick(void) {
    extern float energy_balance, energy_solar, energy_used;
    
    // Update peak marker for Bar2 (used)
    ui_update_bar2_peak_marker(energy_peak_used);
    // Update peak marker for Bar1 (solar)
    ui_update_bar1_peak_marker(energy_peak_solar);

    if (!energy_app_is_screen_active()) {
        ESP_LOGW(TAG, "energy_controller_tick called but screen_active is false. Skipping UI update.");
        return;
    }
    ESP_LOGD(TAG, "energy_controller_tick: ui_balance=%p ui_Bar1=%p ui_Bar2=%p", ui_balance, ui_Bar1, ui_Bar2);
    ESP_LOGD(TAG, "energy_controller_tick: energy_balance=%.2f energy_solar=%.2f energy_used=%.2f", energy_balance, energy_solar, energy_used);
    if (ui_balance) {
        ESP_LOGD(TAG, "Updating ui_balance arc (sqrt scale)");
        // Square root scale: preserve sign, compress extremes
        float abs_val = fabsf(energy_balance);
        float scaled = sqrtf(abs_val);
        float max_in = 6000.0f;
        float max_out = sqrtf(max_in);
        float arc_val = (energy_balance >= 0) ? (scaled * max_in / max_out) : -(scaled * max_in / max_out);
        lv_arc_set_value(ui_balance, (int)arc_val);
    }
    if (ui_Bar1) {
        ESP_LOGD(TAG, "Updating ui_Bar1 bar (sqrt scale)");
        float abs_val = fabsf(energy_solar);
        float scaled = sqrtf(abs_val);
        float max_in = 4000.0f;
        float max_out = sqrtf(max_in);
        float bar_val = scaled * max_in / max_out;
        lv_bar_set_value(ui_Bar1, (int)bar_val, LV_ANIM_OFF);
    }
    if (ui_Bar2) {
        ESP_LOGD(TAG, "Updating ui_Bar2 bar (sqrt scale)");
        float abs_val = fabsf(energy_used);
        float scaled = sqrtf(abs_val);
        float max_in = 6000.0f;
        float max_out = sqrtf(max_in);
        float bar_val = scaled * max_in / max_out;
        lv_bar_set_value(ui_Bar2, (int)bar_val, LV_ANIM_OFF);
    }
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
                    lvgl_manager_set_vrms(payload);
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
    if (ui_balance) {
        lv_arc_set_value(ui_balance, balance);
        ESP_LOGD(TAG, "energy_controller_update_balance: set ui_balance to %d", balance);
    }
}

