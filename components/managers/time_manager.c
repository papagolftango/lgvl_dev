#include "time_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include "esp_log.h"
#include "esp_sntp.h"
#include <esp_timer.h>
#include "time_manager.h"

static const char *TAG = "time_manager";

#define MAX_DAY_CALLBACKS 4
#define MAX_HOUR_CALLBACKS 4

static volatile bool sntp_synced = false;
static time_manager_day_cb_t day_callbacks[MAX_DAY_CALLBACKS] = {NULL, NULL, NULL, NULL};
static time_manager_hour_cb_t hour_callbacks[MAX_HOUR_CALLBACKS] = {NULL, NULL, NULL, NULL};
static int last_day = -1;
static int last_hour = -1;

void time_manager_set_timezone_uk(void) {
	setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
	tzset();
}

time_t time_manager_now(void) {
	time_t now;
	time(&now);
	return now;
}

void time_manager_get_localtime(struct tm *out_tm) {
	if (!out_tm) return;
	time_t now;
	time(&now);
	localtime_r(&now, out_tm);
}

static void time_sync_notification_cb(struct timeval *tv) {
	sntp_synced = true;
	ESP_LOGI(TAG, "SNTP time synchronized");
}

// Helper to check and fire callbacks
void time_manager_periodic_check(void *arg) {
	time_t now;
	struct tm tm_now;
	time(&now);
	localtime_r(&now, &tm_now);
	if (tm_now.tm_mday != last_day) {
		last_day = tm_now.tm_mday;
		for (int i = 0; i < MAX_DAY_CALLBACKS; ++i) {
			if (day_callbacks[i]) day_callbacks[i]();
		}
	}
	if (tm_now.tm_hour != last_hour) {
		last_hour = tm_now.tm_hour;
		for (int i = 0; i < MAX_HOUR_CALLBACKS; ++i) {
			if (hour_callbacks[i]) hour_callbacks[i]();
		}
	}
}

void time_manager_init(void) {
	time_manager_set_timezone_uk();
	// Start periodic timer to check for day/hour change
	const esp_timer_create_args_t timer_args = {
		.callback = &time_manager_periodic_check,
		.name = "time_mgr_cb"
	};
	static esp_timer_handle_t periodic_timer = NULL;
	esp_timer_create(&timer_args, &periodic_timer);
	esp_timer_start_periodic(periodic_timer, 60 * 1000000); // 60s

	// SNTP initialization
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
	for (int i = 0; i < MAX_DAY_CALLBACKS; ++i) {
		if (!day_callbacks[i]) {
			day_callbacks[i] = cb;
			break;
		}
	}
}

void time_manager_unregister_day_callback(time_manager_day_cb_t cb) {
	for (int i = 0; i < MAX_DAY_CALLBACKS; ++i) {
		if (day_callbacks[i] == cb) {
			day_callbacks[i] = NULL;
			break;
		}
	}
}

void time_manager_register_hour_callback(time_manager_hour_cb_t cb) {
	for (int i = 0; i < MAX_HOUR_CALLBACKS; ++i) {
		if (!hour_callbacks[i]) {
			hour_callbacks[i] = cb;
			break;
		}
	}
}

void time_manager_unregister_hour_callback(time_manager_hour_cb_t cb) {
	for (int i = 0; i < MAX_HOUR_CALLBACKS; ++i) {
		if (hour_callbacks[i] == cb) {
			hour_callbacks[i] = NULL;
			break;
		}
	}
}
