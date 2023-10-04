#include "frame_queue.h"

namespace voip
{

frame_queue::config_t::config_t(std::size_t max_queue_size)
    : max_queue_size(max_queue_size)
{

}

frame_queue::stats_t::stats_t(std::size_t push
                              , std::size_t pop
                              , std::size_t drop
                              , std::size_t pending)
    : push(push)
    , pop(pop)
    , drop(drop)
    , pending(pending)
{

}

void frame_queue::stats_t::reset()
{
    *this = {};
}

frame_queue::frame_queue(const config_t &config)
    : m_config(config)
{

}

void frame_queue::push_frame(i_media_frame::u_ptr_t&& media_frame)
{
    std::lock_guard lock(m_safe_mutex);
    m_queue.emplace(std::move(media_frame));
    m_stats.push++;
    while(m_queue.size() > std::max<std::size_t>(m_config.max_queue_size, 1))
    {
        m_stats.drop++;
        m_queue.pop();
    }
    m_stats.pending = m_queue.size();
    m_signal.notify_one();
}

i_media_frame::u_ptr_t frame_queue::pop_frame(uint32_t timeout_ms)
{
    std::unique_lock lock(m_safe_mutex);
    if (m_queue.empty()
            && timeout_ms > 0)
    {
        m_signal.wait_for(lock
                          , std::chrono::milliseconds(timeout_ms));
    }

    if (!m_queue.empty())
    {
        auto frame = std::move(m_queue.front());
        m_stats.pop++;
        m_stats.pending = m_queue.size();
        m_queue.pop();
        return frame;
    }

    return nullptr;
}

const frame_queue::stats_t &frame_queue::stats() const
{
    return m_stats;
}

void frame_queue::reset()
{
    std::lock_guard lock(m_safe_mutex);
    m_queue = {};
    m_stats.reset();
    m_signal.notify_one();
}

}
