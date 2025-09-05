
# ESP-IDF component for managers (app, controller, system)

idf_component_register(
   SRCS
	   "app_manager.c"
	   "encoder_manager.c"
	   "haptic_manager.c"
	   "lvgl_manager.c"
	   "mqtt_manager.c"
	   "persistent_data_manager.c"
	   "power_manager.c"
	   "provisioning_server.c"
	   "time_manager.c"
	   "touch_manager.c"
	   "wifi_manager.c"
   INCLUDE_DIRS "." "../../drivers"
	PRIV_REQUIRES lvgl nvs_flash esp_http_server esp_event esp_wifi freertos lcd_touch_bsp
	REQUIRES main i2c_bsp clock
)



