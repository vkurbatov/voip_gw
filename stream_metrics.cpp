#include "stream_metrics.h"

namespace voip
{

stream_metrics_t::stream_metrics_t(std::size_t octets
                               , std::size_t frames
                               , std::uint32_t bitrate
                               , std::uint32_t framerate
                               , std::size_t packets
                               , std::int64_t lost
                               , std::int64_t jitter_ms
                               , std::int64_t rtt_ms
                               , const std::string& format)
    : octets(octets)
    , frames(frames)
    , bitrate(bitrate)
    , framerate(framerate)
    , packets(packets)
    , lost(lost)
    , jitter_ms(jitter_ms)
    , rtt_ms(rtt_ms)
    , format(format)
{

}

void stream_metrics_t::reset()
{
    *this = {};
}

}
