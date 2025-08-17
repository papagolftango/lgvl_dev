
#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

#include "lvgl_manager.h"
#include "user_config.h"
#include "display_driver.h"

#ifdef __cplusplus
extern "C" {
#endif
void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area);
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
                ESP_LOGW("lvgl_manager", "LVGL task_delay_ms too low (%d), forcing minimum.", delay);
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

    // Create mutex for LVGL locking
    lvgl_mux = xSemaphoreCreateMutex();
    assert(lvgl_mux);

    return disp;
}

