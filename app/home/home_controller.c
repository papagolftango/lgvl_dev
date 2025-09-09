#include "home_controller.h"
#include "ui/screens/home_screen.h"
#include <string.h>

static char s_motd[128] = "Welcome home – Message of the day";

void home_controller_init(void) {
    // Ensure home screen exists when app becomes active
    if (!home_screen_get_root()) {
        home_screen_create(NULL);
    }
    home_screen_set_motd(s_motd);
}

void home_controller_tick(void) {
    // Could react to time or other events; ticker scrolls automatically
}

void home_controller_cleanup(void) {
    // Cleanup controller logic for home app (if needed)
}

void home_controller_destroy(void) {
    // Destroy controller logic for home app (if needed)
}

void home_controller_process(void) {
    // Process controller logic for home app (if needed)
}

void home_controller_touch(void) {
    // Touch event handler for home app (if needed)
}

void home_controller_set_motd(const char *text) {
    if (!text) return;
    strncpy(s_motd, text, sizeof(s_motd) - 1);
    s_motd[sizeof(s_motd) - 1] = '\0';
    home_screen_set_motd(s_motd);
}
