#include "timestamp.h"

TimeStamp::TimeStamp()
{
    this->start.tv_nsec = 0;
    this->start.tv_sec = 0;
    this->reset();
}

void TimeStamp::reset()
{
    clock_gettime( CLOCK_REALTIME, &this->start);
}

double TimeStamp::tsdiff(timespec& start, timespec& end) const
                         {
    timespec result;
    double retval = 0.0;
    /* Perform the carry for the later subtraction by updating */
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = end.tv_sec - start.tv_sec - 1;
        result.tv_nsec = (long int) (1E9) + end.tv_nsec - start.tv_nsec;
    } else {
        result.tv_sec = end.tv_sec - start.tv_sec;
        result.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    retval = result.tv_sec;
    retval += (double) result.tv_nsec / (double) 1E9;
    return retval;
}

double TimeStamp::seconds_until_now()
{
    timespec now;
    clock_gettime( CLOCK_REALTIME, &now);
    double stampdiff = this->tsdiff(this->start, now);
    return stampdiff;
}

