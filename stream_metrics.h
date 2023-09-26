#ifndef VOIP_STREAM_METRICS_H
#define VOIP_STREAM_METRICS_H

#include "media_types.h"
#include <string>

namespace voip
{

struct stream_metrics_t
{
    std::size_t     octets;
    std::size_t     frames;
    std::uint32_t   bitrate;
    std::uint32_t   framerate;
    std::size_t     packets;
    std::int64_t    lost;
    std::int64_t    jitter_ms;
    std::int64_t    rtt_ms;
    std::string     format;

    stream_metrics_t(std::size_t octets = 0
                  , std::size_t frames = 0
                  , std::uint32_t bitrate = 0
                  , std::uint32_t framerate = 0
                  , std::size_t packets = 0
                  , std::int64_t lost = 0
                  , std::int64_t jitter_ms = 0
                  , std::int64_t rtt_ms = 0
                  , const std::string& format = {});

    void reset();
};

}

#endif // VOIP_STREAM_METRICS_H
