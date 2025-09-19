#include <stdio.h>
#include "lcd_bl_pwm_bsp.h"
#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// Synchronize fade operations across threads
static SemaphoreHandle_t s_fade_mutex = NULL;
// Track LEDC init state (must be declared before first use)
static bool s_ledc_inited = false;

void gpio_init(void)
{
  gpio_config_t gpio_conf = {};
  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_OUTPUT;
  gpio_conf.pin_bit_mask = ((uint64_t)0X01<<EXAMPLE_PIN_NUM_BK_LIGHT);
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));
}
void lcd_bl_pwm_bsp_init(uint16_t duty)
{ 
  ledc_timer_config_t timer_conf = 
  {
    .speed_mode =  LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT, //256
    .timer_num =  LEDC_TIMER_3,
    .freq_hz = 50 * 1000,
    .clk_cfg = LEDC_SLOW_CLK_RC_FAST,
  };
  ledc_channel_config_t ledc_conf = 
  {
    .gpio_num = EXAMPLE_PIN_NUM_BK_LIGHT,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel =  LEDC_CHANNEL_1,
    .intr_type =  LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_3,
    .duty = duty,   //占空比
    .hpoint = 0,    //相位
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&timer_conf));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&ledc_conf));
  // Install LEDC fade service once
  static bool s_fade_installed = false;
  if (!s_fade_installed) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_fade_func_install(0));
    s_fade_installed = true;
  }
  // Mark initialized for later guards
  s_ledc_inited = true;
  if (s_fade_mutex == NULL) {
    s_fade_mutex = xSemaphoreCreateMutex();
  }
}

void setUpduty(uint16_t duty)
{
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}

void lcd_bl_pwm_bsp_fade_to(uint16_t duty, uint32_t duration_ms)
{
  // Non-blocking convenience wrapper
  lcd_bl_pwm_bsp_fade_to_wait(duty, duration_ms, false);
}

void lcd_bl_pwm_bsp_fade_to_wait(uint16_t duty, uint32_t duration_ms, bool wait)
{
  // Configure fade to target duty over duration_ms milliseconds
  // Guard against calling before init to avoid crashes
  if (!s_ledc_inited) {
    // Try a best-effort minimal init if missed
    // Note: rely on lcd_bl_pwm_bsp_init in normal flow; this is a safety net
    // to prevent null ISR/fade state when power callbacks race with init.
    lcd_bl_pwm_bsp_init(duty);
  }
  if (s_fade_mutex) { xSemaphoreTake(s_fade_mutex, portMAX_DELAY); }
  esp_err_t err = ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty, duration_ms);
  ESP_ERROR_CHECK_WITHOUT_ABORT(err);
  if (err == ESP_OK) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, wait ? LEDC_FADE_WAIT_DONE : LEDC_FADE_NO_WAIT));
  }
  if (s_fade_mutex) { xSemaphoreGive(s_fade_mutex); }
}