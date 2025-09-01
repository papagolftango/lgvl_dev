#include "haptic_manager.h"
#include "i2c_bsp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "haptic_manager"
#define DRV_ADDR 0x5A

// DRV2605L registers
#define REG_MODE        0x01
#define REG_WAVESEQ1    0x03
#define REG_GO          0x0C
#define REG_FEEDBACK    0x1A
#define REG_CTRL3       0x1B
#define REG_RATED_V     0x16
#define REG_OD_CLAMP    0x17

static uint8_t wr(uint8_t reg, uint8_t val){
    return i2c_write_buff(drv2605_dev_handle, reg, &val, 1);
}

esp_err_t haptic_manager_init(void){
    // drv2605_dev_handle is created by i2c_master_Init() using default addr 0x5A
    if(drv2605_dev_handle == NULL){
        ESP_LOGE(TAG, "drv2605_dev_handle is NULL. Call i2c_master_Init() first.");
        return ESP_FAIL;
    }
    // Quick bring-up for LRA in open-loop (good enough to start)
    wr(REG_MODE, 0x00);       // out of standby, internal trigger
    wr(REG_RATED_V, 0x7F);    // tune later
    wr(REG_OD_CLAMP, 0x3F);   // tune later
    wr(REG_CTRL3, 0xA0);      // LRA open-loop
    wr(REG_WAVESEQ1, 0x00);   // clear
    return ESP_OK;
}

esp_err_t haptic_manager_play(uint8_t effect){
    if(effect == 0) return ESP_ERR_INVALID_ARG;
    wr(REG_WAVESEQ1, effect);
    wr(REG_GO, 1);
    return ESP_OK;
}
