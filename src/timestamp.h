#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <ctime>

class TimeStamp {
public:
    TimeStamp();
    ~TimeStamp(){};
    void reset();
    double seconds_until_now();
    double tsdiff(timespec& start, timespec& end) const;
    private:
    timespec start;
};

#endif /* TIMESTAMP_H_ */
