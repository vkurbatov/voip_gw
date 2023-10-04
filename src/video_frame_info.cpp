#include "video_frame_info.h"

namespace voip
{

video_frame_info_t::video_frame_info_t(const std::string_view& format
                                       , uint32_t left
                                       , uint32_t top
                                       , uint32_t width
                                       , uint32_t height)
    : format(format)
    , left(left)
    , top(top)
    , width(width)
    , height(height)
{

}

bool video_frame_info_t::operator ==(const video_frame_info_t &other) const
{
    return format == other.format
            && left == other.left
            && top == other.top
            && width == other.width
            && height == other.height;
}

bool video_frame_info_t::operator !=(const video_frame_info_t &other) const
{
    return ! operator == (other);
}

bool video_frame_info_t::is_valid() const
{
    return !format.empty();
}

}
