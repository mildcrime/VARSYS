#include "Clock.h"
#include <sys/time.h>
#include <time.h>

namespace Clock {

void begin() {
    struct tm t = {};
    t.tm_year = 2025 - 1900;   // дефолт до синхронизации по NTP
    t.tm_mon  = 0;
    t.tm_mday = 1;
    t.tm_hour = 12;
    t.tm_min  = 0;
    time_t epoch = mktime(&t);
    struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
}

void timeString(char* buf, size_t n) {
    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(buf, n, "%H:%M", &tm_now);
}

} // namespace Clock
