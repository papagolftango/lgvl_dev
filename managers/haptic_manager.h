#pragma once
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize DRV2605L on the shared I2C bus. Call after i2c_master_Init().
esp_err_t haptic_manager_init(void);

// Play a single DRV2605L library effect (1..123). Non-blocking.
esp_err_t haptic_manager_play(uint8_t effect);

// Convenience presets
static inline void haptic_click(void){ haptic_manager_play(1); }       // short click
static inline void haptic_confirm(void){ haptic_manager_play(15); }    // confirm pulse
static inline void haptic_alert(void){ haptic_manager_play(47); }      // ramp

#ifdef __cplusplus
}
#endif
