#ifndef JERICHO_UTIL_CLOCK_H_
#define JERICHO_UTIL_CLOCK_H_

#include <ctime>
#include <string>
#include <chrono>

#include "prizm/prizm.h"

typedef std::chrono::nanoseconds Nano;
typedef std::chrono::milliseconds Milli;
typedef std::chrono::microseconds Micro;
typedef std::chrono::system_clock Time;
typedef std::chrono::minutes Min;
typedef std::chrono::seconds Sec;
typedef std::chrono::hours Hour;
// only c++20
// typedef std::chrono::days Days;
// typedef std::chrono::weeks Weeks;
// typedef std::chrono::months Months;
// typedef std::chrono::years Years;
typedef std::chrono::time_point<Time, std::chrono::duration<double>> TimePoint;
// typedef std::chrono::time_point<Time> TimePoint;

class Clock {
  public:
    static time_t now() {
        return Time::to_time_t(Time::now());
    }

    static TimePoint now_chrono() {
        return Time::now();
    }

    static TimePoint to_chrono(time_t time) {
        return Time::from_time_t(time);
    }

    static time_t nano(TimePoint time) {
        return std::chrono::duration_cast<Nano>(time.time_since_epoch()).count();
    }

    static time_t micro(TimePoint time) {
        return std::chrono::duration_cast<Micro>(time.time_since_epoch()).count();
    }

    static time_t milli(TimePoint time) {
        return std::chrono::duration_cast<Milli>(time.time_since_epoch()).count();
    }

    static time_t sec(TimePoint time) {
        return std::chrono::duration_cast<Sec>(time.time_since_epoch()).count();
    }

    static time_t duration(TimePoint start) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(Time::now() - start).count();
    }

    // not re-entrant
    static std::string prettyDate(time_t time) {
        char buf[50];
        strftime(buf, 50, "%a, %b %e, %Y %I:%M:%S %p %Z", localtime(&time));
        return std::string(buf);
    }

};

#endif