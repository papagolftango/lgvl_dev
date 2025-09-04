#include <lvgl.h>
#include "screens/ui_Energy.h"
#include "energy_controller.h"
#include <math.h>
#include "energy_app.h" // for balance variable, if needed
#include "managers/mqtt_manager.h"
#include "managers/lvgl_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h" // For esp_mqtt_event_handle_t, MQTT_EVENT_*, esp_mqtt_client_subscribe
#include "../ui/screens/ui_Energy.h"
#include <lvgl.h>
#include <math.h>
#include "esp_log.h"

#define TAG "energy_app"


        // Set both main and indicator arc colors so the whole arc is the same color
extern float energy_balance, energy_solar, energy_used;

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
    static float last_balance = NAN;
    if (energy_balance != last_balance) {
        draw_pointer_for_balance(energy_balance);
        last_balance = energy_balance;
    }

    if (!energy_app_is_screen_active()) {
        ESP_LOGW(TAG, "energy_controller_tick called but screen_active is false. Skipping UI update.");
        return;
    }
    ESP_LOGD(TAG, "energy_controller_tick: energy_balance=%.2f energy_solar=%.2f energy_used=%.2f", energy_balance, energy_solar, energy_used);
    // (ui_balance removed: now handled by draw_pointer_for_balance)
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
    draw_pointer_for_balance((float)balance);
}



// Persistent pointer line object and style
static lv_obj_t *pointer_line = NULL;
static lv_point_t line_points[2];
static lv_style_t style_line_blue;
static bool style_initialized = false;
static int sweep_angle = -135;

void draw_pointer_for_balance(float energy_balance) {

    // Map: -4kW (FSD left) to -135°, 0kW (center) to 0°, +6kW (FSD right) to +135°
    // Use sqrt scale for both sides
    float angle = 0.0f;
    if (energy_balance < 0.0f) {
        // Negative: map [-4000, 0] W to [-135, 0] deg
        float frac = fminf(1.0f, sqrtf(fabsf(energy_balance) / 4000.0f));
        angle = -135.0f * frac;
    } else if (energy_balance > 0.0f) {
        // Positive: map [0, 6000] W to [0, +135] deg
        float frac = fminf(1.0f, sqrtf(energy_balance / 6000.0f));
        angle = 135.0f * frac;
    } else {
        angle = 0.0f;
    }

    float rad = angle * (M_PI / 180.0f);
    int r = 125;
    int x0 = r, y0 = r; // center of the object
    int x1 = r + (int)roundf(r * sinf(rad));
    int y1 = r - (int)roundf(r * cosf(rad));
    line_points[0].x = x0;
    line_points[0].y = y0;
    line_points[1].x = x1;
    line_points[1].y = y1;

    lv_obj_t *parent = lv_scr_act();
    if (!pointer_line) {
        pointer_line = lv_line_create(parent);
        lv_obj_set_pos(pointer_line, 180 - r, 180 - r);
        lv_obj_set_size(pointer_line, 2*r, 2*r);
        lv_obj_move_foreground(pointer_line);
        if (!style_initialized) {
            lv_style_init(&style_line_blue);
            lv_style_set_line_width(&style_line_blue, 5);
            lv_style_set_line_color(&style_line_blue, lv_color_hex(0x0000FF));
            lv_style_set_line_rounded(&style_line_blue, true);
            style_initialized = true;
        }
        lv_obj_add_style(pointer_line, &style_line_blue, LV_PART_MAIN);
    }
    lv_line_set_points(pointer_line, line_points, 2);
}