#ifndef VOIP_I_AUDIO_FRAME_H
#define VOIP_I_AUDIO_FRAME_H

#include "i_media_frame.h"
#include <string>

namespace voip
{

class i_audio_frame : public i_media_frame
{
public:
    virtual std::uint32_t sample_rate() const = 0;
    virtual std::uint32_t channels() const = 0;

    virtual void set_sample_rate(std::uint32_t sample_rate) = 0;
    virtual void set_channels(std::uint32_t channels) = 0;
};

}

#endif // VOIP_I_AUDIO_FRAME_H
