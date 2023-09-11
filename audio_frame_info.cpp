#include "audio_frame_info.h"

namespace voip
{

audio_frame_info_t::audio_frame_info_t(const std::string_view& format
                                       , uint32_t sample_rate
                                       , uint32_t channels)
    : format(format)
    , sample_rate(sample_rate)
    , channels(channels)
{

}

bool audio_frame_info_t::operator ==(const audio_frame_info_t &other) const
{
    return format == other.format
            && sample_rate == other.sample_rate
            && channels == other.channels;
}

bool audio_frame_info_t::operator !=(const audio_frame_info_t &other) const
{
    return ! operator == (other);
}

bool audio_frame_info_t::is_valid() const
{
    return !format.empty()
            && sample_rate > 0
            && channels > 0;
}

}
