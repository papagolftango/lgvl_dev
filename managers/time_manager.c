// --- STUBS FOR MISSING TIME MANAGER FUNCTIONS ---
#include "time_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void time_manager_init(void) {}
bool time_manager_is_synced(void) { return true; }
void time_manager_get_timestr(char *buf, size_t bufsize) {
	if (buf && bufsize > 0) strncpy(buf, "1970-01-01 00:00:00", bufsize-1);
	if (buf) buf[bufsize-1] = '\0';
}
void time_manager_register_day_callback(time_manager_day_cb_t cb) {}
void time_manager_unregister_day_callback(time_manager_day_cb_t cb) {}
// ...existing code from components/managers/time_manager.c...
