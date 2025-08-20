// Utility to get the current TZ string (e.g. "GMT0BST,M3.5.0/1,M10.5.0")
#include <stdlib.h>
#include <string.h>

void get_current_tz(char *buf, size_t bufsize) {
    const char *tz = getenv("TZ");
    if (tz && *tz) {
        strncpy(buf, tz, bufsize-1);
        buf[bufsize-1] = '\0';
    } else {
        strncpy(buf, "(unknown)", bufsize-1);
        buf[bufsize-1] = '\0';
    }
}
