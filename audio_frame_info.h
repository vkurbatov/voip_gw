#ifndef VOIP_AUDIO_FRAME_INFO_H
#define VOIP_AUDIO_FRAME_INFO_H

#include <cstdint>
#include <string>

namespace voip
{

struct audio_frame_info_t
{
    std::string     format;
    std::uint32_t   sample_rate;
    std::uint32_t   channels;

    audio_frame_info_t(const std::string_view& format = {}
                       , std::uint32_t sample_rate = 0
                       , std::uint32_t channels = 0);

    bool operator == (const audio_frame_info_t& other) const;
    bool operator != (const audio_frame_info_t& other) const;

    bool is_valid() const;
};

}

#endif // VOIP_AUDIO_FRAME_INFO_H
