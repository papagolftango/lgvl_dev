#include <lvgl.h>
#include "screens/ui_Energy.h"
#include "energy_controller.h"
#include <math.h>
#include "energy_app.h" /*for balance variable, if needed*/
#include "mqtt_manager.h"
#include "lvgl_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h" /*For esp_mqtt_event_handle_t, MQTT_EVENT_*, esp_mqtt_client_subscribe*/

#define TAG "energy_app"

/*Arc color definitions*/
#define ARC_COLOR_GREEN  0x40FF6D
#define ARC_COLOR_RED    0xFF4040
#define ARC_COLOR_ORANGE 0xFFA500

/**
 * Update peak marker for Bar2 (used energy)
 * @param peak_value the peak used energy value to display
 */
static void ui_update_bar2_peak_marker(float peak_value)
{
    /*TODO: Implement actual peak marker update for Bar2 (used)*/
    /*Example: set a custom indicator or annotation on ui_Bar2*/
    ESP_LOGD(TAG, "ui_update_bar2_peak_marker: peak=%.2f", peak_value);
}

/**
 * Update peak marker for Bar1 (solar energy)
 * @param peak_value the peak solar energy value to display
 */
static void ui_update_bar1_peak_marker(float peak_value)
{
    /*TODO: Implement actual peak marker update for Bar1 (solar)*/
    /*Example: set a custom indicator or annotation on ui_Bar1*/
    ESP_LOGD(TAG, "ui_update_bar1_peak_marker: peak=%.2f", peak_value);
}


/**
 * Update all UI elements from the energy model data
 * This function is called periodically to refresh the display
 */
void energy_controller_tick(void)
{
    extern void draw_pointer_for_balance(float energy_balance);
    extern float energy_balance, energy_solar, energy_used;
    
    if(!energy_app_is_screen_active()) {
        ESP_LOGW(TAG, "energy_controller_tick called but screen_active is false. Skipping UI update.");
        return;
    }
    
    ESP_LOGD(TAG, "energy_controller_tick: energy_balance=%.2f energy_solar=%.2f energy_used=%.2f", 
             energy_balance, energy_solar, energy_used);
    
    /*Update the balance pointer display*/
    draw_pointer_for_balance(energy_balance);
    
    /*Update peak markers for both solar and used energy bars*/
    ui_update_bar2_peak_marker(energy_peak_used);
    ui_update_bar1_peak_marker(energy_peak_solar);
}

/**
 * Handle VRMS energy data from MQTT
 * @param payload the string payload containing the VRMS value
 */
static void handle_vrms_data(const char *payload)
{
    ESP_LOGI(TAG, "Received vrms: %s", payload);
    energy_vrms = strtof(payload, NULL);
    lvgl_manager_set_vrms(payload);
}

/**
 * Handle solar energy data from MQTT
 * @param payload the string payload containing the solar energy value
 */
static void handle_solar_data(const char *payload)
{
    float value = strtof(payload, NULL);
    ESP_LOGI(TAG, "Processed solar: %.2f", value);
    energy_solar = value;
    if(value > energy_peak_solar) {
        energy_peak_solar = value;
        ESP_LOGI(TAG, "New peak solar: %.2f", energy_peak_solar);
    }
    /*TODO: update UI with solar value*/
}

/**
 * Handle used energy data from MQTT
 * @param payload the string payload containing the used energy value
 */
static void handle_used_data(const char *payload)
{
    float value = strtof(payload, NULL);
    ESP_LOGI(TAG, "Processed used: %.2f", value);
    energy_used = value;
    if(value > energy_peak_used) {
        energy_peak_used = value;
        ESP_LOGI(TAG, "New peak used: %.2f", energy_peak_used);
    }
    /*TODO: update UI with used value*/
}

/**
 * Handle balance energy data from MQTT
 * @param payload the string payload containing the balance value
 */
static void handle_balance_data(const char *payload)
{
    float value = strtof(payload, NULL);
    ESP_LOGI(TAG, "Processed balance: %.2f", value);
    energy_balance = value;
    /*Update UI arc via controller*/
    energy_controller_update_balance((int)value);
}

/**
 * Process MQTT data message by dispatching to appropriate handler
 * @param event the MQTT event containing topic and data
 */
static void process_mqtt_data_message(esp_mqtt_event_handle_t event)
{
    char topic[128] = {0};
    char payload[128] = {0};
    int tlen = event->topic_len < sizeof(topic)-1 ? event->topic_len : sizeof(topic)-1;
    int dlen = event->data_len < sizeof(payload)-1 ? event->data_len : sizeof(payload)-1;
    
    strncpy(topic, event->topic, tlen);
    topic[tlen] = '\0';
    strncpy(payload, event->data, dlen);
    payload[dlen] = '\0';
    
    if(strcmp(topic, "emon/emontx3/vrms") == 0) {
        handle_vrms_data(payload);
    } else if(strcmp(topic, "emon/emontx3/solar") == 0) {
        handle_solar_data(payload);
    } else if(strcmp(topic, "emon/emontx3/used") == 0) {
        handle_used_data(payload);
    } else if(strcmp(topic, "emon/emontx3/balance") == 0) {
        handle_balance_data(payload);
    }
}

/**
 * Subscribe to all energy-related MQTT topics
 * @param client the MQTT client handle
 */
static void subscribe_to_energy_topics(esp_mqtt_client_handle_t client)
{
    esp_mqtt_client_subscribe(client, "emon/emontx3/vrms", 0);
    esp_mqtt_client_subscribe(client, "emon/emontx3/solar", 0);
    esp_mqtt_client_subscribe(client, "emon/emontx3/used", 0);
    esp_mqtt_client_subscribe(client, "emon/emontx3/balance", 0);
}

/**
 * Main MQTT event handler for energy application
 * @param handler_args optional handler arguments (unused)
 * @param base the event base
 * @param event_id the specific event ID
 * @param event_data pointer to event data
 */
static void energy_app_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch(event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected (energy app)");
            subscribe_to_energy_topics(event->client);
            break;
        case MQTT_EVENT_DATA:
            if(event->topic_len && event->data_len) {
                process_mqtt_data_message(event);
            }
            break;
        default:
            break;
    }
}

/**
 * Initialize the energy MQTT handlers
 * Registers the energy app MQTT event handler with the MQTT manager
 */
void energy_app_mqtt_init(void)
{
    mqtt_manager_register_app_event_handler(energy_app_mqtt_event_handler, NULL);
}

/**
 * Initialize the energy controller
 * Start controller logic, timers, event handlers, etc.
 */
void energy_controller_init(void)
{
    /*Start controller logic, timers, event handlers, etc.*/
    /*(Implement as needed)*/
}

/**
 * Clean up the energy controller resources
 * Stop controller logic, timers, event handlers, etc.
 */
void energy_controller_cleanup(void)
{
    /*Stop controller logic, timers, event handlers, etc.*/
    /*(Implement as needed)*/
    /*No need to access screen_active directly; use app destroy/cleanup logic if needed.*/
}

/**
 * Update the UI arc to reflect the current balance value
 * @param balance the energy balance value to display
 */
void energy_controller_update_balance(int balance)
{
    draw_pointer_for_balance((float)balance);
}

/*Display constants*/
#define DISPLAY_CENTER_X    180
#define DISPLAY_CENTER_Y    180
#define POINTER_RADIUS      140
#define DISPLAY_SIZE        360
#define MIN_BALANCE         -4000.0f
#define MAX_BALANCE         6000.0f
#define MAX_ANGLE           135.0f

/**
 * Calculate the angle for the balance pointer
 * @param energy_balance the energy balance value
 * @return angle in degrees
 */
static float calculate_balance_angle(float energy_balance)
{
    float angle = 0.0f;
    if(energy_balance < 0) {
        float frac = sqrtf(fabsf(energy_balance / MIN_BALANCE));
        angle = -MAX_ANGLE * frac;
    } else if(energy_balance > 0) {
        float frac = sqrtf(energy_balance / MAX_BALANCE);
        angle = MAX_ANGLE * frac;
    }
    return angle;
}

/**
 * Create debug background for the display
 * @param parent the parent LVGL object
 * @return pointer to the created debug background object
 */
static lv_obj_t * create_debug_background(lv_obj_t *parent)
{
    lv_obj_t *debug_bg = lv_obj_create(parent);
    lv_obj_set_size(debug_bg, DISPLAY_SIZE, DISPLAY_SIZE);
    lv_obj_set_pos(debug_bg, 0, 0);
    lv_obj_set_style_bg_color(debug_bg, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(debug_bg, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_width(debug_bg, 0, LV_PART_MAIN);
    lv_obj_move_background(debug_bg);
    return debug_bg;
}

/**
 * Create and style the pointer line
 * @param parent the parent LVGL object
 * @param tip_x the x coordinate of the pointer tip
 * @param tip_y the y coordinate of the pointer tip
 * @return pointer to the created line object
 */
static lv_obj_t * create_pointer_line(lv_obj_t *parent, int tip_x, int tip_y)
{
    static lv_point_t line_points[2];
    static lv_style_t style_line_red;
    
    line_points[0].x = 0;
    line_points[0].y = 0;
    line_points[1].x = tip_x - DISPLAY_CENTER_X;
    line_points[1].y = tip_y - DISPLAY_CENTER_Y;
    
    lv_obj_t *pointer_line = lv_line_create(parent);
    lv_line_set_points(pointer_line, line_points, 2);
    lv_obj_set_pos(pointer_line, DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
    
    lv_style_init(&style_line_red);
    lv_style_set_line_width(&style_line_red, 16);
    lv_style_set_line_color(&style_line_red, lv_color_hex(0xFF0000));
    lv_style_set_line_rounded(&style_line_red, true);
    lv_obj_add_style(pointer_line, &style_line_red, LV_PART_MAIN);
    
    return pointer_line;
}

/**
 * Draw pointer for energy balance on the display
 * @param energy_balance the energy balance value to display
 */
void draw_pointer_for_balance(float energy_balance)
{
    ESP_LOGI("UI", "draw_pointer_for_balance called: input=%.2f, ui_Energy=%p", energy_balance, ui_Energy);
    
    lv_obj_t *parent = lv_scr_act();
    float angle = calculate_balance_angle(energy_balance);
    ESP_LOGI("UI", "draw_pointer_for_balance: scaled angle=%.2f deg", angle);
    
    float rad = angle * (M_PI / 180.0f);
    ESP_LOGI("UI", "draw_pointer_for_balance: rad=%.2f", rad);
    
    /*Calculate pointer tip coordinates*/
    int tip_x = DISPLAY_CENTER_X + (int)(POINTER_RADIUS * sinf(rad));
    int tip_y = DISPLAY_CENTER_Y - (int)(POINTER_RADIUS * cosf(rad));
    ESP_LOGI("UI", "draw_pointer_for_balance: line from (%d,%d) to (%d,%d)", 
             DISPLAY_CENTER_X, DISPLAY_CENTER_Y, tip_x, tip_y);
    
    /*Create debug background and pointer line*/
    create_debug_background(parent);
    create_pointer_line(parent, tip_x, tip_y);
}

