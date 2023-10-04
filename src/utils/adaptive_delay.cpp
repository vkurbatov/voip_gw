#include "adaptive_delay.h"
#include <thread>

namespace voip
{

adaptive_delay::time_point_t adaptive_delay::now()
{
    return std::chrono::system_clock::now();
}

void adaptive_delay::sleep(time_duration_t wait_period)
{
    std::this_thread::sleep_for(wait_period);
}

adaptive_delay::adaptive_delay()
    : m_last_time(now())
{

}

adaptive_delay::time_duration_t adaptive_delay::elapsed() const
{
    return now() - m_last_time;
}

void adaptive_delay::wait(time_duration_t wait_period)
{
    m_last_time = m_last_time + wait_period;
    auto now_tp = now();
    if (m_last_time > now_tp)
    {
        sleep(m_last_time - now_tp);
    }
}

void adaptive_delay::reset()
{
    m_last_time = now();
}

}
