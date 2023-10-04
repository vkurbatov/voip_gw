#ifndef VOIP_FRAME_QUEUE_H
#define VOIP_FRAME_QUEUE_H

#include "i_media_frame.h"

#include <queue>
#include <condition_variable>
#include <mutex>

namespace voip
{

class frame_queue
{
public:
    struct config_t
    {
        std::size_t    max_queue_size;
        config_t(std::size_t max_queue_size = 0);
    };

    struct stats_t
    {
        std::size_t     push;
        std::size_t     pop;
        std::size_t     drop;
        std::size_t     pending;

        stats_t(std::size_t push = 0
                , std::size_t pop = 0
                , std::size_t drop = 0
                , std::size_t pending = 0);

        void reset();
    };

private:
    using mutex_t = std::mutex;
    using cond_t = std::condition_variable;
    using queue_t = std::queue<i_media_frame::u_ptr_t>;

    mutable std::mutex      m_safe_mutex;
    config_t                m_config;
    stats_t                 m_stats;

    queue_t                 m_queue;
    cond_t                  m_signal;

public:
    frame_queue(const config_t& config = {});

    void push_frame(i_media_frame::u_ptr_t&& media_frame);
    i_media_frame::u_ptr_t pop_frame(std::uint32_t timeout_ms = 0);
    const stats_t& stats() const;
    void reset();
};

}

#endif // VOIP_FRAME_QUEUE_H
