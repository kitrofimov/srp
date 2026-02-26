#pragma once

#include <time.h>

#define TIME_SECTION(var, section) \
    { \
        struct timespec _start, _end; \
        timespec_get(&_start, TIME_UTC); \
        section \
        timespec_get(&_end, TIME_UTC); \
        var = (_end.tv_sec - _start.tv_sec) + (_end.tv_nsec - _start.tv_nsec)/1e9; \
    }

typedef struct FrameLimiter {
    float targetFPS;
    float targetFrameTime;  // in seconds
    struct timespec lastFrameTime;
} FrameLimiter;

void frameLimiterInit(FrameLimiter* limiter, float fps);
void frameLimiterBegin(FrameLimiter* limiter);
float frameLimiterEnd(FrameLimiter* limiter);
