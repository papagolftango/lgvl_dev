
#include "esp_log.h"
#include "lvgl_manager.h"
#include "lvgl.h"
#include <string.h>

lv_obj_t *vrms_label = NULL;
lv_obj_t *vrms_value_label = NULL;

void lvgl_manager_set_vrms(const char *vrms) {
    ESP_LOGI("lvgl_manager", "lvgl_manager_set_vrms called with: '%s'", vrms ? vrms : "(null)");
    lvgl_manager_lock();
    if (vrms_value_label && vrms) {
        // Format to 1 decimal place
        float value = atof(vrms);
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", value);
        lv_label_set_text(vrms_value_label, buf);
    } else {
        ESP_LOGW("lvgl_manager", "vrms_value_label is NULL or vrms is NULL");
    }
    lvgl_manager_unlock();
}

#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

#include "lvgl_manager.h"
#include "../drivers/user_config.h"
#include "../drivers/display_driver.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define LVGL_TICK_PERIOD_MS 2
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static SemaphoreHandle_t lvgl_mux = NULL;
static void lvgl_increase_tick(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static esp_timer_handle_t lvgl_tick_timer = NULL;

void lvgl_manager_start_tick_timer(void)
{
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_increase_tick,
        .name = "lvgl_tick"
    };
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));
}

static bool lvgl_lock(int timeout_ms)
{
    assert(lvgl_mux && "lvgl_manager_init must be called first");
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

static void lvgl_unlock(void)
{
    assert(lvgl_mux && "lvgl_manager_init must be called first");
    xSemaphoreGive(lvgl_mux);
}

void lvgl_manager_lock(void) {
    lvgl_lock(-1);
}

void lvgl_manager_unlock(void) {
    lvgl_unlock();
}

static void lvgl_port_task(void *arg)
{
    ESP_LOGI("lvgl_manager", "Starting LVGL task");
    while (1) {
        uint32_t delay = LVGL_TASK_MAX_DELAY_MS;
        if (lvgl_lock(-1)) {
            delay = lv_timer_handler();
            lvgl_unlock();
            if (delay > LVGL_TASK_MAX_DELAY_MS) {
                delay = LVGL_TASK_MAX_DELAY_MS;
            } else if (delay < LVGL_TASK_MIN_DELAY_MS) {
                static int warn_count = 0;
                warn_count++;
                if (warn_count >= 100) {
                    ESP_LOGW("lvgl_manager", "LVGL task_delay_ms too low (%d), forcing minimum.", delay);
                    warn_count = 0;
                }
                delay = LVGL_TASK_MIN_DELAY_MS;
            }
        } else {
            delay = LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

void lvgl_manager_start_task(void)
{
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);
}

lv_disp_t *lvgl_manager_init(esp_lcd_panel_handle_t panel_handle) {

    lv_init();

    // Allocate draw buffers used by LVGL
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(LCD_H_RES * LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * LVGL_BUF_HEIGHT);

    // Register display driver to LVGL
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = display_driver_flush_cb;
    disp_drv.rounder_cb = display_driver_rounder_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    // Set the screen background to black for dark theme (must be after disp is registered)
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    // Create mutex for LVGL locking
    lvgl_mux = xSemaphoreCreateMutex();
    assert(lvgl_mux);


    // Place both labels directly on the screen, stacked and centered
    // Use the largest available font for both labels
        /* Create the VRMS label */
        vrms_label = lv_label_create(lv_scr_act());
        lv_label_set_text(vrms_label, "VRMS");
        lv_obj_set_style_text_color(vrms_label, lv_color_white(), 0);
        /* Use a medium font for the label */
    #if LV_FONT_MONTSERRAT_24
        lv_obj_set_style_text_font(vrms_label, &lv_font_montserrat_24, 0);
    #elif LV_FONT_MONTSERRAT_16
        lv_obj_set_style_text_font(vrms_label, &lv_font_montserrat_16, 0);
    #endif
        /* Center the label near the top */
        lv_obj_align(vrms_label, LV_ALIGN_TOP_MID, 0, 40);

        /* Create the VRMS value label */
        vrms_value_label = lv_label_create(lv_scr_act());
        lv_label_set_text(vrms_value_label, "0.0");
        lv_obj_set_style_text_color(vrms_value_label, lv_color_white(), 0);
        /* Use the largest font for the value */
    #if LV_FONT_MONTSERRAT_48
        lv_obj_set_style_text_font(vrms_value_label, &lv_font_montserrat_48, 0);
    #elif LV_FONT_MONTSERRAT_24
        lv_obj_set_style_text_font(vrms_value_label, &lv_font_montserrat_24, 0);
    #elif LV_FONT_MONTSERRAT_16
        lv_obj_set_style_text_font(vrms_value_label, &lv_font_montserrat_16, 0);
    #endif
        /* Center the value label and make it take up 1/4 of the screen height */
        lv_obj_align(vrms_value_label, LV_ALIGN_CENTER, 0, 40);
        lv_obj_set_width(vrms_value_label, lv_pct(100));
        lv_obj_set_style_text_align(vrms_value_label, LV_TEXT_ALIGN_CENTER, 0);
        /* Optionally, set a minimum height to ensure it takes up space */
        lv_obj_set_height(vrms_value_label, lv_pct(25));

    return disp;
}

