#include <threads.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "framelimiter.h"

void frameLimiterInit(FrameLimiter* limiter, double fps)
{
    limiter->targetFPS = fps;
    limiter->targetFrameTime = 1.0 / fps;
    timespec_get(&limiter->lastFrameTime, TIME_UTC);
}

void frameLimiterBegin(FrameLimiter* limiter)
{
    timespec_get(&limiter->lastFrameTime, TIME_UTC);
}

double frameLimiterEnd(FrameLimiter* limiter)
{
    struct timespec now;
    timespec_get(&now, TIME_UTC);

    double dt = (now.tv_sec - limiter->lastFrameTime.tv_sec) +
                (now.tv_nsec - limiter->lastFrameTime.tv_nsec) / 1e9;

    double sleepTime = limiter->targetFrameTime - dt;
    if (sleepTime > 0)
    {
        struct timespec ts = {
            .tv_sec = (time_t)sleepTime,
            .tv_nsec = (long)((sleepTime - (time_t)sleepTime) * 1e9)
        };
        thrd_sleep(&ts, NULL);
        dt = limiter->targetFrameTime; // approximate after sleep
    }

    return dt;
}
