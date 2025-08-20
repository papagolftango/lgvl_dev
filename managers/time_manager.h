
#pragma once
#include <time.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize time manager (starts SNTP, etc)
void time_manager_init(void);

// Get current time as time_t (seconds since epoch)
time_t time_manager_now(void);

// Get current time as struct tm (local time)
void time_manager_get_localtime(struct tm *out_tm);

// Get current time as formatted string (e.g., "2025-08-20 12:34:56")
void time_manager_get_timestr(char *buf, size_t bufsize);

// Check if time is synchronized (NTP)
bool time_manager_is_synced(void);

#ifdef __cplusplus
}
#endif
