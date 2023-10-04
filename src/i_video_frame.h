#ifndef VOIP_I_VIDEO_FRAME_H
#define VOIP_I_VIDEO_FRAME_H

#include "i_media_frame.h"

namespace voip
{


class i_video_frame : public i_media_frame
{
public:
    virtual std::uint32_t left() const = 0;
    virtual std::uint32_t top() const = 0;
    virtual std::uint32_t width() const = 0;
    virtual std::uint32_t height() const = 0;

    virtual void set_left(std::uint32_t left) = 0;
    virtual void set_top(std::uint32_t top) = 0;
    virtual void set_width(std::uint32_t width) = 0;
    virtual void set_height(std::uint32_t height) = 0;
};

}

#endif // VOIP_I_VIDEO_FRAME_H
