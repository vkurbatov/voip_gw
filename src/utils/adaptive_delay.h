#ifndef VIOP_ADAPTIVE_DELAY_H
#define VIOP_ADAPTIVE_DELAY_H

#include <chrono>

namespace voip
{

class adaptive_delay
{
    using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
    using time_duration_t = std::chrono::nanoseconds;
    time_point_t    m_last_time;

public:
    static time_point_t now();
    static void sleep(time_duration_t wait_period);

    adaptive_delay();
    time_duration_t elapsed() const;
    void wait(time_duration_t wait_period);
    void reset();
};

}

#endif // VIOP_ADAPTIVE_DELAY_H
