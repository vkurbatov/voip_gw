#ifndef VOIP_STREAM_METRICS_H
#define VOIP_STREAM_METRICS_H

#include "media_types.h"

namespace voip
{

struct stream_metrics
{
    std::size_t     octets;
    std::size_t     frames;
    std::uint32_t   bitrate;
    std::size_t     packets;
    std::size_t     lost;
    std::uint32_t   jitter_ms;
    std::uint32_t   rtt_ms;

    stream_metrics(std::size_t octets = 0
                  , std::size_t frames = 0
                  , std::uint32_t bitrate = 0
                  , std::size_t packets = 0
                  , std::size_t lost = 0
                  , std::uint32_t jitter_ms = 0
                  , std::uint32_t rtt_ms = 0);

    void reset();
};

}

#endif // VOIP_STREAM_METRICS_H
