#pragma once

#include <chrono>
#include <mutex>

class Timer {
public:
    Timer()
    {
#ifdef WIN32
        QueryPerformanceFrequency(&frequency);
        startCount.QuadPart = 0;
        endCount.QuadPart = 0;
#else
        startCount.tv_sec = startCount.tv_nsec = 0;
        endCount.tv_sec = endCount.tv_nsec = 0;
#endif
        stopped = 0;
        startTimeInMicroSec = 0;
        endTimeInMicroSec = 0;
    }

    virtual ~Timer()
    {

    }

    void    reset()
    {
        stop();
        start();
    }

    void   start()
    {
        stopped = 0; // reset stop flag
#ifdef WIN32
        QueryPerformanceCounter(&startCount);
#else
        // gettimeofday(&startCount, NULL);
        clock_gettime(CLOCK_MONOTONIC, &startCount);
#endif
    }

    void   stop()
    {
        stopped = 1; // set timer stopped flag

#ifdef WIN32
        QueryPerformanceCounter(&endCount);
#else
        // gettimeofday(&endCount, NULL);
        clock_gettime(CLOCK_MONOTONIC, &endCount);
#endif
    }

    double elapsed()
    {
        return elapsed_microsec() / 1000000.0;
    }

    double elapsed_microsec()
    {
        return elapsed_nanosec() / 1000.0;
    }

    double elapsed_nanosec()
    {
#ifdef WIN32
        if(!stopped)
            QueryPerformanceCounter(&endCount);

        startTimeInMicroSec = startCount.QuadPart * (1000000.0 / frequency.QuadPart);
        endTimeInMicroSec = endCount.QuadPart * (1000000.0 / frequency.QuadPart);
#else
        if(!stopped)
            // gettimeofday(&endCount, NULL);
            clock_gettime(CLOCK_MONOTONIC, &endCount);

        startTimeInMicroSec = (startCount.tv_sec * 1000000000.0) + startCount.tv_nsec;
        endTimeInMicroSec = (endCount.tv_sec * 1000000000.0) + endCount.tv_nsec;
#endif
        return endTimeInMicroSec - startTimeInMicroSec;
    }

private:
    double startTimeInMicroSec;                 // starting time in micro-second
    double endTimeInMicroSec;                   // ending time in micro-second
    int    stopped;                             // stop flag
#ifdef WIN32
    LARGE_INTEGER frequency;                    // ticks per second
    LARGE_INTEGER startCount;                   //
    LARGE_INTEGER endCount;                     //
#else
    struct timespec startCount;                         //
    struct timespec endCount;                           //
#endif

};
