#include <lvgl.h>
#include "ui/screens/energy_screen.h"
#include "energy_controller.h"
#include <math.h>
#include "energy_app.h" // for balance variable, if needed
#include "managers/mqtt_manager.h"
#include "managers/power_manager.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h" // For esp_mqtt_event_handle_t, MQTT_EVENT_*, esp_mqtt_client_subscribe
// Removed time-based throttling; updates only on data change

#include <lvgl.h>
#include <math.h>
#include "esp_log.h"

#define TAG "energy_app"


// Set both main and indicator arc colors so the whole arc is the same color
extern float energy_balance, energy_solar, energy_used;
extern float energy_peak_solar, energy_peak_used;
extern int energy_pulse_count;
extern int cumulative_pulse;

static volatile bool s_dirty = false; // set on MQTT data change, read/cleared in tick

void energy_controller_tick(void) {
    if (!energy_app_is_screen_active()) return;
    if (!s_dirty) return;
    // Clear first to coalesce mid-update changes into next tick
    s_dirty = false;
    draw_pointer_and_peaks(energy_balance, energy_peak_solar, energy_peak_used, energy_solar, energy_used);
}

void energy_controller_next_mode(void) {
    // Rotate through center modes (UI only); keep values the same
    energy_screen_next_mode();
}

void energy_controller_prev_mode(void) {
    energy_screen_prev_mode();
}

static void energy_app_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected (energy app)");
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/solar", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/used", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/balance", 0);
            esp_mqtt_client_subscribe(event->client, "emon/emontx3/pulse", 0);
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

                if (strcmp(topic, "emon/emontx3/solar") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed solar: %.2f", value);
                    if (value != energy_solar) { energy_solar = value; s_dirty = true; }
                    if (value > energy_peak_solar) {
                        energy_peak_solar = value;
                        ESP_LOGI(TAG, "New peak solar: %.2f", energy_peak_solar);
                        s_dirty = true;
                    }
                    // TODO: update UI with solar value
                } else if (strcmp(topic, "emon/emontx3/used") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed used: %.2f", value);
                    if (value != energy_used) { energy_used = value; s_dirty = true; }
                    if (value > energy_peak_used) {
                        energy_peak_used = value;
                        ESP_LOGI(TAG, "New peak used: %.2f", energy_peak_used);
                        s_dirty = true;
                    }
                    // TODO: update UI with used value
                } else if (strcmp(topic, "emon/emontx3/balance") == 0) {
                    float value = strtof(payload, NULL);
                    ESP_LOGI(TAG, "Processed balance: %.2f", value);
                    if (value != energy_balance) { energy_balance = value; s_dirty = true; }
                    // UI update is handled in tick/UI logic
                } else if (strcmp(topic, "emon/emontx3/pulse") == 0) {
                    char *endp = NULL;
                    long n = strtol(payload, &endp, 10);
                    if (endp == payload) {
                        ESP_LOGW(TAG, "Invalid pulse payload: '%s'", payload);
                        break;
                    }
                    if (n < 0) n = 0; // guard against negatives
                    // Baseline handling: if meter reset or first reading after boot, rebase cumulative
                    if (n < cumulative_pulse) {
                        ESP_LOGW(TAG, "Pulse counter decreased (%ld < %d), rebasing cumulative to %ld", n, cumulative_pulse, n);
                        cumulative_pulse = (int)n;
                    } else if (cumulative_pulse == 0 && energy_pulse_count == 0 && n > 0) {
                        // First non-zero reading after boot (no persisted baseline): set baseline to current
                        ESP_LOGI(TAG, "Pulse baseline initialized to %ld", n);
                        cumulative_pulse = (int)n;
                    }
                    if ((int)n != energy_pulse_count) {
                        energy_pulse_count = (int)n;
                        ESP_LOGI(TAG, "Processed pulse: %d (baseline=%d)", energy_pulse_count, cumulative_pulse);
                        s_dirty = true;
                    }
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



