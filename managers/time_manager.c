#include "time_manager.h"
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "time_manager";
static bool sntp_started = false;
static bool time_synced = false;

static void time_sync_notification_cb(struct timeval *tv) {
    time_synced = true;
    ESP_LOGI(TAG, "Time synchronized via SNTP");
}

void time_manager_init(void) {
    if (sntp_started) return;
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_interval(86400000); // 24 hours in ms
    sntp_setservername(0, "time.google.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    sntp_started = true;
    ESP_LOGI(TAG, "SNTP started (interval: 24h)");
}

time_t time_manager_now(void) {
    return time(NULL);
}

void time_manager_get_localtime(struct tm *out_tm) {
    time_t now = time(NULL);
    localtime_r(&now, out_tm);
}

void time_manager_get_timestr(char *buf, size_t bufsize) {
    struct tm tm;
    time_manager_get_localtime(&tm);
    strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &tm);
}

bool time_manager_is_synced(void) {
    return time_synced;
}
