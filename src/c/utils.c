#include <time.h>
#include <stdio.h>

void sleepMilliseconds(int milliseconds) {
    struct timespec ts;
    int res;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    res = nanosleep(&ts, &ts);
}