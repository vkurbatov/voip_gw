#include "stream_metrics.h"

namespace voip
{

stream_metrics::stream_metrics(std::size_t octets
                               , std::size_t frames
                               , uint32_t bitrate
                               , std::size_t packets
                               , std::size_t lost
                               , uint32_t jitter_ms
                               , uint32_t rtt_ms)
    : octets(octets)
    , frames(frames)
    , bitrate(bitrate)
    , packets(packets)
    , lost(lost)
    , jitter_ms(jitter_ms)
    , rtt_ms(rtt_ms)
{

}

void stream_metrics::reset()
{
    *this = {};
}

}
