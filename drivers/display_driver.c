

#include "esp_lcd_sh8601.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "lcd_bl_pwm_bsp.h"
#include "user_config.h"
#include "i2c_bsp.h"
#include "lcd_touch_bsp.h"
#include <stdint.h>
#include "lvgl.h"

// Logging tag for this module
static const char *TAG = "display_driver";

#if CONFIG_LV_COLOR_DEPTH == 32
#define LCD_BIT_PER_PIXEL       (24)
#elif CONFIG_LV_COLOR_DEPTH == 16
#define LCD_BIT_PER_PIXEL       (16)
#endif

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
	{0xF0, (uint8_t[]){0x28}, 1, 0},
	{0xF2, (uint8_t[]){0x28}, 1, 0},
	{0x73, (uint8_t[]){0xF0}, 1, 0},
	{0x7C, (uint8_t[]){0xD1}, 1, 0},
	{0x83, (uint8_t[]){0xE0}, 1, 0},
	{0x84, (uint8_t[]){0x61}, 1, 0},
	{0xF2, (uint8_t[]){0x82}, 1, 0},
	{0xF0, (uint8_t[]){0x00}, 1, 0},
	{0xF0, (uint8_t[]){0x01}, 1, 0},
	{0xF1, (uint8_t[]){0x01}, 1, 0},
	{0xB0, (uint8_t[]){0x56}, 1, 0},
	{0xB1, (uint8_t[]){0x4D}, 1, 0},
	{0xB2, (uint8_t[]){0x24}, 1, 0},
	{0xB4, (uint8_t[]){0x87}, 1, 0},
	{0xB5, (uint8_t[]){0x44}, 1, 0},
	{0xB6, (uint8_t[]){0x8B}, 1, 0},
	{0xB7, (uint8_t[]){0x40}, 1, 0},
	{0xB8, (uint8_t[]){0x86}, 1, 0},
	{0xBA, (uint8_t[]){0x00}, 1, 0},
	{0xBB, (uint8_t[]){0x08}, 1, 0},
	{0xBC, (uint8_t[]){0x08}, 1, 0},
	{0xBD, (uint8_t[]){0x00}, 1, 0},
	{0xC0, (uint8_t[]){0x80}, 1, 0},
	{0xC1, (uint8_t[]){0x10}, 1, 0},
	{0xC2, (uint8_t[]){0x37}, 1, 0},
	{0xC3, (uint8_t[]){0x80}, 1, 0},
	{0xC4, (uint8_t[]){0x10}, 1, 0},
	{0xC5, (uint8_t[]){0x37}, 1, 0},
	{0xC6, (uint8_t[]){0xA9}, 1, 0},
	{0xC7, (uint8_t[]){0x41}, 1, 0},
	{0xC8, (uint8_t[]){0x01}, 1, 0},
	{0xC9, (uint8_t[]){0xA9}, 1, 0},
	{0xCA, (uint8_t[]){0x41}, 1, 0},
	{0xCB, (uint8_t[]){0x01}, 1, 0},
	{0xD0, (uint8_t[]){0x91}, 1, 0},
	{0xD1, (uint8_t[]){0x68}, 1, 0},
	{0xD2, (uint8_t[]){0x68}, 1, 0},
	{0xF5, (uint8_t[]){0x00, 0xA5}, 2, 0},
	{0xDD, (uint8_t[]){0x4F}, 1, 0},
	{0xDE, (uint8_t[]){0x4F}, 1, 0},
	{0xF1, (uint8_t[]){0x10}, 1, 0},
	{0xF0, (uint8_t[]){0x00}, 1, 0},
	{0xF0, (uint8_t[]){0x02}, 1, 0},
	{0xE0, (uint8_t[]){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
	{0xE1, (uint8_t[]){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
	{0xF0, (uint8_t[]){0x10}, 1, 0},
	{0xF3, (uint8_t[]){0x10}, 1, 0},
	{0xE0, (uint8_t[]){0x07}, 1, 0},
	{0xE1, (uint8_t[]){0x00}, 1, 0},
	{0xE2, (uint8_t[]){0x00}, 1, 0},
	{0xE3, (uint8_t[]){0x00}, 1, 0},
	{0xE4, (uint8_t[]){0xE0}, 1, 0},
	{0xE5, (uint8_t[]){0x06}, 1, 0},
	{0xE6, (uint8_t[]){0x21}, 1, 0},
	{0xE7, (uint8_t[]){0x01}, 1, 0},
	{0xE8, (uint8_t[]){0x05}, 1, 0},
	{0xE9, (uint8_t[]){0x02}, 1, 0},
	{0xEA, (uint8_t[]){0xDA}, 1, 0},
	{0xEB, (uint8_t[]){0x00}, 1, 0},
	{0xEC, (uint8_t[]){0x00}, 1, 0},
	{0xED, (uint8_t[]){0x0F}, 1, 0},
	{0xEE, (uint8_t[]){0x00}, 1, 0},
	{0xEF, (uint8_t[]){0x00}, 1, 0},
	{0xF8, (uint8_t[]){0x00}, 1, 0},
	{0xF9, (uint8_t[]){0x00}, 1, 0},
	{0xFA, (uint8_t[]){0x00}, 1, 0},
	{0xFB, (uint8_t[]){0x00}, 1, 0},
	{0xFC, (uint8_t[]){0x00}, 1, 0},
	{0xFD, (uint8_t[]){0x00}, 1, 0},
	{0xFE, (uint8_t[]){0x00}, 1, 0},
	{0xFF, (uint8_t[]){0x00}, 1, 0},
	{0x60, (uint8_t[]){0x40}, 1, 0},
	{0x61, (uint8_t[]){0x04}, 1, 0},
	{0x62, (uint8_t[]){0x00}, 1, 0},
	{0x63, (uint8_t[]){0x42}, 1, 0},
	{0x64, (uint8_t[]){0xD9}, 1, 0},
	{0x65, (uint8_t[]){0x00}, 1, 0},
	{0x66, (uint8_t[]){0x00}, 1, 0},
	{0x67, (uint8_t[]){0x00}, 1, 0},
	{0x68, (uint8_t[]){0x00}, 1, 0},
	{0x69, (uint8_t[]){0x00}, 1, 0},
	{0x6A, (uint8_t[]){0x00}, 1, 0},
	{0x6B, (uint8_t[]){0x00}, 1, 0},
	{0x70, (uint8_t[]){0x40}, 1, 0},
	{0x71, (uint8_t[]){0x03}, 1, 0},
	{0x72, (uint8_t[]){0x00}, 1, 0},
	{0x73, (uint8_t[]){0x42}, 1, 0},
	{0x74, (uint8_t[]){0xD8}, 1, 0},
	{0x75, (uint8_t[]){0x00}, 1, 0},
	{0x76, (uint8_t[]){0x00}, 1, 0},
	{0x77, (uint8_t[]){0x00}, 1, 0},
	{0x78, (uint8_t[]){0x00}, 1, 0},
	{0x79, (uint8_t[]){0x00}, 1, 0},
	{0x7A, (uint8_t[]){0x00}, 1, 0},
	{0x7B, (uint8_t[]){0x00}, 1, 0},
	{0x80, (uint8_t[]){0x48}, 1, 0},
	{0x81, (uint8_t[]){0x00}, 1, 0},
	{0x82, (uint8_t[]){0x06}, 1, 0},
	{0x83, (uint8_t[]){0x02}, 1, 0},
	{0x84, (uint8_t[]){0xD6}, 1, 0},
	{0x85, (uint8_t[]){0x04}, 1, 0},
	{0x86, (uint8_t[]){0x00}, 1, 0},
	{0x87, (uint8_t[]){0x00}, 1, 0},
	{0x88, (uint8_t[]){0x48}, 1, 0},
	{0x89, (uint8_t[]){0x00}, 1, 0},
	{0x8A, (uint8_t[]){0x08}, 1, 0},
	{0x8B, (uint8_t[]){0x02}, 1, 0},
	{0x8C, (uint8_t[]){0xD8}, 1, 0},
	{0x8D, (uint8_t[]){0x04}, 1, 0},
	{0x8E, (uint8_t[]){0x00}, 1, 0},
	{0x8F, (uint8_t[]){0x00}, 1, 0},
	{0x90, (uint8_t[]){0x48}, 1, 0},
	{0x91, (uint8_t[]){0x00}, 1, 0},
	{0x92, (uint8_t[]){0x0A}, 1, 0},
	{0x93, (uint8_t[]){0x02}, 1, 0},
	{0x94, (uint8_t[]){0xDA}, 1, 0},
	{0x95, (uint8_t[]){0x04}, 1, 0},
	{0x96, (uint8_t[]){0x00}, 1, 0},
	{0x97, (uint8_t[]){0x00}, 1, 0},
	{0x98, (uint8_t[]){0x48}, 1, 0},
	{0x99, (uint8_t[]){0x00}, 1, 0},
	{0x9A, (uint8_t[]){0x0C}, 1, 0},
	{0x9B, (uint8_t[]){0x02}, 1, 0},
	{0x9C, (uint8_t[]){0xDC}, 1, 0},
	{0x9D, (uint8_t[]){0x04}, 1, 0},
	{0x9E, (uint8_t[]){0x00}, 1, 0},
	{0x9F, (uint8_t[]){0x00}, 1, 0},
	{0xA0, (uint8_t[]){0x48}, 1, 0},
	{0xA1, (uint8_t[]){0x00}, 1, 0},
	{0xA2, (uint8_t[]){0x05}, 1, 0},
	{0xA3, (uint8_t[]){0x02}, 1, 0},
	{0xA4, (uint8_t[]){0xD5}, 1, 0},
	{0xA5, (uint8_t[]){0x04}, 1, 0},
	{0xA6, (uint8_t[]){0x00}, 1, 0},
	{0xA7, (uint8_t[]){0x00}, 1, 0},
	{0xA8, (uint8_t[]){0x48}, 1, 0},
	{0xA9, (uint8_t[]){0x00}, 1, 0},
	{0xAA, (uint8_t[]){0x07}, 1, 0},
	{0xAB, (uint8_t[]){0x02}, 1, 0},
	{0xAC, (uint8_t[]){0xD7}, 1, 0},
	{0xAD, (uint8_t[]){0x04}, 1, 0},
	{0xAE, (uint8_t[]){0x00}, 1, 0},
	{0xAF, (uint8_t[]){0x00}, 1, 0},
	{0xB0, (uint8_t[]){0x48}, 1, 0},
	{0xB1, (uint8_t[]){0x00}, 1, 0},
	{0xB2, (uint8_t[]){0x09}, 1, 0},
	{0xB3, (uint8_t[]){0x02}, 1, 0},
	{0xB4, (uint8_t[]){0xD9}, 1, 0},
	{0xB5, (uint8_t[]){0x04}, 1, 0},
	{0xB6, (uint8_t[]){0x00}, 1, 0},
	{0xB7, (uint8_t[]){0x00}, 1, 0},
	{0xB8, (uint8_t[]){0x48}, 1, 0},
	{0xB9, (uint8_t[]){0x00}, 1, 0},
	{0xBA, (uint8_t[]){0x0B}, 1, 0},
	{0xBB, (uint8_t[]){0x02}, 1, 0},
	{0xBC, (uint8_t[]){0xDB}, 1, 0},
	{0xBD, (uint8_t[]){0x04}, 1, 0},
	{0xBE, (uint8_t[]){0x00}, 1, 0},
	{0xBF, (uint8_t[]){0x00}, 1, 0},
	{0xC0, (uint8_t[]){0x10}, 1, 0},
	{0xC1, (uint8_t[]){0x47}, 1, 0},
	{0xC2, (uint8_t[]){0x56}, 1, 0},
	{0xC3, (uint8_t[]){0x65}, 1, 0},
	{0xC4, (uint8_t[]){0x74}, 1, 0},
	{0xC5, (uint8_t[]){0x88}, 1, 0},
	{0xC6, (uint8_t[]){0x99}, 1, 0},
	{0xC7, (uint8_t[]){0x01}, 1, 0},
	{0xC8, (uint8_t[]){0xBB}, 1, 0},
	{0xC9, (uint8_t[]){0xAA}, 1, 0},
	{0xD0, (uint8_t[]){0x10}, 1, 0},
	{0xD1, (uint8_t[]){0x47}, 1, 0},
	{0xD2, (uint8_t[]){0x56}, 1, 0},
	{0xD3, (uint8_t[]){0x65}, 1, 0},
	{0xD4, (uint8_t[]){0x74}, 1, 0},
	{0xD5, (uint8_t[]){0x88}, 1, 0},
	{0xD6, (uint8_t[]){0x99}, 1, 0},
	{0xD7, (uint8_t[]){0x01}, 1, 0},
	{0xD8, (uint8_t[]){0xBB}, 1, 0},
	{0xD9, (uint8_t[]){0xAA}, 1, 0},
	{0xF3, (uint8_t[]){0x01}, 1, 0},
	{0xF0, (uint8_t[]){0x00}, 1, 0},
	{0x21, (uint8_t[]){0x00}, 1, 0},
	{0x11, (uint8_t[]){0x00}, 1, 120},
	{0x29, (uint8_t[]){0x00}, 1, 0},
#ifdef EXAMPLE_Rotate_90
	{0x36, (uint8_t[]){0x60}, 1, 0},
#else
	{0x36, (uint8_t[]){0x00}, 1, 0},
#endif
};

esp_lcd_panel_handle_t display_init(void)
{
	lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255);

	ESP_LOGI(TAG, "Initialize SPI bus");
	const spi_bus_config_t buscfg = {
		.data0_io_num = EXAMPLE_PIN_NUM_LCD_DATA0,
		.data1_io_num = EXAMPLE_PIN_NUM_LCD_DATA1,
		.sclk_io_num = EXAMPLE_PIN_NUM_LCD_PCLK,
		.data2_io_num = EXAMPLE_PIN_NUM_LCD_DATA2,
		.data3_io_num = EXAMPLE_PIN_NUM_LCD_DATA3,
	.max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
	};
	ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

	ESP_LOGI(TAG, "Install panel IO");
	esp_lcd_panel_io_handle_t io_handle = NULL;
	const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS,
																				NULL, // flush_ready_cb
																				NULL); // user_ctx
	sh8601_vendor_config_t vendor_config = {
		.init_cmds = lcd_init_cmds,
		.init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
		.flags = {
			.use_qspi_interface = 1,
		},
	};
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

	esp_lcd_panel_handle_t panel_handle = NULL;
	const esp_lcd_panel_dev_config_t panel_config = {
		.reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
		.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
		.bits_per_pixel = LCD_BIT_PER_PIXEL,
		.vendor_config = &vendor_config,
	};
	ESP_LOGI(TAG, "Install SH8601 panel driver");
	ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
	i2c_master_Init();
#if EXAMPLE_USE_TOUCH
	lcd_touch_init();
	ESP_LOGI(TAG, "display_init complete, returning panel_handle=%p", panel_handle);

#endif
	return panel_handle;
}

// LVGL display driver flush callback
void display_driver_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
	esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
	int32_t x1 = area->x1;
	int32_t y1 = area->y1;
	int32_t x2 = area->x2;
	int32_t y2 = area->y2;
	int32_t w = x2 - x1 + 1;
	int32_t h = y2 - y1 + 1;
	// Flush the area to the display
	esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2 + 1, y2 + 1, color_map);
	lv_disp_flush_ready(drv);
}

// LVGL display driver rounder callback (optional, for alignment)
void display_driver_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
	// Example: round area to even values for alignment if needed
	area->x1 &= ~1;
	area->x2 |= 1;
}
