

#include <stdlib.h>
#include <time.h>

void time_manager_set_timezone_uk(void) {
    setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1); // UK: GMT, DST starts last Sunday in March at 1am, ends last Sunday in October at 2am
    tzset();
}
#include "time_manager.h"
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "time_manager";
static bool sntp_started = false;
static bool time_synced = false;

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MAX_DAY_CBS 4
#define MAX_HOUR_CBS 4
static time_manager_day_cb_t day_cbs[MAX_DAY_CBS];
static time_manager_hour_cb_t hour_cbs[MAX_HOUR_CBS];
static int num_day_cbs = 0;
static int num_hour_cbs = 0;

static void time_manager_event_task(void *arg);

static void time_sync_notification_cb(struct timeval *tv) {
    time_synced = true;
    ESP_LOGI(TAG, "Time synchronized via SNTP");
}

void time_manager_init(void) {
    if (sntp_started) return;
    // Set timezone to UK (GMT/BST with DST)
    time_manager_set_timezone_uk();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_interval(86400000); // 24 hours in ms
    sntp_setservername(0, "time.google.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    sntp_started = true;
    ESP_LOGI(TAG, "SNTP started (interval: 24h)");

    // Start event task
    xTaskCreate(time_manager_event_task, "time_evt", 2048, NULL, 2, NULL);
}
// --- Event callback registration ---
void time_manager_register_day_callback(time_manager_day_cb_t cb) {
    if (num_day_cbs < MAX_DAY_CBS) {
        day_cbs[num_day_cbs++] = cb;
    }
}

void time_manager_unregister_day_callback(time_manager_day_cb_t cb) {
    for (int i = 0; i < num_day_cbs; ++i) {
        if (day_cbs[i] == cb) {
            for (int j = i; j < num_day_cbs - 1; ++j) day_cbs[j] = day_cbs[j+1];
            --num_day_cbs;
            break;
        }
    }
}

void time_manager_register_hour_callback(time_manager_hour_cb_t cb) {
    if (num_hour_cbs < MAX_HOUR_CBS) {
        hour_cbs[num_hour_cbs++] = cb;
    }
}

void time_manager_unregister_hour_callback(time_manager_hour_cb_t cb) {
    for (int i = 0; i < num_hour_cbs; ++i) {
        if (hour_cbs[i] == cb) {
            for (int j = i; j < num_hour_cbs - 1; ++j) hour_cbs[j] = hour_cbs[j+1];
            --num_hour_cbs;
            break;
        }
    }
}

// --- Event dispatch task ---
static void time_manager_event_task(void *arg) {
    struct tm last_tm = {0};
    time_manager_get_localtime(&last_tm);
    int last_day = last_tm.tm_mday;
    int last_hour = last_tm.tm_hour;
    while (1) {
        struct tm now_tm;
        time_manager_get_localtime(&now_tm);
        if (now_tm.tm_mday != last_day) {
            last_day = now_tm.tm_mday;
            for (int i = 0; i < num_day_cbs; ++i) {
                if (day_cbs[i]) day_cbs[i]();
            }
        }
        if (now_tm.tm_hour != last_hour) {
            last_hour = now_tm.tm_hour;
            for (int i = 0; i < num_hour_cbs; ++i) {
                if (hour_cbs[i]) hour_cbs[i]();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // check every 10s
    }
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
