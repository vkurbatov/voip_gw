#include "video_frame_info.h"

namespace voip
{

video_frame_info_t::video_frame_info_t(uint32_t left
                                       , uint32_t top
                                       , uint32_t width
                                       , uint32_t height)
    : left(left)
    , top(top)
    , width(width)
    , height(height)
{

}

bool video_frame_info_t::operator ==(const video_frame_info_t &other) const
{
    return left == other.left
            && top == other.top
            && width == other.width
            && height == other.height;
}

bool video_frame_info_t::operator !=(const video_frame_info_t &other) const
{
    return ! operator == (other);
}

}
