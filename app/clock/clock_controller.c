#include "clock_controller.h"
#include "managers/time_manager.h"
#include "ui/screens/clock_screen.h"
#include <stdio.h>
#include <time.h>

static int last_sec = -1;
static int last_min = -1;
static bool use_24h = true;

void clock_controller_init(void) {
    last_sec = -1;
}

void clock_controller_tick(void) {
    // Update display once per second (local time comes from time_manager)
    struct tm now_tm;
    time_manager_get_localtime(&now_tm);

    // Update time if seconds changed
    if (now_tm.tm_sec != last_sec) {
        last_sec = now_tm.tm_sec;
        char tbuf[16];
        if (use_24h) {
            snprintf(tbuf, sizeof(tbuf), "%02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
        } else {
            int hour12 = now_tm.tm_hour % 12; if (hour12 == 0) hour12 = 12;
            snprintf(tbuf, sizeof(tbuf), "%02d:%02d:%02d", hour12, now_tm.tm_min, now_tm.tm_sec);
        }
        clock_screen_set_time(tbuf);
    }

    // Update date once per minute
    if (now_tm.tm_min != last_min) {
        last_min = now_tm.tm_min;
    char dbuf[24];
    // e.g., Mon 9 Sep (abbr weekday, day, abbr month)
        static const char *W[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        static const char *M[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    snprintf(dbuf, sizeof(dbuf), "%s %d %s", W[now_tm.tm_wday], now_tm.tm_mday, M[now_tm.tm_mon]);
        clock_screen_set_date(dbuf);
    }
}

void clock_controller_cleanup(void) {
    // Nothing yet
}

void clock_controller_destroy(void) {
    // Nothing yet
}

void clock_controller_process(void) {
    // Not used; updates handled in tick
}

void clock_controller_touch(void) {
    // Toggle 12/24-hour format on tap
    use_24h = !use_24h;
    // Force immediate refresh of time string
    last_sec = -1;
}

void clock_controller_toggle_12_24(void) {
    clock_controller_touch();
}

void clock_controller_show_date_briefly(void) {
    // Ask screen to show date for ~3s like a short press did
    extern void clock_screen_show_date_briefly(void);
    clock_screen_show_date_briefly();
}
