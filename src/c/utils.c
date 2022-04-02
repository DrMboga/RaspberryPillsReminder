#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void sleepMilliseconds(int milliseconds)
{
    struct timespec ts;
    int res;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    res = nanosleep(&ts, &ts);
}

char *today()
{
    char *currentTime = malloc(21);
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(currentTime, 21, "%d.%m.%Y %H:%M.%S", timeinfo);
    return currentTime;
}

char *nowTimeWithSeconds()
{
    char *currentTime = malloc(10);
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(currentTime, 10, "%H:%M.%S", timeinfo);
    return currentTime;
}

int hourOfTheDay()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return timeinfo->tm_hour;
}