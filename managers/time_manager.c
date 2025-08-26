
#include "time_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"

static const char *TAG = "time_manager";
static volatile bool sntp_synced = false;

static void time_sync_notification_cb(struct timeval *tv) {
	sntp_synced = true;
	ESP_LOGI(TAG, "SNTP time synchronized");
}

void time_manager_init(void) {
	// Set UK timezone with daylight saving (BST)
	setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
	tzset();

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();
	ESP_LOGI(TAG, "SNTP initialized, waiting for sync...");
}

bool time_manager_is_synced(void) {
	return sntp_synced;
}

void time_manager_get_timestr(char *buf, size_t bufsize) {
	if (!buf || bufsize < 20) return;
	time_t now = 0;
	struct tm timeinfo = {0};
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

void time_manager_register_day_callback(time_manager_day_cb_t cb) {
	// Optional: implement if you want to notify on day change
}

void time_manager_unregister_day_callback(time_manager_day_cb_t cb) {
	// Optional: implement if you want to notify on day change
}
