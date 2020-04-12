/** \file timer.h
 *  \brief Header: simple timer
 */

#pragma once

#include <sys/time.h>

class mc_timer_t
{
private:
    uint64_t start;
public:
    mc_timer_t()
    {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        this->start = static_cast<uint64_t>(tv.tv_sec) * G_USEC_PER_SEC + static_cast<uint64_t>(tv.tv_usec);
    }

    uint64_t mc_timer_elapsed ()
    {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        return (static_cast<uint64_t>(tv.tv_sec) * G_USEC_PER_SEC + static_cast<uint64_t>(tv.tv_usec) - this->start);

    }
};
