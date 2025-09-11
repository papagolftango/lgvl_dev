#include "home_controller.h"
#include "ui/screens/home_screen.h"
#include <string.h>

static char s_motd[128] = "Message of the day";
static volatile bool s_motd_dirty = false;

void home_controller_init(void) {
    // Ensure home screen exists when app becomes active
    if (!home_screen_get_root()) {
        home_screen_create(NULL);
    }
    home_screen_set_motd(s_motd);
}

void home_controller_tick(void) {
    // Apply pending MOTD updates under LVGL lock (tick runs under app_manager lock)
    if (s_motd_dirty) {
        s_motd_dirty = false;
        home_screen_set_motd(s_motd);
    }
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
    // Mark dirty; UI update will happen in tick (thread-safe)
    s_motd_dirty = true;
}
