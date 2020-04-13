/** \file timer.h
 *  \brief Header: simple timer
 */

#pragma once

#include <sys/time.h>

/**
 * Timer:
 *
 * Opaque datatype that records a start time.
 * #Timer records a start time, and counts microseconds elapsed since
 * that time.
 **/
class Timer
{
private:
    uint64_t start;
public:
    Timer()
    {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        this->start = static_cast<uint64_t>(tv.tv_sec) * G_USEC_PER_SEC + static_cast<uint64_t>(tv.tv_usec);
    }

    /**
     * Obtains the time since the timer was started.
     *
     * @timer: an #mc_timer_t.
     *
     * @return: microseconds elapsed, the time since the timer was started
     *
     **/
    uint64_t mc_timer_elapsed ()
    {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        return (static_cast<uint64_t>(tv.tv_sec) * G_USEC_PER_SEC + static_cast<uint64_t>(tv.tv_usec) - this->start);

    }
};
