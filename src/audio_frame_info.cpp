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

uint32_t audio_frame_info_t::bps() const
{
    if (format.find("PCM-16") == 0)
    {
        return 16;
    }
    else if (format.find("PCM-32") == 0)
    {
        return 32;
    }
    return 0;
}

std::size_t audio_frame_info_t::sample_size() const
{
    return (bps() * channels) / 8;
}

uint64_t audio_frame_info_t::duration_from_size(std::size_t size) const
{
    if (sample_rate > 0)
    {
        return (samples_from_size(size) * 1000000) / sample_rate;
    }

    return 0;
}

uint32_t audio_frame_info_t::samples_from_size(std::size_t size) const
{
    if (auto ss = sample_size())
    {
        return size / ss;
    }

    return 0;
}

bool audio_frame_info_t::is_valid() const
{
    return !format.empty()
            && sample_rate > 0
            && channels > 0;
}

}
