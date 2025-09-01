#include <lvgl.h>
#include "screens/ui_Energy.h"
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

#define ARC_COLOR_GREEN  0x40FF6D
#define ARC_COLOR_RED    0xFF4040
#define ARC_COLOR_ORANGE 0xFFA500

void energy_controller_tick(void) {
    extern void draw_pointer_for_balance(float energy_balance);
    draw_pointer_for_balance(energy_balance);
        // Set arc color based on balance
        lv_color_t arc_color;
        if (energy_balance < 0) {
            arc_color = lv_color_hex(ARC_COLOR_GREEN);
        } else if (energy_balance > 1000) {
            arc_color = lv_color_hex(ARC_COLOR_RED);
        } else {
            arc_color = lv_color_hex(ARC_COLOR_ORANGE);
        }
        // Set both main and indicator arc colors so the whole arc is the same color
    extern float energy_balance, energy_solar, energy_used;
    
    // Update peak marker for Bar2 (used)
    ui_update_bar2_peak_marker(energy_peak_used);
    // Update peak marker for Bar1 (solar)
    ui_update_bar1_peak_marker(energy_peak_solar);

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

#include "../ui/screens/ui_Energy.h"
#include <lvgl.h>
#include <math.h>
#include "esp_log.h"

void draw_pointer_for_balance(float energy_balance) {
    ESP_LOGI("UI", "draw_pointer_for_balance called: input=%.2f, ui_Energy=%p", energy_balance, ui_Energy);
    lv_obj_t *parent = lv_scr_act();
    float min = -4000.0f, max = 6000.0f;
    float angle = 0.0f;
    if (energy_balance < 0) {
        float frac = sqrtf(fabsf(energy_balance / min));
        angle = -135.0f * frac;
    } else if (energy_balance > 0) {
        float frac = sqrtf(energy_balance / max);
        angle = 135.0f * frac;
    }
    ESP_LOGI("UI", "draw_pointer_for_balance: scaled angle=%.2f deg", angle);
    float rad = angle * (M_PI / 180.0f);
    ESP_LOGI("UI", "draw_pointer_for_balance: rad=%.2f", rad);
    // Center for 360x360 round display
    const int cx = 180, cy = 180, r = 140;
    int tip_x = cx + (int)(r * sinf(rad));
    int tip_y = cy - (int)(r * cosf(rad));
    ESP_LOGI("UI", "draw_pointer_for_balance: line from (%d,%d) to (%d,%d)", cx, cy, tip_x, tip_y);
    lv_obj_t *debug_bg = lv_obj_create(parent);
    lv_obj_set_size(debug_bg, 360, 360);
    lv_obj_set_pos(debug_bg, 0, 0);
    lv_obj_set_style_bg_color(debug_bg, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(debug_bg, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_width(debug_bg, 0, LV_PART_MAIN);
    lv_obj_move_background(debug_bg);
    static lv_point_t line_points[2];
    line_points[0].x = 0;
    line_points[0].y = 0;
    line_points[1].x = tip_x - cx;
    line_points[1].y = tip_y - cy;
    lv_obj_t *pointer_line = lv_line_create(parent);
    lv_line_set_points(pointer_line, line_points, 2);
    lv_obj_set_pos(pointer_line, cx, cy);
    static lv_style_t style_line_red;
    lv_style_init(&style_line_red);
    lv_style_set_line_width(&style_line_red, 16);
    lv_style_set_line_color(&style_line_red, lv_color_hex(0xFF0000));
    lv_style_set_line_rounded(&style_line_red, true);
    lv_obj_add_style(pointer_line, &style_line_red, LV_PART_MAIN);
}

