#ifndef LCD_BL_PWM_BSP_H
#define LCD_BL_PWM_BSP_H

// For bool type in C
#include <stdbool.h>


#define  LCD_PWM_MODE_0   0
#define  LCD_PWM_MODE_25  25
#define  LCD_PWM_MODE_50  50
#define  LCD_PWM_MODE_75  75
#define  LCD_PWM_MODE_100 100
#define  LCD_PWM_MODE_125 125
#define  LCD_PWM_MODE_150 150
#define  LCD_PWM_MODE_175 175
#define  LCD_PWM_MODE_200 200
#define  LCD_PWM_MODE_225 225
#define  LCD_PWM_MODE_255 255


#ifdef __cplusplus
extern "C" {
#endif

void lcd_bl_pwm_bsp_init(uint16_t duty);
void setUpduty(uint16_t duty);
// Smoothly fade to target duty over duration_ms milliseconds using LEDC fade
void lcd_bl_pwm_bsp_fade_to(uint16_t duty, uint32_t duration_ms);
// Fade to target duty, optionally waiting for completion (true = block until done)
void lcd_bl_pwm_bsp_fade_to_wait(uint16_t duty, uint32_t duration_ms, bool wait);

#ifdef __cplusplus
}
#endif

#endif